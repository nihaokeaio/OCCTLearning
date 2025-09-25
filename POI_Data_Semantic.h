//
// Created by gmh on 24-8-19.
//

#ifndef ROSEFINCH_POI_DATA_SEMANTIC_H
#define ROSEFINCH_POI_DATA_SEMANTIC_H

#include "Data_Semantic.h"
#include "bvh/v2/vec.h"


/*
 * 该类用于描述吸附 POI 的语义信息，作为吸附结果的一部分返回。
 *
 * 比如，管道的起点，终点。墙的定义线等等。 实现规范参见 Data_Semantic.h
 * */

class POI_Data_Semantic: public Data_Semantic<POI_Data_Semantic>{
 public:
  inline static const std::string Unknown = "Unknown";

  inline static const std::vector<std::string_view> Views = {
      std::string_view(Unknown)
  };

 public:
  POI_Data_Semantic() {
      m_Semantic.push_back(PreDefined(Unknown));
  };
  DECLARE_SEMANTIC_SUCCESSOR_PARAM_CONSTRUCTOR(POI_Data_Semantic)
};



#endif//ROSEFINCH_POI_DATA_SEMANTIC_H
