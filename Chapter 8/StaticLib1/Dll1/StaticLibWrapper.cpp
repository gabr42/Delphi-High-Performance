// Dll1.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CppClass.h"

#define EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

#define MAXOBJECTS 100

// #### Object allocation ####

class IndexAllocator
{
private:
	void* objs[MAXOBJECTS];
	CRITICAL_SECTION lock;
public:
	IndexAllocator() {
		InitializeCriticalSection(&lock);
		for (int deviceIndex = 0; deviceIndex < MAXOBJECTS; deviceIndex++) objs[deviceIndex] = NULL;
	}
	virtual ~IndexAllocator() {
		DeleteCriticalSection(&lock);
	}
	bool Allocate(int& deviceIndex, void* obj) {
		EnterCriticalSection(&lock);
		_try{
			for (deviceIndex = 0; deviceIndex < MAXOBJECTS; deviceIndex++) {
				if (objs[deviceIndex] == NULL) {
					objs[deviceIndex] = obj;
					return TRUE;
				}
			}
		return FALSE;
		}
		_finally{
			LeaveCriticalSection(&lock);
		}
	}
	bool Release(int deviceIndex) {
		if ((deviceIndex < 0) || (deviceIndex >= MAXOBJECTS) || (objs[deviceIndex] == NULL)) {
			return FALSE;
		}
		objs[deviceIndex] = NULL;
		return TRUE;
	}
	void* Get(int deviceIndex) { 
		if ((deviceIndex < 0) || (deviceIndex >= MAXOBJECTS))
			return NULL;
		else
			return objs[deviceIndex];
	}
};

// #### DLL ####

IndexAllocator* GAllocator = NULL;

extern "C" int WINAPI Initialize()
{
#pragma EXPORT
	if (GAllocator != NULL)
		return -1;
	else {
		GAllocator = new IndexAllocator();
		return 0;
	}
}

extern "C" int WINAPI Finalize()
{
#pragma EXPORT

	if (GAllocator == NULL)
		return -1;
	else {
		delete GAllocator;
		GAllocator = NULL;
	}
	return 0;
}

// #### CppClass ####

extern "C" int WINAPI CreateCppClass (int& index)
{
#pragma EXPORT

	CppClass* instance = new CppClass;

	if (!GAllocator->Allocate(index, (void*)instance)) {
		delete instance;
		return -1;
	}
	else 
		return 0;
}

extern "C" int WINAPI DestroyCppClass(int index)
{
#pragma EXPORT

	CppClass* instance = (CppClass*)GAllocator->Get(index);
	if (!GAllocator->Release(index))
		return -1;
	else {
		delete instance;
		return 0;
	}
}

extern "C" int WINAPI CppClass_setValue(int index, int value)
{
#pragma EXPORT

	CppClass* instance = (CppClass*)GAllocator->Get(index);
	if (instance == NULL) {
		return -1;
	}
	else {
		instance->setData(value);
		return 0;
	}
}

extern "C" int WINAPI CppClass_getSquare(int index, int& value)
{
#pragma EXPORT

	CppClass* instance = (CppClass*)GAllocator->Get(index);
	if (instance == NULL)
		return -1;
	else {
		value = instance->getSquare();
		return 0;
	}
}
