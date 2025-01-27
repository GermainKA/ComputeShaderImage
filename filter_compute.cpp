#include <vector>
#include <fstream>
#include <chrono>

#include "app.h"
#include "program.h"
#include "image_io.h"
#include "Utils.h"
#include "uniforms.h"
struct ComputeBuffer : public App
{
    ComputeBuffer( ) : App(1280, 768, 4,3) {}
    
    int init( )
    {

        //std::string shaderPath = "TP/COMPUTE/gaussian_compute.glsl";
        std::string shaderPath = "TP/COMPUTE/DACSPASS5.glsl";

        // Load and compile the shader
        computeProgram = createComputeProgram(shaderPath);
        if (computeProgram == 0) {
            std::cerr << "Failed to create compute shader program." << std::endl;
            return -1;
        }
        
       // m_InputImage = read_image("TP/COMPUTE/DATA/SC.png");
        m_InputImage = read_image("TP/COMPUTE/DATA/renderpass4.png");
        m_ComputeImage = read_image("TP/COMPUTE/DATA/screen060.jpg");



        
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, m_InputImage.data());

        //Compute
        glGenTextures(1, &computeTexture);
        glBindTexture(GL_TEXTURE_2D, computeTexture);
        {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, m_ComputeImage.data());

        
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
        glBindImageTexture(2, computeTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
        // Activer le programme
        glUseProgram(computeProgram);

        GLint location= glGetUniformLocation(computeProgram, "inputImage");
        glUniform1i(location, 0);

        location= glGetUniformLocation(computeProgram, "outputImage");
        glUniform1i(location, 1); 

        // Définir le nombre de groupes de travail
        int groupX = (width + 7) / 8; // Arrondi supérieur
        int groupY = (height + 7) / 8;

        glDispatchCompute(groupX, groupY, 1);

        // Attendre que le calcul soit terminé
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        



        // Lire les résultats (par exemple, en téléchargeant `textureOutput` vers le CPU)
        glBindTexture(GL_TEXTURE_2D, textureOutput);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, m_OutputImage.data());
        
        

        write_image(m_OutputImage, "TP/COMPUTE/DATA/renderpass5.png");

        
        return 0;
    }

    Image m_InputImage ,m_OutputImage,m_ComputeImage;
    GLuint computeProgram;
    int width ,height;
    GLuint textureInput, textureOutput,computeTexture;
};


struct ComputeHistoCPUvsGPU : public App
{
    ComputeHistoCPUvsGPU(const char * filename ) : App(1280, 768, 4,3) {
        m_InputImage = read_image_data(filename);
    }
    
    int init( )
    {
        if(m_InputImage.pixels.size()==0)
            return -1;
        GLenum data_type = GL_UNSIGNED_BYTE;
        GLenum data_format = GL_RGBA;
        if(m_InputImage.channels == 1)
            data_format = GL_RED;
        else if (m_InputImage.channels == 2)
            data_format = GL_RG;
        else if (m_InputImage.channels == 3)
            data_format = GL_RGB;

        

        //std::string shaderPath = "TP/COMPUTE/gaussian_compute.glsl";
        // Load and compile the shader
        computeProgram = read_program("TP/COMPUTE/HISTO.glsl");
        //computeProgram= read_program("tutos/M2/histogram.glsl");
        program_print_errors(computeProgram);
        glGetProgramiv(computeProgram, GL_COMPUTE_WORK_GROUP_SIZE, m_threads);


        width = m_InputImage.width;
        height = m_InputImage.height;

        glGenTextures(1,&textureInput);
        glBindTexture(GL_TEXTURE_2D,textureInput);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,data_format,data_type,m_InputImage.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glGenBuffers(1, &m_Bufferhistogram);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Bufferhistogram);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 16, nullptr, GL_STATIC_COPY);

        // mesure temps gpu
        glGenQueries(1, &m_TimeQuery);
        
        // histogramme cpu pour comparer...
        {
            auto start= std::chrono::high_resolution_clock::now();
            
            int histogram[16]= { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 };
            for(int y= 0; y < height; y++)
            for(int x= 0; x < width; x++)
            {
                size_t offset= m_InputImage.offset(x, y);
            #if 0
                float r= float(m_InputImage.pixels[offset]) / 255;
                float g= float(m_InputImage.pixels[offset+1]) / 255;
                float b= float(m_InputImage.pixels[offset+2]) / 255;
                float grey= (r+g+b) / 3;
                int bin= int(15 * grey);
            #else
                // nettement plus rapide ...
                int r= m_InputImage.pixels[offset];
                int g= m_InputImage.pixels[offset+1];
                int b= m_InputImage.pixels[offset+2];
                int bin= 15 * (r+g+b) / (255*3);                
            #endif
                histogram[bin]++;
            }
            
            auto stop= std::chrono::high_resolution_clock::now();
            int cpu_time= std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
            
            for(int i= 0; i < 16; i++)
                printf("bin %d: %d pixels, %d%%\n", i, histogram[i], 100*histogram[i] / (width * height));
            
            printf("cpu %dus\n", cpu_time);
        }

       

        
        return 0;
    }
    
    int quit( )
    {
        //release_program(computeProgram);
        
        return 0;
    }
    
    int render( )
    {
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(computeProgram);
        
        // storage buffer 0
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_Bufferhistogram);
        // remet a zero l'histogramme
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
        
        // image texture 0, ecriture seule, mipmap 0 + format rgba8 classique
        glBindImageTexture(0, textureInput, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
        // configurer le shader
        program_uniform(computeProgram, "image", 0);

        int nx= width / m_threads[0] / 4;
        int ny= height / m_threads[1] / 4;
        // on suppose que les dimensions de l'image sont multiples de 8*4...
        // sinon calculer correctement le nombre de groupes pour x et y. 
        
        glBeginQuery(GL_TIME_ELAPSED, m_TimeQuery);
        {
            // go !!
            //~ for(int i= 0; i < 100; i++)     // eventuellement recommencer plein de fois pour stabiliser les frequences du gpu...
            glDispatchCompute(nx, ny, 1);
            
            // attendre le resultat
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        }
        glEndQuery(GL_TIME_ELAPSED);
        
        // relire le buffer resultat
        int histogram[16];
    #if 1
        {
            // creer un buffer temporaire pour le transfert depuis le buffer resultat
            GLuint buffer= 0;
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_COPY_READ_BUFFER, buffer);
            glBufferData(GL_COPY_READ_BUFFER, sizeof(int) * 16, nullptr, GL_DYNAMIC_READ);
            
            // copie les resultats
            glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_COPY_READ_BUFFER, 0, 0, sizeof(int) * 16);
            
            // recupere les resultats depuis le buffer intermediaire
            glGetBufferSubData(GL_COPY_READ_BUFFER, 0, sizeof(int) * 16, histogram);
            
            // detruit le buffer intermediaire
            glDeleteBuffers(1, &buffer);
        }
    #else
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * 16, histogram);
    #endif
        
        for(int i= 0; i < 16; i++)
            printf("bin %d: %d pixels, %d%%\n", i, histogram[i], 100*histogram[i] / (width * height));
        
        printf("\n%dx%d groups, %d threads\n", nx, ny, nx*ny*m_threads[0]*m_threads[1]);
        // attendre le resultat de la requete
        GLint64 gpu_time= 0;
        glGetQueryObjecti64v(m_TimeQuery, GL_QUERY_RESULT, &gpu_time);
        //~ gpu_time/= 100;
        printf("gpu  %02dms %03dus\n\n", int(gpu_time / 1000000), int((gpu_time / 1000) % 1000)); 

        
        return 0;
    }

    ImageData m_InputImage;
    GLuint computeProgram;
    GLuint m_Bufferhistogram;
    GLuint m_TimeQuery;
    int width ,height;
    GLuint textureInput;
    GLint m_threads[3];
};

int main( )
{
    //ComputeBuffer app;
    const char * filename = "TP/COMPUTE/DATA/screen060.jpg";
    
    ComputeBuffer app;
    app.run();
    
    return 0;
}
