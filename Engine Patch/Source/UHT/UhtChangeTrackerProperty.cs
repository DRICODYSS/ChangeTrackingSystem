using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Text;
using EpicGames.Core;
using EpicGames.UHT.Tables;
using EpicGames.UHT.Tokenizer;
using EpicGames.UHT.Utils;

namespace EpicGames.UHT.Types
{
	[UnrealHeaderTool]
	[UhtEngineClass(Name = "ChangeTrackerProperty", IsProperty = true)]
	public class UhtChangeTrackerProperty : UhtContainerBaseProperty
	{
		public override string EngineClassName => "ChangeTrackerProperty";
		protected override string CppTypeText => "TChangeTracker";
		protected override string PGetMacroText => "TCHANGETRACKER";
		protected override UhtPGetArgumentType PGetTypeArgument => UhtPGetArgumentType.TypeText;

		public UhtChangeTrackerProperty(UhtPropertySettings propertySettings, UhtProperty value)
			: base(propertySettings, value)
		{
			ValidateInner(value);

			PropertyFlags |= ValueProperty.PropertyFlags & (EPropertyFlags.UObjectWrapper | EPropertyFlags.TObjectPtr);
			if (ValueProperty.MetaData.ContainsKey(UhtNames.NativeConst))
			{
				MetaData.Add(UhtNames.NativeConstTemplateArg, "");
				ValueProperty.MetaData.Remove(UhtNames.NativeConst);
			}

			PropertyCaps |= UhtPropertyCaps.PassCppArgsByRef;
			PropertyCaps &= ~UhtPropertyCaps.CanBeContainerKey;

			UpdateCaps();
			EnforceCaps();

			NormalizeInner();
		}

		#region VALIDATION

		private static bool IsSupportedInner(UhtProperty prop)
		{
			return prop is UhtBoolProperty
				|| prop is UhtFloatProperty
				|| prop is UhtDoubleProperty
				|| prop is UhtIntProperty
				|| prop is UhtInt64Property
				|| prop is UhtByteProperty
				|| prop is UhtNameProperty
				|| prop is UhtEnumProperty
				|| prop is UhtStructProperty
				|| prop is UhtObjectPropertyBase
				|| prop is UhtArrayProperty
				|| prop is UhtSetProperty
				|| prop is UhtMapProperty;
		}

		private void ValidateInner(UhtProperty value)
		{
			if (!IsSupportedInner(value))
			{
				value.LogError($"Type '{value.GetType().Name}' is not supported in TChangeTracker.");
			}
		}

		#endregion

		#region CAPS

		private void EnforceCaps()
		{
			PropertyCaps &= ~(
				UhtPropertyCaps.CanBeContainerKey |
				UhtPropertyCaps.CanBeContainerValue |
				UhtPropertyCaps.SupportsRigVM
			);
		}

		private void UpdateCaps()
		{
			PropertyCaps &= ~(
				UhtPropertyCaps.IsParameterSupportedByBlueprint |
				UhtPropertyCaps.IsMemberSupportedByBlueprint |
				UhtPropertyCaps.CanExposeOnSpawn
			);

			PropertyCaps |= ValueProperty.PropertyCaps & UhtPropertyCaps.CanExposeOnSpawn;

			if (ValueProperty.PropertyCaps.HasAnyFlags(UhtPropertyCaps.IsParameterSupportedByBlueprint))
			{
				PropertyCaps |= UhtPropertyCaps.IsParameterSupportedByBlueprint |
								UhtPropertyCaps.IsMemberSupportedByBlueprint;
			}
		}

		#endregion

		#region NORMALIZE INNER

		private void NormalizeInner()
		{
			ValueProperty.SourceName = SourceName;
			ValueProperty.EngineName = EngineName;

			ValueProperty.PropertyFlags =
				(ValueProperty.PropertyFlags & EPropertyFlags.PropagateKeepInInner) |
				(PropertyFlags & EPropertyFlags.PropagateToOptionalInner);

			ValueProperty.Outer = this;
			ValueProperty.MetaData.Clear();
		}

		#endregion

		#region RESOLVE

		protected override bool ResolveSelf(UhtResolvePhase phase)
		{
			bool result = base.ResolveSelf(phase);

			if (phase == UhtResolvePhase.Final)
			{
				ValidateInner(ValueProperty);

				PropertyFlags |= ResolveAndReturnNewFlags(ValueProperty, phase);
				MetaData.Add(ValueProperty.MetaData);

				ValueProperty.PropertyFlags =
					(ValueProperty.PropertyFlags & EPropertyFlags.PropagateKeepInInner) |
					(PropertyFlags & EPropertyFlags.PropagateToOptionalInner);

				ValueProperty.MetaData.Clear();

				PropagateFlagsFromInnerAndHandlePersistentInstanceMetadata(this, MetaData, ValueProperty);

				UpdateCaps();
				EnforceCaps();
			}

			return result;
		}

		#endregion

		#region OVERRIDES

		public override IEnumerable<UhtType> EnumerateReferencedTypes()
		{
			foreach (UhtType type in ValueProperty.EnumerateReferencedTypes())
			{
				yield return type;
			}
		}

		public override StringBuilder AppendText(StringBuilder builder, UhtPropertyTextType textType, bool isTemplateArgument)
		{
			switch (textType)
			{
				case UhtPropertyTextType.SparseShort:
					builder.Append("TChangeTracker");
					break;

				case UhtPropertyTextType.FunctionThunkParameterArgType:
					builder.AppendFunctionThunkParameterArrayType(ValueProperty, true);
					break;

				case UhtPropertyTextType.VerseMangledType:
					builder.Append('?');
					if (PropertyExportFlags.HasAnyFlags(UhtPropertyExportFlags.VerseNamed))
					{
						builder.Append($"{VerseName}:");
					}
					builder.AppendPropertyVerseMangledType(ValueProperty);
					break;

				default:
					builder.Append("TChangeTracker<")
						   .AppendPropertyText(ValueProperty, textType, true)
						   .Append('>');
					break;
			}
			return builder;
		}

		public override StringBuilder AppendMetaDataDecl(StringBuilder builder, IUhtPropertyMemberContext context, string name, string nameSuffix, int tabs)
		{
			ValueProperty.AppendMetaDataDecl(builder, context, name, GetNameSuffix(nameSuffix, "_Inner"), tabs);
			return base.AppendMetaDataDecl(builder, context, name, nameSuffix, tabs);
		}

		public override StringBuilder AppendMemberDecl(StringBuilder builder, IUhtPropertyMemberContext context, string name, string nameSuffix, int tabs)
		{
			builder.AppendMemberDecl(ValueProperty, context, name, GetNameSuffix(nameSuffix, "_Inner"), tabs);
			return AppendMemberDecl(builder, context, name, nameSuffix, tabs, "FGenericPropertyParams");
		}

		public override StringBuilder AppendConstInitMemberDecl(StringBuilder builder, IUhtPropertyMemberContext context, string name, string nameSuffix, int tabs)
		{
			builder.AppendConstInitMemberDecl(ValueProperty, context, name, GetNameSuffix(nameSuffix, "_Inner"), tabs);
			return base.AppendConstInitMemberDecl(builder, context, name, nameSuffix, tabs);
		}

		public override StringBuilder AppendMemberDef(StringBuilder builder, IUhtPropertyMemberContext context, string name, string nameSuffix, string? offset, int tabs)
		{
			builder.AppendMemberDef(ValueProperty, context, name, GetNameSuffix(nameSuffix, "_Inner"), "0", tabs);

			AppendMemberDefStart(builder, context, name, nameSuffix, offset, tabs,
				"FGenericPropertyParams",
				"UECodeGen_Private::EPropertyGenFlags::ChangeTracker");

			AppendMemberDefEnd(builder, context, name, nameSuffix);
			return builder;
		}

		public override StringBuilder AppendConstInitMemberDef(StringBuilder builder, IUhtPropertyMemberContext context, string name, string nameSuffix, Action<StringBuilder>? outerFunc, string? offset, int tabs)
		{
			builder.AppendConstInitMemberDef(
				ValueProperty,
				context,
				name,
				GetNameSuffix(nameSuffix, "_Inner"),
				(builder) => builder.AppendConstInitMemberPtr(this, context, name, nameSuffix, tabs, ""),
				"0",
				tabs
			);

			AppendConstInitMemberDefStart(builder, context, name, nameSuffix, outerFunc, offset, tabs);
			builder.AppendConstInitMemberPtr(ValueProperty, context, name, GetNameSuffix(nameSuffix, "_Inner"), tabs, ", ");
			AppendConstInitMemberDefEnd(builder, context);

			return builder;
		}

		public override StringBuilder AppendMemberPtr(StringBuilder builder, IUhtPropertyMemberContext context, string name, string nameSuffix, int tabs)
		{
			builder.AppendMemberPtr(ValueProperty, context, name, GetNameSuffix(nameSuffix, "_Inner"), tabs);
			base.AppendMemberPtr(builder, context, name, nameSuffix, tabs);
			return builder;
		}

		public override void AppendObjectHashes(StringBuilder builder, int startingLength, IUhtPropertyMemberContext context)
		{
			ValueProperty.AppendObjectHashes(builder, startingLength, context);
		}

		public override StringBuilder AppendNullConstructorArg(StringBuilder builder, bool isInitializer)
		{
			builder.AppendPropertyText(this, UhtPropertyTextType.Construction).Append("()");
			return builder;
		}

		public override bool SanitizeDefaultValue(IUhtTokenReader defaultValueReader, StringBuilder innerDefaultValue)
		{
			return false;
		}

		public override void Validate(UhtStruct outerStruct, UhtProperty outermostProperty, UhtValidationOptions options)
		{
			base.Validate(outerStruct, outermostProperty, options);

			ValidateInner(ValueProperty);

			if (PropertyFlags.HasAnyFlags(EPropertyFlags.Net))
			{
				this.LogError("Replicated TChangeTracker is not supported");
			}
		}

		public override bool IsSameType(UhtProperty other)
		{
			if (other is UhtChangeTrackerProperty otherProp)
			{
				return ValueProperty.IsSameType(otherProp.ValueProperty);
			}
			return false;
		}

		#endregion

		#region KEYWORD

		[UhtPropertyType(Keyword = "TChangeTracker")]
		[SuppressMessage("CodeQuality", "IDE0051", Justification = "Attribute accessed")]
		private static UhtProperty? ChangeTrackerProperty(UhtPropertyResolveArgs args)
		{
			UhtPropertySettings propertySettings = args.PropertySettings;
			IUhtTokenReader tokenReader = args.TokenReader;

			using UhtMessageContext tokenContext = new("TChangeTracker");

			if (!args.SkipExpectedType())
			{
				return null;
			}

			tokenReader.Require('<');

			UhtProperty? value = args.ParseTemplateParam(propertySettings.SourceName);
			if (value == null)
			{
				return null;
			}

			tokenReader.Require('>');

			return new UhtChangeTrackerProperty(propertySettings, value);
		}

		#endregion
	}
}