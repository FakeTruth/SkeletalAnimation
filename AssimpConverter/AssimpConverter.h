#pragma once

// SkeletalModel
namespace SA
{
	class SkeletalModel;
}

// Assimp
struct aiScene;

namespace AssimpConverter
{
	// Converts an Assimp scene to something we can render and animate
	bool Convert(const aiScene* a_pScene, SA::SkeletalModel& a_OutModel);
};