#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

using std::cout, std::endl;
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

class VBO {
public:
  VBO(std::vector<float> vertices) {
    glGenBuffers(1, &m_id);
    if (m_id != 0) {
      cout << "Failed to generate Vertex Buffer Object" << endl;
    }
    bind();
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);
  }
  ~VBO() { glDeleteBuffers(1, &m_id); }
  void bind() { glBindBuffer(GL_ARRAY_BUFFER, m_id); }
  void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
  GLuint id() { return m_id; }

private:
  GLuint m_id{};
};

int main() {
  // Initialize ImGui
  std::cout << "Initializing ImGui..." << std::endl;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  std::cout << "Initializing ImGui GLFW backend..." << std::endl;
  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    std::cerr << "Failed to initialize ImGui GLFW backend!" << std::endl;
    return -1;
  }

  std::cout << "Initializing ImGui OpenGL backend..." << std::endl;
  if (!ImGui_ImplOpenGL3_Init("#version 330")) {
    std::cerr << "Failed to initialize ImGui OpenGL backend!" << std::endl;
    return -1;
  }

  // Make the OpenGL context current
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = GL_TRUE; // Ensure GLEW uses modern OpenGL techniques
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  // Set the viewport
  glViewport(0, 0, 800, 600);

  // Register the framebuffer size callback
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  static bool isPaused = true;
  static int speed = true;
  static float cellSize = 0.1;

  // Main render loop
  while (!glfwWindowShouldClose(window)) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Settings");

    if (isPaused) {
      if (ImGui::Button("Play")) {
        isPaused = false;
      }
    } else {
      if (ImGui::Button("Pause")) {
        isPaused = true;
      }
    }

    ImGui::SameLine();
    ImGui::Text("Current State: %s", isPaused ? "Paused" : "Running");

    // Speed Slider
    ImGui::SliderInt("Speed", &speed, 0, 100, "Speed: %d%%");

    // Cell Size Slider
    ImGui::SliderFloat("Cell Size", &cellSize, 0.001f, 0.1f,
                       "Cell Size: %0.3f%");

    // Reset Button
    if (ImGui::Button("Reset")) {
      isPaused = true;
    }

    ImGui::End();

    // Render OpenGL
    glClearColor(0.2f, 0.4f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Process user input
    processInput(window);

    // Swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Clean up and exit
  glfwDestroyWindow(window);
  glfwTerminate();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  return 0;
}
