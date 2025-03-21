#include <GLContext.h>
#include <GLFunctions.h>
#include <GLTexture.h>

#include <iostream>
#include <vector>
#include <chrono>

#include <d3d11.h>
#include <dxgiformat.h>
#include <d3d11_3.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main()
{
    D3D_DRIVER_TYPE driver_types[] = {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
    };

    D3D_FEATURE_LEVEL feature_levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
    };

    ID3D11DeviceContext* device_context = nullptr;
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL feature_level;
    HRESULT hr = S_OK;
    for (UINT index = 0; index < ARRAYSIZE(driver_types); index++)
    {
        UINT flags = 0;
        D3D_DRIVER_TYPE driver_type = driver_types[index];
            hr = D3D11CreateDevice(nullptr, driver_type, nullptr, flags, feature_levels, ARRAYSIZE(feature_levels),
                               D3D11_SDK_VERSION, &device, &feature_level, &device_context);
        if (hr == E_INVALIDARG)
            hr = D3D11CreateDevice(nullptr, driver_type, nullptr, flags, &feature_levels[1], ARRAYSIZE(feature_levels) - 1,
                               D3D11_SDK_VERSION, &device, &feature_level, &device_context);
        if (SUCCEEDED(hr))
            break;
    }

    int width, height, channels;
    uint8_t* image = stbi_load(ASSETS_DIR"a.png", &width, &height, &channels, STBI_rgb_alpha);

    D3D11_TEXTURE2D_DESC desc = {
        .Width = static_cast<uint32_t>(width),
        .Height = static_cast<uint32_t>(height),
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
        .CPUAccessFlags = 0,
        .MiscFlags = D3D11_RESOURCE_MISC_SHARED
    };

    D3D11_SUBRESOURCE_DATA subresource_data = {
        .pSysMem = image,
        .SysMemPitch = static_cast<uint32_t>(width * 4),
        .SysMemSlicePitch = 0
    };

    ID3D11Texture2D* dx_texture;
    device->CreateTexture2D(&desc, &subresource_data, &dx_texture);
    IDXGIResource* dxgi_resource;
    dx_texture->QueryInterface(__uuidof(IDXGIResource), (void**)&dxgi_resource);
    HANDLE shared_handle;
    dxgi_resource->GetSharedHandle(&shared_handle);
    dxgi_resource->Release();

    GL::GLContext* context = GL::create_offscreen_context(false);
    context->activate();

    auto func = context->get_func();

    GL::GLTexture* texture = new GL::GLTexture(shared_handle, width, height);

    GLuint id = texture->id();
    GLuint fbo;
    func->glGenFramebuffers(1, &fbo);
    func->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    func->glBindTexture(GL_TEXTURE_2D, id);
    func->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);
    func->glFinish();

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<uint8_t> v(height * width * 4);
    func->glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, v.data());

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << diff.count() << std::endl;

    stbi_write_png("text.png", width, height, 4, v.data(), 0);

    delete texture;

    context->release();
    GL::destroy_context(context);
    return 0;
}