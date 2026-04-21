#!/bin/bash

ask_ue_path() {
    while true; do
        read -p "Enter path to Unreal Engine root: " UE_PATH
        if [ -z "$UE_PATH" ]; then
            echo "Error: No path entered."
        elif [ ! -d "$UE_PATH" ]; then
            echo "Error: Path does not exist: $UE_PATH"
        else
            break
        fi
    done
    echo "UE Path set to: $UE_PATH"
    echo
}

PATCH_DIR="$(cd "$(dirname "$0")/Source" && pwd)"

echo "Choose action:"
echo "1) Dry-run patch"
echo "2) Apply patch"
echo "3) Rollback patch"
read -p "Enter number: " ACTION

case $ACTION in
    1)
        ask_ue_path
        echo "Running dry-run..."
        python3 "$PATCH_DIR/patcher.py" "$UE_PATH" --dry-run
        ;;
    2)
        ask_ue_path
        echo "Applying patch..."
        python3 "$PATCH_DIR/patcher.py" "$UE_PATH"
        ;;
    3)
        ask_ue_path
        echo "Rolling back patch..."
        python3 "$PATCH_DIR/patcher.py" "$UE_PATH" --rollback
        ;;
    *)
        echo "Invalid option. Exiting."
        ;;
esac
