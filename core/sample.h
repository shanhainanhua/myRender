#pragma once
#include"../shader/myshader.h"
//用于纹理采样 目前主要是采样skybox纹理

vec3 cubemap_sample(vec3 direction, cubemap_t* cubemap);
vec3 texture_sample(vec2 uv, TGAImage* image);