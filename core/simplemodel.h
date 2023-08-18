#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"
#include <random>
typedef struct cubemap {
	TGAImage* faces[6];
}cubemap_t;
typedef struct iblmap {
	int mip_levels;
	cubemap_t* irradiance_map;
	cubemap_t* prefileter_maps[15];
	TGAImage* brdf_lut;
}iblmap_t;


class SimpleModel {
public:
	cubemap_t* environment_map;//天空盒贴图
	TGAImage* diffusemap; //漫反射纹理
	TGAImage* normalmap_tangent; //切线空间法线纹理
	TGAImage* specmap; //镜面反射纹理图
	int is_skybox;
private:
	std::vector<vec3> verts_; //顶点
	//std::vector<std::vector<int> > faces_; //面 由三个顶点组成 
	std::vector<std::vector<vec3> > faces_; //改成存三个vertex/uv/normal
	std::vector<vec3> norms_; //法线向量
	std::vector<vec2> uv_; //纹理uv


public:
	SimpleModel(const char* filename,int is_skybox);
	~SimpleModel();
	int nverts();
	int nfaces();
	vec3 vert(int i);
	vec3 vert(int iface, int nvert);
	std::vector<int> face(int idx);
	vec2 uv(int iface, int nvert);
	vec2 uv01(int iface, int nvert);
	vec3 normal(int iface, int nvert);
	vec3 diffuse(vec2 uv);
	vec3 specular(vec2 uv);
	vec3 normal_tangent(vec3& normal, vec3* world_coords, const vec2* uvs, vec2& uv);
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
	void load_cubemap(const char* filename);
	void load_textures(const char* filename);

};

#endif //__MODEL_H__