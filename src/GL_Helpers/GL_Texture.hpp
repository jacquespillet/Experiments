#pragma once

#include <GL/glew.h>
#include <stb_image.h>

struct TextureCreateInfo {
    GLint wrapS = GL_MIRRORED_REPEAT;
    GLint wrapT = GL_MIRRORED_REPEAT;
    GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLint magFilter = GL_LINEAR;
    bool generateMipmaps=true;
};

class GL_Texture {
public:
    GL_Texture() : loaded(false), width(0), height(0), nChannels(0){}
    GL_Texture(GLuint glTex, bool loaded, int width, int height) : glTex(glTex), loaded(loaded), width(width), height(height){}
    
    GL_Texture(std::string filename, TextureCreateInfo createInfo) {
        this->filename = filename;
        
        glGenTextures(1, &glTex);
        glBindTexture(GL_TEXTURE_2D, glTex);

        
        stbi_set_flip_vertically_on_load(true);  
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nChannels, 0);
        
        GLint texType = 0;
        if(nChannels == 4) {
            texType = GL_RGBA;
        } else if(nChannels == 3) {
            texType = GL_RGB;
        } else if(nChannels == 1) {
            texType = GL_RED;
        }

        std::cout << "Texture:Constructor: Num channels " << filename << "  " <<  nChannels << std::endl;
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, texType, width, height, 0, texType, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cout << "Texture:Constructor: ERROR::Failed to load texture" << std::endl;
            return;
        } 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, createInfo.wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, createInfo.wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, createInfo.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, createInfo.magFilter);
        if(createInfo.generateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
    

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);
        loaded = true;        
    }

    void Unload()
    {
        glDeleteTextures(1, &glTex);
    }

    GLuint glTex;
    bool loaded=false;
    int width, height, nChannels;
    std::string filename;

};

class GL_Texture3D {
public:
    GLuint glTex;
    int size, nChannels;
};
