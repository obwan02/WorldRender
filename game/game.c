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

typedef struct App {
	GBuffer *buffer;
	GShader *shader;
	GRenderDevice *renderDevice;
} App;

void InitApp(App *app, GDevice* device) {
		

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
        GBufferDescription{
            .data = bufferContents,
            .size = sizeof(bufferContents),
        },
        &app->buffer);

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

    int res = device->create_shader_from_string(data, &app->shader);
    ASSERT(res == 0);
  }

void app_update(App *app) {
}

void app_render(App *app, GPresentParams presentParams) {
	app->renderDevice->present(presentParams);
}

