#version 120
attribute vec4 vPosition;

uniform vec3 color;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main() {
   vec4 eyePosition = modelViewMatrix * vPosition;  
   gl_Position = projectionMatrix * eyePosition;
   gl_FrontColor = vec4(color, 1.0);
}
