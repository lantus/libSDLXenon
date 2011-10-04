.global prim_textured_vertex_shader_size
.set prim_textured_vertex_shader_size, 0f-prim_textured_vertex_shader
.global prim_textured_vertex_shader
prim_textured_vertex_shader:
	.incbin "src/video/xenon/prim_textured_vertex.shader"
0:

.align 4

.global prim_textured_pixel_shader_size
.set prim_textured_pixel_shader_size, 0f-prim_textured_pixel_shader
.global prim_textured_pixel_shader
prim_textured_pixel_shader:
	.incbin "src/video/xenon/prim_textured_pixel.shader"
0:
