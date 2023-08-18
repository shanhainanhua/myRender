#pragma once
#include "../core/tgaimage.h"
#include "../core/geometry.h"
#include "../shader/myshader.h"
#include "../camera/camera.h"
#include "../core/macro.h"

extern Camera camera;
extern const int WIDTH;
extern const int HEIGHT;
extern float* shadowmap;

//mat4 viewport(int x, int y, int w, int h);
//void projection(float coeff = 0.f); // coeff = -1/c
//void lookat(vec3 eye, vec3 center, vec3 up);
//void update_matrix(Camera camera);
void update_matrix(Camera& camera, mat4 view_mat, mat4 perspective_mat,IShader* shader_model, IShader* shader_skybox);
mat4 mat4_lookat(vec3 eye, vec3 target, vec3 up);
mat4 mat4_ortho(float left, float right, float bottom, float top,
    float near, float far);
mat4 mat4_perspective(float fovy, float aspect, float near, float far);
void draw_triangles(unsigned char* framebuffer, float* zbuffer, IShader& shader, int nface,int calshadow);
//void triangle(vec4* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);
//void triangle(vec4* pts, IShader& shader, float* zbuffer, unsigned char* framebuffer);
//void triangle_MSAA(vec4* pts, IShader& shader, TGAImage& image, TGAImage& superimage, TGAImage& superzbuffer);
void rasterize_shadow(vec4* clipcoord_attri, unsigned char* framebuffer, float* shadowbuffer, IShader& shader);
void draw_triangles_shadow(unsigned char* framebuffer, float* shadowbuffer, IShader& shader, int nface);