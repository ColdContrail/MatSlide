#version 450 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D accumResult;
uniform int frameCount;

void main()
{
    vec4 accum = texture(accumResult, TexCoord);

    vec3 color = accum.rgb / float(frameCount);

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    float alpha = clamp(accum.a / float(frameCount), 0.0, 1.0);

    FragColor = vec4(color * alpha, alpha);
}
