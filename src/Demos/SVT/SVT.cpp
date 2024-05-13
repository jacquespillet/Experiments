#include "SVT.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>
#include <queue>

#include "imgui.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <glm/gtc/type_ptr.hpp>
SVT::SVT() {
}


void SVT::BuildPages()
{
    numMipmaps=0;

    TextureCreateInfo info = {};
    info.srgb = true;
    GL_Texture tmpTexture("resources/textures/svt/smiley_face.png", info);
    std::vector<uint8_t> pData(pageSize * pageSize * tmpTexture.nChannels);
    
    std::vector<uint8_t> pixels(tmpTexture.width * tmpTexture.height * tmpTexture.nChannels);

    virtualTextureWidth = tmpTexture.width;
    virtualTextureHeight = tmpTexture.height;

    numPagesX = virtualTextureWidth / pageSize;
    numPagesY = virtualTextureHeight / pageSize;
    
    int numMips = (int)std::log2(virtualTextureWidth);
    int mipWidth = virtualTextureWidth;
    int mipHeight = virtualTextureHeight;
    glBindTexture(GL_TEXTURE_2D, tmpTexture.glTex);
    for(int i=0; i<numMips; i++)
    {
        glGetTexImage(GL_TEXTURE_2D, i, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        int mipNumPagesX = mipWidth  / pageSize;
        int mipNumPagesY = mipHeight / pageSize;
        for(int py=0; py<mipNumPagesY; py++)
        {
            int startIndexY = py * pageSize;
            for(int px=0; px<mipNumPagesX; px++)
            {
                int startIndexX = px * pageSize;
                
                //Actually write the page
                for(int y=0; y<pageSize; y++)
                {
                    int indexY = startIndexY + y;
                    for(int x=0; x<pageSize; x++)
                    {
                        int indexX = startIndexX + x;
                        uint32_t flatIndexImage = indexY * mipWidth * tmpTexture.nChannels + indexX * tmpTexture.nChannels;
                        uint32_t flatIndexPage = (pageSize-1-y) * pageSize * tmpTexture.nChannels + x * tmpTexture.nChannels;
                        for(int j=0; j<tmpTexture.nChannels; j++)
                        {
                            pData[flatIndexPage + j] = pixels[flatIndexImage + j]; 
                        }
                    }
                }

                std::string pageFileName = "resources/textures/svt/pages/smiley_face_MIP";
                pageFileName += std::to_string(i) + "_" + std::to_string(px) + "_" + std::to_string(py) + ".png";
                std::cout << "Saving page " << pageFileName << std::endl;

                // stbi_write_jpg(pageFileName.c_str(), pageSize, pageSize, tmpTexture.nChannels, pData.data(), 90);
                stbi_write_png(pageFileName.c_str(), pageSize, pageSize, 4, pData.data(), pageSize * 4);

            }
        }

        mipWidth /=2;
        mipHeight /=2;
        numMipmaps++;
        if(mipWidth < pageSize) break;
    }    
    glBindTexture(GL_TEXTURE_2D, 0);
    tmpTexture.Unload();

    presentPages.resize(numMipmaps);
    pageTable.resize(numMipmaps);
    pageTableFilled.resize(numMipmaps);
}


void SVT::BuildVisibilityFramebuffer()
{
    visibilityFramebufferWidth = windowWidth / downSampleFactor;
    visibilityFramebufferHeight = windowHeight / downSampleFactor;

    
    glGenTextures(1, &visibilityTexture);
    glBindTexture(GL_TEXTURE_2D, visibilityTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, visibilityFramebufferWidth, visibilityFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &visibilityFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, visibilityFramebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, visibilityTexture, 0);
    unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, &attachments[0]);  

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << windowWidth << "  " << windowHeight << std::endl;
    } else {
        std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    visibilityShader = GL_Shader("shaders/SVT/visibility.vert", "", "shaders/SVT/visibility.frag");
}

void SVT::BuildPhysicalTextures()
{
    //0
    glGenTextures(1, &physicalTexture);
    glBindTexture(GL_TEXTURE_2D, physicalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, physicalTextureSizeX, physicalTextureSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

    int width = numPagesX;
    int height = numPagesY;
    for(int i=0; i<numMipmaps; i++)
    {
        pageTable[i].resize(width * height, {0,0,0,255});
        pageTableFilled[i].resize(width * height, {0,0,0,255});
        width /=2;
        height /=2;
    }
    glGenTextures(1, &pageTableTexture);
    glBindTexture(GL_TEXTURE_2D, pageTableTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, numPagesX, numPagesY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void SVT::Load() {
    frame=0;
    physicalTextureSizeX = pageSize * 20;
    physicalTextureSizeY = pageSize * 20;

    MeshShader = GL_Shader("shaders/SVT/MeshShader.vert", "", "shaders/SVT/MeshShader.frag");
   
    Mesh = GetQuad();
    Mesh->Rotate(glm::vec3(90, 0, 0));
    Mesh->SetScale(glm::vec3(50, 50, 50));
    Mat = new GL_Material();
    Mesh->material = Mat;

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 10, 10));  
    

    BuildVisibilityFramebuffer();

    BuildPages();

    BuildPhysicalTextures();

    pixmap.resize(visibilityFramebufferWidth * visibilityFramebufferHeight * 4);
}

void SVT::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }
    ImGui::SliderFloat("Mipmap", &sampleMipmap, 0, 5);
    ImGui::Checkbox("Spread", &doSpread);
    if(ImGui::Button("Build Pages"))
    {
        BuildPages();
    }

    ImGui::Image((void*)(intptr_t)physicalTexture,  ImVec2((float)0.5 * physicalTextureSizeX,(float)0.5 * physicalTextureSizeY));
    ImGui::Image((void*)(intptr_t)pageTableTexture,  ImVec2((float)10 * numPagesX,(float)10 * numPagesY));


    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void SVT::LoadVisiblePages()
{
    //Reads back the buffer
    glReadPixels(0,0,visibilityFramebufferWidth,visibilityFramebufferHeight,GL_RGBA,GL_UNSIGNED_BYTE,pixmap.data());
    struct pageData
    {
        uint32_t index;
        uint8_t mipmap;
    };

    int added = 0;
	bool mustBreak = false;

    //Pages that are already added - used to not add 2 times the same page
    std::vector<std::unordered_map<uint32_t, PageInfo>> addedPages = presentPages; 

    std::queue<pageData> toAdd;
    std::unordered_map<uint32_t, uint64_t> visiblePages;
    for(uint32_t i=0; i<(uint32_t)visibilityFramebufferWidth * visibilityFramebufferHeight; i++)
    {
        //Add all mipmaps of the visible tile.
		int numPages = 1;
		uint32_t mipDivider= (uint32_t)std::exp2(numMipmaps-1);
		for (int mip = numMipmaps - 1; mip >= 0; mip--)
		{
			uint32_t x = (uint32_t)pixmap[i].r / mipDivider;
			uint32_t y = (uint32_t)pixmap[i].g / mipDivider;
			uint32_t pageID = y * numPages + x;
			visiblePages[pageID] = 0;

            //Mark it as visible at this frame
			presentPages[mip][pageID].frame=frame;
			//If its not already present
			std::unordered_map<uint32_t, PageInfo>::iterator iterator = addedPages[mip].find(pageID);
			if(iterator == addedPages[mip].end())
			{                
				toAdd.push({pageID, (uint8_t)mip});
				addedPages[mip][pageID] = {frame, 0}; //Mark this index as present in the texture, but we don't know where yet

				if (++added >= limitAddPerFrame) {
					mustBreak = true;
					break;
				}
			}

			mipDivider /= 2;
			numPages *= 2;
		}
		if (mustBreak)break;
    }
    
    //Find which indices can be removed if we get close to filling the cache
    struct toRemoveInfo
    {
        uint32_t index;
        uint8_t mip;
    };
    std::queue<toRemoveInfo> toRemoveQueue;
    if(lastUsedIndex >= (uint32_t)(pagesPerLine * pagesPerLine - 4))
    {
        int toRemove = pagesPerLine * pagesPerLine - lastUsedIndex;
        while(toRemove>0)
        {
            for(int mip=0; mip<numMipmaps; ++mip)
            {
                for(auto &page : presentPages[mip])
                {
                    if(page.second.frame != frame)
                    {
                        toRemoveQueue.push({page.first, (uint8_t)mip});
                        toRemove--;
                    }
                }
            }
        }
    }
    
    
    //If there are new pages to add
    while(toAdd.size() !=0)
    {
        //The page we are adding
        pageData data = toAdd.front();
        toAdd.pop();
        uint32_t pageToAdd = data.index;
        uint8_t mipmap = data.mipmap;

        //Find where to add
        uint32_t indexToReplace = 0;

        //If still space in the cache
        if(lastUsedIndex < (uint32_t)(pagesPerLine * pagesPerLine))
        {
            indexToReplace = lastUsedIndex++;
        }
        else
        {
            //remove the old one from presentPages
            toRemoveInfo info = toRemoveQueue.front();
            toRemoveQueue.pop();
            PageInfo pageToReplace = presentPages[info.mip][info.index];
            presentPages[info.mip].erase(info.index);
            indexToReplace = pageToReplace.tablePosition;
        }

        //Add the new one in presentPages
        presentPages[mipmap][pageToAdd] = {frame, indexToReplace};


        int destXPageSpace = (indexToReplace % pagesPerLine);
        int destYPageSpace = (indexToReplace / pagesPerLine);
        int destX = destXPageSpace * pageSize;
        int destY = destYPageSpace * pageSize;

        //Move tile index to toRemoveIndex
        {

            //Load page
			int mipNumPages = (int)std::exp2(numMipmaps-1- mipmap);

            int px = pageToAdd % mipNumPages;
            int py = pageToAdd / mipNumPages;
            std::string pageFileName = "resources/textures/svt/pages/smiley_face_MIP";
            pageFileName += std::to_string(mipmap) + "_" + std::to_string(px) + "_" + std::to_string(py) + ".png";
            int width, height, nChannels;
			unsigned char *pageData = stbi_load(pageFileName.c_str(), &width, &height, &nChannels, 0);

            //Send to gpu
            glBindTexture(GL_TEXTURE_2D, physicalTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, destX, destY, pageSize, pageSize, GL_RGBA, GL_UNSIGNED_BYTE, pageData);       
            glBindTexture(GL_TEXTURE_2D, 0);
                
			stbi_image_free(pageData);
                
            //Maintain page table
            pageTable[mipmap][pageToAdd] = {(uint8_t)destXPageSpace, (uint8_t)destYPageSpace, (uint8_t)(mipmap), 0};
        }

    }

    pageTableFilled = pageTable;
    
    if(doSpread)
    {
    //     // //Spreading data down the mip pyramid
        for(int baseMip=0; baseMip < numMipmaps-1; baseMip++)
        {
            //8-4-2
            int baseMipSize = (int)pow(2, (numMipmaps - 1)-baseMip);
            for(int y=0; y<baseMipSize; y++)
            {
                for(int x=0; x<baseMipSize; x++)
                {
                    int baseIndex = y * baseMipSize + x;
                    //pageTableFilled[baseMip][baseIndex] = {0,0,0,0};
                    bool filled = pageTable[baseMip][baseIndex].a != 255;
                    if(filled)
                    {
						pageTableFilled[baseMip][baseIndex].b = (uint8_t)(baseMip);
						continue;
                    }
                    int parentMip = baseMip+1;
                    int parentMipSize = (int)pow(2, (numMipmaps - 1) - parentMip);
                    while(!filled)
                    {
                        //Sample parent at this position
                        int xparent = (int)(((float)x / (float)baseMipSize) * parentMipSize);
                        int yparent = (int)(((float)y / (float)baseMipSize) * parentMipSize);
                        int parentIndex = yparent * parentMipSize + xparent;
                        rgba parentRGBA = pageTable[parentMip][parentIndex];

                        //If it's filled, use the same value
                        if(parentRGBA.a != 255) 
                        {
                            pageTableFilled[baseMip][baseIndex] = parentRGBA;
                            pageTableFilled[baseMip][baseIndex].b = (uint8_t)(parentMip);
                            break;
                        }

                        parentMip++;
                        parentMipSize /= 2;
                    }
                }            
            }
        }
    }

    //Upload new page table
    glBindTexture(GL_TEXTURE_2D, pageTableTexture);
    int width = numPagesX;
    int height = numPagesY;
    for(int mip=0; mip<numMipmaps; mip++)
    {
        glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pageTableFilled[mip].data());
        width /=2;
        height /=2;
    }
    glBindTexture(GL_TEXTURE_2D, 0);    
    
}

void SVT::Render() {
	glEnable(GL_DEPTH_TEST);
	{
		glViewport(0, 0, visibilityFramebufferWidth, visibilityFramebufferHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, visibilityFramebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);
		glUseProgram(visibilityShader.programShaderObject);

		glUniform1i(glGetUniformLocation(visibilityShader.programShaderObject, "numMipmaps"), numMipmaps);
		glUniform1i(glGetUniformLocation(visibilityShader.programShaderObject, "numPagesX"), numPagesX);
		glUniform1i(glGetUniformLocation(visibilityShader.programShaderObject, "numPagesY"), numPagesY);
		glUniform2fv(glGetUniformLocation(visibilityShader.programShaderObject, "virtualTextureSize"), 1, glm::value_ptr(glm::vec2(virtualTextureWidth, virtualTextureHeight)));

		Mesh->Render(cam, visibilityShader.programShaderObject);

		LoadVisiblePages();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}




    glViewport(0, 0, windowWidth, windowHeight);
    glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

    glUseProgram(MeshShader.programShaderObject);
    glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
    glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
        
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pageTableTexture);
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "pageTableTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, physicalTexture);
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "physicalTexture"), 1);

    glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "sampleMipMap"), sampleMipmap);
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "numMipmaps"), numMipmaps);

    glUniform2fv(glGetUniformLocation(MeshShader.programShaderObject, "physicalTexturePageSize"), 1, glm::value_ptr(glm::vec2(pagesPerLine, pagesPerLine)));
    
    Mesh->Render(cam, MeshShader.programShaderObject);
    // Mesh->RenderShader(MeshShader.programShaderObject);
    
    frame++;   
}

void SVT::Unload() {
    Mesh->Unload();
    delete Mesh;
    MeshShader.Unload();
    Mat->Unload();
    delete Mat;

    glDeleteFramebuffers(1, &visibilityFramebuffer);
    visibilityShader.Unload();
    glDeleteTextures(1, &visibilityTexture);
    glDeleteRenderbuffers(1, &rbo);

    glDeleteTextures(1, &physicalTexture);
    glDeleteTextures(1, &pageTableTexture);

}


void SVT::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void SVT::LeftClickDown() {

    cam.mousePressEvent(0);
 
}

void SVT::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void SVT::RightClickDown() {


    cam.mousePressEvent(1);
}

void SVT::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void SVT::Scroll(float offset) {
    cam.Scroll(offset);
}
