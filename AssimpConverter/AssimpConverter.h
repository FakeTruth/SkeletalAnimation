#pragma once

#include <SkeletalAnimation/SkeletalModel.h>

// Assimp
struct aiScene;

namespace AssimpConverter
{
	// Converts an Assimp scene to something we can render and animate
	bool Convert(const aiScene* a_pScene, SA::AnimatedModel& a_OutModel);
};