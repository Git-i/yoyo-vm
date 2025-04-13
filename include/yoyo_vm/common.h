#pragma once
#ifdef _MSC_VER
#ifdef YOYO_VM_DLL
#define YVM_API __declspec(dllexport)
#else
#define YVM_API __declspec(dllimport)
#endif
#else
#define YVM_API
#endif