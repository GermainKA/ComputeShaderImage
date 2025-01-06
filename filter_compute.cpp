#include <vector>
#include <fstream>

#include "app.h"
#include "program.h"
#include "image_io.h"
#include "Utils.h"

struct ComputeBuffer : public App
{
    ComputeBuffer( ) : App(1280, 768, 4,3) {}
    
    int init( )
    {

        //std::string shaderPath = "TP/COMPUTE/gaussian_compute.glsl";
        std::string shaderPath = "TP/COMPUTE/sobel_compute.glsl";

        // Load and compile the shader
        computeProgram = createComputeProgram(shaderPath);
        if (computeProgram == 0) {
            std::cerr << "Failed to create compute shader program." << std::endl;
            return -1;
        }
        
       // m_InputImage = read_image("TP/COMPUTE/DATA/SC.png");
        m_InputImage = read_image("TP/COMPUTE/DATA/screen060.jpg");



        
        width = m_InputImage.width();
        height = m_InputImage.height();
        m_OutputImage = Image(width,height);

        //source
        glGenTextures(1, &textureInput);
        glBindTexture(GL_TEXTURE_2D, textureInput);
        {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, m_InputImage.data());

        //cible
        glGenTextures(1, &textureOutput);
        glBindTexture(GL_TEXTURE_2D, textureOutput);
        {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

        
        return 0;
    }
    
    int quit( )
    {
        //release_program(computeProgram);
        
        return 0;
    }
    
    int render( )
    {
        //  binding point 
        glBindImageTexture(0, textureInput, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, textureOutput, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        // Activer le programme
        glUseProgram(computeProgram);

        GLint location= glGetUniformLocation(computeProgram, "inputImage");
        glUniform1i(location, 0);

        location= glGetUniformLocation(computeProgram, "outputImage");
        glUniform1i(location, 1); 

        // Définir le nombre de groupes de travail
        int groupX = (width + 15) / 16; // Arrondi supérieur
        int groupY = (height + 15) / 16;

        glDispatchCompute(groupX, groupY, 1);

        // Attendre que le calcul soit terminé
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Lire les résultats (par exemple, en téléchargeant `textureOutput` vers le CPU)
        glBindTexture(GL_TEXTURE_2D, textureOutput);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, m_OutputImage.data());
        
        

        write_image(m_OutputImage, "TP/COMPUTE/DATA/render.png");

        
        return 0;
    }

    Image m_InputImage ,m_OutputImage;
    GLuint computeProgram;
    int width ,height;
    GLuint textureInput, textureOutput;
};

int main( )
{
    ComputeBuffer app;
    app.run();
    
    return 0;
}
