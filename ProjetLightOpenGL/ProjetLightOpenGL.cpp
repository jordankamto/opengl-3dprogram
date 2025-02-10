#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
unsigned int compileShader(const char* vertexSource, const char* fragmentSource);

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Vertex shader source code
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main() {
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine results
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

// Variables globales
glm::vec3 cubePosition(0.0f, 0.0f, 0.0f); // Position du cube
glm::vec3 cubeColor(1.0f, 0.5f, 0.31f);  // Couleur du cube
float lightIntensity = 1.0f;             // Intensité de la lumière
int rotationLevel = 0;                   // Niveau de rotation (0 = arrêté, 1-5 = vitesses)
bool isRotating = false;                 // Indique si le cube est en rotation
std::vector<float> defaultVertices;      // Vertices par défaut du cube
std::vector<float> currentVertices;      // Vertices actuels du cube

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Rotating 3D Cube", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load all OpenGL function pointers using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Compile shaders
    unsigned int shaderProgram = compileShader(vertexShaderSource, fragmentShaderSource);

    // Cube vertices with normals
    defaultVertices = {
        // Positions          // Normals
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

         // Bottom face
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

         // Top face
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    // Copy default vertices to current vertices
    currentVertices = defaultVertices;

    // Set up VAO and VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, currentVertices.size() * sizeof(float), currentVertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Update vertices if modified
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, currentVertices.size() * sizeof(float), currentVertices.data(), GL_STATIC_DRAW);

        // Apply transformations
        glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePosition);

        // Apply rotation if enabled
        if (isRotating) {
            float rotationSpeed = rotationLevel * 0.5f; // Vitesse de rotation
            model = glm::rotate(model, (float)glfwGetTime() * rotationSpeed, glm::vec3(0.5f, 1.0f, 0.0f));
        }

        // Pass matrices to shader
        glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Pass light and object properties to shader
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 2.0f, 2.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 2.0f, 2.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), lightIntensity, lightIntensity, lightIntensity);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), cubeColor.r, cubeColor.g, cubeColor.b);

        // Draw the cube
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, currentVertices.size() / 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}

// Function to handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Function to process input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Translation verticale (haut/bas)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cubePosition.y += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cubePosition.y -= 0.01f;

    // Translation horizontale (gauche/droite)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cubePosition.x -= 0.01f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cubePosition.x += 0.01f;

    // Changer la couleur du cube
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cubeColor.r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        cubeColor.g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        cubeColor.b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    // Ajouter un vertex au cube
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        float newVertex[] = {
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f,
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f,
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f,
            0.0f, 0.0f, 0.0f // Normale par défaut
        };
        currentVertices.insert(currentVertices.end(), newVertex, newVertex + 6);
    }

    // Réinitialiser les vertices
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        currentVertices = defaultVertices;
    }

    // Ajuster l'intensité de la lumière
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) // Touche "+"
        lightIntensity += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) // Touche "-"
        lightIntensity -= 0.1f;

    // Gérer la rotation du cube
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (rotationLevel < 5) {
            rotationLevel++;
            isRotating = true;
        }
        else {
            rotationLevel = 0;
            isRotating = false;
        }
        // Attendre un peu pour éviter les multiples détections de la touche
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

// Function to compile shaders
unsigned int compileShader(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete shaders as they're linked into the program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}