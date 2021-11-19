#include "Util.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/material.h>
#include <sstream>
#include <fstream>

GL_Mesh *MeshFromFile(std::string filename, bool swapYZ, int subMeshIndex) {
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
		
		glm::vec3 tangent, bitangent;
		if (mesh->HasTangentsAndBitangents())
		{
			tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}
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

    GL_Mesh *gl_mesh = new GL_Mesh(vertices, triangles);
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
            if(path.length>0)
            {
                gl_material->LoadTexture(diffuseTexFilename.str(), GL_Material::TEXTURE_TYPE::DIFFUSE);
            }
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_SPECULAR, 0, &path);
            std::stringstream specularTexFilename;
            specularTexFilename << folderPath << "\\" << path.C_Str();
            if(path.length>0)
            {
                gl_material->LoadTexture(specularTexFilename.str(), GL_Material::TEXTURE_TYPE::SPECULAR);
            }
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_OPACITY, 0, &path);
            std::stringstream opacityTexFilename;
            opacityTexFilename << folderPath << "\\" << path.C_Str();
            if(path.length>0)
            {
                gl_material->LoadTexture(opacityTexFilename.str(), GL_Material::TEXTURE_TYPE::OPACITY);
            }
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_NORMALS, 0, &path);
            std::stringstream normalTexFilename;
            normalTexFilename << folderPath << "\\" << path.C_Str();
            if(path.length>0)
            {
                gl_material->LoadTexture(normalTexFilename.str(), GL_Material::TEXTURE_TYPE::NORMAL);
            }
        }

        {
            aiString path;
            material->GetTexture (aiTextureType_AMBIENT, 0, &path);
            std::stringstream ambientTexFilename;
            ambientTexFilename << folderPath << "\\" << path.C_Str();
            if(path.length>0)
            {
                gl_material->LoadTexture(ambientTexFilename.str(), GL_Material::TEXTURE_TYPE::AMBIENT);
            }
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

GL_Mesh *PlaneMesh(float sizeX, float sizeY, int subdivX, int subdivY)
{
    std::vector<unsigned int> triangles;
    std::vector<GL_Mesh::Vertex> vertices;   
    
    float xOffset = sizeX / (float)subdivX;
    float yOffset = sizeY / (float)subdivY;

    int numAdded=0;
    for(float y=-sizeX/2, yInx=0; yInx<subdivY; y+=yOffset, yInx++) {
        for(float x=-sizeY/2, xInx=0; xInx<subdivX; x+= xOffset, xInx++) {
            vertices.push_back({
                glm::vec3(x, 0, y),
                glm::vec3(0, 1, 0),
                glm::vec3(1, 0, 0),
                glm::vec3(0, 0, 1),
                glm::vec2(0,0)
            });

            if(xInx < subdivX-1 && yInx < subdivY-1) {
                triangles.push_back(numAdded);
                triangles.push_back(numAdded + subdivX + 1);
                triangles.push_back(numAdded + subdivX);
                
                triangles.push_back(numAdded);
                triangles.push_back(numAdded + 1);
                triangles.push_back(numAdded + subdivX + 1);
            }
            numAdded++;
        }
    }

    GL_Mesh *gl_mesh = new GL_Mesh(vertices, triangles);
    return gl_mesh;
}

void CreateComputeShader(std::string filename, GLint *programShaderObject)
{
    std::ifstream t(filename);
    std::stringstream shaderBuffer;
    shaderBuffer << t.rdbuf();
    std::string shaderSrc = shaderBuffer.str();
    GLint shaderObject;

    t.close();
	//make array to pointer for source code (needed for opengl )
	const char* vsrc[1];
	vsrc[0] = shaderSrc.c_str();
	
	//compile vertex and fragment shaders from source
	shaderObject = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shaderObject, 1, vsrc, NULL);
	glCompileShader(shaderObject);
	
	//link vertex and fragment shader to create shader program object
	*programShaderObject = glCreateProgram();
	glAttachShader(*programShaderObject, shaderObject);
	glLinkProgram(*programShaderObject);
	std::cout << "Shader:Compile: checking shader status" << std::endl;
	
	//Check status of shader and log any compile time errors
	int linkStatus;
	glGetProgramiv(*programShaderObject, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) 
	{
		char log[5000];
		int lglen; 
		glGetProgramInfoLog(*programShaderObject, 5000, &lglen, log);
		std::cerr << "Shader:Compile: Could not link program: " << std::endl;
		std::cerr << log << std::endl;
		
        glGetShaderInfoLog(shaderObject, 5000, &lglen, log);
		std::cerr << "vertex shader log:\n" << log << std::endl;
		
        glDeleteProgram(*programShaderObject);
		*programShaderObject = 0;
	}
	else
	{
		std::cout << "Shader:Compile: compile success " << std::endl;
	}
}

void CreateComputeShader(const std::vector<std::string>& filenames, GLint *programShaderObject)
{
    std::stringstream shaderBuffer;
    shaderBuffer << "#version 430 core \n";
    for(int i=0; i<filenames.size(); i++)
    {
        std::ifstream t(filenames[i]);
        shaderBuffer << t.rdbuf();
        t.close();

        shaderBuffer << "\n";
    }
    std::string shaderSrc = shaderBuffer.str();
    GLint shaderObject;

	//make array to pointer for source code (needed for opengl )
	const char* vsrc[1];
	vsrc[0] = shaderSrc.c_str();
	
	//compile vertex and fragment shaders from source
	shaderObject = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shaderObject, 1, vsrc, NULL);
	glCompileShader(shaderObject);
	
	//link vertex and fragment shader to create shader program object
	*programShaderObject = glCreateProgram();
	glAttachShader(*programShaderObject, shaderObject);
	glLinkProgram(*programShaderObject);
	std::cout << "Shader:Compile: checking shader status" << std::endl;
	
	//Check status of shader and log any compile time errors
	int linkStatus;
	glGetProgramiv(*programShaderObject, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) 
	{
		char log[5000];
		int lglen; 
		glGetProgramInfoLog(*programShaderObject, 5000, &lglen, log);
		std::cerr << "Shader:Compile: Could not link program: " << std::endl;
		std::cerr << log << std::endl;
		
        glGetShaderInfoLog(shaderObject, 5000, &lglen, log);
		std::cerr << "vertex shader log:\n" << log << std::endl;
		
        glDeleteProgram(*programShaderObject);
		*programShaderObject = 0;
	}
	else
	{
		std::cout << "Shader:Compile: compile success " << std::endl;
	}
}


void BindSSBO(GLuint shader, GLuint ssbo, std::string name, GLuint bindingPoint)
{
	glUseProgram(shader);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, ssbo);
}

float Clamp01(float input)
{
	float Result = std::min(1.0f, std::max(0.001f, input));
	return Result;
}

float Clamp(float input, float min, float max)
{
	float Result = std::min(max, std::max(min, input));
	return Result;
}


float Lerp(float a, float b, float f)
{
    return a + f * (b - a);
}  

float RandomFloat(float min, float max)
{
	float Result = (float)std::rand() / (float)RAND_MAX;
	float range = max - min;
	Result *= range;
	Result += min;
	return Result;
}

int RandomInt(int min, int max)
{
	float ResultF = RandomFloat((float)min, (float)max);
	int Result = (int)ResultF;
	return Result;
}

glm::vec3 RandomVec3(glm::vec3 min, glm::vec3 max)
{
    glm::vec3 Result = {
        RandomFloat(min.x, max.x),
        RandomFloat(min.y, max.y),
        RandomFloat(min.z, max.z)
    };
    return Result;
}

bool RayTriangleIntersection( 
    glm::vec3 &orig, glm::vec3 &dir, 
    glm::vec3 &v0,glm::vec3 &v1,glm::vec3 &v2,
    float *t, glm::vec2*uv) 
{ 
    float kEpsilon = 1e-8f; 
    float u, v;

    glm::vec3 v0v1 = v1 - v0; 
    glm::vec3 v0v2 = v2 - v0; 
    glm::vec3 pvec = glm::cross(dir, v0v2);
    float det = glm::dot(v0v1, pvec);

    // ray and triangle are parallel if det is close to 0
    if (std::abs(det) < kEpsilon) return false; 

    float invDet = 1 / det; 
   
    glm::vec3 tvec = orig - v0; 
    u = glm::dot(tvec, pvec) * invDet; 
    if (u < 0 || u > 1) return false; 

    glm::vec3 qvec = glm::cross(tvec, v0v1); 
    v = glm::dot(dir, qvec) * invDet; 
    if (v < 0 || u + v > 1) return false; 

    *t = glm::dot(v0v2, qvec) * invDet; 
    *uv = glm::vec2(u, v);

    // return true;
	if (*t > 0) return true;
	else return false; 
} 

bool RayAABBIntersection(glm::vec3 &orig, glm::vec3 &invDir, int *sign, AABB &aabb)
{
    float tmin, tmax, tymin, tymax, tzmin, tzmax; 
 
    tmin = (aabb.bounds[sign[0]].x - orig.x) * invDir.x; 
    tmax = (aabb.bounds[1-sign[0]].x - orig.x) * invDir.x; 
    tymin = (aabb.bounds[sign[1]].y - orig.y) * invDir.y; 
    tymax = (aabb.bounds[1-sign[1]].y - orig.y) * invDir.y; 
 
    if ((tmin > tymax) || (tymin > tmax)) 
        return false; 
    if (tymin > tmin) 
        tmin = tymin; 
    if (tymax < tmax) 
        tmax = tymax; 
 
    tzmin = (aabb.bounds[sign[2]].z - orig.z) * invDir.z; 
    tzmax = (aabb.bounds[1-sign[2]].z - orig.z) * invDir.z; 
 
    if ((tmin > tzmax) || (tzmin > tmax)) 
        return false; 
    if (tzmin > tmin) 
        tmin = tzmin; 
    if (tzmax < tmax) 
        tmax = tzmax; 
 
    return true;
}


void CalculateTangents(std::vector<GL_Mesh::Vertex>& vertices, std::vector<uint32_t> &triangles) {
	std::vector<glm::vec4> tan1(vertices.size(), glm::vec4(0));
	std::vector<glm::vec4> tan2(vertices.size(), glm::vec4(0));
	
    for(uint64_t i=0; i<triangles.size(); i+=3) {
		glm::vec3 v1 = vertices[triangles[i]].position;
		glm::vec3 v2 = vertices[triangles[i + 1]].position;
		glm::vec3 v3 = vertices[triangles[i + 2]].position;

		glm::vec2 w1 = vertices[triangles[i]].uv;
		glm::vec2 w2 = vertices[triangles[i+1]].uv;
		glm::vec2 w3 = vertices[triangles[i+2]].uv;

		double x1 = v2.x - v1.x;
		double x2 = v3.x - v1.x;
		double y1 = v2.y - v1.y;
		double y2 = v3.y - v1.y;
		double z1 = v2.z - v1.z;
		double z2 = v3.z - v1.z;

		double s1 = w2.x - w1.x;
		double s2 = w3.x - w1.x;
		double t1 = w2.y - w1.y;
		double t2 = w3.y - w1.y;

  		double r = 1.0F / (s1 * t2 - s2 * t1);
		glm::vec4 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r, 0);
		glm::vec4 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r, 0);

		tan1[triangles[i]] += sdir;
		tan1[triangles[i + 1]] += sdir;
		tan1[triangles[i + 2]] += sdir;
		
		tan2[triangles[i]] += tdir;
		tan2[triangles[i + 1]] += tdir;
		tan2[triangles[i + 2]] += tdir;
	}

	for(uint64_t i=0; i<vertices.size(); i++) { 
		glm::vec3 n = vertices[i].normal;
		glm::vec3 t = glm::vec3(tan1[i]);

		vertices[i].tan = glm::normalize((t - n * glm::dot(n, t)));
    
        float w = (glm::dot(glm::cross(n, t), glm::vec3(tan2[i])) < 0.0F) ? -1.0F : 1.0F;
		vertices[i].bitan =  glm::normalize(glm::cross(n, t) * w);
	}
}


GL_Mesh *GetQuad()
{
    std::vector<GL_Mesh::Vertex> vertices = 
    {
        {
            glm::vec3(-1.0, -1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(0, 0)
        },
        {
            glm::vec3(-1.0, 1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(0, 1)
        },
        {
            glm::vec3(1.0, 1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(1, 1)
        },
        {
            glm::vec3(1.0, -1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(1, 0)
        }
    };

    std::vector<uint32_t> triangles = {0,2,1,3,2,0};
    GL_Mesh *quad = new GL_Mesh(vertices, triangles);	
    return quad;
}