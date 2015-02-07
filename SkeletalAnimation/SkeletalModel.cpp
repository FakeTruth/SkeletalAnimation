#include "SkeletalModel.h"

//////////////////////////////////////////////////////////////////////////
// Based on: http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html //
//////////////////////////////////////////////////////////////////////////

namespace SA
{
	SkeletalModel::SkeletalModel() 
		: m_AnimationTime(0.0f)
	{
		Clear();
	}





	SkeletalModel::~SkeletalModel()
	{
		Clear();
	}





	void SkeletalModel::Clear()
	{
		for (unsigned int i = 0; i < m_Meshes.size(); ++i)
		{
			delete[] m_Meshes[i].pVertices;
			delete[] m_Meshes[i].pNormals;
			delete[] m_Meshes[i].pTransformedVertices;
			delete[] m_Meshes[i].pTransformedNormals;
			delete[] m_Meshes[i].pIndices;
		}
		m_Meshes.clear();

		for (unsigned int i = 0; i < m_Skeleton.Bones.size(); ++i)
		{
			delete[] m_Skeleton.Bones[i].pWeights;
			delete[] m_Skeleton.Bones[i].pChildren;
		}
		m_Skeleton.Bones.clear();

		m_Animation.NodeAnimations.clear();
		m_Animation.Duration = 0.0f;
		m_Animation.TicksPerSecond = 0.0f;

		m_GlobalInverseTransform = glm::mat4x4(1);
		m_AnimationTime = 0.0f;
	}





	void SkeletalModel::Update(float a_Dt)
	{
		m_AnimationTime = fmodf(m_AnimationTime + a_Dt * m_Animation.TicksPerSecond, m_Animation.Duration);
		//

		ReadNodeHierarchy(m_AnimationTime, m_Animation, m_Skeleton, m_Skeleton.Bones[0], glm::mat4x4(1));
		TransformVertices(m_Skeleton);
	}





	void SkeletalModel::ReadNodeHierarchy(float AnimationTime, sAnimation& a_Animation, sSkeleton& a_Skeleton, sBone& a_Bone, const glm::mat4x4& ParentTransform)
	{
		std::string NodeName(a_Bone.Name);
		glm::mat4x4 NodeTransformation(a_Bone.NodeTransform);
		const sNodeAnimation* pNewNodeAnim = FindNodeAnim(a_Animation, NodeName);

		if (pNewNodeAnim)
		{
			glm::vec3 Translation = NodeAnimation_FindInterpolatedPosition(*pNewNodeAnim, AnimationTime);
			glm::quat RotationQ = NodeAnimation_FindInterpolatedRotation(*pNewNodeAnim, AnimationTime);

// 			glm::vec3 Scaling2(1, 1, 1);
// 			glm::mat4x4 ScalingM2 = glm::scale(Scaling2);

			glm::mat4x4 RotationM2 = glm::toMat4(RotationQ);

			glm::mat4x4 TranslationM2 = glm::translate(Translation);

			// Combine the above transformations
			NodeTransformation = TranslationM2 * RotationM2;// * ScalingM2;
		}

		glm::mat4x4 GlobalTransformation = ParentTransform * NodeTransformation;

		unsigned int BoneIndex = Skeleton_FindBoneIndex(a_Skeleton, NodeName);
		if (BoneIndex != -1)
		{
			sBone* pBone = &a_Skeleton.Bones[BoneIndex];
			pBone->FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * pBone->OffsetMatrix;
		}

		for (unsigned int i = 0; i < a_Bone.NumChildren; i++)
		{
			ReadNodeHierarchy(AnimationTime, a_Animation, a_Skeleton, a_Skeleton.Bones[a_Bone.pChildren[i]], GlobalTransformation);
		}
	}





	void SkeletalModel::TransformVertices(const sSkeleton& a_Skeleton)
	{
		for (unsigned int i = 0; i < m_Meshes.size(); ++i)
		{
			// Reset mesh vertices and normals
			sAnimatedMesh& AnimMesh = m_Meshes[i];
			memset(AnimMesh.pTransformedVertices, 0, AnimMesh.NumVertices*sizeof(glm::vec3));
			memset(AnimMesh.pTransformedNormals, 0, AnimMesh.NumVertices*sizeof(glm::vec3));

			//
			for (unsigned int i = 0; i < a_Skeleton.Bones.size(); ++i)
			{
				const sBone& Bone = a_Skeleton.Bones[i];
				//
				glm::mat4x4 Transformation = Bone.FinalTransformation;
				glm::mat3x3 Rotation = glm::mat3x3(Transformation);
				//
				for (unsigned int i = 0; i < Bone.NumWeights; ++i)
				{
					sWeight Weight = Bone.pWeights[i];
					//
					glm::vec3 inVertex = AnimMesh.pVertices[Weight.VertexID];
					glm::vec3& outVertex = AnimMesh.pTransformedVertices[Weight.VertexID];
					outVertex += glm::vec3((Transformation * glm::vec4(inVertex, 1)) * Weight.Weight);
					//
					glm::vec3 inNormal = AnimMesh.pNormals[Weight.VertexID];
					glm::vec3& outNormal = AnimMesh.pTransformedNormals[Weight.VertexID];
					outNormal += (Rotation * inNormal) * Weight.Weight;
				}
			}
		}

		// Normalize normals
		for (unsigned int i = 0; i < m_Meshes.size(); ++i)
		{
			sAnimatedMesh& AnimMesh = m_Meshes[i];
			for (unsigned int i = 0; i < AnimMesh.NumVertices; ++i)
			{
				AnimMesh.pTransformedNormals[i] = glm::normalize(AnimMesh.pTransformedNormals[i]);
			}
		}
	}





	const sNodeAnimation* FindNodeAnim(const sAnimation& a_Animation, const std::string& a_NodeName)
	{
		for (unsigned int i = 0; i < a_Animation.NodeAnimations.size(); ++i)
		{
			const sNodeAnimation& NodeAnim = a_Animation.NodeAnimations[i];
			if (NodeAnim.Name == a_NodeName)
			{
				return &NodeAnim;
			}
		}

		return NULL;
	}





	glm::vec3 InterpolateValue(const glm::vec3& a_Start, const glm::vec3& a_End, float a_Factor)
	{
		return glm::lerp(a_Start, a_End, a_Factor);
	}





	glm::quat InterpolateValue(const glm::quat& a_Start, const glm::quat& a_End, float a_Factor)
	{
		return glm::slerp(a_Start, a_End, a_Factor);
	}





	unsigned int Skeleton_FindBoneIndex(const sSkeleton& a_Skeleton, const std::string& a_Name)
	{
		for (unsigned int i = 0; i < a_Skeleton.Bones.size(); ++i)
		{
			if (a_Skeleton.Bones[i].Name == a_Name)
				return i;
		}
		return -1;
	}





	sBone* Skeleton_FindBone(sSkeleton& a_Skeleton, const std::string& a_Name)
	{
		return &a_Skeleton.Bones[Skeleton_FindBoneIndex(a_Skeleton, a_Name)];
	}
}