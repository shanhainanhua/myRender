#include "sample.h"


/*用于贴图3D立方体的立方体贴图可以使用立方体的位置作为纹理坐标来采样。当立方体处于原点(0, 0, 0)时，它的每一个位置向量都是从原点出发的方向向量。这个方向向量正是获取立方体上特定位置的纹理值所需要的。正是因为这个，我们只需要提供位置向量而不用纹理坐标了。https://learnopengl-cn.github.io/04%20Advanced%20OpenGL/06%20Cubemaps/
*/

/// <summary>
/// 计算立方体贴图中给定方向的uv和面索引
/// </summary>
/// <param name="direction"></param>
/// <param name="uv"></param>
/// <returns></returns>
int cal_cubemap_uv(vec3 direction, vec2& uv)
{

	int face_index = -1;
	float ma = 0, sc = 0, tc = 0;
	float abs_x = fabs(direction[0]), abs_y = fabs(direction[1]), abs_z = fabs(direction[2]);

	if (abs_x > abs_y && abs_x > abs_z)			/* major axis -> x */
	{
		ma = abs_x;
		if (direction.x > 0)					/* positive x */
		{
			face_index = 0;
			sc =direction.z;
			tc =direction.y;
		}
		else									/* negative x */
		{
			face_index = 1;
			sc =-direction.z;
			tc +=direction.y;
		}
	}
	else if (abs_y > abs_z)						/* major axis -> y */
	{
		ma = abs_y;
		if (direction.y > 0)					/* positive y */
		{
			face_index = 2;
			sc =direction.x;
			tc =direction.z;
		}
		else									/* negative y */
		{
			face_index = 3;
			sc =direction.x;
			tc =-direction.z;
		}
	}
	else										/* major axis -> z */
	{
		ma = abs_z;
		if (direction.z > 0)					/* positive z */
		{
			face_index = 4;
			sc =-direction.x;
			tc =direction.y;
		}
		else									/* negative z */
		{
			face_index = 5;
			sc =direction.x;
			tc =direction.y;
		}
	}

	uv[0] = (sc / ma+ 1.0f) / 2.0f;
	uv[1] = (tc / ma + 1.0f) / 2.0f;

	return face_index;
}

vec3 texture_sample(vec2 uv, TGAImage* image)
{
	uv[0] = fmod(uv[0], 1);
	uv[1] = fmod(uv[1], 1);

	int uv0 = uv[0] * image->get_width();
	int uv1 = uv[1] * image->get_height();
	TGAColor c = image->get(uv0, uv1);
	vec3 res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f;
	return res;
}

vec3 cubemap_sample(vec3 direction, cubemap_t* cubemap)
{
	vec2 uv;
	vec3 color;
	int face_index = cal_cubemap_uv(direction, uv);

	color = texture_sample(uv, cubemap->faces[face_index]);
	return color;
}