R"(
	#version 450

	in vec2 tex_coords;

	out vec4 frag_color;

	uniform sampler2D scene;

	void main(void) {
		frag_color = texture(scene, tex_coords);
	}
)"
