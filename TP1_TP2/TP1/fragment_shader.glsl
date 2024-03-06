#version 330 core

// Ouput data
out vec4 color;
in vec2 TextCoord;
uniform sampler2D snow;
uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D carte;

in float h;
void main(){
		    vec4 herbe=texture(grass,TextCoord);
        vec4 pierre=texture(rock,TextCoord);
        vec4 neige=texture(snow,TextCoord);

		  if(h<0.5f){
        color=herbe;
      }else{
        if(h<0.8f){
          color=pierre;
        }else{
          color=neige;
        }
      }  
      

       
		
    	//color =  vec4(vec3(1., 0.,0.), 1.0);

    	//color = texture(rock,TextCoord);
 
    }

