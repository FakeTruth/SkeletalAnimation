#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// STD
#include <vector>
#include <string>

namespace SA
{
	struct sWeight
	{
		unsigned int VertexID;
		float Weight;
	};

	struct sBone
	{
		std::string Name;

		glm::mat4x4 NodeTransform;
		glm::mat4x4 OffsetMatrix; // T-Pose to local bone space
		glm::mat4x4 FinalTransformation;

		unsigned int NumWeights;
		sWeight* pWeights;

		unsigned int NumChildren;
		unsigned int* pChildren;
	};

	struct sAnimatedMesh
	{
		unsigned int NumVertices;

		glm::vec3* pVertices;
		glm::vec3* pNormals;

		glm::vec3* pTransformedVertices;
		glm::vec3* pTransformedNormals;

		unsigned int NumIndices;
		unsigned int* pIndices;
	};

	struct sSkeleton
	{
		std::vector<sBone> Bones;
	};

	template <typename _Ty>
	struct sNodeAnimationKey
	{
		_Ty Value;
		float Time;
	};

	struct sNodeAnimation
	{
		std::string Name;

		std::vector<sNodeAnimationKey<glm::vec3> > PositionKeys;
		std::vector<sNodeAnimationKey<glm::quat> > RotationKeys;
	};

	struct sAnimation
	{
		std::vector<sNodeAnimation> NodeAnimations;

		float TicksPerSecond;
		float Duration;
	};
}