//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Nemes Lili
// Neptun : GLJD1D
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

struct Hit {
    float t;
    vec3 position;
    vec3 normal;
    Hit() { t = -1; }
};

struct Ray {
    vec3 start;
    vec3 dir;
    Ray(vec3 s, vec3 d) {
        start = s; dir = d;
        dir=normalize(dir);
        start = normalize(start);
    }
};

struct Intersectable {
    virtual Hit intersect(const Ray& ray) = 0;
};
struct Cone : public Intersectable {
    vec3 point;
    float alpha;
    vec3 dir;
    float height;
    vec3 color;

    Cone(const vec3& _point, const vec3& _n, float a, float h, vec3 _c) {
        point = _point;
        height = h;
        alpha = a;
        dir = normalize(_n);
        color=_c;
    }

    Hit intersect(const Ray& ray) {
        Hit hit;

        vec3 rd = normalize(ray.dir);

        float dv = dot(rd, dir);
        vec3 co = ray.start-point;
        float cov = dot(co, dir);
        float dco= dot(co, rd);
        float coco = dot(co, co);
        float a = dv*dv-cosf(alpha)*cosf(alpha);
        float b = 2.0f*(dv*cov-dco*cosf(alpha)*cosf(alpha));
        float c = cov*cov-coco*cosf(alpha)*cosf(alpha);

        float discr = b * b - 4.0f * a * c;
        if (discr < 0) return hit;
        float sqrt_discr = sqrtf(discr);
        float t1 = (-b + sqrt_discr) / (2.0f * a);
        float t2 = (-b - sqrt_discr) / (2.0f * a);

        if (t1 <= 0 && t2<=0) return hit;

        float t_smaller=(t1<=t2?t1:t2);

        bool isThereSecondoption = true;

        if(t1<=0) { t_smaller=t2; isThereSecondoption=false; }

        else if( t2<=0) { t_smaller=t1; isThereSecondoption=false; }

        vec3 p = ray.start + (normalize(rd) * t_smaller);
        vec3 temp = p-point;

        float dist = dot(temp,dir);

        if(dist>0  && dist<=height ) {
            hit.t = t_smaller;
            hit.position = ray.start + normalize(rd) * hit.t;
            hit.normal = normalize(2*dot(temp, dir)*dir-2*temp*cosf(alpha)*cosf(alpha));
       }
        else if(isThereSecondoption) {
            if(t_smaller==t1)
                t_smaller=t2;
            else
                t_smaller=t1;
            p = ray.start + (normalize(rd) * t_smaller);
            temp=p-point;
            dist = dot(temp, dir);
            if(dist>0  && dist<=height) {
                hit.t = t_smaller;
                hit.position = ray.start + normalize(rd) * hit.t;
                hit.normal = normalize(2*dot(temp, dir)*dir-2*(temp)*cosf(alpha)*cosf(alpha));
            }
        }
        return hit;
    }

    Hit Reverse(const Ray& ray, float t, vec3& light) {
        Hit hit;

        vec3 rd = normalize(ray.dir);

        float dv = dot(rd, dir);
        vec3 co = ray.start-light;
        float cov = dot(co, dir);
        float dco= dot(co, rd);
        float coco = dot(co, co);
        float a = dv*dv-cosf(alpha)*cosf(alpha);
        float b = 2.0f*(dv*cov-dco*cosf(alpha)*cosf(alpha));
        float c = cov*cov-coco*cosf(alpha)*cosf(alpha);

        float discr = b * b - 4.0f * a * c;
        if (discr < 0) return hit;
        float sqrt_discr = sqrtf(discr);
        float t1 = (-b + sqrt_discr) / (2.0f * a);
        float t2 = (-b - sqrt_discr) / (2.0f * a);

        if (t1 <= 0 && t2<=0) return hit;

        if(t1<=0) {
            if(t2==t) {
                vec3 p = ray.start + (normalize(rd) * t2);
                vec3 temp = p - light;
                float dist = dot(temp, dir);
                if (dist > 0) {
                    hit.t = t2;
                    hit.position = ray.start + normalize(rd) * hit.t;
                    hit.normal = normalize(2*dot(temp, dir)*dir-2*(temp)*cosf(alpha)*cosf(alpha));
                }
            }
        }
        else if(t2<=0) {
            if(t1==t) {
                vec3 p = ray.start + (normalize(rd) * t1);
                vec3 temp = p - light;
                float dist = dot(temp, dir);
                if (dist > 0) {
                    hit.t = t1;
                    hit.position = ray.start + normalize(rd) * hit.t;
                    hit.normal = normalize(2*dot(temp, dir)*dir-2*(temp)*cosf(alpha)*cosf(alpha));
                }
            }
        }
        else {
            if(t1<=t && t2>=t ||(t1>=t && t2<=t)) {
                hit.t = t1;
                hit.position = ray.start + normalize(rd) * hit.t;
                hit.normal = normalize(2*dot(hit.position-point, dir)*dir-2*(hit.position-point)*cosf(alpha)*cosf(alpha));
            }

        }


        return hit;
    }
};



struct Triangle : Intersectable {
    vec3 vertice1;
    vec3 vertice2;
    vec3 vertice3;

    Triangle(vec3 v1, vec3 v2, vec3 v3) {
        vertice1 = v1;
        vertice2 = v2;
        vertice3 = v3;
    }

    Hit intersect(const Ray& ray) {
        Hit hit;
        //normálvektorgyártás
        vec3 normal = normalize(cross((vertice2-vertice1),(vertice3-vertice1)));
        if(dot(normalize(normal), ray.dir)<0)
            normal = normalize(normal*-1);
        float t = dot((vertice1-ray.start),normal)/dot(ray.dir,normal);
        if(t<=0) {
            return hit;
        }
        vec3 p = ray.start + (normalize(ray.dir) *t);
        if(dot(cross((vertice2-vertice1), (p-vertice1)), normal) >0 &&
            dot(cross((vertice3-vertice2), (p-vertice2)), normal) >0 &&
            dot(cross((vertice1-vertice3), (p-vertice3)), normal) > 0)
        {
            hit.t=t;
            hit.position = p;
            hit.normal = normalize(normal);

            return hit;

        }
        else {
            return hit;
        }
    }
};

struct Triangle2 : Intersectable {
    vec3 vertice1;
    vec3 vertice2;
    vec3 vertice3;

    Triangle2(vec3 v1, vec3 v2, vec3 v3) {
        vertice1 = v1;
        vertice2 = v2;
        vertice3 = v3;
    }

    Hit intersect(const Ray& ray) {
        Hit hit;
        //normálvektorgyártás
        vec3 normal = normalize(cross((vertice2-vertice1),(vertice3-vertice1)));
        float t = dot((vertice1-ray.start),normal)/dot(ray.dir,normal);
        if(t<=0) {
            return hit;
        }
        vec3 p = ray.start + (normalize(ray.dir) *t);
        if(dot(cross((vertice2-vertice1), (p-vertice1)), normal) >0 &&
           dot(cross((vertice3-vertice2), (p-vertice2)), normal) >0 &&
           dot(cross((vertice1-vertice3), (p-vertice3)), normal) > 0)
        {
            hit.t=t;
            hit.position = p;
            hit.normal = normalize(normal);

            return hit;

        }
        else {
            return hit;
        }
    }
};

class Camera {
    vec3 eye, lookat, right, up;
public:
    void set(vec3 _eye, vec3 _lookat, vec3 vup, float fov) {
        eye = _eye;
        lookat = _lookat;
        vec3 w = eye - lookat;
        float focus = length(w);
        right = normalize(cross(vup, w)) * focus * tanf(fov / 2);
        up = normalize(cross(w, right)) * focus * tanf(fov / 2);
    }
    Ray getRay(int X, int Y) {
        vec3 p = lookat + right * (2.0f * (X + 0.5f) / windowWidth - 1) + up * (2.0f * (Y + 0.5f) / windowHeight - 1);
        vec3 dir= normalize(p-eye);
        return Ray(eye, dir);
    }
};

const float epsilon = 0.0001f;
float rnd() { return (float)rand() / RAND_MAX; }

class Scene {
    std::vector<Intersectable *> objects;
    std::vector<Cone *> cones;
    Camera camera;
public:
    void build() {
        vec3 eye = vec3(1.75, 0.0, 2.0), vup = vec3(0, 1, 0), lookat = vec3(0.0, 0.0, 0);
        float fov = 45 * M_PI / 180;
        camera.set(eye, lookat, vup, fov);
        //cube
        float vertices[] = {0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0,
                            1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0};
        for (int i=0; i<24; i++) {
            vertices[i]-=0.5;
            vertices[i]= 0.4f *vertices[i];
        }
        vec3 verts[8];
        for(int i=0; i<24; i+=3) {
            if(i==0)
                verts[0]= *new vec3(vertices[i], vertices[i+1], vertices[i+2]);
            else{
                verts[i/3]= *new vec3(vertices[i], vertices[i+1], vertices[i+2]);
            }
        }
        objects.push_back(new Triangle(verts[0],verts[6], verts[4]));
        objects.push_back(new Triangle(verts[0],verts[2], verts[6]));
        objects.push_back(new Triangle(verts[0],verts[3], verts[2]));
        objects.push_back(new Triangle(verts[0],verts[1], verts[3]));
        objects.push_back(new Triangle(verts[2],verts[7], verts[6]));
        objects.push_back(new Triangle(verts[2],verts[3], verts[7]));
        objects.push_back(new Triangle(verts[4],verts[6], verts[7]));
        objects.push_back(new Triangle(verts[4],verts[7], verts[5]));
        objects.push_back(new Triangle(verts[0],verts[4], verts[5]));
        objects.push_back(new Triangle(verts[0],verts[5], verts[1]));
        objects.push_back(new Triangle(verts[1],verts[5], verts[7]));
        objects.push_back(new Triangle(verts[1],verts[7], verts[3]));

        //icosahedron
        float vertices_i[] =  {0,  -0.525731,  0.850651, 0.850651,  0,  0.525731, 0.850651,  0,  -0.525731, -0.850651,
                               0,  -0.525731, -0.850651,  0,  0.525731, -0.525731,  0.850651,  0, 0.525731,  0.850651,
                               0, 0.525731,  -0.850651,  0, -0.525731,  -0.850651,  0, 0,  -0.525731,  -0.850651, 0,
                               0.525731,  -0.850651, 0,  0.525731,  0.850651};

        for (int i=0; i<36; i++) {
            vertices_i[i]= 0.1f *vertices_i[i];
        }
        vec3 verts_i[12];
        for(int i=0; i<36; i+=3) {
            if(i==0)
                verts_i[0]= *new vec3(vertices_i[i], vertices_i[i+1], vertices_i[i+2]);
            else{
                verts_i[i/3]= *new vec3(vertices_i[i], vertices_i[i+1], vertices_i[i+2]);
            }
        }
        for(int i=0; i<12; i++) {
            verts_i[i].y=verts_i[i].y-0.1;
            verts_i[i].x=verts_i[i].x+0.1;
        }
        objects.push_back(new Triangle2(verts_i[1],verts_i[2], verts_i[6]));
        objects.push_back(new Triangle2(verts_i[1],verts_i[7], verts_i[2]));
        objects.push_back(new Triangle2(verts_i[3],verts_i[4], verts_i[5]));
        objects.push_back(new Triangle2(verts_i[4],verts_i[3], verts_i[8]));
        objects.push_back(new Triangle2(verts_i[6],verts_i[5], verts_i[11]));
        objects.push_back(new Triangle2(verts_i[5],verts_i[6], verts_i[10]));
        objects.push_back(new Triangle2(verts_i[9],verts_i[10], verts_i[2]));
        objects.push_back(new Triangle2(verts_i[10],verts_i[9], verts_i[3]));
        objects.push_back(new Triangle2(verts_i[7],verts_i[8], verts_i[9]));
        objects.push_back(new Triangle2(verts_i[8],verts_i[7], verts_i[0]));
        objects.push_back(new Triangle2(verts_i[11],verts_i[0], verts_i[1]));
        objects.push_back(new Triangle2(verts_i[0],verts_i[11], verts_i[4]));
        objects.push_back(new Triangle2(verts_i[6],verts_i[2], verts_i[10]));
        objects.push_back(new Triangle2(verts_i[1],verts_i[6], verts_i[11]));
        objects.push_back(new Triangle2(verts_i[3],verts_i[5], verts_i[10]));
        objects.push_back(new Triangle2(verts_i[5],verts_i[4], verts_i[11]));
        objects.push_back(new Triangle2(verts_i[2],verts_i[7], verts_i[9]));
        objects.push_back(new Triangle2(verts_i[7],verts_i[1], verts_i[0]));
        objects.push_back(new Triangle2(verts_i[3],verts_i[9], verts_i[8]));
        objects.push_back(new Triangle2(verts_i[4],verts_i[8], verts_i[0]));

        //dodecahedron
        float vertices_d[] =  {-0.57735,  -0.57735,  0.57735, 0.934172,  0.356822,  0, 0.934172,  -0.356822,  0,
                               -0.934172,  0.356822,  0, -0.934172,  -0.356822,  0, 0,  0.934172,  0.356822, 0,
                               0.934172,  -0.356822, 0.356822,  0,  -0.934172, -0.356822,  0,  -0.934172, 0,
                               -0.934172,  -0.356822, 0,  -0.934172,  0.356822, 0.356822,  0,  0.934172, -0.356822,
                               0,  0.934172, 0.57735,  0.57735, -0.57735, 0.57735,  0.57735,  0.57735, -0.57735,
                               0.57735,  -0.57735, -0.57735,  0.57735,  0.57735, 0.57735,  -0.57735,  -0.57735,
                               0.57735,  -0.57735,  0.57735, -0.57735,  -0.57735,  -0.57735};

        for (int i=0; i<60; i++) {
            vertices_d[i]= 0.1f *vertices_d[i];
        }
        vec3 verts_d[20];
        for(int i=0; i<60; i+=3) {
            if(i==0)
                verts_d[0]= *new vec3(vertices_d[i], vertices_d[i+1], vertices_d[i+2]);
            else{
                verts_d[i/3]= *new vec3(vertices_d[i], vertices_d[i+1], vertices_d[i+2]);
            }
        }
        for(int i=0; i<20; i++) {
            verts_d[i].y=verts_d[i].y-0.1;
            verts_d[i].z=verts_d[i].z+0.15;
            verts_d[i].x=verts_d[i].x-0.02;
        }
        objects.push_back(new Triangle2(verts_d[18],verts_d[2], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[11],verts_d[18], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[14],verts_d[11], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[7],verts_d[13], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[17],verts_d[7], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[2],verts_d[17], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[19],verts_d[4], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[8],verts_d[19], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[15],verts_d[8], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[12],verts_d[16], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[0],verts_d[12], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[4],verts_d[0], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[6],verts_d[15], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[5],verts_d[6], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[16],verts_d[5], verts_d[3]));
        objects.push_back(new Triangle2(verts_d[5],verts_d[14], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[6],verts_d[5], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[13],verts_d[6], verts_d[1]));
        objects.push_back(new Triangle2(verts_d[9],verts_d[17], verts_d[2]));
        objects.push_back(new Triangle2(verts_d[10],verts_d[9], verts_d[2]));
        objects.push_back(new Triangle2(verts_d[18],verts_d[10], verts_d[2]));
        objects.push_back(new Triangle2(verts_d[10],verts_d[0], verts_d[4]));
        objects.push_back(new Triangle2(verts_d[9],verts_d[10], verts_d[4]));
        objects.push_back(new Triangle2(verts_d[19],verts_d[9], verts_d[4]));
        objects.push_back(new Triangle2(verts_d[19],verts_d[8], verts_d[7]));
        objects.push_back(new Triangle2(verts_d[9],verts_d[19], verts_d[7]));
        objects.push_back(new Triangle2(verts_d[17],verts_d[9], verts_d[7]));
        objects.push_back(new Triangle2(verts_d[8],verts_d[15], verts_d[6]));
        objects.push_back(new Triangle2(verts_d[7],verts_d[8], verts_d[6]));
        objects.push_back(new Triangle2(verts_d[13],verts_d[7], verts_d[6]));
        objects.push_back(new Triangle2(verts_d[11],verts_d[14], verts_d[5]));
        objects.push_back(new Triangle2(verts_d[12],verts_d[11], verts_d[5]));
        objects.push_back(new Triangle2(verts_d[16],verts_d[12], verts_d[5]));
        objects.push_back(new Triangle2(verts_d[12],verts_d[0], verts_d[10]));
        objects.push_back(new Triangle2(verts_d[11],verts_d[12], verts_d[10]));
        objects.push_back(new Triangle2(verts_d[18],verts_d[11], verts_d[10]));

        //cones

        Cone red = *new Cone(vec3(0.1,  0.2,  0.075), vec3(0.0,-1.0,0.0), 0.3f, 0.05f, vec3(1.0f,0,0));
        Cone red2 = *new Cone(vec3(0.09995,  0.19995,  0.07495), vec3(0.0,-1.0,0.0), 0.3f, 0.04995f, vec3(1.0f,0,0));
        objects.push_back(&red);
        cones.push_back(&red2);
        Cone green = *new Cone(vec3(0.1,  0.18,  0.030), vec3(0.0,-1.0,0.0), 0.3f, 0.05f, vec3(0, 1.0f,0));
        Cone green2 = *new Cone(vec3(0.09995,  0.17995,  0.02995), vec3(0.0,-1.0,0.0), 0.3f, 0.04995f, vec3(0, 1.0f,0));
        objects.push_back(&green);
        cones.push_back(&green2);
        Cone blue2 = *new Cone(vec3(0.09995,  0.19995,  0.14995), vec3(0.0,-1.0,0.0), 0.3f, 0.04995f, vec3(0, 0,1.0f));
        Cone blue = *new Cone(vec3(0.1,  0.2,  0.15), vec3(0.0,-1.0,0.0), 0.3f, 0.05f, vec3(0, 0,1.0f));
        objects.push_back(&blue);
        cones.push_back(&blue2);

    }



    void render(std::vector<vec4>& image) {
        for (int Y = 0; Y < windowHeight; Y++) {
//#pragma omp parallel for
            for (int X = 0; X < windowWidth; X++) {
                vec3 color = trace(camera.getRay(X, Y));
                image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
            }
        }
    }


    Hit firstIntersect(Ray ray) {
        Hit bestHit;
        for (Intersectable * object : objects) {
            Hit hit = object->intersect(ray);
            if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))
                bestHit = hit;
        }
        if (dot(ray.dir, bestHit.normal) > 0) bestHit.normal = bestHit.normal * (-1);
        return bestHit;
    }

    bool shadowIntersect(Ray ray) {	// for directional lights
        for (Intersectable * object : objects) if (object->intersect(ray).t > 0) return true;
        return false;
    }


    vec3 trace(Ray ray) {
        Hit hit = firstIntersect(ray);
        vec3 black(0, 0, 0);
        if (hit.t < 0) return black;
        vec3 outRadiance = 0.2f * (1 + dot(normalize(hit.normal), normalize(ray.dir)*-1)) *vec3(1,1,1);
        for(Cone* cone : cones) {
            vec3 light = cone->point ;
            Hit helper = cone->Reverse(ray, hit.t, light);
            float cosTheta = dot(normalize(hit.normal), light);

            Ray newRay(hit.position +normalize(hit.normal)*epsilon , light);
            Hit shadowHit = firstIntersect(newRay);
            float dist = length(light-hit.position);
            float maxdist = 0.45f;

            //shadowHit.position-(hit.position+hit.normal*epsilon)) <= length(light-(hit.position+hit.normal*epsilon))
            if(!shadowIntersect(newRay)  && helper.t>0 && cosTheta > 0 ) {
                if (dist <= maxdist)
                {
                    float lightStrength = (maxdist - dist) / maxdist;
                    outRadiance =(1-lightStrength)*outRadiance + lightStrength*cone->color;
                }
            }
        }
        return outRadiance;
    }

};


GPUProgram gpuProgram; // vertex and fragment shaders
Scene scene;

// vertex shader in GLSL
const char *vertexSource = R"(
	#version 330
    precision highp float;

	layout(location = 0) in vec2 cVertexPosition;	// Attrib Array 0
	out vec2 texcoord;

	void main() {
		texcoord = (cVertexPosition + vec2(1, 1))/2;							// -1,1 to 0,1
		gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1); 		// transform to clipping space
	}
)";

// fragment shader in GLSL
const char *fragmentSource = R"(
	#version 330
    precision highp float;

	uniform sampler2D textureUnit;
	in  vec2 texcoord;			// interpolated texture coordinates
	out vec4 fragmentColor;		// output that goes to the raster memory as told by glBindFragDataLocation

	void main() {
		fragmentColor = texture(textureUnit, texcoord);
	}
)";

class FullScreenTexturedQuad {
    unsigned int vao;	// vertex array object id and texture id
    Texture texture;
public:
    FullScreenTexturedQuad(int windowWidth, int windowHeight, std::vector<vec4>& image)
            : texture(windowWidth, windowHeight, image)
    {
        glGenVertexArrays(1, &vao);	// create 1 vertex array object
        glBindVertexArray(vao);		// make it active

        unsigned int vbo;		// vertex buffer objects
        glGenBuffers(1, &vbo);	// Generate 1 vertex buffer objects

        // vertex coordinates: vbo0 -> Attrib Array 0 -> vertexPosition of the vertex shader
        glBindBuffer(GL_ARRAY_BUFFER, vbo); // make it active, it is an array
        float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    void Draw() {
        glBindVertexArray(vao);
        gpuProgram.setUniform(texture, "textureUnit");
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
};

FullScreenTexturedQuad * fullScreenTexturedQuad;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    scene.build();

    std::vector<vec4> image(windowWidth * windowHeight);
    long timeStart = glutGet(GLUT_ELAPSED_TIME);
    scene.render(image);
    long timeEnd = glutGet(GLUT_ELAPSED_TIME);
    printf("Rendering time: %d milliseconds\n", (timeEnd - timeStart));

    // copy image to GPU as a texture
    fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight, image);

    // create program for the GPU
    gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");
}

// Window has become invalid: Redraw
void onDisplay() {
    fullScreenTexturedQuad->Draw();
    glutSwapBuffers();									// exchange the two buffers
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
    if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;
    printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {
    float cX = 2.0f * pX / windowWidth - 1;
    float cY = 1.0f - 2.0f * pY / windowHeight;

    char * buttonStat;
    switch (state) {
        case GLUT_DOWN: buttonStat = "pressed"; break;
        case GLUT_UP:   buttonStat = "released"; break;
    }

    switch (button) {
        case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
        case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
        case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
    }
}

void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME);
}