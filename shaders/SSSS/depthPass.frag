#version 400 core
//output
layout(location = 0) out float outputDepth; 
//main

in float fragDepth;

 


void main()
{
    outputDepth = fragDepth;
}