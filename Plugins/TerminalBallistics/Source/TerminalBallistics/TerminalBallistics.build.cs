// Copyright Erik Hedberg. All rights reserved.

using System.IO;
using UnrealBuildTool;
 
public class TerminalBallistics : ModuleRules
{
	public TerminalBallistics(ReadOnlyTargetRules Target) : base(Target)
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

        PublicDependencyModuleNames.AddRange(new string[] { "Chaos", "Core", "CoreUObject", "Engine", "PhysicsCore", "Niagara", "GameplayTags", "DeveloperSettings", "NetCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { "TBContactMechanics" });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}