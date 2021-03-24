#include "library.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

void CompileShaders();

void InitUI(void *hwnd, void* context, void* device)
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init((HWND)hwnd);
	ImGui_ImplDX11_Init((ID3D11Device*)device, (ID3D11DeviceContext*)context);
}

void BeginFrameUI()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void RenderUI(Data& data)
{
    {
        static float f = 0.0f;

        ImGui::Begin("Settings");
        ImGui::SliderFloat("mMetallic", &data.mesh->cbuffer.Get().vars.x, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderFloat("Variable 2", &data.mesh->cbuffer.Get().vars.y, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderFloat("Variable 3", &data.mesh->cbuffer.Get().vars.z, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderFloat("Variable 4", &data.mesh->cbuffer.Get().vars.w, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::SliderFloat("Scale", &data.mesh->cbuffer.Get().scale.x, 0.0f, 3.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("Color", (float*)&data.mesh->cbuffer.Get().albedo); // Edit 3 floats representing a color
        {
            const char* items[] = { "Cube", "Plane", "Teapot"};
            const char* current_item = items[data.meshIndex];

            if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(items[n], is_selected))
                        data.meshIndex = n;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
        }
        {
            const char* items[] = { "Room", "Angar", "Night"};
            const char* current_item = items[data.cubemapIndex];

            if (ImGui::BeginCombo("##combo1", current_item)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(items[n], is_selected))
                        data.cubemapIndex = n;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
        }
        ImGui::Checkbox("Rotate", &data.rotate);
        ImGui::SliderFloat("Rotation Speed", &data.rotationSpeed, 0.0f, 1.0f);
        if (ImGui::Button("Recompile (F5)"))
        {
            CompileShaders();
        }

        ImGui::InputTextMultiline("##text", data.CompileError.data(), data.CompileError.size());

        ImGui::Checkbox("VSync", &data.VSync);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void FreeUI()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
