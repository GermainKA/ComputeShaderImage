#pragma once 
#include <string>
#include <iostream>
#include <GL/glew.h>

std::string loadShaderSource(const std::string& filepath);
GLuint createComputeProgram(const std::string& shaderPath);