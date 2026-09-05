#version 150 core
#define M_PI 3.141592653589793238

out vec4 outColor;
in vec3 fs_vn;
in vec2 fs_text_coord;
in vec3 fs_fn;
in vec3 fs_tangt;
in vec4 fs_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 camera;
uniform mat4 proj;
uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 ambient;
uniform vec3 light_pos;
uniform float p;
uniform float kg;
uniform float light_intensity;
uniform int render_mode;
uniform int useTex;

uniform sampler2D tex_normal1;
uniform sampler2D tex_normal2;
uniform sampler2D tex_normal3;
uniform sampler2D tex_normal4;
uniform sampler2D tex_normal5;

float ni = 1.0;
float nt = 2.0;
float alpha = 1.0;

vec3 blinn_phong(vec3 ka, vec3 kd, vec3 ks, float p, vec3 n, vec3 v, vec3 l) {
	float light_strength = light_intensity * 0.02;
	vec3 I = vec3(light_strength, light_strength, light_strength);
	float factor = max(n.x * l.x + n.y * l.y + n.z * l.z, -(n.x * l.x + n.y * l.y + n.z * l.z));
	vec3 diffuse = factor * I * kd;

	vec3 h = normalize(v + l);
	factor = pow(max((n.x * h.x + n.y * h.y + n.z * h.z), -(n.x * h.x + n.y * h.y + n.z * h.z)), p);
	vec3 specular = factor * I * ks;

	vec3 ambient = ka;
	return ambient + diffuse + specular;
}

vec3 ShadePBRSpecular(vec3 C_diff, vec3 F0, float a, vec3 N, vec3 L, vec3 V) {
	vec3 new_C_diff = C_diff * (1 - F0);
	float new_a = pow((1 - a), 2);

	vec3 H = normalize(V + L);

	float dotVH = max(dot(V, H), 1e-6);
    float dotNL = max(dot(N, L), 1e-6); // n*wi: light to p
    float dotNV = max(dot(N, V), 1e-6); // n*wo: p to viewer
    float dotNH = max(dot(N, H), 1e-6);

    vec3 F = F0 + (1-F0) * pow((1-dotVH), 10);

    float a2 = new_a * new_a;
    float denom = dotNH * dotNH * (a2 - 1.0) + 1.0;
    float D = a2 / (denom * denom * M_PI);

    float denom1 = dotNL * sqrt(dotNV * dotNV * (1 - a2) + a2);
    float denom2 = dotNV * sqrt(dotNL * dotNL * (1 - a2) + a2);
    float Vis = 0.01 / (denom1 + denom2);
    vec3 spec = F * Vis * D; // fr

	vec3 diff = (1-F) * new_C_diff * 0.31830988618379067154; // Rd / pi
    vec3 shade_pbr = (diff + spec) * dotNL; // 

	return shade_pbr;
}

vec3 ShadePBRSpecularV2(vec3 C_diff, vec3 C_spc, float a, vec3 N, vec3 L, vec3 V) {
	vec3 H = normalize(V + L);

	float dotVH = max(dot(V, H), 1e-6);
	float dotNL = max(dot(N, L), 1e-6); // n*wi: light to p
	float dotNV = max(dot(N, V), 1e-6); // n*wo: p to viewer
	float dotNH = max(dot(N, H), 1e-6);

	//float m = a * 100;
	vec3 diffuse = C_diff;

	//float D = pow(dotNH, m) * ((m + 8) / (8 * M_PI));

	float r2 = pow((1 - a), 2);
    float denom = dotNH * dotNH * (r2 - 1.0) + 1.0;
    float D = r2 / (denom * denom * M_PI);

    float k = pow((1 - a) + 1, 2)/8.0;
    float denom1 = dotNL / (k + (1-k) * dotNL);
    float denom2 = dotNV / (k + (1-k) * dotNV);

	//float Vis = 1;
	float Vis = (denom1 * denom2) / (4 * dotNL * dotNV);
	vec3 F0 = C_spc + (1-C_spc) * pow((1-dotVH), 5);;
	vec3 specular = Vis * D * F0;

	return dotNL * (diffuse + specular);
}

void main() {
	vec3 texture_color = texture(tex_normal1, fs_text_coord).xyz;
	vec3 texture_glossiness = texture(tex_normal3, fs_text_coord).xyz;
	vec3 texture_specular = texture(tex_normal4, fs_text_coord).xyz;
	vec3 texture_normal = texture(tex_normal2, fs_text_coord).xyz;

	vec3 kd = diffuse/255.0;
	//vec3 kd = texture_color.xyz;
	vec3 ka = ambient/(10 * 255.0);
	vec3 ks = specular/255.0;

	vec3 bi_norm = cross(fs_vn, fs_tangt);
	mat3 toTangentSpc = mat3(
						fs_tangt.x, bi_norm.x, fs_vn.x,
						fs_tangt.y, bi_norm.y, fs_vn.y,
						fs_tangt.z, bi_norm.z, fs_vn.z);
	texture_normal = normalize(texture_normal * 2.0 - 1.0);


	vec3 light_d = normalize(light_pos - fs_pos.xyz);
	float r = length(light_pos - fs_pos.xyz);
	vec3 L = normalize(light_pos - fs_pos.xyz);
	vec3 V = normalize(light_pos - fs_pos.xyz);
	float radiance = min(1.0 / (r * r / 1e3), 8.0); // r2
	radiance = max(radiance, 5.0);


	vec4 vertex_normal_4d = model * vec4(fs_vn[0], fs_vn[1], fs_vn[2], 0.0);
	vec3 vertex_normal_3d = vec3(vertex_normal_4d[0], vertex_normal_4d[1], vertex_normal_4d[2]);
	if (vertex_normal_3d.x * light_d.x + vertex_normal_3d.y * light_d.y + vertex_normal_3d.z * light_d.z < 0) {
		vertex_normal_3d = -vertex_normal_3d;
	}
	vec3 tngt_light = toTangentSpc * light_d;
	if (vertex_normal_3d.x * light_d.x + vertex_normal_3d.y * light_d.y + vertex_normal_3d.z * light_d.z < 0) {
		vertex_normal_3d = -vertex_normal_3d;
	}
	if (texture_normal.x * tngt_light.x + texture_normal.y * tngt_light.y + texture_normal.z * tngt_light.z < 0) {
		texture_normal = -texture_normal;
	}

	if (render_mode == 0) {
		if (useTex == 1) {
			outColor = vec4(blinn_phong(ka, texture_color, texture_specular, p, normalize(texture_normal), toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
			//outColor = vec4(1.0, 1.0, 1.0, 1.0);
		} else {
			outColor = vec4(blinn_phong(ka, kd, ks, p, normalize(vertex_normal_3d), light_d, light_d), 1.0);
			//outColor = vec4(1.0, 1.0, 1.0, 1.0);
		}
	} else {
		if (useTex == 1) {
			//outColor = vec4(ShadePBRSpecularV2(texture_color, pow(texture_specular, vec3(1/1.2, 1/1.2, 1/1.2)), texture_glossiness.x * 1.1, texture_normal, toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
			
			//outColor = vec4(ShadePBRSpecularV2(texture_color, pow(abs(texture_specular - 0.05), vec3(1/1.0, 1/1.0, 1/1.0)), texture_glossiness.x * 1.1, texture_normal, toTangentSpc * light_d, toTangentSpc * light_d), 1.0);

			outColor = vec4((light_intensity * 0.02) * ShadePBRSpecularV2(texture_color, abs(texture_specular - 0.05), texture_glossiness.x * 1.1, texture_normal, toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
			//outColor = vec4(texture(tex_normal4, fs_text_coord).xyz, 1.0);
			
			//outColor = vec4(ShadePBRSpecularV2(kd, ks, kg/100.0, texture_normal, toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
		} else {
			outColor = vec4((light_intensity * 0.02) * ShadePBRSpecularV2(kd, ks, kg/100.0, normalize(vertex_normal_3d), light_d, light_d), 1.0);
		}
	}

	//outColor = vec4(blinn_phong(ka, kd, ks, p, normalize(vertex_normal_3d), light_d, light_d), 1.0);
	//outColor = vec4(ShadePBRSpecular(kd, ks, 0.9, normalize(vertex_normal_3d), light_d, light_d) * radiance, 1.0);
	//outColor = vec4(ShadePBRSpecular(texture_color, texture_specular, texture_glossiness.x, texture_normal, toTangentSpc * L, toTangentSpc * V) * radiance, 1.0);
	//outColor = vec4(blinn_phong(ka, kd, ks, p, normalize(texture_normal), toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
}
