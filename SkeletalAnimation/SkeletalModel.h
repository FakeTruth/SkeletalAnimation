#pragma once

#include "Types.h"

namespace SA
{
	class SkeletalModel
	{
	public:
		SkeletalModel();
		~SkeletalModel();

		void                            Update(float a_Dt);

		unsigned int                    GetNumMeshes() const { return m_Meshes.size(); }
		const sAnimatedMesh&            GetMesh(unsigned int i) const { return m_Meshes[i]; }
		void                            AddMesh(const sAnimatedMesh& a_Mesh) { m_Meshes.push_back(a_Mesh); }

		void                            SetGlobalInverseTransform(const glm::mat4x4& a_Transform) { m_GlobalInverseTransform = a_Transform; }
		const glm::mat4x4&              GetGlobalInverseTransform() const { return m_GlobalInverseTransform; }

		sSkeleton&                      GetSkeleton() { return m_Skeleton; }
		const sSkeleton&                GetSkeleton() const { return m_Skeleton; }

		sAnimation&                     GetAnimation() { return m_Animation; }
		const sAnimation&               GetAnimation() const { return m_Animation; }

		void                            Clear();

	private:
		std::vector<sAnimatedMesh>      m_Meshes;
		sSkeleton                       m_Skeleton;
		sAnimation                      m_Animation;
		glm::mat4x4                     m_GlobalInverseTransform;

		float                           m_AnimationTime;

	private:
		void                            ReadNodeHierarchy(float AnimationTime, sAnimation& a_Animation, sSkeleton& a_Skeleton, sBone& a_Bone, const glm::mat4x4& ParentTransform);
		void                            TransformVertices(const sSkeleton& a_Skeleton);
	};
}





namespace SA
{
	// Helper functions
	static glm::vec3                       NodeAnimation_FindInterpolatedPosition(const sNodeAnimation& a_NodeAnimation, float a_AnimationTime);
	static glm::quat                       NodeAnimation_FindInterpolatedRotation(const sNodeAnimation& a_NodeAnimation, float a_AnimationTime);
	template <typename _Ty> static  _Ty    NodeAnimation_FindInterpolatedValue(std::vector<sNodeAnimationKey<_Ty> > a_Keys, float a_AnimationTime);
	template <typename _Ty> static unsigned int NodeAnimation_FindIndex(const _Ty& a_Keys, float a_AnimationTime);

	extern const sNodeAnimation*           FindNodeAnim(const sAnimation& a_Animation, const std::string& a_NodeName);
	extern glm::vec3                       InterpolateValue(const glm::vec3& a_Start, const glm::vec3& a_End, float a_Factor);
	extern glm::quat                       InterpolateValue(const glm::quat& a_Start, const glm::quat& a_End, float a_Factor);
	extern unsigned int                    Skeleton_FindBoneIndex(const sSkeleton& a_Skeleton, const std::string& a_Name);
	extern sBone*                          Skeleton_FindBone(sSkeleton& a_Skeleton, const std::string& a_Name);
}





namespace SA
{
	glm::vec3 NodeAnimation_FindInterpolatedPosition(const sNodeAnimation& a_NodeAnimation, float a_AnimationTime)
	{
		return NodeAnimation_FindInterpolatedValue(a_NodeAnimation.PositionKeys, a_AnimationTime);
	}





	glm::quat NodeAnimation_FindInterpolatedRotation(const sNodeAnimation& a_NodeAnimation, float a_AnimationTime) 
	{ 
		return NodeAnimation_FindInterpolatedValue(a_NodeAnimation.RotationKeys, a_AnimationTime); 
	}





	template <typename _Ty>
	_Ty NodeAnimation_FindInterpolatedValue(std::vector<sNodeAnimationKey<_Ty> > a_Keys, float a_AnimationTime)
	{
		if (a_Keys.size() == 1)
		{
			return a_Keys[0].Value;
		}

		unsigned int PositionIndex = NodeAnimation_FindIndex(a_Keys, a_AnimationTime);
		unsigned int NextPositionIndex = (PositionIndex + 1);
		//CORE_ASSERT(NextPositionIndex < a_Keys.size());
		float DeltaTime = a_Keys[NextPositionIndex].Time - a_Keys[PositionIndex].Time;
		float Factor = (a_AnimationTime - a_Keys[PositionIndex].Time) / DeltaTime;
		//CORE_ASSERT(Factor >= 0.0f && Factor <= 1.0f);
		const _Ty& StartPosition = a_Keys[PositionIndex].Value;
		const _Ty& EndPosition = a_Keys[NextPositionIndex].Value;

		return InterpolateValue(StartPosition, EndPosition, Factor);
	}





	template <typename _Ty>
	unsigned int NodeAnimation_FindIndex(const _Ty& a_Keys, float a_AnimationTime)
	{
		for (unsigned int i = 0; i < a_Keys.size() - 1; ++i)
		{
			if (a_AnimationTime < a_Keys[i + 1].Time)
				return i;
		}

		//CORE_ASSERT(false);
		return -1;
	}
}