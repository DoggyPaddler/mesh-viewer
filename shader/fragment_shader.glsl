#version 150 core
#define M_PI 3.141592653589793238

out vec4 outColor;
in vec3 fs_vn;
in vec2 fs_text_coord;
in vec3 fs_fn;
in vec3 fs_tangt;
in vec4 fs_pos;
in float fs_ao;

uniform mat4 view;
uniform mat4 camera;
uniform mat4 proj;
uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 ambient;
uniform vec3 pbr_ambient;
uniform vec3 light_pos;
uniform vec3 camera_pos;
uniform float p;
uniform float kg;
uniform float roughness;
uniform float metallic;
uniform float light_intensity;
uniform int render_mode;
uniform int useTex;
uniform int use_environment_map;
uniform sampler2D environment_map;

uniform sampler2D tex_normal1;
uniform sampler2D tex_normal2;
uniform sampler2D tex_normal3;
uniform sampler2D tex_normal4;
uniform sampler2D tex_normal5;

float ni = 1.0;
float nt = 2.0;
float alpha = 1.0;

vec3 blinn_phong(vec3 ka, vec3 kd, vec3 ks, float p, vec3 n, vec3 v, vec3 l, float light_strength) {
	vec3 I = vec3(light_strength, light_strength, light_strength);
	float factor = max(dot(n, l), 0.0);
	vec3 diffuse = factor * I * kd;

	vec3 h = normalize(v + l);
	factor = pow(max(dot(n, h), 0.0), p);
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

vec3 ShadePBRMetallic(vec3 base_color, float metallic_value, float roughness_value,
					 vec3 N, vec3 L, vec3 V) {
	float dotNL = dot(N, L);
	float dotNV = dot(N, V);
	if (dotNL <= 0.0 || dotNV <= 0.0)
		return vec3(0.0);

	vec3 H = normalize(V + L);
	float dotNH = max(dot(N, H), 0.0);
	float dotVH = max(dot(V, H), 0.0);
	float r = max(roughness_value, 0.04);
	float alpha2 = r * r * r * r;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	float D = alpha2 / max(M_PI * denom * denom, 1e-6);

	float k = (r + 1.0) * (r + 1.0) / 8.0;
	float Gv = dotNV / (dotNV * (1.0 - k) + k);
	float Gl = dotNL / (dotNL * (1.0 - k) + k);
	float G = Gv * Gl;

	vec3 F0 = mix(vec3(0.04), base_color, metallic_value);
	vec3 F = F0 + (vec3(1.0) - F0) * pow(1.0 - dotVH, 5.0);
	vec3 specular_brdf = (D * G * F) / max(4.0 * dotNV * dotNL, 1e-6);
	vec3 diffuse_brdf = (vec3(1.0) - F) * (1.0 - metallic_value) * base_color / M_PI;
	return (diffuse_brdf + specular_brdf) * dotNL;
}

vec2 environment_uv(vec3 direction) {
	direction = normalize(direction);
	return vec2(
		atan(direction.z, direction.x) / (2.0 * M_PI) + 0.5,
		0.5 - asin(clamp(direction.y, -1.0, 1.0)) / M_PI);
}

vec3 sample_environment(vec3 direction, float lod) {
	return textureLod(environment_map, environment_uv(direction), lod).rgb;
}

vec3 sample_diffuse_environment(vec3 normal) {
	return sample_environment(normal, 8.0);
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
	vec3 view_d = normalize(camera_pos - fs_pos.xyz);
	float r = length(light_pos - fs_pos.xyz);
	vec3 L = light_d;
	vec3 V = view_d;
	float radiance = min(1.0 / (r * r / 1e3), 8.0); // r2
	radiance = max(radiance, 5.0);
	float point_light_strength = use_environment_map == 1 ? 0.0 : light_intensity * 0.02;


	vec3 vertex_normal_3d = normalize(fs_vn);
	vec3 tngt_light = toTangentSpc * light_d;
	vec3 tngt_view = toTangentSpc * view_d;

	if (render_mode == 0) {
		if (useTex == 1) {
			outColor = vec4(blinn_phong(ka, texture_color, texture_specular, p, normalize(texture_normal), tngt_view, tngt_light, point_light_strength), 1.0);
			//outColor = vec4(1.0, 1.0, 1.0, 1.0);
		} else {
			outColor = vec4(blinn_phong(ka, kd, ks, p, normalize(vertex_normal_3d), view_d, light_d, point_light_strength), 1.0);
			//outColor = vec4(1.0, 1.0, 1.0, 1.0);
		}
	} else if (render_mode == 1) {
		if (useTex == 1) {
			//outColor = vec4(ShadePBRSpecularV2(texture_color, pow(texture_specular, vec3(1/1.2, 1/1.2, 1/1.2)), texture_glossiness.x * 1.1, texture_normal, toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
			
			//outColor = vec4(ShadePBRSpecularV2(texture_color, pow(abs(texture_specular - 0.05), vec3(1/1.0, 1/1.0, 1/1.0)), texture_glossiness.x * 1.1, texture_normal, toTangentSpc * light_d, toTangentSpc * light_d), 1.0);

			vec3 ambient_light = (pbr_ambient / 255.0) * texture_color;
			vec3 direct_spec = ShadePBRSpecularV2(texture_color, abs(texture_specular - 0.05), texture_glossiness.x * 1.1, texture_normal, tngt_light, tngt_view);
			float direct_spec_boost = 1.0 + 0.15 * smoothstep(0.05, 0.8, max(max(texture_specular.r, texture_specular.g), texture_specular.b));
			vec3 direct_light = point_light_strength * direct_spec * direct_spec_boost;
			outColor = vec4(ambient_light + direct_light, 1.0);
			//outColor = vec4(texture(tex_normal4, fs_text_coord).xyz, 1.0);
			
			//outColor = vec4(ShadePBRSpecularV2(kd, ks, kg/100.0, texture_normal, toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
		} else {
			vec3 ambient_light = (pbr_ambient / 255.0) * kd;
			vec3 direct_spec = ShadePBRSpecularV2(kd, ks, kg/100.0, normalize(vertex_normal_3d), light_d, view_d);
			float direct_spec_boost = 1.0 + 0.12 * smoothstep(0.05, 0.8, max(max(ks.r, ks.g), ks.b));
			vec3 direct_light = point_light_strength * direct_spec * direct_spec_boost;
			outColor = vec4(ambient_light + direct_light, 1.0);
		}
	} else {
		if (useTex == 1) {
			vec3 ambient_light = (pbr_ambient / 255.0) * texture_color;
			vec3 direct_spec = ShadePBRMetallic(
				texture_color, metallic, roughness, normalize(texture_normal), tngt_light, tngt_view);
			float direct_spec_boost = mix(1.08, 1.35, clamp(metallic, 0.0, 1.0));
			vec3 direct_light = point_light_strength * direct_spec * direct_spec_boost;
			outColor = vec4(ambient_light + direct_light, 1.0);
		} else {
			vec3 ambient_light = (pbr_ambient / 255.0) * kd;
			vec3 direct_spec = ShadePBRMetallic(
				kd, metallic, roughness, normalize(vertex_normal_3d), light_d, view_d);
			float direct_spec_boost = mix(1.08, 1.35, clamp(metallic, 0.0, 1.0));
			vec3 direct_light = point_light_strength * direct_spec * direct_spec_boost;
			outColor = vec4(ambient_light + direct_light, 1.0);
		}
	}

	if (use_environment_map == 1) {
		float ambient_occlusion = clamp(fs_ao, 0.0, 1.0);
		outColor.rgb *= ambient_occlusion;
		vec3 normal = normalize(vertex_normal_3d);
		if (useTex == 1) {
			vec3 tangent = normalize(fs_tangt - dot(fs_tangt, normal) * normal);
			vec3 bitangent = normalize(cross(normal, tangent));
			mat3 tangent_to_world = mat3(tangent, bitangent, normal);
			normal = normalize(tangent_to_world * texture_normal);
		}
		vec3 base_color = useTex == 1 ? texture_color : kd;
		float material_color_floor = render_mode == 0 ? 0.18 : 0.10;
		outColor.rgb += base_color * material_color_floor * ambient_occlusion;

		float reflection_roughness;
		vec3 f0;
		if (render_mode == 2) {
			reflection_roughness = roughness;
			f0 = mix(vec3(0.04), base_color, metallic);
		} else if (render_mode == 1) {
			reflection_roughness = useTex == 1
				? clamp(1.0 - texture_glossiness.x * 1.1, 0.0, 1.0)
				: clamp(1.0 - kg / 100.0, 0.0, 1.0);
			f0 = useTex == 1 ? clamp(abs(texture_specular - vec3(0.05)), 0.0, 1.0) : ks;
		} else {
			reflection_roughness = clamp(sqrt(2.0 / (max(p, 1.0) + 2.0)), 0.08, 1.0);
			f0 = useTex == 1 ? clamp(texture_specular, 0.0, 1.0) : ks;
		}
		float specular_strength = max(max(f0.r, f0.g), f0.b);
		float glossiness = 1.0 - reflection_roughness;
		float reflective_material = clamp(specular_strength * glossiness, 0.0, 1.0);
		float environment_diffuse_weight = mix(0.10, 0.65,
			smoothstep(0.03, 0.65, reflective_material));
		float diffuse_weight = render_mode == 2
			? 1.0 - metallic
			: (render_mode == 1 ? mix(1.0, 0.72, smoothstep(0.05, 0.8, reflective_material)) : 1.0);
		vec3 environment_diffuse_sample = sample_diffuse_environment(normal);
		float environment_diffuse_luma = dot(environment_diffuse_sample, vec3(0.2126, 0.7152, 0.0722));
		environment_diffuse_sample = mix(environment_diffuse_sample, vec3(environment_diffuse_luma), 0.24);
		vec3 environment_diffuse = environment_diffuse_sample * base_color *
			diffuse_weight * environment_diffuse_weight * ambient_occlusion;

		vec3 reflection_direction = reflect(-view_d, normal);
		vec3 environment_reflection = sample_environment(reflection_direction,
			mix(8.0, 10.0, clamp(reflection_roughness, 0.0, 1.0)));
		float view_facing = max(dot(normal, view_d), 0.0);
		vec3 fresnel = f0 + (vec3(1.0) - f0) * pow(1.0 - view_facing, 5.0);
		float reflection_weight = mix(0.05, 1.0,
			smoothstep(0.03, 0.65, reflective_material));
		if (render_mode == 1) {
			reflection_weight *= mix(1.08, 1.28, smoothstep(0.05, 0.8, reflective_material));
		}
		float environment_specular_boost = 1.0;
		float environment_specular_scale = 0.04;
		if (render_mode == 2) {
			environment_specular_boost = mix(1.15, 1.65, clamp(metallic, 0.0, 1.0));
			environment_specular_scale = mix(0.042, 0.052, clamp(metallic, 0.0, 1.0));
		} else if (render_mode == 1) {
			environment_specular_boost = mix(1.12, 1.55,
				smoothstep(0.05, 0.75, reflective_material));
			environment_specular_scale = mix(0.042, 0.052,
				smoothstep(0.05, 0.75, reflective_material));
		}
		vec3 environment_specular = environment_reflection * fresnel * reflection_weight * environment_specular_scale * environment_specular_boost;
		vec3 environment_contribution = environment_diffuse + environment_specular;
		outColor.rgb += pow( min(max(environment_contribution, vec3(0.0)), vec3(8.0)) * vec3(0.6), vec3(1.0) );

		vec3 mapped_color = max(outColor.rgb, vec3(0.0));
		mapped_color = mapped_color / (mapped_color + vec3(1.0));
		outColor.rgb = pow(mapped_color, vec3(1.0 / 2.2));
	}

	//outColor = vec4(blinn_phong(ka, kd, ks, p, normalize(vertex_normal_3d), light_d, light_d), 1.0);
	//outColor = vec4(ShadePBRSpecular(kd, ks, 0.9, normalize(vertex_normal_3d), light_d, light_d) * radiance, 1.0);
	//outColor = vec4(ShadePBRSpecular(texture_color, texture_specular, texture_glossiness.x, texture_normal, toTangentSpc * L, toTangentSpc * V) * radiance, 1.0);
	//outColor = vec4(blinn_phong(ka, kd, ks, p, normalize(texture_normal), toTangentSpc * light_d, toTangentSpc * light_d), 1.0);
}
