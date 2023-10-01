/* Creation Date: 6/7/2023
 * Creator: obwan02
 * ========================
 *
 * This file contains the main
 * game/shared logic for the game
 * that isn't platform dependent
 *
 */
#include "core.h"
#include "graphics/api.h"
#include "util/assert.h"

namespace wrld {

class Application {
public:
  Application(g::Device *device) : m_RenderDevice(device), buffer(g::Buffer::make_uninit()), shader(g::Shader::make_uninit()) {

    static float bufferContents[] = {
        -0.5f,
        -0.5f,
        0.0f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        0.5f,
        0.0f,
    };

    device->create_buffer(
        g::BufferDescription{
            .data = bufferContents,
            .size = sizeof(bufferContents),
        },
        &this->buffer);

    // TODO: Make device dependent
    const char *data = "\
#include <metal_stdlib>\
using namespace metal;\
\
vertex float4\
vertexShader(uint vertexID [[vertex_id]],\
             constant simd::float3* vertexPositions)\
{\
    float4 vertexOutPositions = float4(vertexPositions[vertexID][0],\
                                       vertexPositions[vertexID][1],\
                                       vertexPositions[vertexID][2],\
                                       1.0f);\
    return vertexOutPositions;\
}\
\
fragment float4 fragmentShader(float4 vertexOutPositions [[stage_in]]) {\
    return float4(182.0f/255.0f, 240.0f/255.0f, 228.0f/255.0f, 1.0f);\
}";

    int res = device->create_shader_from_string(data, &this->shader);
    ASSERT(res == 0);
  }

  void update() {
  }

  void render(g::PresentParams presentParams) {
    m_RenderDevice->present(presentParams);
  }

private:
  g::Device *m_RenderDevice;
  g::Buffer buffer;
  g::Shader shader;
};

} // namespace wrld
