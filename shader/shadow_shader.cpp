#include"myshader.h"
void ShadowShader::vertex(int nfaces, int nvertex)
{
	vec4 temp_vert = to_vec4(payload.model->vert(nfaces, nvertex));
	payload.clipcoord_attri[nvertex] = payload.mvp_matrix * temp_vert;

}

vec3 ShadowShader::fragment(vec3 bar) {
	return { 0,0,0 };
}
