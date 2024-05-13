#include "MeshManipulation.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

void SubdivModel::MoveVertex(int vertexIndex, const glm::vec3& newPosition)
{
    //Gets the old position of this vertex
    glm::vec3 oldPosition = vertices[vertexIndex].position;
    
    //Remove it from the mapping
    positionToVertex.erase(oldPosition);

    if(positionToVertex.find(newPosition) != positionToVertex.end())
    {
        assert(0);
    }

    //Add to the mapping
    positionToVertex[newPosition] = vertexIndex;
    
    //Change the position with the new one
    vertices[vertexIndex].position = newPosition;
}

int SubdivModel::GetVertexIndex(const glm::vec3 &position)
{
    //Check if the position is already in the list
    auto it = positionToVertex.find(position);
    if(it != positionToVertex.end())
    {
        return it->second;
    }   

    //If not, add it
    int lastIndex= static_cast<int>(vertices.size());

    
    SubdivVertex newVertRecord;
    newVertRecord.position = position;
    vertices.push_back(newVertRecord);
    positionToVertex[position] = lastIndex;

    return lastIndex;
}

int SubdivModel::GetEdgeIndex(const glm::ivec2& vertexPair)
{
    auto sortedVertexPair = vertexPair.x < vertexPair.y ? vertexPair : glm::ivec2(vertexPair.y, vertexPair.x);

    //Check if the edge is already present in the list   
    auto it = vertexPairToEdge.find(sortedVertexPair);
    if(it != vertexPairToEdge.end())
    {
        return it->second;
    }

    int lastIndex = static_cast<int>(edges.size());

    SubdivEdge newEdge;
    newEdge.lowVertex = sortedVertexPair.x;
    newEdge.highVertex = sortedVertexPair.y;
    edges.push_back(newEdge);
    vertexPairToEdge[sortedVertexPair] = lastIndex;

    return lastIndex;
}

MeshManipulation::MeshManipulation() {
}

void MeshManipulation::Load() {

    MeshShader = GL_Shader("shaders/MeshManipulation/MeshShader.vert", "", "shaders/MeshManipulation/MeshShader.frag");
    WireframeShader = GL_Shader("shaders/MeshManipulation/MeshShader.vert", "", "shaders/MeshManipulation/WireframeShader.frag");

    MeshesFromFile("resources/models/suzanne/Suzanne.gltf", &Meshes, &Materials);
    
    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 0, 0));  
    cam.SetDistance(2);

}

SubdivModel MeshManipulation::PrepareMesh(GL_Mesh *mesh)
{
    SubdivModel outModel = {};

    //For all triangles in mesh
    for(uint32_t k=0; k<mesh->triangles.size(); k+=3)
    {
        
        //Populates the SubdivVertex list of outModel
        //Fills in the map between position and index
        int vertexIndices[3] = {-1,-1,-1};
        for(int j=0; j<3; j++)
        {
            //This also populates the list of vertices in outModel
            vertexIndices[j] = outModel.GetVertexIndex(mesh->vertices[mesh->triangles[k+j]].position);
        }

        //Populates the SubdivEdge of outModel
        //Fills in the map between vertex indices and edge
        //Creates the link between vertex and edge
        int edgeIndices[3] = {-1,-1,-1};
        for(int j=0; j<3; j++)
        {
            //Edge vertices
            int startVertex = vertexIndices[j];
            int endVertex = vertexIndices[(j+1)%3];

            //Get the edge index in the SubdivEdge list
            //Also populates the edge list of outModel
            int edgeIndex = outModel.GetEdgeIndex({startVertex, endVertex});
            
            edgeIndices[j] = edgeIndex;


            if(startVertex < endVertex)
            {
                //Sets the edge property of the 2 vertices with the edgeIndex
                outModel.vertices[startVertex].edges.push_back(edgeIndex);
                outModel.vertices[endVertex].edges.push_back(edgeIndex);
            }
        }
        
        //Create a face
        SubdivFace newFace;
        
        //Sets the vertex indices of the face
        for(int j=0; j<3; j++)
        {
            newFace.vertices[j] = vertexIndices[j];
        }

        //Sets the edge indices of the face
        for(int j=0; j<3; j++)
        {
            newFace.edges[j] = edgeIndices[j];
        }

        //Adds the face to the list
        int lastIndex = static_cast<int>(outModel.faces.size());
        outModel.faces.push_back(newFace);

        //Sets the face property to each vertex
        for(int j=0; j<3; j++)
        {
            auto &vertexRecord = outModel.vertices[vertexIndices[j]];
            vertexRecord.faces.push_back(lastIndex);
        }

        //Sets the face property to each edge
        for(int j=0; j<3;j++)
        {
            SubdivEdge &edgeRecord = outModel.edges[edgeIndices[j]];
            if(edgeRecord.faceCount>=2) assert(0);
            edgeRecord.faces[edgeRecord.faceCount++] = lastIndex;
        }
    }
    return outModel;
}

GL_Mesh *MeshManipulation::ApplySubdivision(SubdivModel& model)
{ 

    //Contains all the centroids of the faces of the model
    std::vector<glm::vec3> faceCentroids(model.faces.size());
    
    //Calculate each centroid : average of each position of a face
    for(size_t i=0; i<faceCentroids.size(); i++)
    {
        auto &f = model.faces[i];
        int vertexCount=3;

        glm::vec3 vertexSum;
        for(int j=0; j<3; j++)
        {
            vertexSum += model.vertices[f.vertices[j]].position;
        }
        faceCentroids[i] = vertexSum / static_cast<float>(vertexCount);
    }

    //Contains all the mid points of the edges of the model
    std::vector<glm::vec3> edgeAveragePoints(model.edges.size());
    for(size_t i=0; i<edgeAveragePoints.size(); i++)
    {
        auto& e = model.edges[i];
        if(e.faceCount != 2)
        {
            //If the edge is shared by only 1 triangle, the position is just the middle point on the line
            edgeAveragePoints[i] = 0.5f * (model.vertices[e.lowVertex].position + model.vertices[e.highVertex].position);
        }
        else
        {
            //If it's shared by 2 triangles,
            //we take into account the central position of the 2 faces
            edgeAveragePoints[i] = 0.25f * (model.vertices[e.lowVertex].position + model.vertices[e.highVertex].position + faceCentroids[e.faces[0]] + faceCentroids[e.faces[1]]);
        }
    }

    //Contains all the original positions of the vertices.
    std::vector<glm::vec3> oldPositions;
    oldPositions.reserve(model.vertices.size());
    for(auto &v : model.vertices)
    {
        oldPositions.push_back(v.position);
    }

    //Calculates a new position for each vertex
    for(size_t i=0; i<model.vertices.size(); i++)
    {
        auto &v = model.vertices[i];

        //How many faces share this vertex ?
        int n = static_cast<int>(v.faces.size());

        //Weight for calculating new position
        float oldPositionWeight = static_cast<float>(n-3)/n; //Will weigh the original position
        float facePositionWeight = 1.0f / n; //Will weigh the average face position
        float edgeMidPointWeight = 2.0f / n; //will weigh the average edge position
        

        //Calculate the average position of each face that share this vertex
        glm::vec3 averageFacePosition;
        for(auto fi : v.faces)
        {
            averageFacePosition += faceCentroids[fi];
        }
        averageFacePosition /= static_cast<float>(n);

        //Calculate the average position of each edge that share this vertex
        glm::vec3 averageEdgeMid;
        for(auto edgeIndex : v.edges)
        {
            auto &edge = model.edges[edgeIndex];
            averageEdgeMid += 0.5f * (oldPositions[edge.lowVertex] + oldPositions[edge.highVertex]);
        }
        averageEdgeMid /= static_cast<float>(v.edges.size());

        //Calculate a new position for the vertex
        glm::vec3 newPosition = oldPositionWeight * oldPositions[i] + facePositionWeight * averageFacePosition + edgeMidPointWeight * averageEdgeMid;
        
        //Moves the vertex in the outModel
        model.MoveVertex(static_cast<int>(i), newPosition);
    }

    //Clears the mesh data
    GL_Mesh *newMesh = Meshes[0];
    newMesh->triangles.clear();
    newMesh->vertices.clear();


    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    //For each face
    for(size_t i=0; i<model.faces.size(); i++)
    {
        auto &f = model.faces[i];
        
        //Get the 3 altered vertex positions
        glm::vec3 va = model.vertices[f.vertices[0]].position;
        glm::vec3 vb = model.vertices[f.vertices[1]].position;
        glm::vec3 vc = model.vertices[f.vertices[2]].position;

        //Get the 3 edges mid points (weighted by the surrounding faces)
        glm::vec3 eab = edgeAveragePoints[model.GetEdgeIndex({f.vertices[0], f.vertices[1]})];
        glm::vec3 ebc = edgeAveragePoints[model.GetEdgeIndex({f.vertices[1], f.vertices[2]})];
        glm::vec3 eca = edgeAveragePoints[model.GetEdgeIndex({f.vertices[2], f.vertices[0]})];
        
        //get the mid point of this face
        glm::vec3 fabc = faceCentroids[i];
    
        //We will split one face into 6 smaller triangles
        auto aIndex= static_cast<uint32_t>(vertices.size());
        auto bIndex = aIndex+1;
        auto cIndex = aIndex+2;

        auto eabIndex = aIndex+3;
        auto ebcIndex = aIndex+4;
        auto ecaIndex = aIndex+5;

        auto fabcIndex = aIndex + 6;

        //Add the original points
        vertices.push_back(va);
        vertices.push_back(vb);
        vertices.push_back(vc);

        //Add the edge points
        vertices.push_back(eab);
        vertices.push_back(ebc);
        vertices.push_back(eca);
        
        //Add the face centroid
        vertices.push_back(fabc);

        //Add all the triangles
        indices.push_back(ecaIndex); indices.push_back(aIndex); indices.push_back(eabIndex);  
        indices.push_back(ecaIndex); indices.push_back(eabIndex); indices.push_back(fabcIndex);
        
        indices.push_back(eabIndex); indices.push_back(bIndex); indices.push_back(ebcIndex);  
        indices.push_back(eabIndex); indices.push_back(ebcIndex); indices.push_back(fabcIndex);
        
        indices.push_back(ebcIndex); indices.push_back(cIndex); indices.push_back(ecaIndex);  
        indices.push_back(ebcIndex); indices.push_back(ecaIndex); indices.push_back(fabcIndex);
    }

    
    for(int i=0; i<indices.size(); i+=3)
    {
        glm::vec3 v0 = vertices[indices[i+0]];
        glm::vec3 v1 = vertices[indices[i+1]];
        glm::vec3 v2 = vertices[indices[i+2]];
        glm::vec3 n = glm::normalize(glm::cross(v1-v0, v2-v1));

        newMesh->vertices.push_back({ v0, n, glm::vec3(0), glm::vec3(0), glm::vec2(0)});
        newMesh->vertices.push_back({ v1, n, glm::vec3(0), glm::vec3(0), glm::vec2(0)});
        newMesh->vertices.push_back({ v2, n, glm::vec3(0), glm::vec3(0), glm::vec2(0)});

        newMesh->triangles.push_back(i+0);
        newMesh->triangles.push_back(i+1);
        newMesh->triangles.push_back(i+2);
    }
    return newMesh;
}

void MeshManipulation::RenderGUI() {
    ImGui::Begin("Parameters : ");
    ImGui::Checkbox("Wireframe", &wireframe);
    ImGui::Checkbox("Solid", &solid);
    if(ImGui::Button("Apply"))
    {
        SubdivModel originalModel = PrepareMesh(Meshes[0]);
        Meshes[0] = ApplySubdivision(originalModel);   
        Meshes[0]->RebuildBuffers();     
    }

    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void MeshManipulation::Render() {
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    for(int i=0; i<1; i++)
    {
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);
        if(solid)
        {
            glUseProgram(MeshShader.programShaderObject);
            glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
            glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
                    
            glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "normalTextureSet"), (int)normalTextureSet);
            glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "specularTextureSet"), (int)specularTextureSet);
            
            Meshes[i]->Render(cam, MeshShader.programShaderObject);
        }

        if(wireframe)
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glUseProgram(WireframeShader.programShaderObject);
            Meshes[i]->SetScale(glm::vec3(1.01f));
            Meshes[i]->Render(cam, WireframeShader.programShaderObject);
            Meshes[i]->SetScale(glm::vec3(1));
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
    }
}

void MeshManipulation::Unload() {
    for(int i=0; i<Materials.size(); i++)
    {
        Materials[i]->Unload();
        delete Materials[i];
    }
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->Unload();
        delete Meshes[i];
    }
}


void MeshManipulation::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void MeshManipulation::LeftClickDown() {
    cam.mousePressEvent(0);
}

void MeshManipulation::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void MeshManipulation::RightClickDown() {
    cam.mousePressEvent(1);
}

void MeshManipulation::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void MeshManipulation::Scroll(float offset) {
    cam.Scroll(offset);
}
