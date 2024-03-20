#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 pos;
//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
	
	
        gl_Position = projection * view * model * vec4(pos,1);
}

