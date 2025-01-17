#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

std::string vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    out float fCellState;
    void main() {
      gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
      fCellState = aPos.z;
    }
  )";
std::string fragmentShaderSource = R"(
    #version 330 core
    in float fCellState;
    out vec4 FragColor;

    void main() {
      float c = fCellState == 1.0 ? 1.0 : 0.0;

      FragColor = vec4(c, c, c, 1.0f);
    }
  )";

using std::cout, std::endl;
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

class ShaderProgram {
public:
  ShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
    m_id = glCreateProgram();
    glAttachShader(m_id, vertexShaderID);
    glAttachShader(m_id, fragmentShaderID);
    glLinkProgram(m_id);

    int success;
    char infoLog[512];
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(m_id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
                << infoLog << std::endl;
    }
  }
  ~ShaderProgram() { glDeleteProgram(m_id); }
  void use() { glUseProgram(m_id); }
  GLuint id() { return m_id; }

private:
  GLuint m_id{};
};

class Shader {
public:
  Shader(std::string shaderSource, GLenum type) {
    shader_type = type;
    m_id = glCreateShader(shader_type);
    const char *p = shaderSource.c_str();
    glShaderSource(m_id, 1, &p, NULL);
    glCompileShader(m_id);

    int success;
    char infoLog[512];
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(m_id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                << infoLog << std::endl;
    }
  }
  ~Shader() { glDeleteShader(m_id); }
  GLuint id() { return m_id; }

private:
  GLuint m_id{};
  GLenum shader_type{};
};

class VAO {
public:
  VAO() {
    glGenVertexArrays(1, &m_id);
    if (m_id == 0) {
      cout << "Failed to generate Vertex Array Object" << endl;
    }
  }
  ~VAO() { glDeleteVertexArrays(1, &m_id); }
  void bind() { glBindVertexArray(m_id); }
  void unbind() { glBindVertexArray(0); }
  void setAttribPointer(GLuint index, GLint size, GLenum type,
                        GLboolean normalized, GLsizei stride,
                        const GLvoid *pointer) {
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
  }
  GLuint id() { return m_id; }

private:
  GLuint m_id{};
};

class VBO {
public:
  VBO(std::vector<float> vertices) {
    glGenBuffers(1, &m_id);
    if (m_id == 0) {
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
  glViewport(0, 0, 800, 800);

  // Register the framebuffer size callback
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  static bool isPaused = true;
  static int speed = true;
  static float cellSize = 0.1;

  std::vector<float> vertices = {
      -0.5f, -0.5f, 1.0, //
      0.5f,  -0.5f, 1.0, //
      0.0f,  0.5f,  1.0  //
  };

  // std::vector<float> vertices = {};

  // for (int y = 0; y < 1.0 / cellSize; y++)
  //   for (int x = 0; x < 1.0 / cellSize; x++)
  //     vertices.push_back()

  VBO vbo(vertices);
  VAO vao;
  vbo.bind();
  vao.bind();

  Shader vertexShader(vertexShaderSource, GL_VERTEX_SHADER);
  Shader fragmentShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
  ShaderProgram shaderProgram(vertexShader.id(), fragmentShader.id());

  vao.setAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  shaderProgram.use();

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

    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);

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
