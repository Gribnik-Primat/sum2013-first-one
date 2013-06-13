/* FILE NAME : a.frag
 * PURPOSE : Simple fragment shader.
 * PROGRAMMER : bg3.
 * LAST UPDATE : 10.06.2013
 */

#version 400

out vec4 OutColor;

uniform float Time;

uniform vec4 UseColor;
in vec4 DrawColor;
in vec4 DrawPos;

/* Main function */
void main( void )
{
  OutColor = DrawColor;// + vec4(0.1, 0.1, 0.1, 1) * sin(Time);
} /* End of 'main' function */

/* End of 'a.frag' file */