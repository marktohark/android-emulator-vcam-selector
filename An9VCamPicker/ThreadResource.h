#pragma once

#include <map>
#include <windows.h>
#include <mutex>
#include "objidl.h"

using namespace std;

#ifndef THREAD_RESOURCE_H
#define THREAD_RESOURCE_H

class Context {
public:
	IMoniker* m_IMoniker{ nullptr };
};

class ThreadResource {
private:
	map<DWORD, Context*> m_contexts;
	mutex m_lock;
public:
	~ThreadResource() {
		for (auto i = m_contexts.begin(); i != m_contexts.end(); i++) {
			if (i->second != nullptr) {
				delete i->second;
			}
		}
	}

	Context* GetContext() {
		DWORD tid = GetCurrentThreadId();
		Context* ctx = nullptr;
		m_lock.lock();
		if (m_contexts.find(tid) == m_contexts.end()) {
			ctx = new Context;
			m_contexts[tid] = ctx;
		}
		else {
			ctx = m_contexts[tid];
		}
		m_lock.unlock();
		return ctx;
	}
};

#endif


