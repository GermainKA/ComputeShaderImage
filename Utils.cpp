
#include "Utils.h"

#include <fstream>
#include <sstream>


std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint createComputeProgram(const std::string& shaderPath) {
    // Load the shader source
    std::string shaderSource = loadShaderSource(shaderPath);
    if (shaderSource.empty()) {
        return 0; // Failed to load the shader
    }
    
    const char* source = shaderSource.c_str();
    
    // Create the compute shader
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &source, nullptr);
    glCompileShader(computeShader);
    
    // Check for compilation errors
    GLint success;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
        std::cerr << "Error compiling compute shader: " << infoLog << std::endl;
        glDeleteShader(computeShader);
        return 0;
    }
    
    // Create and link the program
    GLuint program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);
    
    // Check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error linking compute shader program: " << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }
    
    // Cleanup
    glDeleteShader(computeShader);
    return program;
}