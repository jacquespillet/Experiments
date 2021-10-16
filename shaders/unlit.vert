 //attribs
#version 440
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 fragNormal;
out vec3 fragPos;

//main
void main()
{
    fragNormal = normal;
    fragPos = (modelMatrix * vec4(position,1.0)).xyz;
    vec4 wPos = projectionMatrix * viewMatrix * modelMatrix * vec4(position,1.0);

    // gl_Position = vec4(position.x, position.y, 0.1, 1);
    gl_Position = wPos;
}