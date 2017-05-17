#version 450

// ssao
buffer Transformation { mat4 mvp[]; } transformation;

// vertex attribs
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

// outputs to fs
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragTexCoord;

void main() {
    gl_Position  = transformation.mvp[gl_InstanceID] * vec4(inPosition, 1.0);
    fragColor    = vec4(inColor, 0.0);
    fragTexCoord = vec4(inTexCoord, 0.0, 0.0);
}
