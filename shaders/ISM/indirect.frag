//inputs
#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
//main

uniform sampler2D colorTexture;

uniform sampler2D rsmPositions; 
uniform sampler2D rsmNormals;
uniform sampler2D rsmFlux;
uniform sampler2D samplingPattern;

void main()
{
    outputColor = vec4(1, 0, 0, 1);    
}