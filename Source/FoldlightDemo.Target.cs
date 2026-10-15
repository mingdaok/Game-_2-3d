using UnrealBuildTool;
public class FoldlightDemoTarget : TargetRules
{
    public FoldlightDemoTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("NEWMyProject");
    }
}
