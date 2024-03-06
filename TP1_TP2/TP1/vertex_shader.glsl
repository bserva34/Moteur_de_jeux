#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
//TODO create uniform transformations matrices Model View Projection
// Values that stay constant for the whole mesh.
out vec2 TextCoord;
out float h;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float resolution;

uniform sampler2D carte;

void main(){
	
		TextCoord = texCoord;
        // TODO : Output position of the vertex, in clip space : MVP * position
        float stock =texture(carte,TextCoord).r;

        h=stock;

        
        vec3 posi=vec3(pos.x,stock*4,pos.z);
        gl_Position = projection * view * model * vec4(posi,1.0f);

	

		/*
		TextCoord = texCoord;
		vec3 posi=vec3(pos.x,pos.y,0);
        gl_Position = projection * view * model * vec4(posi,1);*/
}

