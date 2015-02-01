#pragma once

#include <string>

namespace SA
{
	class SkeletalModel;

	// Serialization methods
	extern std::string  ModelToData(const SkeletalModel& a_Model);
	extern bool         ModelToData(const SkeletalModel& a_Model, std::string& a_OutData);
	extern unsigned int ModelToData(const SkeletalModel& a_Model, char* a_pOutData);
	extern bool         DataToModel(const char* a_pData, SkeletalModel& a_OutModel);
};