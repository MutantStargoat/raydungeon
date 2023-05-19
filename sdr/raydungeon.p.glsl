void main()
{
	vec3 col = vec3(gl_TexCoord[0].st, 1.0);
	gl_FragColor = vec4(col, 1.0);
}
