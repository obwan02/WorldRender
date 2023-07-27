/* Creation Date: 23/6/2023
 * Creator: obwan02
 * ========================
 *
 * TODO: Description
 *
 */
#pragma once

#include "Metal/MTLBuffer.hpp"
#include "Metal/MTLCommandQueue.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLLibrary.hpp"
#include "MetalKit/MTKView.hpp"
#include "api.h"
#include <Metal/Metal.hpp>

namespace wrld {
namespace g {

struct PresentParams {
  // The view used to present our images to
  MTK::View *_metalView;
};

struct BarrierDescription;
struct BufferDescription {
  size_t size;
  const void *data;
};

class Buffer {
private:
  explicit Buffer(){};
  Buffer(MTL::Buffer *buffer) : _metalBuffer(buffer){};

  friend class Device;

public:
  ~Buffer();
  // Common functionality, should
  // be implemeted for all platforms, and
  // can generically be relied on
  uint32_t stride();
  uint32_t size();
  uint32_t num_elems();
  void *get_data();

  static Buffer make_uninit();

  MTL::Buffer *_metalBuffer;
};

class Shader {
private:
  explicit Shader() {}
  Shader(MTL::Library *shader) : _metalLibrary(shader){};

  friend class Device;

public:
  ~Shader();
  static Shader make_uninit();

  MTL::Library *_metalLibrary;
};

class Pipeline {};

class Context {
public:
  void begin();
  void end();
  void resource_barrier(BarrierDescription);
};

class GraphicsContext : public Context {
private:
  explicit GraphicsContext() {}
  GraphicsContext(MTL::CommandQueue *queue) : _metalCmdQueue(queue){};

public:
  ~GraphicsContext();
  static GraphicsContext make_uninit();

  void set_vertex_buffer(Buffer *);
  void set_index_buffer(Buffer *);
  void draw();

  MTL::CommandQueue *_metalCmdQueue;
};

class ComputeContext : public Context {
public:
  void set_pipeline(Pipeline *);
  void dispatch();
};

class UploadContext : public Context {
public:
  void upload_buffer(Buffer *);
  void upload_texture(Buffer *);
};

class Device {
public:
  Device(const Device &) = delete;
  explicit Device();
  ~Device();

  void create_graphics_context(GraphicsContext *out);
  void create_buffer(BufferDescription, Buffer *out);
  int create_shader_from_string(const char *, Shader *out);
  void present(PresentParams);

  MTL::Device *_metalDevice;
};

} // namespace g
} // namespace wrld
