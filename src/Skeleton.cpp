//=============================================================================================
// Mintaprogram: Z�ld h�romsz�g. Ervenyes 2019. osztol.
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

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330
	precision highp float;

	layout(location = 0) in vec2 vp;
    layout(location = 1) in vec3 vc;
    out vec3 color;

	void main() {
        color = vc;
		gl_Position = vec4(vp.x, vp.y, 0, 1);
    }
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330
	precision highp float;	// normal floats, makes no difference on desktop computers

    uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders
unsigned int vao;	   // virtual world on the GPU

float HyperDot(vec3 a, vec3 b) {
    return (a.x*b.x+a.y*b.y-a.z*b.z);
}

inline float HyperLength(const vec3& v) { return sqrtf(HyperDot(v, v)); }

inline vec3 HyperNormalize(const vec3& v) { return v * (1 / HyperLength(v)); }


//egy irányra merőleges irány állítása
vec3 SheerVector(const vec3 point, const vec3 vector) {
    vec3 newPoint = *new vec3(point.x, point.y, -1*point.z);
    vec3 newVector = *new vec3(vector.x, vector.y, -1*vector.z);
    return cross(newVector, newPoint);
}

//Adott pontból és sebesség vektorral induló pont helyének és sebesség vektorának számítása t idővel később
vec3 PlaceAfterTime(vec3 point, float time, vec3 vector) {
    vec3 place = point*cosh(time) + vector*sinh(time);
    return place;
}

vec3 VelocityAfterTime(vec3 point, float time, vec3 vector) {
    vec3 velocity = point*sinh(time) + vector*cosh(time);
    return velocity;
}

//Egy ponthoz képest egy másik pont irányának és távolságának meghatározása
float OtherPointDistance(vec3 pointother, vec3 pointthis) {
    vec3 copyother=*new vec3(pointother.x, pointother.y, pointother.z);
    copyother.operator-();
    return HyperDot(pointother,copyother);

}
vec3 OtherPointDirection(vec3 pointother, vec3 pointthis, float distance) {
    return (pointthis*distance*-1*cosh(1)*sinh(distance)+pointother*distance)/(sinh(distance)*sinh(distance));
}

//Egy ponthoz képest adott irányban és távolságra lévő pont előállítása
vec3 NewPoint(vec3 point,vec3 vector, float distance) {
    vec3 normalizedVector=HyperNormalize(vector);
    vec3 newPoint=point*cosh(distance)+normalizedVector*sinh(distance);
    return newPoint;
}

//Egy pontban egy vektor elforgatása adott szöggel
vec3 TurnedVector(vec3 vector, vec3 sheerVector, float fi) {
    vec3 turned = vector*cos(fi)+sheerVector*sin(fi);
    return turned;
}

//Egy közelítő pont és sebességvektorhoz a geometria szabályait teljesítő , közeli pont és sebesség választása
vec3 CorrectionPoint(vec3 point) {
    point.z=sqrt(point.x*point.x+point.y*point.y+1);
    return point;
}
vec3 CorrectionVector(vec3 vector, vec3 point){
    float lambda = HyperDot(vector, point);
    vec3 result = vector + (point * lambda);
    return result;
}

vec2 toPoincare (vec3 obj) {
    return {obj.x/(obj.z+1), obj.y/(obj.z+1)};
}

// Initialization, create an OpenGL context
void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    glGenVertexArrays(1, &vao);	// get 1 vao id
    glBindVertexArray(vao);		// make it active

    unsigned int vbo;		// vertex buffer object
    glGenBuffers(1, &vbo);	// Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Geometry with 24 bytes (6 floats or 3 x 2 coordinates)
    float vertices[] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.8f, -0.2f };
    //float vertices[] = { -0.2f, -0.8f, -0.6f, 1.0f, 0.8f, -0.2f };
    glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
                 sizeof(vertices),  // # bytes
                 vertices,	      	// address
                 GL_STATIC_DRAW);	// we do not change later

    glEnableVertexAttribArray(0);  // AttribArray 0
    glVertexAttribPointer(0,       // vbo -> AttribArray 0
                          2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
                          0, NULL); 		     // stride, offset: tightly packed

    // create program for the GPU
    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

struct Shape {

    unsigned int vao;
    int size;
    vec3 color;
    bool closed;

    Shape(std::vector<vec2> vertices, vec3 color, bool closed = false)
    {
        this->color = color;
        this->closed = closed;
        glGenVertexArrays(1, &vao);	// get 1 vao id
        glBindVertexArray(vao);		// make it active
        unsigned int vbo;		// vertex buffer object
        glGenBuffers(1, &vbo);	// Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        this-> size = vertices.size();

        glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
                     vertices.size() * sizeof(vec2),  // # bytes
                     &vertices[0],	      	// address
                     GL_STATIC_DRAW);	// we do not change later

        glEnableVertexAttribArray(0);  // AttribArray 0
        glVertexAttribPointer(0,       // vbo -> AttribArray 0
                              2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
                              0, NULL); 		     // stride, offset: tightly packed
    }

    void Draw() {

        int location = glGetUniformLocation(gpuProgram.getId(), "color");
        glUniform3f(location, color.x, color.y, color.z ); // 3 floats

        glBindVertexArray(vao);		// make it active
        glDrawArrays(closed ? GL_LINE_LOOP : GL_LINE_STRIP, 0, size);
    }
};

struct Circle {

    Shape *underlyingShape;

    Circle (vec2 origo, vec3 color, float size = 1)
    {
        std::vector<vec2> vecs = {};
        for (size_t i = 0; i < 100; i++)
        {
            float fi = i * 2 * M_PI / 100;
            vec2 v = vec2(cosf(fi) * size + origo.x , sinf(fi) * size + origo.y);
            vecs.push_back(v);
        }
        this->underlyingShape = new Shape(vecs, color, true);
    }

    void Draw() {
        this->underlyingShape->Draw();
    }

};

struct Triangle {
    vec2 a;
    vec2 b;
    vec2 c;

    Triangle(vec2 a, vec2 b, vec2 c) {
        this->a = a;
        this->b = b;
        this->c = c;
    }
};

struct Triangles {

    unsigned int vao;
    int size;
    vec3 color;

    Triangles(std::vector<Triangle> triangles, vec3 color)
    {
        this->color = color;
        glGenVertexArrays(1, &vao);	// get 1 vao id
        glBindVertexArray(vao);		// make it active
        unsigned int vbo;		// vertex buffer object
        glGenBuffers(1, &vbo);	// Generate 1 buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        this-> size = triangles.size() * 3;

        glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
                     triangles.size() * sizeof(vec2) * 3,  // # bytes
                     &triangles[0],	      	// address
                     GL_STATIC_DRAW);	// we do not change later

        glEnableVertexAttribArray(0);  // AttribArray 0
        glVertexAttribPointer(0,       // vbo -> AttribArray 0
                              2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
                              0, NULL); 		     // stride, offset: tightly packed
    }

    void Draw() {

        int location = glGetUniformLocation(gpuProgram.getId(), "color");
        glUniform3f(location, color.x, color.y, color.z ); // 3 floats

        glBindVertexArray(vao);		// make it active
        glDrawArrays(GL_TRIANGLES, 0 /*startIdx*/, size * 3 /*# Elements*/);
    }
};

struct Disk {

    Triangles *underlyingShape;

    Disk (vec2 origo, vec3 color, float size = 1)
    {
        std::vector<Triangle> vecs = {};
        vec2 prev;
        vec2 first;
        for (size_t i = 0; i < 100; i++)
        {
            float fi = i * 2 * M_PI / 100;
            vec2 v = vec2(cosf(fi) * size + origo.x , sinf(fi) * size + origo.y);
            if (i > 0)
            {
                Triangle tr = Triangle(origo, v, prev);
                vecs.push_back(tr);
            }
            else
            {
                first = v;
            }
            prev = v;
        }
        vecs.push_back(Triangle(origo, prev, first));
        this->underlyingShape = new Triangles(vecs, color);
    }

    void Draw() {
        this->underlyingShape->Draw();
    }

};

struct HyperDisk {

    Triangles *triangles;
    HyperDisk() {
        triangles=nullptr;
    }

    HyperDisk (vec3 origoi, vec3 vectori, vec3 color, float dist) {

        std::vector<Triangle> triangleList = { };

        vec3 origo = CorrectionPoint(origoi);
        vec3 vector = HyperNormalize(CorrectionVector(vectori, origo));
        vec3 perp_vector = HyperNormalize(SheerVector(origo, vector));
        vec2 porigo= toPoincare(origo);

        vec3 v1 = vector;
        vec3 v2;

        for (size_t i = 0; i < 100; i++) {
            float fi = i* 2 * M_PI / 100;
            v2 = HyperNormalize(CorrectionVector(TurnedVector(vector, perp_vector, fi), origo));
            vec2 pv1 = toPoincare(NewPoint(origo, v1, dist));
            vec2 pv2 = toPoincare(NewPoint(origo, v2, dist));
            Triangle tr = Triangle(porigo, pv1, pv2);
            triangleList.push_back(tr);
            v1 = v2;
        }
        triangleList.push_back(Triangle(porigo, toPoincare(NewPoint(origo, v1, dist)),
                                        toPoincare(NewPoint(origo, vector, dist))));


        this->triangles = new Triangles(triangleList, color);
    }

    void Draw() {
        this->triangles->Draw();
    }

};

void poincareTest()
{
    vec3 start = CorrectionPoint(*new vec3(-1100.0f,-1000.0f,1));
    vec3 dir = CorrectionVector(*new vec3(0.5f,1.5f,0),start);
    std::vector<vec2> vecList = { toPoincare(start) };
    vec3 cur = start;
    for (int i=0; i<400; i++)
    {
        cur = CorrectionPoint(NewPoint(cur, dir, 0.05f));
        dir = HyperNormalize(CorrectionVector(dir, cur));
        vec2 pcur = toPoincare(cur);
        vecList.push_back(pcur);
        printf("Cur:%f:%f:%f -- %f:%f\n", cur.x, cur.y, cur.z, pcur.x, pcur.y);
    }
    Shape sp = Shape(vecList, *new vec3(0.5f, 0.0f, 0.5f));
    sp.Draw();
}

void poincareTest2()
{
    vec3 cur = CorrectionPoint(*new vec3(-2.0f,-5.0f,1));
    vec2 pcur = toPoincare(cur);
    printf("Cur:%f:%f:%f -- %f:%f\n", cur.x, cur.y, cur.z, pcur.x, pcur.y);

    Disk sp = Disk(pcur, *new vec3(0.5f, 0.0f, 0.5f), 0.01f);
    sp.Draw();
}

void poincareTest3()
{
    std::vector<Triangle> triangleList = { };

    float dist = 2.5f;

    vec3 origo = CorrectionPoint(*new vec3(1.5f,1.5f,-1));
    vec3 vector = HyperNormalize(CorrectionVector(*new vec3(0.0f, 1.0f, 0.0f), origo));
    vec3 perp_vector = HyperNormalize(SheerVector(origo, vector));
    vec2 porigo= toPoincare(origo);

    vec3 v1 = vector;
    vec3 v2;

    for (size_t i = 0; i < 100; i++) {
        float fi = i* 2 * M_PI / 100;
        v2 = HyperNormalize(CorrectionVector(TurnedVector(vector, perp_vector, fi), origo));
        vec2 pv1 = toPoincare(NewPoint(origo, v1, dist));
        vec2 pv2 = toPoincare(NewPoint(origo, v2, dist));
        Triangle tr = Triangle(porigo, pv1, pv2);
        triangleList.push_back(tr);
        v1 = v2;
    }
    triangleList.push_back(Triangle(porigo, toPoincare(NewPoint(origo, v1, dist)),
                                    toPoincare(NewPoint(origo, vector, dist))));


    Triangles trsDisplay = Triangles(triangleList, *new vec3(0.5f, 0.0f, 0.5f));
    trsDisplay.Draw();
    Disk odsk = Disk(porigo, *new vec3(1.0f, 0.0f, 0.0f), 0.01f);
    odsk.Draw();
}

struct Hami {
    vec3 centerPoint;
    vec3 vector;
    float size;
    vec3 color;
    HyperDisk body;
    HyperDisk eyes[2];
    HyperDisk eyesBlack[2];

    Hami(vec3 center, vec3 vec, float s, vec3 color)  {
        centerPoint=center;
        vector=vec;
        size=s;
        color=color;
        body=HyperDisk(centerPoint, vector, color, size);

        vec3 eyeCenter1= CorrectionPoint(NewPoint(center, CorrectionVector(HyperNormalize(vector),center), size));
        eyes[0]=HyperDisk(eyeCenter1, vector, *new vec3(1,1,1), size/3);
        vec3 eyeCenter2= CorrectionPoint(NewPoint(center,CorrectionVector(TurnedVector(HyperNormalize(vector),SheerVector(center, HyperNormalize(vector)), M_PI/2.5),center),size));
        eyes[1]=HyperDisk(eyeCenter2, vector, *new vec3(1,1,1), size/3);

        //vec3 eyeCenter3= NewPoint(center, HyperNormalize(vector), size);
        //vec3 eyeCenter4= NewPoint(center,TurnedVector(HyperNormalize(vector),SheerVector(center, HyperNormalize(vector)), M_PI/4),size);
        eyesBlack[0]=HyperDisk(eyeCenter1, vector, *new vec3(0,0,0), size/4.5f);
        eyesBlack[0]=HyperDisk(eyeCenter2, vector, *new vec3(0,0,0), size/4.5f);

    }
    void Draw() {
        body.Draw();
        eyes[0].Draw();
        eyes[1].Draw();
        //eyesBlack[0].Draw();
        //eyesBlack[1].Draw();
    }


};

// Window has become invalid: Redraw
void onDisplay() {
    glClearColor(0, 0, 0, 0);     // background color
    glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

//    // Set color to (0, 1, 0) = green
//    int location = glGetUniformLocation(gpuProgram.getId(), "color");
//    glUniform3f(location, 0.0f, 1.0f, 0.0f); // 3 floats
//
//	float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix,
//							  0, 1, 0, 0,    // row-major!
//							  0, 0, 1, 0,
//							  0, 0, 0, 1 };
//
//	location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
//	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location
//
//	glBindVertexArray(vao);  // Draw call
//	glDrawArrays(GL_LINE_STRIP, 0 /*startIdx*/, 3 /*# Elements*/);

    Disk dsk = Disk(*new vec2(0.0f, 0.0f),*new vec3(0.1f, 0.1f, 0.1f), 1.0f);
    dsk.Draw();
    Hami red = Hami(*new vec3(0,0,1), *new vec3(0,1,0), 0.2, *new vec3(1,0,0));
    red.Draw();
    Hami green = Hami(*new vec3(2,2,3), *new vec3(3,3,4), 0.2, *new vec3(0,1,0));
    green.Draw();
//    std::vector<vec2> v = { *new vec2(0,0), *new vec2(-0.5f, -0.5f ), *new vec2(0.2f, -0.4f), *new vec2(0.1f, -0.5f) };
//    Shape sh = Shape(v, *new vec3(1.0f, 0.0f, 0.0f));
//    sh.Draw();
//
//    std::vector<vec2> v2 = { *new vec2(-0.9f,0.1f), *new vec2(-0.4f, -0.3f ), *new vec2(0.2f, -0.4f) };
//    sh = Shape(v2, *new vec3(0.1f, 0.1f, 1.0f), true);
//    sh.Draw();
//
//    Circle cr = Circle(*new vec2(0.2f, 0.2f),*new vec3(1.0f, 0.0f, 0.0f), 0.5f);
//    cr.Draw();
//
//    Triangle tr1 = Triangle(*new vec2(-0.9f,0.1f), *new vec2(-0.4f, -0.3f ), *new vec2(0.2f, -0.4f));
//    Triangle tr2 = Triangle(*new vec2(0.9f,-0.1f), *new vec2(0.4f, 0.3f ), *new vec2(-0.2f, 0.4f));
//    std::vector<Triangle> t = { tr1, tr2 };
//    Triangles tr = Triangles(t, *new vec3(0.5f, 0.0f, 0.5f));
//    tr.Draw();
//

    //poincareTest3();

   // HyperDisk hd = HyperDisk(*new vec3(1.5f,1.5f,-1), *new vec3(0.0f, 1.0f, 0.0f), *new vec3(1.0f, 0.0f, 0.0f), 1.0f);
    //hd.Draw();

    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
    printf("key pressed: %c\n", key);
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
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
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

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}