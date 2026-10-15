using UnrealBuildTool;
public class FoldlightDemoEditorTarget : TargetRules
{
    public FoldlightDemoEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("NEWMyProject");
    }
}
