/* Creation Date: 18/6/2023
 * Creator: obwan02
 * ========================
 *
 * This file should be the main compilation point
 * for platforms that do not have any platform code
 * written for them. As such, this file tries to handle
 * all systems in the most generic way possible, and will
 * not use any platform specific knowledge, such as SIMD
 * intrinsics, or specific platform APIs.
 *
 * NOTE: Currently this is the main compilation
 * unit for all files
 *
 */
#include <GLFW/glfw3.h>
#include <stdio.h>

int main(int argc, const char *argv[]) {
  if (!glfwInit()) {
    puts("Failed to initialise GLFW. Exiting ...");
    return -1;
  }

  GLFWwindow *window =
      glfwCreateWindow(680, 420, "A World-Scale Renderer", nullptr, nullptr);

  glfwMakeContextCurrent(window);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  return 0;
}
