#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include<io.h>
#include "simplemodel.h"


SimpleModel::SimpleModel(const char* filename, int is_skybox):is_skybox(is_skybox)
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        //compare比较相同返回0 
        //顶点
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts_.push_back(v);
        }
        //顶点法向量
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec3 n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        }
        //顶点纹理坐标
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            for (int i = 0; i < 2; i++) iss >> uv[i];
            uv_.push_back(uv);
        }
        else if (!line.compare(0, 2, "f ")) {

            std::vector<vec3> f;
            vec3 tmp;
            iss >> trash;
            while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
                for (int i = 0; i < 3; i++) tmp[i]--; //因为原来的索引都是从1开始的，实际数组里从0开始
                f.push_back(tmp);
            }
            faces_.push_back(f);
        }
    }

    environment_map =NULL;
    if (is_skybox) {
        environment_map = new cubemap_t();
        load_cubemap(filename);
    }
    else {
        load_textures(filename);
    }

    //std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;

}

SimpleModel::~SimpleModel() {
    if (diffusemap) delete diffusemap; diffusemap = NULL;
    if (normalmap_tangent) delete normalmap_tangent; normalmap_tangent = NULL;
    if (specmap) delete specmap; specmap = NULL;
    if (environment_map) {
        for (int i = 0; i < 6; i++)
            delete environment_map->faces[i];
        delete environment_map;
    }
}

/// <summary>
/// 模型顶点数目
/// </summary>
/// <returns></returns>
int SimpleModel::nverts() {
    return (int)verts_.size();
}

/// <summary>
/// 模型面元数目
/// </summary>
/// <returns></returns>
int SimpleModel::nfaces() {
    return (int)faces_.size();
}

/// <summary>
/// 每个三角形面元由三个顶点组成，每个顶点包含三个属性 第一个属性就是顶点坐标索引
/// 这里访问信息 构建三个顶点的vector返回 只包含顶点坐标索引
/// </summary>
/// <param name="idx"></param>
/// <returns>返回一个面片的顶点</returns>
std::vector<int> SimpleModel::face(int idx) {
    std::vector<int> face;
    for (int i = 0; i < (int)faces_[idx].size(); i++) face.push_back(faces_[idx][i][0]);
    return face;
}


/// <summary>
/// 输入顶点坐标索引 返回实际的顶点坐标
/// </summary>
/// <param name="i">输入顶点坐标索引 </param>
/// <returns>返回实际的顶点坐标</returns>
vec3 SimpleModel::vert(int i) {
    return verts_[i];
}
vec3 SimpleModel::vert(int iface, int nvert) {
    return verts_[faces_[iface][nvert][0]];
}
void SimpleModel::load_texture(std::string filename, const char* suffix, TGAImage& img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

void SimpleModel::load_cubemap(const char* filename) {
    //按x，y，z正负顺序排列 右手系
    environment_map->faces[0] = new TGAImage();
    load_texture(filename, "_right.tga", *environment_map->faces[0]);
    environment_map->faces[1] = new TGAImage();
    load_texture(filename, "_left.tga", *environment_map->faces[1]);
    environment_map->faces[2] = new TGAImage();
    load_texture(filename, "_top.tga", *environment_map->faces[2]);
    environment_map->faces[3] = new TGAImage();
    load_texture(filename, "_bottom.tga", *environment_map->faces[3]);
    environment_map->faces[4] = new TGAImage();
    load_texture(filename, "_back.tga", *environment_map->faces[4]);
    environment_map->faces[5] = new TGAImage();
    load_texture(filename, "_front.tga", *environment_map->faces[5]);
}

void SimpleModel::load_textures(const char* filename)
{
    diffusemap = NULL;
    normalmap_tangent = NULL;
    specmap = NULL;
    
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    texfile = texfile.substr(0, dot) + std::string("_diffuse.tga");
    if (_access(texfile.data(), 0) != -1) {
        diffusemap = new TGAImage();
        load_texture(filename, "_diffuse.tga", *diffusemap);
    }

    texfile = texfile.substr(0, dot) + std::string("_nm_tangent.tga");
    if (_access(texfile.data(), 0) != -1) {
        normalmap_tangent = new TGAImage();
        load_texture(filename, "_nm_tangent.tga", *normalmap_tangent);
    }

    texfile = texfile.substr(0, dot) + std::string("_spec.tga");
    if (_access(texfile.data(), 0) != -1) {
        specmap = new TGAImage();
        load_texture(filename, "_spec.tga", *specmap);
    }

}



vec3 SimpleModel::diffuse(vec2 uv) {
    //规范到0，1
    uv[0] = fmod(uv[0], 1);
    uv[1] = fmod(uv[1], 1);
    //获取实际贴图大小
    int uv0 = uv[0] * diffusemap->get_width();
    int uv1 = uv[1] * diffusemap->get_height();

    //获取颜色
    TGAColor c = diffusemap->get(uv0, uv1);
    vec3 res;
    //赋值vec3 返回
    for (int i = 0; i < 3; i++)
        res[2 - i] = (float)c[i] / 255.f;
    return res;
}

vec3 SimpleModel::specular(vec2 uv) {
    //规范到0，1
    uv[0] = fmod(uv[0], 1);
    uv[1] = fmod(uv[1], 1);
    //获取实际贴图大小
    int uv0 = uv[0] * specmap->get_width();
    int uv1 = uv[1] * specmap->get_height();

    //获取颜色
    TGAColor c = specmap->get(uv0, uv1);
    vec3 res;
    //赋值vec3 返回
    for (int i = 0; i < 3; i++)
        res[2 - i] = (float)c[i] / 255.f;
    return res;
}

/// <summary>
/// 计算切线空间 将切线空间法向量转为实际向量
/// 切线空间是法线贴图的本地空间 法线贴图向量就是正z方向
/// 此外还需要计算出切线和副切线向量  需要世界坐标和纹理坐标
/// https://learnopengl-cn.github.io/05%20Advanced%20Lighting/04%20Normal%20Mapping/
/// </summary>
/// <param name="normal"></param>
/// <param name="world_coords"></param>
/// <param name="uvs"></param>
/// <param name="uv"></param>
/// <returns></returns>
vec3 SimpleModel::normal_tangent(vec3& normal, vec3* world_coords, const vec2* uvs,vec2& uv){
    //先计算三角形的两条边 和 deltaUV坐标
    vec3 edge1 = world_coords[2] - world_coords[0];
    vec3 edge2 = world_coords[1] - world_coords[0];
    vec2 deltaUV1 = uvs[2] - uvs[0];
    vec2 deltaUV2 = uvs[1] - uvs[0];
    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    vec3 tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
   
    vec3 bitangent;
    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
 
    
    normal = normal.normalized();
    //施密特正交化
    tangent = (tangent - (tangent * normal) * normal).normalized();
    bitangent = (bitangent - (bitangent * normal) * normal - (bitangent * tangent) * tangent).normalized();

    vec3 sample;
    //规范到0，1
    uv[0] = fmod(uv[0], 1);
    uv[1] = fmod(uv[1], 1);
    //获取实际贴图大小
    int uv0 = uv[0] * normalmap_tangent->get_width();
    int uv1 = uv[1] * normalmap_tangent->get_height();

    //获取颜色
    TGAColor c = normalmap_tangent->get(uv0, uv1);
  
    for (int i = 0; i < 3; i++)
        sample[2 - i] = (float)c[i] / 255.f;
    
    //法线向量范围从0，1到-1，1
    sample = vec3(sample[0] * 2 - 1, sample[1] * 2 - 1, sample[2] * 2 - 1);
    vec3 normal_real = tangent * sample[0] + bitangent * sample[1] + normal * sample[2];
    return normal_real;
}


/// <summary>
/// 得到uv坐标 用这个结合diffuse会出现奇怪的效果 
/// </summary>
/// <param name="iface">面元的索引</param>
/// <param name="nvert">面元中第几个顶点</param>
/// <returns></returns>
vec2 SimpleModel::uv(int iface, int nvert) {
    int idx = faces_[iface][nvert][1];
    vec2 result = { uv_[idx].x * diffusemap->get_width(), uv_[idx].y * diffusemap->get_height() };
    return result;
}

vec2 SimpleModel::uv01(int iface, int nvert) {
    int idx = faces_[iface][nvert][1];
    vec2 result = { uv_[idx].x , uv_[idx].y  };
    return result;
}


/// <summary>
/// 得到顶点的法线向量
/// </summary>
/// <param name="iface">面元的索引</param>
/// <param name="nvert">面元中第几个顶点</param>
/// <returns>归一化的法线</returns>
vec3 SimpleModel::normal(int iface, int nvert) {
    int idx = faces_[iface][nvert][2];
    return norms_[idx].normalized();
}
