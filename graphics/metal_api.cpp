#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "../util/log.h"
#include "Foundation/NSError.hpp"
#include "Foundation/NSString.hpp"
#include "Metal/MTLLibrary.hpp"
#include "api.h"
#include "metal_api.h"

#ifndef WRLD_METAL
#error WRLD_METAL should be defined when compiling \
for the Metal API
#endif

namespace wrld {
namespace g {

Device::Device() {
  this->_metalDevice = MTL::CreateSystemDefaultDevice();
}

Device::~Device() { this->_metalDevice->release(); }

void Device::create_buffer(BufferDescription desc, Buffer *out) {

  MTL::Buffer *buffer{nullptr};
  if (desc.data != nullptr) {
    buffer = this->_metalDevice->newBuffer(desc.data, desc.size,
                                           MTL::ResourceStorageModeShared);
  } else {
    buffer =
        this->_metalDevice->newBuffer(desc.size, MTL::ResourceStorageModeShared);
  }

  *out = Buffer(buffer);
}

int Device::create_shader_from_string(const char *source, Shader *out) {
  NS::String *metalSource = NS::String::string(source, NS::UTF8StringEncoding);
  MTL::CompileOptions *opts = MTL::CompileOptions::alloc()->init();
  NS::Error *error = nullptr;

  MTL::Library *lib = this->_metalDevice->newLibrary(metalSource, opts, &error);

  if (error != nullptr) {
    log::logln("Error occured:"_S);
    log::logln(String::from_cstr(error->description()->utf8String()));
    return -1;
  }

  *out = Shader(lib);

  metalSource->release();
  opts->release();
  return 0;
}

void Device::present(PresentParams params) {
  // TODO: Fill out
}

Buffer Buffer::make_uninit() {
  return Buffer{};
}

Buffer::~Buffer() {
  // For some reason this crashes?
  this->_metalBuffer->autorelease();
}

void *Buffer::get_data() {
  return this->_metalBuffer->contents();
}

Shader Shader::make_uninit() {
  return Shader{};
}

Shader::~Shader() {
  this->_metalLibrary->autorelease();
}

GraphicsContext GraphicsContext::make_uninit() {
  return GraphicsContext{};
}

GraphicsContext::~GraphicsContext() {
  this->_metalCmdQueue->autorelease();
}

void GraphicsContext::set_vertex_buffer(Buffer *buffer) {
}

void GraphicsContext::draw() {
  /* MTL::Function *vertexShader = metalDefaultLibrary->newFunction(NS::String::string("vertexShader", NS::ASCIIStringEncoding)); */
  /* assert(vertexShader); */
  /* MTL::Function *fragmentShader = metalDefaultLibrary->newFunction(NS::String::string("fragmentShader", NS::ASCIIStringEncoding)); */
  /* assert(fragmentShader); */

  /* MTL::RenderPipelineDescriptor *renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init(); */
  /* renderPipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline", NS::ASCIIStringEncoding)); */
  /* renderPipelineDescriptor->setVertexFunction(vertexShader); */
  /* renderPipelineDescriptor->setFragmentFunction(fragmentShader); */
  /* assert(renderPipelineDescriptor); */
  /* MTL::PixelFormat pixelFormat = (MTL::PixelFormat)metalLayer.pixelFormat; */
  /* renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat); */

  /* NS::Error *error; */
  /* metalRenderPSO = metalDevice->newRenderPipelineState(renderPipelineDescriptor, &error); */

  /* renderPipelineDescriptor->release(); */
}

} // namespace g
} // namespace wrld
