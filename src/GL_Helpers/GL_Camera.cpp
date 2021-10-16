#include "GL_Camera.hpp"
#include <algorithm>

void GL_Camera::RecalculateLookat() {
    worldPosition = target + sphericalPosition * distance;
    invModelMatrix = glm::lookAt(worldPosition, target, up);
    modelMatrix = glm::inverse(invModelMatrix);
}


void GL_Camera::mousePressEvent(int button) {
    if(locked) return;
    if(button==0) {
        IsLeftMousePressed=true;
    } else if(button==1) {
        IsRightMousePressed=true;
    }
}

void GL_Camera::mouseReleaseEvent(int button) {
    IsLeftMousePressed=false;
    IsRightMousePressed=false;
    prevPos = glm::ivec2(-1, -1);
}

void GL_Camera::mouseMoveEvent(float x, float y) {
    if(locked) return;
    glm::vec2 currentPos = glm::vec2(x, y);
    if(IsLeftMousePressed) {
        if(prevPos.x >0) {
            glm::vec2 diff = currentPos - prevPos;
            
            phi += (float)diff.x * 0.005f;
            theta -= (float)diff.y * 0.005f;
            
            phi = (phi>2*PI)? phi - 2.0f * PI : phi;
            phi = (phi<0) ? 2.0f * PI + phi : phi;

            theta = std::max(0.00001f, std::min(theta,(float)PI-0.00001f));

            sphericalPosition.x = sin(theta) * cos(phi);
            sphericalPosition.z = sin(theta) * sin(phi);                  
            sphericalPosition.y = cos(theta);
            RecalculateLookat();    
        }
    } else if (IsRightMousePressed) {
        if(prevPos.x >0) {
            glm::ivec2 diff = currentPos - prevPos;
            target -= (float)diff.x * 0.005 * distance * glm::vec3(modelMatrix[0]);
            target += (float)diff.y * 0.005 * distance *  glm::vec3(modelMatrix[1]);
            RecalculateLookat();
        }
    }
    prevPos = currentPos;
}

void GL_Camera::Scroll(float offset){
    if(locked) return;
    distance -= (float)offset * distance * 0.05f;
    RecalculateLookat();   
}