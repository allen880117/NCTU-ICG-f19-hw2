#version 430

in vec2 Texcoord; // From vertex shader

out vec4 frag_color;

uniform sampler2D earthTexture;

const vec4 RED = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 GEREN = vec4(0.0, 1.0, 0.0, 1.0);
const vec4 BLUE = vec4(0.0, 0.0, 1.0, 1.0);

void main() {
    //frag_color = vec4(0.0, 1.0, 0.0, 1.0);
    frag_color = texture2D(earthTexture, Texcoord);
}
