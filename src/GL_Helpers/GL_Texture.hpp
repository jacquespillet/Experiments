#pragma once

#include <GL/glew.h>
#include <stb_image.h>

struct TextureCreateInfo {
    GLint wrapS = GL_MIRRORED_REPEAT;
    GLint wrapT = GL_MIRRORED_REPEAT;
    GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLint magFilter = GL_LINEAR;
    bool generateMipmaps=true;
    bool srgb=false;
    bool flip=true;
};

class GL_Texture {
public:
    GL_Texture() : loaded(false), width(0), height(0), nChannels(0){}
    GL_Texture(GLuint glTex, bool loaded, int width, int height) : glTex(glTex), loaded(loaded), width(width), height(height){}
    
    GL_Texture(std::string filename, TextureCreateInfo createInfo) {
        this->filename = filename;
        
        glGenTextures(1, &glTex);
        glBindTexture(GL_TEXTURE_2D, glTex);

        if(createInfo.flip) stbi_set_flip_vertically_on_load(true);  
        data = stbi_load(filename.c_str(), &width, &height, &nChannels, 0);
        
        GLint texFormat = 0;
        GLint texFormatInternal = 0;
        if(createInfo.srgb)
        {
            if(nChannels == 4) {
                texFormat = GL_RGBA;
                texFormatInternal = GL_SRGB_ALPHA;
            } else if(nChannels == 3) {
                texFormat = GL_RGB;
                texFormatInternal = GL_SRGB;
            } else if(nChannels == 1) {
                texFormat = GL_RED;
                texFormatInternal = GL_RED;
            }            
        }
        else
        {
            if(nChannels == 4) {
                texFormat = GL_RGBA;
                texFormatInternal = GL_RGBA;
            } else if(nChannels == 3) {
                texFormat = GL_RGB;
                texFormatInternal = GL_RGB;
            } else if(nChannels == 1) {
                texFormat = GL_RED;
                texFormatInternal = GL_RED;
            }
        }

        std::cout << "Texture:Constructor: Num channels " << filename << "  " <<  nChannels << std::endl;
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, texFormatInternal, width, height, 0, texFormat, GL_UNSIGNED_BYTE, data);
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
    

        glBindTexture(GL_TEXTURE_2D, 0);
        loaded = true;        
    }

    void Unload()
    {
        glDeleteTextures(1, &glTex);
        stbi_image_free(data);
    }

    uint8_t *Data()
    {
        return data;
    }

    GLuint glTex;
    bool loaded=false;
    int width, height, nChannels;
    std::string filename;
    unsigned char *data;

};

class GL_Texture3D {
public:
    GLuint glTex;
    int size, nChannels;
};
