#pragma once

  
#include "GL_Shader.hpp"
#include "GL_Texture.hpp"
#include <glm/mat4x4.hpp>
#include <GL/glew.h>

class GL_Material {
public:
    enum TEXTURE_TYPE {
		DIFFUSE,
		SPECULAR,
		OPACITY,
		HEIGHT,
        NORMAL,
        AMBIENT,
		NUM_TEXTURES
	};

    GL_Material() {
    }

    void BindShader(GLuint _shader)
    {
        this->shader = _shader;
    }

    void UnbindShader()
    {
        this->shader = (GLuint) -1;
    }

    bool SetShaderUniforms()
    {
        if(shader==(GLuint) -1) return false;
        
        glUseProgram(shader);

        if(diffuseTextureSet) SetTexture("diffuseTexture", diffuseTexture, TEXTURE_TYPE::DIFFUSE);
        if(specularTextureSet) SetTexture("specularTexture", specularTexture, TEXTURE_TYPE::SPECULAR);
        if(opacityTextureSet) SetTexture("opacityTexture", opacityTexture, TEXTURE_TYPE::OPACITY);
        if(ambientTextureSet) SetTexture("ambientTexture", ambientTexture, TEXTURE_TYPE::AMBIENT);
        if(normalTextureSet) SetTexture("normalTexture", normalTexture, TEXTURE_TYPE::NORMAL);
        if(heightTextureSet) SetTexture("heightTexture", heightTexture, TEXTURE_TYPE::HEIGHT);
        
        SetInt("diffuseTextureSet", (int)diffuseTextureSet);
        SetInt("specularTextureSet", (int)specularTextureSet);
        SetInt("opacityTextureSet", (int)opacityTextureSet);
        SetInt("ambientTextureSet", (int)ambientTextureSet);
        SetInt("normalTextureSet", (int)normalTextureSet);     
        SetInt("heightTextureSet", (int)heightTextureSet);     
        
        SetVec3("mat_ambient", ambient);
        SetVec3("mat_diffuse", diffuse);
        SetVec3("mat_specular", specular);
        SetVec3("mat_emissive", emissive);
        SetVec3("mat_transparent", transparent);
        SetFloat("mat_shininess", shininess);
        SetFloat("mat_opacity", opacity);
        SetFloat("mat_roughness", roughness);
        SetFloat("mat_metallic", metallic);


        return true;
    }
    

    GLuint shader;
    void SetMat4(std::string varName, glm::mat4 mat);
    void SetInt(std::string varName, int val);
    void SetFloat(std::string varName, float val);
    void SetVec4(std::string varName, glm::vec4 vec);
    void SetVec3(std::string varName, glm::vec3 vec);
    void SetTexture(std::string varName, const GL_Texture& texture, int numBind);

    void LoadTexture(std::string filename, TEXTURE_TYPE type, bool flip=false)
    {
        TextureCreateInfo info = {};
        switch (type)
        {
        case DIFFUSE:
            info = {};
            info.flip = flip;
            info.srgb=true;
            diffuseTexture = GL_Texture(filename, info);
            diffuseTextureSet=true;
            break;
        case SPECULAR:
            info = {};
            info.flip = flip;
            specularTexture = GL_Texture(filename, info);
            specularTextureSet=true;
            break;
        case HEIGHT:
            heightTexture = GL_Texture(filename, info);
            heightTextureSet=true;
            break;
        case OPACITY:
            info = {};
            info.flip = flip;
            opacityTexture = GL_Texture(filename, info);
            opacityTextureSet=true;
            break;
        case AMBIENT:
            info = {};
            info.flip = flip;
            ambientTexture = GL_Texture(filename, info);
            ambientTextureSet=true;
            break;
        case NORMAL:
            info = {};
            info.flip = flip;
            normalTexture = GL_Texture(filename, info);
            normalTextureSet=true;
            break;
        default:
            break;
        }
    }
    
        
    void Unload() {
        glDeleteTextures(1, &diffuseTexture.glTex);
        glDeleteTextures(1, &specularTexture.glTex);
        glDeleteTextures(1, &opacityTexture.glTex);
        glDeleteTextures(1, &heightTexture.glTex);
        glDeleteTextures(1, &ambientTexture.glTex);
        glDeleteTextures(1, &normalTexture.glTex);
    }

    bool diffuseTextureSet=false;
    bool specularTextureSet=false;
    bool opacityTextureSet=false;
    bool heightTextureSet=false;
    bool ambientTextureSet=false;
    bool normalTextureSet=false;

    GL_Texture diffuseTexture;
	GL_Texture specularTexture;
	GL_Texture opacityTexture;
	GL_Texture heightTexture;
	GL_Texture ambientTexture;
	GL_Texture normalTexture;

    glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 emissive;
	glm::vec3 transparent;
	float shininess;
	float opacity;
    float roughness = 1.0f;
    float metallic = 0.0f;
};