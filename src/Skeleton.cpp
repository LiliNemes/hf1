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
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers

	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders
unsigned int vao;	   // virtual world on the GPU

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
    return dot(pointother,copyother);

}
vec3 OtherPointDirection(vec3 pointother, vec3 pointthis, float distance) {
    return (pointthis*distance*-1*cosh(1)*sinh(distance)+pointother*distance)/(sinh(distance)*sinh(distance));
}

//Egy ponthoz képest adott irányban és távolságra lévő pont előállítása
vec3 NewPoint(vec3 point,vec3 vector, float distance) {
    vec3 normalizedVector=normalize(vector);
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
    float lambda = dot(vector, point);
    if(lambda!=0) {
        point=point*lambda;
    }
    return point;
}

vec2 toPoincare (vec3 obj) {
    return {obj.x/(obj.z+1), obj.y/(obj.z+1)};
}

struct Circle
{
    unsigned int vao, vbo;
    vec3 color;
    vec2 pos;
    float size;

    Circle(vec2 pos, vec3 color, float size = 1)
    {

        this->pos = pos;
        this->color = color;
        this->size = size;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        vec2 vertices[100];
        for (size_t i = 0; i < 100; i++)
        {
            float fi = i * 2 * M_PI / 100;
            vertices[i] = vec2(cosf(fi), sinf(fi));
        }
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vec2) * 100,
                     vertices,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              2, GL_FLOAT, GL_FALSE,
                              0, nullptr);
    }

    void Draw() {
        int location = glGetUniformLocation(gpuProgram.getId(), "color");
        glUniform3f(location, color.x, color.y, color.z); // 3 floats

        float MVPtransf[4][4] = { 1 * size, 0, 0, 0,    // MVP matrix,
                                  0, 1 * size, 0, 0,    // row-major!
                                  0, 0, 1, 0,
                                  pos.x, pos.y, 0, 1 };

        location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
        glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

        glBindVertexArray(vao);  // Draw call
        glDrawArrays(GL_TRIANGLE_FAN, 0 /*startIdx*/, 100 /*# Elements*/);
    }
};


// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

	/*glGenVertexArrays(1, &vao);	// get 1 vao id
	glBindVertexArray(vao);		// make it active

	unsigned int vbo;		// vertex buffer object
	glGenBuffers(1, &vbo);	// Generate 1 buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Geometry with 24 bytes (6 floats or 3 x 2 coordinates)
	float vertices[] = { -0.2f, -0.8f, -0.6f, 1.0f, 0.8f, -0.2f };
	glBufferData(GL_ARRAY_BUFFER, 	// Copy to GPU target
		sizeof(vertices),  // # bytes
		vertices,	      	// address
		GL_STATIC_DRAW);	// we do not change later

	glEnableVertexAttribArray(0);  // AttribArray 0
	glVertexAttribPointer(0,       // vbo -> AttribArray 0
		2, GL_FLOAT, GL_FALSE, // two floats/attrib, not fixed-point
		0, NULL); 		     // stride, offset: tightly packed */

	// create program for the GPU
	//gpuProgram.create(vertexSource, fragmentSource, "outColor");
    vec3 r=*new vec3(0,0,1);
    vec3 color(1,1,0);
    Circle a = *new Circle( *new vec2(0,0) ,color, 0.2);
    a.Draw();
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

	// Set color to (0, 1, 0) = green
	int location = glGetUniformLocation(gpuProgram.getId(), "color");
	glUniform3f(location, 0.0f, 1.0f, 0.0f); // 3 floats

	float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix,
							  0, 1, 0, 0,    // row-major!
							  0, 0, 1, 0,
							  0, 0, 0, 1 };

	location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

	glBindVertexArray(vao);  // Draw call
	glDrawArrays(GL_TRIANGLES, 0 /*startIdx*/, 3 /*# Elements*/);

	glutSwapBuffers(); // exchange buffers for double buffering
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
