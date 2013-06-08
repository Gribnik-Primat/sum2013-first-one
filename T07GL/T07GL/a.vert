/* FILE NAME   : a.vert
 * PURPOSE     : Simple vertex shader.
 * PROGRAMMER  : VG4.
 * LAST UPDATE : 08.06.2013
 */

#version 430

layout(location = 0) in vec4 vPosition;
out vec4 UseColor;

/* Main function */
void main( void )
{
  gl_Position = vPosition;
  UseColor = gl_Color;
} /* End of 'main' function */

/* End of 'a.vert' file */
