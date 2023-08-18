//#include "tgaimage.h"
//#include "simplemodel.h"
//
//const TGAColor white = TGAColor(255, 255, 255, 255);
//const TGAColor red = TGAColor(255, 0, 0, 255);
//const int WIDTH = 500;
//const int HEIGHT = 500;
//
//// �޸�line()������ʵ�֣�ȷ������Խ�����
//void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
//    bool steep = false;
//    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
//        std::swap(x0, y0);
//        std::swap(x1, y1);
//        steep = true;
//    }
//    if (x0 > x1) {
//        std::swap(x0, x1);
//        std::swap(y0, y1);
//    }
//    int dx = x1 - x0;
//    int dy = y1 - y0;
//    int derror2 = std::abs(dy) * 2;
//    int error2 = 0;
//    int y = y0;
//    for (int x = x0; x <= x1; x++) {
//        if (steep) {
//            image.set(y, x, color);
//        }
//        else {
//            image.set(x, y, color);
//        }
//        error2 += derror2;
//        if (error2 > dx) {
//            y += (y1 > y0 ? 1 : -1);
//            error2 -= dx * 2;
//        }
//    }
//}
//
////��`triangle()`�����У�����`double`���ͽ��в�ֵ���㣬�Ա��������������µľ�����ʧ����ʹ���ϸ�ȡ���ķ�ʽ����ֵ���ת��Ϊ�����������꣬�����˲�ֵ�������ͬʱ������˶������ζ���ı߽��飬ȷ�������y���갴�մ�С�����˳���룬�Ӷ������ڻ���������ʱ���ִ����ɨ����˳��
//void triangle(vec2 t0, vec2 t1, vec2 t2, TGAImage& image, TGAColor color) {
//    if (t0.y == t1.y && t0.y == t2.y) return;
//    if (t0.y > t1.y) std::swap(t0, t1);
//    if (t0.y > t2.y) std::swap(t0, t2);
//    if (t1.y > t2.y) std::swap(t1, t2);
//    int height = t2.y - t0.y;
//    for (int i = 0; i < height; i++) {
//        bool second_half = (i <= t1.y - t0.y);
//        double cury = t0.y + i;
//        if (!second_half) {
//            double k1 = (cury - t1.y) / (t2.y - t1.y);
//            double x0 = k1 * (t2.x - t1.x) + t1.x;
//            double k2 = (cury - t0.y) / (t2.y - t0.y);
//            double x1 = k2 * (t2.x - t0.x) + t0.x;
//            line(static_cast<int>(x0 + 0.5), static_cast<int>(cury + 0.5),
//                static_cast<int>(x1 + 0.5), static_cast<int>(cury + 0.5), image, color);
//        }
//        else {
//            double k1 = (cury - t0.y) / (t1.y - t0.y);
//            double x0 = k1 * (t1.x - t0.x) + t0.x;
//            double k2 = (cury - t0.y) / (t2.y - t0.y);
//            double x1 = k2 * (t2.x - t0.x) + t0.x;
//            line(static_cast<int>(x0 + 0.5), static_cast<int>(cury + 0.5),
//                static_cast<int>(
//                    x1 + 0.5), static_cast<int>(cury + 0.5), image, color);
//        }
//    }
//}
//
//
//
//
//int main(int argc, char** argv) {
//    //ָ�����շ���
//    vec3 light_dir = { 0, 0, -1 };
//    TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
//	SimpleModel* model = new SimpleModel("E:/learn_for_job/project/myrender/obj/african_head/african_head.obj");
//	for (int faceIndex = 0;faceIndex<model->nfaces();faceIndex++){
//		auto face = model->face(faceIndex);
//		vec2 triangleScreenPoints[3]; //��Ļ����
//        vec3 triangleWorldPoints[3];//��������  ���ڼ��㷨���� ����ǿ��
//		for (int p = 0; p < 3; p++) {
//			auto t = model->vert(face[p]);
//			vec2 vec2_t; vec2_t.x = (t.x + 1.) * WIDTH / 2.; vec2_t.y = (t.y + 1.) * HEIGHT / 2.;
//			triangleScreenPoints[p] = vec2_t;
//            triangleWorldPoints[p] = t;
//		}
//        //������������㷨���� ���
//        vec3 n = cross((triangleWorldPoints[2] - triangleWorldPoints[0]), (triangleWorldPoints[1] - triangleWorldPoints[0]));
//        n=n.normalized();
//        std::cout << n << std::endl;
//        float intensity = n * light_dir;
// /*       std::cout << intensity << std::endl;*/
//        if (intensity > 0) {
//            triangle(triangleScreenPoints[0], triangleScreenPoints[1], triangleScreenPoints[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
//        }
//
//
//		//triangle(trianglePoints[0], trianglePoints[1], trianglePoints[2], image, white);
//		//����һ�����������  ��������
//		for (int i = 0; i < 3; i++) {
//			vec3 v0 = model->vert(face[i]);
//			vec3 v1 = model->vert(face[(i + 1) % 3]);
//
//			//���ݶ���v0��v1����
//			//��Ҫ����ģ�����굽��Ļ�����ת����  (-1,-1)��Ӧ(0,0)   (1,1)��Ӧ(width,height)
//			int x0 = (v0.x + 1.) * WIDTH / 2.;
//			int y0 = (v0.y + 1.) * HEIGHT / 2.;
//			int x1 = (v1.x + 1.) * WIDTH / 2.;
//			int y1 = (v1.y + 1.) * HEIGHT / 2.;
//			line(x0, y0, x1, y1,image,red);
//		}
//
//	}
//
//    image.flip_vertically();
//    image.write_tga_file("output.tga");
//    image.show();
//    return 0;
//}
