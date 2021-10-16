#pragma once

class Demo {
public:
    virtual void Load(){};
    virtual void MouseMove(float x, float y){};
    virtual void Render(){};
    virtual void RenderGUI(){};
    virtual void Unload(){};

    virtual void LeftClickDown(){}
    virtual void LeftClickUp(){}
    virtual void RightClickDown(){}
    virtual void RightClickUp(){}
    virtual void Scroll(float offset){}

    int windowWidth, windowHeight;

};