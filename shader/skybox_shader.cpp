#include"myshader.h"
#include"../core/sample.h"
void SkyboxShader::vertex(int nfaces, int nvertex)
{
	vec4 temp_vert = to_vec4(payload.model->vert(nfaces, nvertex));

	payload.clipcoord_attri[nvertex] = payload.mvp_matrix * temp_vert;
	payload.in_clipcoord[nvertex] = payload.clipcoord_attri[nvertex];

	for (int i = 0; i < 3; i++) {
		payload.worldcoord_attri[nvertex][i] = temp_vert[i];
		payload.in_worldcoord[nvertex][i] = temp_vert[i];
	}
}

vec3 SkyboxShader::fragment(vec3 bar) {
	float alpha = bar.x, beta = bar.y, gamma = bar.z;
	vec4* clip_coords = payload.clipcoord_attri;
	vec3* world_coords = payload.worldcoord_attri;

	//根据重心插值计算属性 注意透视矫正  
	//透视矫正  推导：https://zhuanlan.zhihu.com/p/144331875
	float Z = 1.0 / (alpha / clip_coords[0].w + beta / clip_coords[1].w + gamma / clip_coords[2].w);

	vec3 worldpos = (alpha * world_coords[0] / clip_coords[0].w + beta * world_coords[1] / clip_coords[1].w +
		gamma * world_coords[2] / clip_coords[2].w) * Z;

	vec3 result_color;
	result_color = cubemap_sample(worldpos, payload.model->environment_map);
	return result_color * 255.f;
}