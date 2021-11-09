//inputs
#version 400 core
//output
layout(location = 0) out vec4 outputColor; 

uniform mat4 invModel;
uniform sampler2D positionTexture;
uniform sampler2D decalTexture;

in vec4 fragPos;
in vec4 fragWorldPos;

void main()
{
    vec2 texelSize = vec2(1.0 / 1280, 1.0 / 720);

    //-1, 1
    vec2 uv = gl_FragCoord.xy * texelSize;
    // uv += vec2(1,1);
    // uv /= 2;


    vec3 pixelWorldPos = texture(positionTexture, uv).xyz;
    vec3 boxPos = (invModel * vec4(pixelWorldPos, 1)).xyz;

    if(abs(boxPos.x) > 0.5 || abs(boxPos.y) > 0.5 || abs(boxPos.z) > 0.5) discard;

    vec2 texUv = boxPos.xy + 0.5;

    vec4 color = texture(decalTexture, texUv);
    if(color.a < 0.5) discard;
    outputColor = vec4(color.xyz, 1);
}