#include "library.h"
#include <chrono>

void CompileShaders();

Data* data;

int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    InitializeLibrary();

    data = new Data;

    data->camera = std::make_unique<Camera>();
    data->camera->position = DirectX::XMFLOAT3(0.0f, 1.0f, -1.6f);
    data->camera->rotation = DirectX::XMFLOAT3(30.0f, 0.0f, 0.0f);

    data->albedoTexture = Texture::Create2D(L"../../textures/albedo.jpg");
    data->metallicTexture = Texture::Create2D(L"../../textures/metalness.jpg");
    data->heighTexture = Texture::Create2D(L"../../textures/toy_box_disp.jpg");
    data->normalTexture = Texture::Create2D(L"../../textures/normal.jpg");
    data->nightEnvironment = Texture::CreateCube(L"../../textures/cube_night.dds");
    data->angarEnvironment = Texture::CreateCube(L"../../textures/cube_angar.dds");
    data->roomEnvironment = Texture::CreateCube(L"../../textures/cube_room.dds");

    CompileShaders();

    data->planeMesh = Mesh::CreatePlane(DirectX::XMFLOAT2(1.0f, 1.0f));
    data->cubeMesh = Mesh::CreateCube(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
    data->teapotMesh = Mesh::CreateFromFile(L"../../teapot.bin");

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    while (UpdateLibrary())
    {
        data->camera->Start();

        std::chrono::duration<float> _durationSec =
            std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);
        float dt = _durationSec.count();

        switch (data->meshIndex)
        {
            case 0:  data->mesh = data->cubeMesh.get(); break;
            case 1:  data->mesh = data->planeMesh.get(); break;
            case 2:  data->mesh = data->teapotMesh.get(); break;
            default: data->mesh = data->planeMesh.get(); break;
        }

        switch (data->cubemapIndex)
        {
        case 0:  data->mesh->cubeTexture = data->roomEnvironment.get(); break;
        case 1:  data->mesh->cubeTexture = data->angarEnvironment.get(); break;
        case 2:  data->mesh->cubeTexture = data->nightEnvironment.get(); break;
        default:  data->mesh->cubeTexture = data->angarEnvironment.get(); break;
        }
        data->mesh->texture = *data->albedoTexture;
        data->mesh->normalTexture = data->normalTexture.get();
        data->mesh->metallicTexture = data->metallicTexture.get();
        data->mesh->shader = data->forward.get();

        if (data->rotate)
            data->mesh->rotation.y += 50.f * data->rotationSpeed * dt;
        
        data->mesh->Draw();

        RenderUI(*data);

        data->camera->Stop();
        start = std::chrono::steady_clock::now();
    }

    FreeUI();
    delete data;

    return 0;
}


