// Created by christinayan01 by Takahiro Yanai. 2023.9.8

using UnrealBuildTool;

public class MoviePipelineShiftLensRenderPass : ModuleRules
{
	public MoviePipelineShiftLensRenderPass(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"ImageWriteQueue",	// yanai add
				"Json",
				"CinematicCamera", // yanai add
			}
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"MovieRenderPipelineCore",
				"MovieRenderPipelineRenderPasses",
				"MovieRenderPipelineSettings", // yanai add
				"ShiftLensCamera", // yanai add
				"RenderCore",
                "RHI",
				"ActorLayerUtilities",
				"OpenColorIO",
			}
			);

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(
			new string[] { "UnrealEd" });
		}
	}
}
