#include "myshader.h"


void PhongShader::vertex(int nfaces, int nvertex)
{
	vec4 temp_vert = to_vec4(payload.model->vert(nfaces, nvertex));
	vec4 temp_normal = to_vec4(payload.model->normal(nfaces, nvertex));

	payload.uv_attri[nvertex] = payload.model->uv01(nfaces, nvertex);
	payload.in_uv[nvertex] = payload.uv_attri[nvertex];
	payload.clipcoord_attri[nvertex] = payload.mvp_matrix * temp_vert;
	payload.in_clipcoord[nvertex] = payload.clipcoord_attri[nvertex];

	for (int i = 0; i < 3; i++) {
		payload.worldcoord_attri[nvertex][i] = temp_vert[i];
		payload.in_worldcoord[nvertex][i] = temp_vert[i];
		payload.norm_attrix[nvertex][i] = temp_normal[i];
		payload.in_normal[nvertex][i] = temp_normal[i];
	}
}


vec3 PhongShader::fragment(vec3 bar) {
	float alpha = bar.x, beta = bar.y, gamma = bar.z;
	vec4* clip_coords = payload.clipcoord_attri;
	vec3* world_coords = payload.worldcoord_attri;
	vec3* normals = payload.norm_attrix;
	vec2* uvs = payload.uv_attri;
	vec4* clip_coords_light = payload.clipcoord_attri_light;



	//根据重心插值计算属性 注意透视矫正  
	//透视矫正  推导：https://zhuanlan.zhihu.com/p/144331875
	float Z = 1.0 / (alpha / clip_coords[0].w + beta / clip_coords[1].w + gamma / clip_coords[2].w);
	vec3 normal = (alpha * normals[0] / clip_coords[0].w + beta * normals[1] / clip_coords[1].w +
		gamma * normals[2] / clip_coords[2].w) * Z;
	vec2 uv = (alpha * uvs[0] / clip_coords[0].w + beta * uvs[1] / clip_coords[1].w +
		gamma * uvs[2] / clip_coords[2].w) * Z;
	vec3 worldpos = (alpha * world_coords[0] / clip_coords[0].w + beta * world_coords[1] / clip_coords[1].w +
		gamma * world_coords[2] / clip_coords[2].w) * Z;

	if(payload.model->normalmap_tangent)
		normal = payload.model->normal_tangent(normal, world_coords, uvs, uv);
	

	//phone 光照模型系数 ka kd ks
	vec3 ka{ 0.5,0.5,0.5 };  //环境光系数 L_a=k_a.I_a
	vec3 kd = payload.model->diffuse(uv); //漫反射系数（颜色）L_d=k_d(I/r^2).max(0,n.l)
	vec3 ks(0.8, 0.8, 0.8);
	if(payload.model->specmap)
		ks=payload.model->specular(uv); //镜面反射系数 L_s=k_s(I/r^2).max(0,n.h)^p

	//设置光照信息得到I
	float p = 150.0;
	vec3 l = (payload.light.pos-worldpos).normalized();
	vec3 light_ambient_intensity = kd;
	vec3 light_diffuse_intensity = vec3(1, 1, 1);
	vec3 light_specular_intensity = vec3(0.1, 0.1, 0.1);

	//在世界坐标下计算光照
	vec3 result_color{ 0,0,0 };
	vec3 ambient, diffuse, specular;
	normal = normal.normalized();
	//计算半程向量
	vec3 v = (payload.camera->eye - worldpos).normalized();
	vec3 h = (l + v).normalized();

	//计算各个光照分量
	ambient = cwise_product(ka, light_ambient_intensity);
	diffuse = cwise_product(kd, light_diffuse_intensity) * std::max(double(0),  normal*l);
	specular = cwise_product(ks, light_specular_intensity) * std::max(double(0), std::pow( normal*h, p));


	//1、直接使用插值后的世界坐标计算
	vec4 light_clip_pos = payload.light_perp_matrix * payload.light_view_matrix * to_vec4(worldpos);
	vec3 screen_pos_light{ 0.5 * (WIDTH ) * (light_clip_pos.x + 1.0),0.5 * (HEIGHT ) * (light_clip_pos.y + 1.0),light_clip_pos.z };
	int index = int(screen_pos_light.x+0.5) + int(screen_pos_light.y+0.5) * WIDTH;
	float cur_shadow_depth = -light_clip_pos.z;
	float shadow = 0.f;
	float bias = float_max(0.5f * (1.0f - normal*l), 0.1f);
	//if (cur_shadow_depth-bias  > shadowmap[index]) shadow = 1.0f;
	//实现PCF算法 优化阴影边缘  多次采样 然后模糊操作
	float shadowSampleSize = 2;
	for (float i = -shadowSampleSize; i <= shadowSampleSize; i += 1.0) {
		for (float j = -shadowSampleSize; j <= shadowSampleSize; j += 1.0) {
			vec2 samplePoint = vec2(screen_pos_light.x + i, screen_pos_light.y + j);
			int sampleIndex = int(samplePoint.x + 0.5) + int(samplePoint.y + 0.5) * WIDTH;
			float sampleDepth = -light_clip_pos.z;

			if (sampleDepth - bias > shadowmap[sampleIndex]) {
				shadow += 1.0;
			}
		}
	}

	// Normalize the accumulated shadow value
	shadow /= pow((2.0 * shadowSampleSize + 1.0), 2.0);
	result_color = ambient +(1-shadow)*(1.2*diffuse +0.8*specular);
	return result_color * 255.f;
}

