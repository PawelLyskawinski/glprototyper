#version 450

uniform sampler2D texSampler;

// inputs from vs
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec4 fragTexCoord;

// render outputs
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord.xy);
}
