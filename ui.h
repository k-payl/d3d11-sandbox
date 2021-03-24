#pragma once
struct Data;

void InitUI(void* hwnd, void *context, void* device);
void FreeUI();
void BeginFrameUI();
void RenderUI(Data& data);