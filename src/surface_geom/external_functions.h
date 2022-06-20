#pragma once
#ifdef RENGA_AUXILIARY_ACTIONS
#define RENGA_AUXILIARY_ACTIONS_API __declspec(dllexport)
#else
#define RENGA_AUXILIARY_ACTIONS_API __declspec(dllimport)
#endif
#include <cstdint>
//extern "C" int32_t RENGA_AUXILIARY_ACTIONS_API __stdcall run_landxml2iges(const char* dir_path);
extern "C"  RENGA_AUXILIARY_ACTIONS_API void run_landxml2iges(const char* dir_path, double global_offset[]);