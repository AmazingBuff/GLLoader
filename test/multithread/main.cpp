#include <GLContext.h>
#include <GLFunctions.h>
#include <GLTexture.h>

#include <thread>
#include <future>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <queue.h>

struct Texture
{
    int width;
    int height;
    int channels;
    GLuint id;
    // in wgl, texture must sync manually, while egl doesn't
    GLsync sync;
};

Queue<Texture> texture_queue;

static constexpr size_t Queue_Max_Size = 2;
static constexpr size_t Loop_Max_Count = 100;

std::atomic<bool> produce_initialize = false;
std::atomic<bool> consume_initialize = false;
std::atomic<bool> consume_stop = false;

void produce()
{
    GL::GLContext* context = GL::create_offscreen_context();
    produce_initialize = true;

    while (!consume_initialize) {}

    // wgl must wait all shared context initialized, while egl doesn't
    context->activate();

    auto func = context->get_func();

    int width, height, channels;
    uint8_t* image = stbi_load(ASSETS_DIR"a.png", &width, &height, &channels, STBI_rgb_alpha);

    size_t loop_count = 0;
    while (true)
    {
        if (loop_count >= Loop_Max_Count)
            break;

        if (texture_queue.size() < Queue_Max_Size)
        {
            GLuint v;
            func->glGenTextures(1, &v);
            func->glBindTexture(GL_TEXTURE_2D, v);
            func->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            func->glBindTexture(GL_TEXTURE_2D, 0);

            GLsync sync = func->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            func->glFlush();

            Texture texture{width, height, 4, v, sync};
            texture_queue.enqueue(texture);

            loop_count++;
        }
    }

    stbi_image_free(image);
    context->release();

    while (!consume_stop) {}
    GL::destroy_context(context);
}

void consume()
{
    while (!produce_initialize) {}

    GL::create_offscreen_context();

    consume_initialize = true;

    auto context = GL::GLContext::current_context();

    std::cout << "is opengles: " << context->is_opengl_es() << std::endl;

    auto func = context->get_func();

    context->activate();

    auto start = std::chrono::high_resolution_clock::now();

    GLuint fbo;
    func->glGenFramebuffers(1, &fbo);
    func->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    size_t loop_count = 0;
    while (true)
    {
        if (loop_count >= Loop_Max_Count)
            break;

        if (!texture_queue.empty())
        {
            Texture texture = texture_queue.dequeue();

            func->glWaitSync(texture.sync, 0, GL_TIMEOUT_IGNORED);
            func->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

            //std::vector<uint8_t> v(texture.height * texture.width * texture.channels);
            //func->glReadPixels(0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, v.data());

            //stbi_write_png((std::to_string(loop_count) + ".png").c_str(), texture.width, texture.height, texture.channels, v.data(), 0);

            loop_count++;
        }
    }

    func->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    func->glDeleteFramebuffers(1, &fbo);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << diff.count() << " seconds" << std::endl;

    context->release();
    GL::destroy_context(context);

    consume_stop = true;
}

int main()
{
    std::thread t1(produce);
    std::thread t2(consume);
    t1.join();
    t2.join();

    return 0;
}