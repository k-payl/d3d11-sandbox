#pragma once

#include "pch.h"

void InitializeLibrary();
bool UpdateLibrary();

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		abort();
}