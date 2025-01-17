#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <vector>

std::string vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in float cellState;
    out float fCellState;
    void main() {
      gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
      fCellState = cellState;
    }
  )";
std::string fragmentShaderSource = R"(
    #version 330 core
    in float fCellState;
    out vec4 FragColor;

    void main() {
      float c = fCellState == 0.0 ? 1.0 : 0.0;

      FragColor = vec4(c, c, c, 1.0f);
    }
  )";

void generateVertices(std::vector<float> *vertices, float cellDivisor) {
  // Generate vertices
  float S = 1.0 / cellDivisor;
  for (int y = 0; y <= cellDivisor; y++) {
    for (int x = 0; x <= cellDivisor; x++) {
      float xPos = (x * S) * 2.0f - 1.0f;
      float yPos = -((y * S) * 2.0f - 1.0f);

      vertices->push_back(xPos);
      vertices->push_back(yPos);
    }
  }
}

void generateIndices(std::vector<unsigned int> *indices, float cellDivisor) {
  // Generate indices
  for (int y = 0; y < cellDivisor; y++) {
    for (int x = 0; x < cellDivisor; x++) {
      unsigned int topLeft = y * (cellDivisor + 1) + x;
      unsigned int topRight = topLeft + 1;
      unsigned int bottomLeft = (y + 1) * (cellDivisor + 1) + x;
      unsigned int bottomRight = bottomLeft + 1;

      indices->push_back(topLeft);
      indices->push_back(bottomLeft);
      indices->push_back(topRight);

      indices->push_back(bottomLeft);
      indices->push_back(bottomRight);
      indices->push_back(topRight);
    }
  }
}

using std::cout, std::endl;
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

class EBO {
public:
  EBO(std::vector<unsigned int> *indices) {
    glGenBuffers(1, &m_id);
    if (m_id == 0) {
      cout << "Failed to generate Element Buffer Object" << endl;
    }
    bind();
    updateData(indices);
  }
  void updateData(std::vector<unsigned int> *indices) {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(float),
                 indices->data(), GL_STATIC_DRAW);
  }
  ~EBO() { glDeleteBuffers(1, &m_id); }
  void bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id); }
  void unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
  GLuint id() { return m_id; }

private:
  GLuint m_id{};
};

class VBO {
public:
  VBO(std::vector<float> *vertices) {
    glGenBuffers(1, &m_id);
    if (m_id == 0) {
      cout << "Failed to generate Vertex Buffer Object" << endl;
    }
    bind();
    updateData(vertices);
  }
  ~VBO() { glDeleteBuffers(1, &m_id); }
  void updateData(std::vector<float> *vertices) {
    bind();
    glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(float),
                 vertices->data(), GL_STATIC_DRAW);
  }
  void bind() { glBindBuffer(GL_ARRAY_BUFFER, m_id); }
  void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
  GLuint id() { return m_id; }

private:
  GLuint m_id{};
};

int main() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::bernoulli_distribution d(0.5);

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
  static int fps = 1;
  static int cellDivisor = 10;

  std::vector<float> vertices = {};
  std::vector<float> cellStates = {};
  std::vector<unsigned int> indices = {};

  generateVertices(&vertices, cellDivisor);
  generateIndices(&indices, cellDivisor);

  for (int i = 0; i < 9 * 9 * 2; i++) {
    cout << i << endl;
    cellStates.push_back(0.0);
  }

  VBO vboVertices(&vertices);
  VBO vboCellStates(&cellStates);
  EBO ebo(&indices);
  VAO vao;

  vao.bind();

  Shader vertexShader(vertexShaderSource, GL_VERTEX_SHADER);
  Shader fragmentShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
  ShaderProgram shaderProgram(vertexShader.id(), fragmentShader.id());

  vboVertices.bind();
  vao.setAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  vboCellStates.bind();
  vao.setAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);

  shaderProgram.use();

  double timeA = glfwGetTime();

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
    ImGui::SliderInt("FPS", &fps, 0, 60, "FPS: %d");

    // Cell Size Slider
    ImGui::SliderInt("Cell Size", &cellDivisor, 2, 128, "Cell Divisor: %d");
    if (ImGui::IsItemEdited()) {
      vertices.clear();
      indices.clear();
      generateVertices(&vertices, cellDivisor);
      generateIndices(&indices, cellDivisor);
      vboVertices.updateData(&vertices);
      ebo.updateData(&indices);
    }

    // Reset Button
    if (ImGui::Button("Reset")) {
      isPaused = true;
    }

    ImGui::End();

    // Render OpenGL
    glClearColor(0.2f, 0.4f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Update according to set FPS
    double timeB = glfwGetTime();
    if (timeB - timeA >= 1.0 / fps) {
      timeA = timeB;
      cellStates.clear();
      for (int i = 0; i < 9 * 9 * 2; i++) {
        bool randomState = d(gen) ? 1.0 : 0.0;
        cellStates.push_back(randomState);
        cellStates.push_back(randomState);
        cellStates.push_back(randomState);
      }
      vboCellStates.updateData(&cellStates);
    }

    // Draw Cells
    ebo.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

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
