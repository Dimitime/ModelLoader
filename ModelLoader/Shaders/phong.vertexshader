#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertex_normal;
out vec3 vert;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	gl_Position = projection*view*model*vec4(vertexPosition_modelspace, 1.0);

    vert = vertexPosition_modelspace;
	normal = mat3(transpose(inverse(model)))*vertex_normal;
}