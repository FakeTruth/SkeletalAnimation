#include "AssimpConverter.h"

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// SkeletalModel
#include <SkeletalAnimation/SkeletalModel.h>

// SkeletalModel
using namespace SA;

namespace AssimpConverter
{
	//////////////////////////////////////////////////////////////////////////
	// Private functions to convert from Assimp data types to glm
	static glm::mat4x4 aiToGlm(const aiMatrix4x4& v)
	{
		glm::mat4x4 out;
		assert(sizeof(out) == sizeof(v));
		memcpy(&out, &v, sizeof(v));
		return glm::transpose(out);
	}





	static glm::vec3 aiToGlm(const aiVector3D& v)
	{
		glm::vec3 out;
		assert(sizeof(out) == sizeof(v));
		memcpy(&out, &v, sizeof(v));
		return out;
	}





	static glm::quat aiToGlm(const aiQuaternion& v)
	{
		return glm::quat(v.w, v.x, v.y, v.z);
	}





	//////////////////////////////////////////////////////////////////////////
	// Recursively add all nodes from scene to skeleton
	static unsigned int AddNodesToSkeleton(aiNode* a_pNode, sSkeleton& a_Skeleton)
	{
		unsigned int BoneID = a_Skeleton.Bones.size();
		{
			sBone NewBone;
			NewBone.Name = a_pNode->mName.C_Str();
			NewBone.NodeTransform = aiToGlm(a_pNode->mTransformation);
			NewBone.NumChildren = a_pNode->mNumChildren;
			NewBone.pChildren = new unsigned int[NewBone.NumChildren];
			NewBone.NumWeights = 0;
			NewBone.pWeights = NULL;
			for (unsigned int i = 0; i < NewBone.NumChildren; ++i)
				NewBone.pChildren[i] = -1;
			//
			a_Skeleton.Bones.push_back(NewBone);
		}

		for (unsigned int i = 0; i < a_pNode->mNumChildren; ++i)
		{
			unsigned int ChildBoneID = AddNodesToSkeleton(a_pNode->mChildren[i], a_Skeleton);
			a_Skeleton.Bones[BoneID].pChildren[i] = ChildBoneID;
		}

		return BoneID;
	}





	//////////////////////////////////////////////////////////////////////////
	// Adds meshes to AnimatedModel and populates bones in skeleton with weights
	static void AddMeshesAndBones(const aiScene* a_pScene, SkeletalModel& a_OutModel)
	{
		sSkeleton& Skeleton = a_OutModel.GetSkeleton();
		//
		for (unsigned int i = 0; i < a_pScene->mNumMeshes; ++i)
		{
			aiMesh* pMesh = a_pScene->mMeshes[i];
			//
			sAnimatedMesh AnimMesh;
			AnimMesh.NumVertices = pMesh->mNumVertices;
			AnimMesh.pVertices = new glm::vec3[AnimMesh.NumVertices];
			AnimMesh.pTransformedVertices = new glm::vec3[AnimMesh.NumVertices];
			AnimMesh.pNormals = new glm::vec3[AnimMesh.NumVertices];
			AnimMesh.pTransformedNormals = new glm::vec3[AnimMesh.NumVertices];
			AnimMesh.NumIndices = pMesh->mNumFaces * 3;
			AnimMesh.pIndices = new unsigned int[AnimMesh.NumIndices];

			memcpy(AnimMesh.pVertices, pMesh->mVertices, AnimMesh.NumVertices*sizeof(aiVector3D));
			memcpy(AnimMesh.pNormals, pMesh->mNormals, AnimMesh.NumVertices*sizeof(aiVector3D));
			for (unsigned int i = 0; i < pMesh->mNumBones; ++i)
			{
				aiBone* pBone = pMesh->mBones[i];
				sBone& Bone = *Skeleton_FindBone(Skeleton, pBone->mName.C_Str());
				Bone.OffsetMatrix = aiToGlm(pBone->mOffsetMatrix);
				Bone.NumWeights = pBone->mNumWeights;
				Bone.pWeights = new sWeight[pBone->mNumWeights];
				for (unsigned int i = 0; i < Bone.NumWeights; ++i)
				{
					Bone.pWeights[i].VertexID = pBone->mWeights[i].mVertexId;
					Bone.pWeights[i].Weight = pBone->mWeights[i].mWeight;
				}
			}

			for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
			{
				aiFace Face = pMesh->mFaces[i];
				//CORE_ASSERT(Face.mNumIndices == 3);
				AnimMesh.pIndices[i * 3 + 0] = Face.mIndices[0];
				AnimMesh.pIndices[i * 3 + 1] = Face.mIndices[1];
				AnimMesh.pIndices[i * 3 + 2] = Face.mIndices[2];
			}

			//
			a_OutModel.AddMesh(AnimMesh);
		}
	}





	//////////////////////////////////////////////////////////////////////////
	// Adds animations to AnimatedModel
	void AddAnimations(const aiScene* a_pScene, SkeletalModel& a_OutModel)
	{
		if (a_pScene->mNumAnimations > 0)
		{
			sAnimation& Animation = a_OutModel.GetAnimation();
			// Only use the first animation
			aiAnimation* pAnimation = a_pScene->mAnimations[0];
			for (unsigned int i = 0; i < pAnimation->mNumChannels; ++i)
			{
				aiNodeAnim* pChannel = pAnimation->mChannels[i];

				sNodeAnimation NodeAnimation;
				NodeAnimation.Name = pChannel->mNodeName.C_Str();
				for (unsigned int i = 0; i < pChannel->mNumPositionKeys; ++i)
				{
					sNodeAnimationKey<glm::vec3> Vec3Key;
					Vec3Key.Time = (float)pChannel->mPositionKeys[i].mTime;
					Vec3Key.Value = aiToGlm(pChannel->mPositionKeys[i].mValue);
					NodeAnimation.PositionKeys.push_back(Vec3Key);
				}

				for (unsigned int i = 0; i < pChannel->mNumRotationKeys; ++i)
				{
					sNodeAnimationKey<glm::quat> QuatKey;
					QuatKey.Time = (float)pChannel->mRotationKeys[i].mTime;
					QuatKey.Value = aiToGlm(pChannel->mRotationKeys[i].mValue);
					NodeAnimation.RotationKeys.push_back(QuatKey);
				}

				Animation.NodeAnimations.push_back(NodeAnimation);
			}

			Animation.TicksPerSecond = (float)a_pScene->mAnimations[0]->mTicksPerSecond;
			Animation.Duration = (float)a_pScene->mAnimations[0]->mDuration;
		}
	}




	
	//////////////////////////////////////////////////////////////////////////
	// Converts aiScene to AnimatedModel
	bool Convert(const aiScene* a_pScene, SkeletalModel& a_OutModel)
	{
		if (a_pScene == NULL)
			return false;

		a_OutModel.SetGlobalInverseTransform(glm::inverse(aiToGlm(a_pScene->mRootNode->mTransformation)));

		AddNodesToSkeleton(a_pScene->mRootNode, a_OutModel.GetSkeleton());
		AddMeshesAndBones(a_pScene, a_OutModel);
		AddAnimations(a_pScene, a_OutModel);

		return true;
	}	
}
