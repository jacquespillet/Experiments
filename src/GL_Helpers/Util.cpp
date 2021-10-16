#include "Util.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/material.h>
#include <sstream>

GL_Mesh MeshFromFile(std::string filename, bool swapYZ, int subMeshIndex) {
    std::vector<uint32_t> triangles;
    std::vector<GL_Mesh::Vertex> vertices;   

    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_CalcTangentSpace ); 

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ModelLoader:LoaderModel: ERROR::" << import.GetErrorString() << std::endl;
        // return;
    }
    std::cout << "Num materials " << scene->mNumMaterials << std::endl;
    std::cout << "Num Meshes " << scene->mNumMeshes << std::endl;
    std::cout << "Num Textures " << scene->mNumTextures << std::endl;

    subMeshIndex = std::min(scene->mNumMeshes, (uint32_t)subMeshIndex);


    aiMesh *mesh = scene->mMeshes[subMeshIndex]; 
    std::cout << "NUM VERTICES " << mesh->mNumVertices << std::endl;
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        glm::vec3 tangent(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        glm::vec3 bitangent(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        
        if(swapYZ)
        {
            std::swap(pos.y, pos.z);
            std::swap(normal.y, normal.z);
        }

        glm::vec2 uv(0);
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            uv.x = mesh->mTextureCoords[0][i].x; 
            uv.y = mesh->mTextureCoords[0][i].y;
        }
        vertices.push_back({
            pos,
            normal,
            tangent,
            bitangent,
            uv
        });
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int k = 0; k < face.mNumIndices; k++)
            triangles.push_back(face.mIndices[k]);
    }

    GL_Mesh gl_mesh(vertices, triangles);
    // processNode(scene->mRootNode, scene, vertex, normals, uv, colors, triangles);
    
    return gl_mesh;
}


void MeshesFromFile(std::string filename, std::vector<GL_Mesh*>* OutMeshes, std::vector<GL_Material*>* OutMaterials)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(filename, aiProcess_GenNormals             |  aiProcess_FixInfacingNormals | 
                                                     aiProcess_Triangulate            | aiProcess_CalcTangentSpace    | 
                                                     aiProcess_JoinIdenticalVertices);

    std::size_t found = filename.find_last_of("/\\");
    std::string folderPath = filename.substr(0,found);
    std::replace( folderPath.begin(), folderPath.end(), '/', '\\'); // replace all 'x' to 'y'
    
    found = filename.find_last_of(".");
    std::string extension = filename.substr(found+1, filename.length());

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ModelLoader:LoaderModel: ERROR::" << import.GetErrorString() << std::endl;
        return;
    }
    
    for(unsigned int j = 0; j < scene->mNumMaterials; j++) {
        aiMaterial *material = scene->mMaterials[j];
        GL_Material *gl_material =  new GL_Material();
        
        int texFlags;
        if(AI_SUCCESS != material->Get(AI_MATKEY_TEXFLAGS(1,0),  texFlags)) {
            // handle epic failure here
        }        

        {
            aiString path;
            material->GetTexture (aiTextureType_DIFFUSE, 0, &path);
            std::stringstream diffuseTexFilename;
            diffuseTexFilename << folderPath << "\\" << path.C_Str();
            gl_material->LoadTexture(diffuseTexFilename.str(), GL_Material::TEXTURE_TYPE::DIFFUSE);
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_SPECULAR, 0, &path);
            std::stringstream specularTexFilename;
            specularTexFilename << folderPath << "\\" << path.C_Str();
            gl_material->LoadTexture(specularTexFilename.str(), GL_Material::TEXTURE_TYPE::SPECULAR);
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_OPACITY, 0, &path);
            std::stringstream opacityTexFilename;
            opacityTexFilename << folderPath << "\\" << path.C_Str();
            gl_material->LoadTexture(opacityTexFilename.str(), GL_Material::TEXTURE_TYPE::OPACITY);
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_NORMALS, 0, &path);
            std::stringstream normalTexFilename;
            normalTexFilename << folderPath << "\\" << path.C_Str();
            gl_material->LoadTexture(normalTexFilename.str(), GL_Material::TEXTURE_TYPE::NORMAL);
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_AMBIENT, 0, &path);
            std::stringstream ambientTexFilename;
            ambientTexFilename << folderPath << "\\" << path.C_Str();
            gl_material->LoadTexture(ambientTexFilename.str(), GL_Material::TEXTURE_TYPE::AMBIENT);
        }

        aiColor3D color;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        gl_material->diffuse = glm::vec3(color.r, color.g, color.b);

        material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        gl_material->specular = glm::vec3(color.r, color.g, color.b);

        material->Get(AI_MATKEY_COLOR_AMBIENT, color);
        gl_material->ambient = glm::vec3(color.r, color.g, color.b);

        material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
        gl_material->emissive = glm::vec3(color.r, color.g, color.b);
        
        material->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
        gl_material->transparent = glm::vec3(color.r, color.g, color.b);

        material->Get(AI_MATKEY_OPACITY, gl_material->opacity);
        material->Get(AI_MATKEY_SHININESS, gl_material->shininess);


        OutMaterials->push_back(gl_material);
    }

    for(unsigned int j = 0; j < scene->mNumMeshes; j++) {
        std::cout << "Start mesh" << std::endl;
        std::vector<unsigned int> triangles;
        std::vector<GL_Mesh::Vertex> vertices;   

        aiMesh *mesh = scene->mMeshes[j]; 
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			
			glm::vec3 tangent, bitangent;
			if(mesh->HasTangentsAndBitangents()) 
            {
                tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
            }

            glm::vec2 uv(0);
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                uv.x = mesh->mTextureCoords[0][i].x; 
                if(extension == "gltf") uv.y = 1-mesh->mTextureCoords[0][i].y;
                else uv.y = mesh->mTextureCoords[0][i].y;
            }
            vertices.push_back({
                pos,
                normal,
                tangent,
                bitangent,
                uv
            });
        }

        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int k = 0; k < face.mNumIndices; k++)
                triangles.push_back(face.mIndices[k]);
        }

        GL_Mesh *gl_mesh = new GL_Mesh(vertices, triangles);
        gl_mesh->material = (*OutMaterials)[mesh->mMaterialIndex];
  
        OutMeshes->push_back(gl_mesh);
    }
}