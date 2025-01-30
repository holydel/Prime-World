#pragma once
#include <map>

struct WebTalentsData {
  std::map<int, std::string> classTalentMap;

  WebTalentsData();
  std::string ConvertFromClassID(int id);
};