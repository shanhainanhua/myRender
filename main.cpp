#include "core/simplemodel.h"
#include "shader/myshader.h"
#include "core/tgaimage.h"
#include "platform/win32.h"
#include"camera/camera.h"
#include"raster/raster.h"
#include<io.h>
#include<fstream>
#include<iostream>
#include<stdlib.h>

using std::string;
using std::vector;
const int WIDTH=600;
const int HEIGHT =600;

//指定光照方向
vec3 light_dir = { 5, 5,5 };
//摄像机位置
vec3 eye{ 0, 0, 10 };
//焦点位置
vec3 center{ 0, 0, 0 };
vec3 up{ 0,1,0 };
//光源视角下的矩阵

mat4 light_view_mat = mat4_lookat(light_dir.normalized(), center, up);
mat4 light_prep_mat = mat4_ortho(-5, 5, -5, 5, -0.1, -5);
//深度缓冲
float* zbuffer = (float*)malloc(sizeof(float) * WIDTH * HEIGHT);
//阴影贴图
float* shadowmap = (float*)malloc(sizeof(float) * WIDTH * HEIGHT);
unsigned char* framebuffer = (unsigned char*)malloc(sizeof(unsigned char) * WIDTH * HEIGHT * 4);


void clear_zbuffer(int width, int height, float* zbuffer);
void clear_framebuffer(int width, int height, unsigned char* framebuffer);



void getAllFiles(string path, vector<string>& files, string fileType)
{     //文件句柄  
    intptr_t hFile = 0;
    struct _finddata_t  fileInfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("/*" + fileType).c_str(), &fileInfo)) != -1)
    {
        do
        {
            files.push_back(p.assign(path).append("/").append(fileInfo.name));
        } while (_findnext(hFile, &fileInfo) == 0);
        _findclose(hFile);//关闭句柄  
    }
}

void loadScene(string path, vector<SimpleModel*> &models)
{
    vector<string> tmp;
    getAllFiles(path, tmp, ".obj");
    for (auto item : tmp) {
        models.emplace_back( new SimpleModel(item.c_str(),0));
    }
}
void renderShadow(vector<SimpleModel*>& models, IShader* shader) {
    for (auto model : models) {
        shader->payload.model = model;
        for (int i = 0; i < model->nfaces(); i++) {
            draw_triangles_shadow(framebuffer, shadowmap, *shader, i);
        }
    }
}

void renderScene(vector<SimpleModel*> &models,IShader* shader)
{
    for (auto model : models) {
        shader->payload.model = model;
        for (int i = 0; i < model->nfaces(); i++) {
            draw_triangles(framebuffer, zbuffer, *shader, i, 0);
        }
    }
}
void renderSky(SimpleModel* model, IShader* shader)
{
    shader->payload.model = model;
    for (int i = 0; i < model->nfaces(); i++) {
        draw_triangles(framebuffer, zbuffer, *shader, i, 0);
    }
}




int main(int argc, char** argv) {

    Camera camera(eye, center, up, (float)(WIDTH) / HEIGHT);
    mat4 model_mat = mat4::identity();
    mat4 view_mat = mat4_lookat(camera.eye, camera.target, camera.up);
    mat4 perspective_mat = mat4_perspective(60, (float)(WIDTH) / HEIGHT, -0.1, -10000);

    //窗口初始化
    window_init(WIDTH, HEIGHT,"MyRender");
    //循环渲染
    int num_frames = 0;
    float print_time = platform_get_time();

    IShader* shader_model = new PhongShader();
    shader_model->payload.camera = &camera;
    shader_model->payload.camera_perp_matrix = perspective_mat;
    shader_model->payload.light_perp_matrix = light_prep_mat;
    shader_model->payload.light_view_matrix = light_view_mat;
    shader_model->payload.light = light{ light_dir,vec3{1,1,1} };

    IShader* shader_shadow = new ShadowShader();

    shader_shadow->payload.camera_perp_matrix = light_prep_mat;
    shader_shadow->payload.camera_view_matrix = light_view_mat;
    shader_shadow->payload.mvp_matrix = light_prep_mat * light_view_mat;


    IShader* shader_skybox = new SkyboxShader();
    shader_skybox->payload.camera = &camera;
    shader_skybox->payload.camera_perp_matrix = perspective_mat;

    vector<SimpleModel*> models;
    loadScene("E:/learn_for_job/project/myrender/obj/african_head", models);
    loadScene("E:/learn_for_job/project/myrender/obj/floor", models);

    SimpleModel* model_sky = new SimpleModel("E:/learn_for_job/project/myrender/obj/skybox2/box.obj",1);
 

    while (!window->is_close)
    {
        float curr_time = platform_get_time();
        //清空缓存
        clear_framebuffer(WIDTH, HEIGHT, framebuffer);
        clear_zbuffer(WIDTH, HEIGHT, zbuffer);
        clear_zbuffer(WIDTH, HEIGHT, shadowmap);
        //根据鼠标移动 更新mvp矩阵
        handle_events(camera);
        update_matrix(camera, view_mat, perspective_mat, shader_model, shader_skybox);
        //渲染天空盒
        renderSky(model_sky, shader_skybox);


       //生成shadowmap
        renderShadow(models,shader_shadow);
        renderScene(models,shader_model);
  
        num_frames += 1;
        if (curr_time - print_time >= 1) {
            int sum_millis = (int)((curr_time - print_time) * 1000);
            int avg_millis = sum_millis / num_frames;
            printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
            num_frames = 0;
            print_time = curr_time;
        }
        window->mouse_info.wheel_delta = 0;
        window->mouse_info.orbit_deleta = vec2{ 0,0 };
        window->mouse_info.fv_delta = vec2{ 0,0 };
        window_draw(framebuffer);
        msg_dispatch();
    }

    free(zbuffer);
    free(framebuffer);
    free(shadowmap);
    window_destroy();
    for (auto item : models) delete item;
    delete shader_model;
    delete shader_shadow;
    system("pause");
    return 0;
}


void clear_zbuffer(int width, int height, float* zbuffer)
{
    for (int i = 0; i < width * height; i++)
        zbuffer[i] = 100000.f;
}

void clear_framebuffer(int width, int height, unsigned char* framebuffer)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int index = (i * width + j) * 4;

            framebuffer[index + 2] = 80;
            framebuffer[index + 1] = 0;
            framebuffer[index] = 0;
        }
    }
}
