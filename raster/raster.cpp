
#include "raster.h"



//视角矩阵
//mat4 viewport(int x, int y, int w, int h) {
//    Viewport = mat4::identity();
//    //第4列表示平移信息
//    Viewport[0][3] = x + w / 2.f;
//    Viewport[1][3] = y + h / 2.f;
//    Viewport[2][3] = 255.f / 2.f;
//    //对角线表示缩放信息
//    Viewport[0][0] = w / 2.f;
//    Viewport[1][1] = h / 2.f;
//    Viewport[2][2] = 255.f / 2.f;
//    return Viewport;
//}

////投影矩阵
//void projection(float coeff) {
//    Projection = mat4::identity();
//    Projection[3][2] = coeff;
//}
//
//
////朝向矩阵，变换矩阵
////更改摄像机视角=更改物体位置和角度，操作为互逆矩阵
////摄像机变换是先旋转再平移，所以物体需要先平移后旋转，且都是逆矩阵
//void lookat(vec3 eye, vec3 center, vec3 up)
//{
//    //摄像机始终看向自己的z轴负方向
//    //先计算它自己的z轴
//    vec3 z = (eye - center).normalized();
//    vec3 x = cross(up, z).normalized();
//    vec3 y = cross(z, x).normalized();
//    mat4 rotation = mat4::identity();
//    mat4 translation = mat4::identity();
//    //先算旋转矩阵
//    for (int i = 0; i < 3; i++) {
//        rotation[0][i] = x[i];
//        rotation[1][i] = y[i];
//        rotation[2][i] = z[i];
//    }
//    //算位移
//    for (int i = 0; i < 3; i++) {
//        translation[i][3] = -center[i];
//    }
//    ModelView = mat4::identity();
//    //先平移物体后旋转
//    ModelView = rotation * translation;
//}




mat4 mat4_lookat(vec3 eye, vec3 target, vec3 up)
{
    mat4 m = mat4::identity();

    vec3 z = (eye - target).normalized();
    vec3 x = cross(up, z).normalized();
    vec3 y =cross(z, x).normalized();

    m[0][0] = x[0];
    m[0][1] = x[1];
    m[0][2] = x[2];

    m[1][0] = y[0];
    m[1][1] = y[1];
    m[1][2] = y[2];

    m[2][0] = z[0];
    m[2][1] = z[1];
    m[2][2] = z[2];

    m[0][3] = -(x*eye); //相当于原来要位移的，在新的坐标系下是位移多少，有个改变
    m[1][3] = -(y*eye);
    m[2][3] = -(z*eye);

    return m;
}


mat4 mat4_ortho(float left, float right, float bottom, float top,
    float near, float far)
{
    float x_range = right - left;
    float y_range = top - bottom;
    float z_range = near - far;  //care the different 
    mat4 m = mat4::identity();
    m[0][0] = 2 / x_range;
    m[1][1] = 2 / y_range;
    m[2][2] = 2 / z_range;
    m[0][3] = -(left + right) / x_range;
    m[1][3] = -(bottom + top) / y_range;
    m[2][3] = -(near + far) / z_range;
    return m;
}

/*
 * fovy: the field of view angle in the y direction, in degrees
 * aspect: the aspect ratio, defined as width divided by height
 * near, far: the coordinates for the near and far clipping planes
 *
 * 1/(aspect*tan(fovy/2))              0             0           0
 *                      0  1/tan(fovy/2)             0           0
 *                      0              0  -(f+n)/(f-n)  -2fn/(f-n)
 *                      0              0            -1           0
 *
 * this is the same as
 *     float half_h = near * (float)tan(fovy / 2);
 *     float half_w = half_h * aspect;
 *     mat4_frustum(-half_w, half_w, -half_h, half_h, near, far);
 *
 * see http://www.songho.ca/opengl/gl_projectionmatrix.html
 *
 * note: my implementation is based on right-handed system, so it is a little different
 */
mat4 mat4_perspective(float fovy, float aspect, float near, float far)
{
    mat4 m = mat4::identity();
    fovy = fovy / 180.0 * PI;
    float t = fabs(near) * tan(fovy / 2);
    float r = aspect * t;

    m[0][0] = near / r;
    m[1][1] = near / t;
    m[2][2] = (near + far) / (near - far);
    m[2][3] = 2 * near * far / (far - near);
    m[3][2] = 1;
    m[3][3] = 0;
    return m;
}




void update_matrix(Camera& camera, mat4 view_mat, mat4 perspective_mat, IShader* shader_model, IShader* shader_skybox)
{
    view_mat = mat4_lookat(camera.eye, camera.target, camera.up);
    mat4 mvp = perspective_mat * view_mat;
    shader_model->payload.camera_view_matrix = view_mat;
    shader_model->payload.mvp_matrix = mvp;
    
    if (shader_skybox != NULL)
    {
        mat4 view_skybox = view_mat;
        view_skybox[0][3] = 0;
        view_skybox[1][3] = 0;
        view_skybox[2][3] = 0;
        shader_skybox->payload.camera_view_matrix = view_skybox;
        shader_skybox->payload.mvp_matrix = perspective_mat * view_skybox;
    }
}


static int get_index(int x, int y) {
    return (HEIGHT - y - 1) * WIDTH + x;
}

static int get_shadowmap_index(int x, int y) {
    return x * WIDTH + y;
}


static void set_color(unsigned char* framebuffer, int x, int y, TGAColor& color)
{
    int index = get_index(x, y) * 4;

    framebuffer[index] = color.bgra[2];
    framebuffer[index + 1] = color.bgra[1];
    framebuffer[index + 2] = color.bgra[0];
    framebuffer[index + 3] = color.bgra[3];
}
static void set_color(unsigned char* framebuffer, int x, int y, unsigned char color[])
{
    int i;
    int index = ((HEIGHT - y - 1) * WIDTH + x) * 4; // the origin for pixel is bottom-left, but the framebuffer index counts from top-left

    for (i = 0; i < 3; i++)
        framebuffer[index + i] = color[i];
}
static vec3 barycentric(const vec2& A, const vec2& B, const vec2& C, const vec2& P) {
    vec3 s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    //[u,v,1]和[AB,AC,PA]对应的x和y向量都垂直，所以叉乘
    vec3 u = cross(s[0], s[1]);
    //三点共线时，会导致u[2]为0，此时返回(-1,1,1)
    vec3 res;
    if (std::abs(u[2]) > 1e-2)
        //若1-u-v，u，v全为大于0的数，表示点在三角形内部
        res = { 1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z };
    else
        res = { -1,1,1 };
    return res;
}
static vec3 compute_barycentric2D(float x, float y, const vec3* v)
{
    float c1 = (x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * y + v[1].x * v[2].y - v[2].x * v[1].y) / (v[0].x * (v[1].y - v[2].y) + (v[2].x - v[1].x) * v[0].y + v[1].x * v[2].y - v[2].x * v[1].y);
    float c2 = (x * (v[2].y - v[0].y) + (v[0].x- v[2].x) * y + v[2].x * v[0].y - v[0].x * v[2].y) / (v[1].x * (v[2].y - v[0].y) + (v[0].x - v[2].x) * v[1].y + v[2].x * v[0].y - v[0].x * v[2].y);
    return vec3(c1, c2, 1 - c1 - c2);
}


static int is_back_facing(vec3 ndc_pos[3])
{
    vec3 a = ndc_pos[0];
    vec3 b = ndc_pos[1];
    vec3 c = ndc_pos[2];
    float signed_area = a.x * b.y - a.y * b.x + b.x * c.y - b.y * c.x + c.x * a.y - c.y * a.x;
    return signed_area <= 0;
}


typedef enum {
    W_PLANE,
    X_RIGHT,
    X_LEFT,
    Y_TOP,
    Y_BOTTOM,
    Z_NEAR,
    Z_FAR
}clip_plane;

static int is_inside_plane(clip_plane c_plane, vec4 vertex)
{
    //根据GAMES101的推导 右手系 znear zfar用的是坐标值 是负数 相机在z轴正向看向负z
    //opengl里用的是距离近平面和远平面的距离都是正数
    //最后得到裁剪空间里的w值是负的 那么判断点是否在面内就是判断满足【w，-w】
    //这段代码实现了视锥体的剪裁，它对顶点进行分类，判断每个顶点是否在视锥体内部。顶点在视锥体内部的条件是：顶点在视锥体的所有裁剪平面的同侧，即在平面的法向量指向的一侧。在这段代码中，裁剪平面的类型是通过枚举类型clip_plane来定义的。

    //    下面对代码中每个裁剪平面的计算方法进行详细解释：

    //    裁剪空间的W分量是相机空间中的z坐标，相机空间中的视点是位于z轴的负半轴方向，因此所有z值为负的点都是相机视锥体的内部点。在这段代码中，将W_PLANE定义为裁剪平面，因此只需要判断顶点的W分量是否小于等于 - EPSILON（EPSILON是一个极小的正数），即可判断该顶点是否在相机视锥体内部。
    //在视锥体坐标系中，相机位于原点，且相机坐标系的x轴指向右侧，y轴指向上方，z轴指向相机的观察方向。在这个坐标系中，视锥体的左侧面和右侧面分别位于x = -w和x = w处。因此，对于位于视锥体内部的顶点，它们的x坐标应该在 - w和w之间。因为顶点的投影点是在相机坐标系中计算的，所以在检查X_RIGHT裁剪平面时，只需检查x坐标是否大于等于w
    switch (c_plane)
    {
    case W_PLANE:
        return vertex.w <= -EPSILON;
    case X_RIGHT:
        return vertex.x >= vertex.w;
    case X_LEFT:
        return vertex.x <= -vertex.w;
    case Y_TOP:
        return vertex.y >= vertex.w;
    case Y_BOTTOM:
        return vertex.y <= -vertex.w;
    case Z_NEAR:
        return vertex.z >= vertex.w;
    case Z_FAR:
        return vertex.z <= -vertex.w;
    default:
        return 0;
    }
}


static float get_intersect_ratio(vec4 prev, vec4 curv, clip_plane c_plane)
{
    switch (c_plane)
    {
    case W_PLANE:
        return prev.w  / (prev.w - curv.w);
    case X_RIGHT:
        return (prev.w - prev.x) / ((prev.w - prev.x) - (curv.w - curv.x));
    case X_LEFT:
        return (prev.w + prev.x) / ((prev.w + prev.x) - (curv.w + curv.x));
    case Y_TOP:
        return (prev.w - prev.y) / ((prev.w - prev.y) - (curv.w - curv.y));
    case Y_BOTTOM:
        return (prev.w + prev.y) / ((prev.w + prev.y) - (curv.w + curv.y));
    case Z_NEAR:
        return (prev.w - prev.z) / ((prev.w - prev.z) - (curv.w - curv.z));
    case Z_FAR:
        return (prev.w + prev.z) / ((prev.w + prev.z) - (curv.w + curv.z));
    default:
        return 0;
    }
}

static int clip_with_plane(clip_plane c_plane, int num_vert, payload_t& payload) {
    int out_vert_num = 0;
    int pre_index, cur_index;
    int is_odd = (c_plane + 1) % 2;

    vec4* in_clipcoord = is_odd ? payload.in_clipcoord : payload.out_clipcoord;
    vec3* in_worldcoord = is_odd ? payload.in_worldcoord : payload.out_worldcoord;
    vec3* in_normal = is_odd ? payload.in_normal : payload.out_normal;
    vec2* in_uv = is_odd ? payload.in_uv : payload.out_uv;
    vec4* out_clipcoord = is_odd ? payload.out_clipcoord : payload.in_clipcoord;
    vec3* out_worldcoord = is_odd ? payload.out_worldcoord : payload.in_worldcoord;
    vec3* out_normal = is_odd ? payload.out_normal : payload.in_normal;
    vec2* out_uv = is_odd ? payload.out_uv : payload.in_uv;

    for (int i = 0; i < num_vert; i++)
    {
        cur_index = i;
        pre_index = (i - 1 + num_vert) % num_vert;
        vec4 cur_vertex = in_clipcoord[cur_index];
        vec4 pre_vertex = in_clipcoord[pre_index];

        int is_cur_inside = is_inside_plane(c_plane, cur_vertex);
        int is_pre_inside = is_inside_plane(c_plane, pre_vertex);
        if (is_cur_inside != is_pre_inside)
        {
            float ratio = get_intersect_ratio(pre_vertex, cur_vertex, c_plane);
            out_clipcoord[out_vert_num] = vec_lerp<vec4>(pre_vertex, cur_vertex, ratio);
            out_worldcoord[out_vert_num] = vec_lerp<vec3>(in_worldcoord[pre_index],in_worldcoord[cur_index],ratio);
            out_normal[out_vert_num] = vec_lerp<vec3>(in_normal[pre_index], in_normal[cur_index], ratio);
            out_uv[out_vert_num] = vec_lerp<vec2>(in_uv[pre_index], in_uv[cur_index], ratio);
            out_vert_num++;
        }
        if (is_cur_inside)
        {
            out_clipcoord[out_vert_num] = cur_vertex;
            out_worldcoord[out_vert_num] = in_worldcoord[cur_index];
            out_normal[out_vert_num] = in_normal[cur_index];
            out_uv[out_vert_num] = in_uv[cur_index];

            out_vert_num++;
        }
    }
    return out_vert_num;
}

static int homo_clipping(payload_t& payload)
{
    int num_vertex = 3;
    for(int i=0;i<=6;i++)
        num_vertex = clip_with_plane(static_cast<clip_plane>(i), num_vertex, payload);
    return num_vertex;
}

static void transform_attri(payload_t& payload, int index0, int index1, int index2)
{
    std::vector<int> indexs{index0,index1,index2};
    for (int i = 0; i < 3; i++) {
        payload.clipcoord_attri[i] = payload.out_clipcoord[indexs[i]];
        payload.worldcoord_attri[i] = payload.out_worldcoord[indexs[i]];
        payload.norm_attrix[i] = payload.out_normal[indexs[i]];
        payload.uv_attri[i] = payload.out_uv[indexs[i]];
    }
}



void rasterize(vec4* clipcoord_attri, unsigned char* framebuffer, float* zbuffer, IShader& shader,int calshadow)
{
    vec3 ndc_pos[3];
    vec3 screen_pos[3];
    unsigned char c[3];
    int width = WIDTH;
    int height = HEIGHT;
    int is_skybox = shader.payload.model->is_skybox;
    //齐次除法 =》ndc
    for (int i = 0; i < 3; i++)
    {
        ndc_pos[i][0] = clipcoord_attri[i][0] / clipcoord_attri[i].w;
        ndc_pos[i][1] = clipcoord_attri[i][1] / clipcoord_attri[i].w;
        ndc_pos[i][2] = clipcoord_attri[i][2] / clipcoord_attri[i].w;
    }


    //视口转换 ndc=》screen [-1,1]=>[0,1]=>[width,height]
    for (int i = 0; i < 3; i++)
    {
        screen_pos[i][0] = 0.5 * (width -1) * (ndc_pos[i][0] + 1.0);
        screen_pos[i][1] = 0.5 * (height -1) * (ndc_pos[i][1] + 1.0);
        screen_pos[i][2] = is_skybox?1000: -clipcoord_attri[i].w;
    }

    //背面剔除
    if (!is_skybox)
        if (is_back_facing(ndc_pos)) return;
    //光栅化 首先设置碰撞盒
    float xmin = std::numeric_limits<float>::max();
    float xmax = std::numeric_limits<float>::min();
    float ymin = std::numeric_limits<float>::max();
    float ymax = std::numeric_limits<float>::min();
    for (int i = 0; i < 3; i++)
    {
        xmin = float_min(xmin, screen_pos[i][0]);
        xmax = float_max(xmax, screen_pos[i][0]);
        ymin = float_min(ymin, screen_pos[i][1]);
        ymax = float_max(ymax, screen_pos[i][1]);
    }

    //光栅化 
    for (int x = static_cast<int>(xmin); x <= static_cast<int>(xmax); x++)
    {
        for (int y = static_cast<int>(ymin); y <= static_cast<int>(ymax); y++)
        {
            //计算重心坐标
            vec3 bar = compute_barycentric2D(static_cast<float>(x + 0.5), static_cast<float>(y + 0.5), screen_pos);
            //判读在当前点在三角形范围内
            if (bar.x>0&&bar.y>0&&bar.z>0) {
                int index = get_index(x, y);
                //透视正确的z插值 深度缓冲
                float normalizer = 1.0 / (bar.x / clipcoord_attri[0].w + bar.y / clipcoord_attri[1].w + bar.z / clipcoord_attri[2].w);
                float z = (bar.x * screen_pos[0].z / clipcoord_attri[0].w + bar.y * screen_pos[1].z / clipcoord_attri[1].w + bar.z * screen_pos[2].z / clipcoord_attri[2].w)* normalizer;
          
    
                if (zbuffer[index] > z) {
                    zbuffer[index] = z;
                    vec3 color = shader.fragment(bar);

                    for (int i = 0; i < 3; i++) {
                        c[i] = static_cast<int>(float_clamp(color[i], 0.0f, 255.f));
                       
                    }
                    set_color(framebuffer, x, y, c);
                }

            }

        }
    }
}



void draw_triangles(unsigned char* framebuffer, float* zbuffer, IShader& shader, int nface,int calshadow)
{
    for (int i = 0; i < 3; i++) {
        shader.vertex(nface, i);
    }
    //齐次裁剪
    int num_vertex = homo_clipping(shader.payload);
    for (int i = 0; i < num_vertex - 2; i++) {
        int index0 = 0;
        int index1 = i + 1;
        int index2 = i + 2;
        transform_attri(shader.payload, index0, index1, index2);
        rasterize(shader.payload.clipcoord_attri, framebuffer, zbuffer, shader,calshadow);
    }
}



void rasterize_shadow(vec4* clipcoord_attri, unsigned char* framebuffer, float* shadowbuffer, IShader& shader)
{
    vec3 ndc_pos[3];
    vec3 screen_pos[3];
    unsigned char c[3];
    int width = WIDTH;
    int height = HEIGHT;

    //齐次除法 =》ndc
    for (int i = 0; i < 3; i++)
    {
        ndc_pos[i][0] = clipcoord_attri[i][0] / clipcoord_attri[i].w;
        ndc_pos[i][1] = clipcoord_attri[i][1] / clipcoord_attri[i].w;
        ndc_pos[i][2] = clipcoord_attri[i][2] / clipcoord_attri[i].w;
    }

    for (int i = 0; i < 3; i++)
    {
        screen_pos[i][0] = 0.5 * (width-1) * (ndc_pos[i][0] + 1.0);
        screen_pos[i][1] = 0.5 * (height-1) * (ndc_pos[i][1] + 1.0);
        screen_pos[i][2] =-clipcoord_attri[i][2];
    }
    //光栅化 首先设置碰撞盒
    float xmin = std::numeric_limits<float>::max();
    float xmax = std::numeric_limits<float>::min();
    float ymin = std::numeric_limits<float>::max();
    float ymax = std::numeric_limits<float>::min();
    for (int i = 0; i < 3; i++)
    {
        xmin = float_min(xmin, screen_pos[i][0]);
        xmax = float_max(xmax, screen_pos[i][0]);
        ymin = float_min(ymin, screen_pos[i][1]);
        ymax = float_max(ymax, screen_pos[i][1]);
    }

    //光栅化 
    for (int x = static_cast<int>(xmin); x <= static_cast<int>(xmax); x++)
    {
        for (int y = static_cast<int>(ymin); y <= static_cast<int>(ymax); y++)
        {
            //计算重心坐标
            vec3 bar = compute_barycentric2D(static_cast<float>(x + 0.5), static_cast<float>(y + 0.5), screen_pos);
            //判读在当前点在三角形范围内
            if (bar.x > 0 && bar.y > 0 && bar.z > 0) {
                int index = x + y * WIDTH;
                //透视正确的z插值 深度缓冲
                float normalizer = 1.0 / (bar.x / clipcoord_attri[0].w + bar.y / clipcoord_attri[1].w + bar.z / clipcoord_attri[2].w);
                float z = (bar.x * screen_pos[0].z / clipcoord_attri[0].w + bar.y * screen_pos[1].z / clipcoord_attri[1].w + bar.z * screen_pos[2].z / clipcoord_attri[2].w) * normalizer;

                if (shadowbuffer[index] > z) {
                    shadowbuffer[index] = z;
                    //vec3 color{ z,255,255 };

                    //for (int i = 0; i < 3; i++) {
                    //    c[i] = static_cast<int>(float_clamp(color[i], 0, 255));
                    //}
                    //set_color(framebuffer, x, y, c);
                }

            }
        }
    }
}


void draw_triangles_shadow(unsigned char* framebuffer, float* shadowbuffer, IShader& shader, int nface)
{

    for (int i = 0; i < 3; i++) {
        shader.vertex(nface, i);
    }
    rasterize_shadow(shader.payload.clipcoord_attri, framebuffer, shadowbuffer, shader);
}





////绘制三角形
//void triangle(vec4* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer) {
//    vec2 bboxmin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
//    vec2 bboxmax(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
//    for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 2; j++) {
//            //这里pts除以了最后一个分量，实现了透视中的缩放，所以作为边界框
//            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
//            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
//        }
//    }
//    //当前像素坐标P，颜色color
//    vec2 P;
//    TGAColor color;
//    //遍历边界框中的每一个像素
//    for (P.x = int(bboxmin.x + 0.5); P.x <= int(bboxmax.x + 0.5); P.x++) {
//        for (P.y = int(bboxmin.y); P.y <= int(bboxmax.y + 0.5); P.y++) {
//            int ztest = 0; //判断是否通过深度测试
//
//            //c为当前P对应的质心坐标
//            //这里pts除以了最后一个分量，实现了透视中的缩放，所以用于判断P是否在三角形内
//            vec3 c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
//            //插值计算P的zbuffer
//            //pts[i]为三角形的三个顶点
//            //pts[i][2]为三角形的z信息(0~255)
//            //pts[i][3]为三角形的投影系数(1-z/c)
//
//            double z_P = (pts[0][2] / pts[0][3]) * c.x + (pts[0][2] / pts[1][3]) * c.y + (pts[0][2] / pts[2][3]) * c.z;
//            int frag_depth = std::max(0, std::min(255, int(z_P + .5)));
//            //P的任一质心分量小于0或者zbuffer小于已有zbuffer，不渲染
//            if (c.x < 0 || c.y < 0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
//            else ztest = 1;
//            if (ztest) {
//                //调用片元着色器计算当前像素颜色
//                bool discard = shader.fragment(c, color);
//                if (!discard) {
//                    //zbuffer
//                    zbuffer.set(P.x, P.y, TGAColor(frag_depth));
//                    //为像素设置颜色
//                    image.set(P.x, P.y, color);
//                }
//            }
//
//        }
//    }
//}
//
//
//
//void triangle(vec4* pts, IShader& shader, float* zbuffer, unsigned char* framebuffer) {
//    vec2 bboxmin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
//    vec2 bboxmax(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
//    for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 2; j++) {
//            //这里pts除以了最后一个分量，实现了透视中的缩放，所以作为边界框
//            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
//            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
//        }
//    }
//    //当前像素坐标P，颜色color
//    vec2 P;
//    TGAColor color;
//    //遍历边界框中的每一个像素
//    for (P.x = int(bboxmin.x + 0.5); P.x <= int(bboxmax.x + 0.5); P.x++) {
//        for (P.y = int(bboxmin.y); P.y <= int(bboxmax.y + 0.5); P.y++) {
//            int ztest = 0; //判断是否通过深度测试
//
//            //c为当前P对应的质心坐标
//            //这里pts除以了最后一个分量，实现了透视中的缩放，所以用于判断P是否在三角形内
//            vec3 c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
//            //插值计算P的zbuffer
//            //pts[i]为三角形的三个顶点
//            //pts[i][2]为三角形的z信息(0~255)
//            //pts[i][3]为三角形的投影系数(1-z/c)
//
//            double z_P = (pts[0][2] / pts[0][3]) * c.x + (pts[0][2] / pts[1][3]) * c.y + (pts[0][2] / pts[2][3]) * c.z;
//
//            //P的任一质心分量小于0或者zbuffer小于已有zb4uffer，不渲染
//            if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer[get_index(P.x, P.y)] < z_P) continue;
//            else ztest = 1;
//            if (ztest) {
//                //调用片元着色器计算当前像素颜色
//                bool discard = shader.fragment(c, color);
//                if (!discard) {
//                    //zbuffer
//                    zbuffer[get_index(P.x, P.y)] = z_P;
//                    //为像素设置颜色
//                    set_color(framebuffer, P.x, P.y, color);
//                }
//            }
//
//        }
//    }
//}

//void triangle_MSAA(vec4* pts, IShader& shader, TGAImage& image, TGAImage& superimage, TGAImage& superzbuffer) {
//    vec2 bboxmin(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
//    vec2 bboxmax(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
//    for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 2; j++) {
//            //这里pts除以了最后一个分量，实现了透视中的缩放，所以作为边界框
//            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
//            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
//        }
//    }
//
//    //MSAA 4次
//    std::vector<vec2> super_sample_step{
//        {0.25,0.25},
//        {0.75,0.25},
//        {0.25,0.75},
//        {0.75,0.75}
//    };
//    //当前像素坐标P，颜色color
//    vec2 P;
//    TGAColor color;
//    //遍历边界框中的每一个像素
//    for (P.x = int(bboxmin.x + 0.5); P.x <= int(bboxmax.x + 0.5); P.x++) {
//        for (P.y = int(bboxmin.y); P.y <= int(bboxmax.y + 0.5); P.y++) {
//            int ztest = 0; //判断是否通过深度测试
//            vec2 PP;
//            for (int i = 0; i < 4; i++) {
//                PP.x = P.x + super_sample_step[i][0];
//                PP.y = P.y + super_sample_step[i][1];
//                vec3 c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(PP));
//                double z_PP = (pts[0][2] / pts[0][3]) * c.x + (pts[0][2] / pts[1][3]) * c.y + (pts[0][2] / pts[2][3]) * c.z;
//                int frag_depth = std::max(0, std::min(255, int(z_PP + .5)));
//                //P的任一质心分量小于0或者zbuffer小于已有zbuffer，不渲染
//                if (c.x < 0 || c.y < 0 || c.z<0 || superzbuffer.get(P.x * 2 + i % 2, P.y * 2 + i / 2)[0]>frag_depth) continue;
//                else {
//                    ztest = 1;
//
//                    //调用片元着色器计算当前像素颜色
//                    bool discard = shader.fragment(c, color);
//                    if (!discard)
//                    {
//                        superzbuffer.set(P.x * 2 + i % 2, P.y * 2 + i / 2, TGAColor(frag_depth));
//                        superimage.set(P.x * 2 + i % 2, P.y * 2 + i / 2, color);
//                    }
//                }
//
//            }
//            if (ztest) {
//                color = superimage.get(P.x * 2, P.y * 2) / 4.0f + superimage.get(P.x * 2, P.y * 2 + 1) / 4.0f + superimage.get(P.x * 2 + 1, P.y * 2) / 4.0f + superimage.get(P.x * 2 + 1, P.y * 2 + 1) / 4.0f;
//                //color /= 4.0f;
//                //为像素设置颜色
//                image.set(P.x, P.y, color);
//            }
//
//        }
//    }
//}