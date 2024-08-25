using System.IO;
using UnrealBuildTool;
 
public class TBContactMechanics : ModuleRules
{
	public TBContactMechanics(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        CppStandard = CppStandardVersion.Cpp20;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        IWYUSupport = IWYUSupport.Full;

#if UE_5_3_OR_LATER
        bWarningsAsErrors = true;
#endif
        bStaticAnalyzerExtensions = true;
        bDisableStaticAnalysis = false;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
 
		PublicIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Public") });
		PrivateIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Private") });
	}
}