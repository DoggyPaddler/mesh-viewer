#version 150 core
#define M_PI 3.141592653589793238

in vec2 fs_ndc;
out vec4 outColor;

uniform mat4 inv_view_proj;
uniform vec3 camera_pos;
uniform sampler2D environment_map;
uniform float environment_zoom;

void main() {
	vec2 zoomed_ndc = fs_ndc * environment_zoom;
	vec4 world = inv_view_proj * vec4(zoomed_ndc, 1.0, 1.0);
	vec3 direction = normalize(world.xyz / world.w - camera_pos);
	vec2 uv = vec2(atan(direction.z, direction.x) / (2.0 * M_PI) + 0.5,
				   0.5 - asin(clamp(direction.y, -1.0, 1.0)) / M_PI);
	vec3 color = texture(environment_map, uv).rgb;
	color = color / (color + vec3(1.0));
	outColor = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), 1.0);
}
