//=============================================================================================
// Mintaprogram: Zold haromszog. Ervenyes 2019. osztol.
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
// Nev    : Csik�s Patrik
// Neptun : E4MZUV
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
const char *const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1);
	}
)";

// fragment shader in GLSL
const char *const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders
const int nTesselatedVertices = 100;


class Poly {
    std::vector<vec2> points;
    std::vector<float> vertexData;

    unsigned int vaoPoints, vboPoints;
    unsigned int vaoCurve, vboCurve;

    int tension = -1;
    std::vector<float> ts;
public:
    vec3 color = vec3(0, 1, 1);

    void addPoint(vec2 new_point) {
        ts.push_back(points.size());
        points.push_back(new_point);

        calcCurve();
        load();
        //loadCurve();
    }

    vec2 Hermite(vec2 p0, vec2 v0, float t0, vec2 p1, vec2 v1, float t1, float t) {
        vec2 a0 = p0;
        vec2 a1 = v0;
        vec2 a2 = (3 * (p1 - p0)) / pow(t1 - t0, 2) - (2 * v0 + v1) / (t1 - t0);
        vec2 a3 = (2 * (p0 - p1)) / pow(t1 - t0, 3) + (v0 + v1) / pow(t1 - t0, 2);

        return a3*pow(t - t0, 3) + a2*pow(t - t0, 2) + a1*(t - t0) + a0;
    }

    vec2 v(vec2 r, vec2 r0, vec2 r1, float t, float t0, float t1) {
        float alpha = (1-tension)/2;
        return alpha * (((r1 - r) / (t1 - t)) + ((r - r0) / (t - t0)));
    }

    vec2 catmullRom(float t){
        for (int index = 0; index < ts.size(); ++index) {
            int i = index + 1 == ts.size() ? 0 : index + 1;
            if(ts[i] <= t && t >= ts[i+1]){
                i = i - 1 < 0 ? (int)ts.size() - 1 : i - 1;

                vec2 v0 = v(points[i], points[i-1], points[i+1], ts[i], ts[i-1], ts[i+1]);
                vec2 v1 = v(points[i+1], points[i], points[i+2], ts[i+1], ts[i], ts[i+2]);

                return Hermite(points[i], v0, ts[i], points[i+1], v1, ts[i+1], t);
            }
        }
    }

    static float tStart(){
        return 0;
    }

    float tEnd(){
        return (float)ts.size();
    }

    Poly(){
        /*glGenVertexArrays(1, &vaoPoints);
        glGenBuffers(1, &vboPoints);
        glGenVertexArrays(1, &vaoCurve);
        glGenBuffers(1, &vboCurve);*/
    }

    void calcCurve(){
        if(points.size() > 2){
            for (int i = 0; i < nTesselatedVertices; ++i) {
                float tNormalized = (float)i / (nTesselatedVertices - 1);
                float t = tStart() + (tEnd() - tStart()) * tNormalized;

                vec2 wVertex = catmullRom(t);
                vertexData.push_back(wVertex.x);
                vertexData.push_back(wVertex.y);
            }
        }
    }

    void load() {
        glGenVertexArrays(1, &vaoPoints);
        glBindVertexArray(vaoPoints);
        glGenBuffers(1, &vboPoints);
        glBindBuffer(GL_ARRAY_BUFFER, vboPoints);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec2), points.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), reinterpret_cast<void*>(0));
    }

    void loadCurve(){
        glGenVertexArrays(1, &vaoCurve);
        glBindVertexArray(vaoCurve);
        glGenBuffers(1, &vboCurve);
        glBindBuffer(GL_ARRAY_BUFFER, vboCurve);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, reinterpret_cast<void*>(0));
        gpuProgram.setUniform(vec3(1, 0, 0), "color");
    }

    void drawCurve() {
        gpuProgram.setUniform(vec3(1, 0, 0), "color");
        glBindVertexArray(vaoCurve);
        glBindBuffer(GL_ARRAY_BUFFER, vboCurve);
        glDrawArrays(GL_LINE_STRIP, 0, nTesselatedVertices);
    }

    void draw() {
        gpuProgram.setUniform(color, "color");
        glBindVertexArray(vaoPoints);
        glBindBuffer(GL_ARRAY_BUFFER, vboPoints);
        glDrawArrays(GL_POINTS, 0, points.size());
    }
};

Poly *polygon;

// Initialization, create an OpenGL context
void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);

    glLineWidth(2);
    glPointSize(5);

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
    polygon = new Poly();
}

// Window has become invalid: Redraw
void onDisplay() {
    glClearColor(0, 0, 0, 0);     // background color
    glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

    polygon->draw();

    glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
    if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw

    if(key == 's'){
        polygon->calcCurve();
        polygon->loadCurve();
        polygon->drawCurve();

        glutPostRedisplay();
    }
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX,
                   int pY) {    // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    // Convert to normalized device space
    float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
    float cY = 1.0f - 2.0f * pY / windowHeight;


}

// Mouse click event
void onMouse(int button, int state, int pX,
             int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Convert to normalized device space
        float cX = 2.0f * pX / windowWidth - 1;    // flip y axis
        float cY = 1.0f - 2.0f * pY / windowHeight;

        printf("Mouse click at coordinates (%.5f, %.5f)\n", cX, cY);
        polygon->addPoint(vec2(cX, cY));

        glutPostRedisplay();
    }
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
    long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}
