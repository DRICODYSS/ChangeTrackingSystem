import argparse
import os
import re
import stat
import json
import shutil
from pathlib import Path

# ==============================
# Config
# ==============================

MANIFEST_PATH = Path(".uepatcher/manifest.json")
BACKUP_DIR = Path(".uepatcher/backups")

OBJECT_MACROS_FILE_PATH = "Engine/Source/Runtime/CoreUObject/Public/UObject/ObjectMacros.h"
OBJECT_GLOBALS_FILE_PATH = "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
OBJECT_GLOBALS_CPP_FILE_PATH = "Engine/Source/Runtime/CoreUObject/Private/UObject/UObjectGlobals.cpp"

# ==============================
# Helpers
# ==============================

def get_content(file_path):
    full_file_path = Path(args.ue_path) / file_path
    with open(full_file_path, "r", encoding="utf-8") as f:
        content = f.read()

    return content

def extract_braced_block(text, start_pos):
    brace_count = 1
    i = start_pos
    while i < len(text) and brace_count > 0:
        if text[i] == '{':
            brace_count += 1
        elif text[i] == '}':
            brace_count -= 1
        i += 1

    return text[start_pos:i-1], i-1

# ==============================
# Manifest / Backup
# ==============================

def load_manifest():
    if MANIFEST_PATH.exists():
        return json.loads(MANIFEST_PATH.read_text())
    return {"files": []}


def save_manifest(data):
    #data_copy = {"files": [str(f) for f in data["files"]]}
    MANIFEST_PATH.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST_PATH.write_text(json.dumps(data, indent=2))
    #ANIFEST_PATH.write_text(json.dumps(data_copy, indent=2))


def backup_file(file_path: Path):
    BACKUP_DIR.mkdir(parents=True, exist_ok=True)

    backup_path = BACKUP_DIR / f"{file_path.name}.bak"

    if not backup_path.exists():
        shutil.copy2(file_path, backup_path)

    manifest = load_manifest()
    entry = {"path": str(file_path), "type": "modified"}

    if entry not in manifest["files"]:
        manifest["files"].append(entry)
        save_manifest(manifest)

# ==============================
# Rollback
# ==============================

def rollback():
    manifest = load_manifest()

    if not manifest.get("files"):
        print("Nothing to rollback")
        return

    for entry in manifest["files"]:
        file_path = Path(entry["path"])
        file_type = entry.get("type", "modified")

        if file_type == "modified":
            backup_path = BACKUP_DIR / f"{file_path.name}.bak"

            if backup_path.exists():
                shutil.copy2(backup_path, file_path)
                print(f"[Restored] {file_path}")
            else:
                print(f"[Warning] No backup for {file_path}")

        elif file_type == "added":
            if file_path.exists():
                file_path.unlink()
                print(f"[Removed] {file_path}")

    MANIFEST_PATH.unlink(missing_ok=True)
    shutil.rmtree(BACKUP_DIR, ignore_errors=True)

    print("Rollback completed")

# ==============================
# File Operations
# ==============================

def copy_patch_file(file_name: str, dst_dir_rel: str, dry_run=False):
    script_root = Path(__file__).parent
    src_files = list(script_root.rglob(file_name))
    if not src_files:
        raise RuntimeError(f"Source file '{file_name}' not found in PATCH_SOURCE_DIR")

    src = src_files[0]
    dst_dir = Path(args.ue_path) / dst_dir_rel
    dst = dst_dir / src.name

    if dst.exists():
        print(f"[Skip] Already exists: {dst}")
        return False

    if dry_run:
        print(f"[Dry-run] Would copy {src} -> {dst}")
        return True

    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    print(f"[Copied] {dst}")

    # обновляем манифест
    manifest = load_manifest()
    entry = {"path": str(dst), "type": "added"}
    if entry not in manifest["files"]:
        manifest["files"].append(entry)
        save_manifest(manifest)

    return True

# ==============================
# Patch Logic
# ==============================

def patch_property_gen_flags_enum(in_content, new_flag_name, alignment=17):
    """Adds a new flag to the EPropertyGenFlags enum. """

    # find enum
    enum_match = re.search(r'enum class EPropertyGenFlags\s*:\s*uint8\s*\{([\s\S]*?)\};', in_content)
    if not enum_match:
        raise RuntimeError("EPropertyGenFlags not found")

    enum_body = enum_match.group(1)

    # Check for re-addition
    if new_flag_name in enum_body:
        print(f"[Ignore] {new_flag_name} is already patched")
        return in_content, False

    # Take all property type flags up to Property-specific
    non_specific_part, _ = enum_body.split("// Property-specific flags", 1)
    type_entries = [(flag, int(bit, 16)) for flag, bit in re.findall(r'(\w+)\s*=\s*(0x[0-9A-Fa-f]+)', non_specific_part)]
    next_bit = max(bit for _, bit in type_entries) + 1
    if next_bit >= 0x40:
        raise RuntimeError("No available bits left for new flag")

    # Insert new flag
    last_flag, last_bit = type_entries[-1]
    pattern_last_flag = rf'(\s*{last_flag}\s*=\s*0x{last_bit:02X},\s*)'

    def insert_flag(match):
        return (
            match.group(1) +
            f"// Change Tracking System patch\n"
            f"\t\t{new_flag_name:<{alignment}} = 0x{next_bit:02X},\n\n\t\t"
        )

    new_enum_body = re.sub(pattern_last_flag, insert_flag, enum_body)
    return in_content.replace(enum_body, new_enum_body), True


def patch_class_cast_flags_enum(in_content, new_flag_name, alignment=39):
    """ Adds a new flag to the EClassCastFlags enum. """

    # find enum
    enum_match = re.search(r'enum EClassCastFlags\s*:\s*uint64\s*\{([\s\S]*?)\};', in_content)
    if not enum_match:
        raise RuntimeError("EClassCastFlags not found")

    enum_body = enum_match.group(1)

    # Check for re-addition
    if new_flag_name in enum_body:
        print(f"[Ignore] {new_flag_name} is already patched")
        return in_content, False

    # Find the first free bit placeholder
    free_bit_match = re.search(r'(//\s*=?\s*(0x[0-9A-Fa-f]+),?)', enum_body)
    if not free_bit_match:
        raise RuntimeError("No available free bits to insert new flag")

    _, free_bit_value = free_bit_match.groups()

    # Insert new flag
    replacement = (f"{new_flag_name:<{alignment}} = {free_bit_value}, // Change Tracking System patch")
    new_enum_body = enum_body.replace(free_bit_match.group(1), replacement, 1)
    return in_content.replace(enum_body, new_enum_body), True


def patch_ConstructFProperty_func(in_content, case_code, include_list=None):
    """ add support for a new FProperty. """

    is_changed = False

    # add includes
    if include_list:
        define_pos = in_content.find("DEFINE_LOG_CATEGORY(LogUObjectGlobals);")
        if define_pos == -1:
            raise RuntimeError("DEFINE_LOG_CATEGORY not found")

        search_text = in_content[:define_pos]
        existing_includes = {m.group(0) for m in re.finditer(r'#include\s+["<][^">]+[">]', search_text)}
        new_includes = [f'#include "{inc}"' for inc in include_list if f'#include "{inc}"' not in existing_includes]
        if new_includes:
            include_matches = list(re.finditer(r'#include\s+["<][^">]+[">]', search_text))
            if not include_matches:
                raise RuntimeError("No #include statements found")

            last_include_match = include_matches[-1]
            insert_pos = last_include_match.end()
            includes_text = "\n".join(new_includes)
            in_content = (in_content[:insert_pos] +
                       f"\n\n// Change Tracking System patch\n{includes_text}" +
                       in_content[insert_pos:]
                   )
            is_changed = True

    # find switch(PropBase->Flags & PropertyTypeMask) from ConstructFProperty
    switch_match = re.search(r'switch\s*\(\s*PropBase->Flags\s*&\s*PropertyTypeMask\s*\)\s*\{', in_content)
    if not switch_match:
        raise RuntimeError("switch(PropBase->Flags & PropertyTypeMask) not found in ConstructFProperty")

    start_pos = switch_match.end()
    switch_body, switch_end_pos = extract_braced_block(in_content, start_pos)

    flag_match = re.search(r'case\s+EPropertyGenFlags::(\w+)\s*:', case_code)
    if not flag_match:
        raise RuntimeError("Could not extract flag name from case_code")
    flag_name = flag_match.group(1)

    if f"EPropertyGenFlags::{flag_name}" in switch_body:
        print(f"[Ignore] Case for {flag_name} is already patched")
        return in_content, is_changed

    # find last case
    case_pattern = r'(case\s+EPropertyGenFlags::\w+:\s*\{[\s\S]*?\}\s*break;)'
    case_matches = list(re.finditer(case_pattern, switch_body))
    if not case_matches:
        raise RuntimeError("No case statements found in switch")

    last_case = case_matches[-1]
    last_case_end = last_case.end()

    # Insert new case
    _comment = f"\n\n\t\t\t// Change Tracking System patch"
    tail = switch_body[last_case_end:]
    tail_shifted = "\n".join(re.sub(r'^\t', '', line) for line in tail.splitlines())
    switch_body_new = switch_body[:last_case_end] + _comment + case_code + tail_shifted
    content_new = in_content[:start_pos] + switch_body_new + in_content[switch_end_pos-1:]

    return content_new, True

def patch_file(file_path, new_content, dry_run=False):
    if dry_run:
        print(f"[Dry-run] {file_path} would be patched")
        return

    file_path_obj = Path(args.ue_path) / file_path
    backup_file(file_path_obj)
    file_path_obj.write_text(new_content, encoding="utf-8")

    print(f"[Patched] {file_path}")

# ==============================
# Main
# ==============================

try:
    parser = argparse.ArgumentParser(
        description="Patch Unreal Engine to add a custom FProperty type and integrate it into the reflection system"
    )

    parser.add_argument("ue_path", help="Path to Unreal Engine root")
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--rollback", action="store_true")

    args = parser.parse_args()
    if args.rollback:
        rollback()

    else:

        # ==============================
        # Patche of existing files
        # ==============================

        case_code = f"""
            case EPropertyGenFlags::ChangeTracker:
            {{
                NewProp = NewFProperty<FChangeTrackerProperty, FGenericPropertyParams>(Outer, *PropBase);

                // Next property is the diff tracker inner
                ReadMore = 1;
            }}
            break;"""
        PATCHES = [
            (OBJECT_MACROS_FILE_PATH, patch_class_cast_flags_enum, ("CASTCLASS_ChangeTrackerProperty",)),
            (OBJECT_GLOBALS_FILE_PATH, patch_property_gen_flags_enum, ("ChangeTracker",)),
            (OBJECT_GLOBALS_CPP_FILE_PATH, patch_ConstructFProperty_func,
             (case_code, ["UObject/PropertyChangeTracker.h"])),
        ]

        for file_path, func, patche_args in PATCHES:
            content = get_content(file_path)
            content, changed = func(content, *patche_args)
            if changed:
                patch_file(file_path, content, dry_run=args.dry_run)

        # ==============================
        # Add custom files
        # ==============================

        copy_patch_file(
            "UhtChangeTrackerProperty.cs",
            "Engine/Source/Programs/Shared/EpicGames.UHT/Types/Properties/",
            args.dry_run
        )

        copy_patch_file(
            "Engine/PropertyChangeTracker.h",
            "Engine/Source/Runtime/CoreUObject/Public/UObject/",
            args.dry_run
        )

        copy_patch_file(
            "Engine/PropertyChangeTracker.cpp",
            "Engine/Source/Runtime/CoreUObject/Private/UObject/",
            args.dry_run
        )

except Exception as e:
    print(f"An error occurred: {e}")
