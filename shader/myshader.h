#pragma once
#include "../core/geometry.h"
#include "../core/tgaimage.h"
#include "../camera/camera.h"
#include "../core/simplemodel.h"
#include "../core/macro.h"
/// <summary>
/// 定义光线 包括位置和强度
/// </summary>
struct light {
    vec3 pos;
    vec3 intensity;
};

extern vec3 light_dir;
extern const int WIDTH;
extern const int HEIGHT;
extern float* shadowmap;


typedef struct payload{
    mat4 model_matrix;
    mat4 camera_view_matrix;
    mat4 light_view_matrix;
    mat4 camera_perp_matrix;
    mat4 light_perp_matrix;
    mat4 mvp_matrix;
    
    Camera* camera;
    SimpleModel* model;

    //顶点属性
    vec3 norm_attrix[3];
    vec2 uv_attri[3];
    vec3 worldcoord_attri[3];
    vec4 clipcoord_attri[3];
    vec4 clipcoord_attri_light[3];


    //齐次裁剪  
    vec3 in_normal[MAX_VERTEX];
    vec2 in_uv[MAX_VERTEX];
    vec3 in_worldcoord[MAX_VERTEX];
    vec4 in_clipcoord[MAX_VERTEX];
    vec3 out_normal[MAX_VERTEX];
    vec2 out_uv[MAX_VERTEX];
    vec3 out_worldcoord[MAX_VERTEX];
    vec4 out_clipcoord[MAX_VERTEX];

    light light; //光照信息
    //for ibl 
    iblmap_t* iblmap;
}payload_t;


struct IShader {
    virtual ~IShader()=0;
    payload_t payload;
    virtual void vertex(int iface, int nthvert) = 0;
    virtual vec3 fragment(vec3 bar) = 0;
  
};


class PhongShader :public IShader {
public :
    void vertex(int iface, int nthvert) override;
    vec3 fragment(vec3 bar) override;
    
};


class ShadowShader :public IShader {
public:
    void vertex(int iface, int nthvert) override;
    vec3 fragment(vec3 bar) override;
};

class SkyboxShader :public IShader {
public:
    void vertex(int iface, int nthvert) override;
    vec3 fragment(vec3 bar) override;
};


static vec4 inline to_vec4(const vec3& t)
{
    return vec4{ t.x,t.y,t.z,1.0 };
}

static vec3 inline to_vec3(const vec4& t)
{
    return vec3{ t.x,t.y,t.z };
}