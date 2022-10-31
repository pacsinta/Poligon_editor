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
// Nev    : Csikos Patrik
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

const float SmoothTension = -1;

// A tanar ur gorbe szerkesztoje altal
const int nTesselatedVertices = 100;

// vertex shader in GLSL
const char *vertexSource = R"(
	#version 330
    precision highp float;

	uniform mat4 MVP;

	layout(location = 0) in vec2 vertexPosition;

	void main() {
		gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, 1) * MVP;
	}
)";

// fragment shader in GLSL
const char *fragmentSource = R"(
	#version 330
    precision highp float;

	uniform vec3 color;
	out vec4 fragmentColor;

	void main() {
		fragmentColor = vec4(color, 1);
	}
)";

// A tanar ur gorbe szerkesztoje altal:
struct Camera {
    float wCx = 0, wCy = 0;
    float wWx = 20, wWy = 20;
public:
    mat4 V() {
        return mat4(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    -wCx, -wCy, 0, 1);
    }

    mat4 P() {
        return mat4(2 / wWx, 0, 0, 0,
                    0, 2 / wWy, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
    }

    mat4 Vinv() {
        return mat4(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    wCx, wCy, 0, 1);
    }

    mat4 Pinv() {
        return mat4(wWx / 2, 0, 0, 0,
                    0, wWy / 2, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1);
    }
};


Camera camera;
GPUProgram gpuProgram;

// A tanar ur gorbe szerkesztoje altal
class Curve {
    unsigned int vaoCurve, vboCurve;
    unsigned int vaoCtrlPoints, vboCtrlPoints;
    unsigned int vaoConvexHull, vboConvexHull;

protected:
    std::vector<vec4> wCtrlPoints;
    std::vector<float> ts;
public:
    Curve() {
        // Curve
        glGenVertexArrays(1, &vaoCurve);
        glBindVertexArray(vaoCurve);

        glGenBuffers(1, &vboCurve);
        glBindBuffer(GL_ARRAY_BUFFER, vboCurve);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                              NULL);

        glGenVertexArrays(1, &vaoCtrlPoints);
        glBindVertexArray(vaoCtrlPoints);

        glGenBuffers(1, &vboCtrlPoints);
        glBindBuffer(GL_ARRAY_BUFFER, vboCtrlPoints);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              NULL);


        glGenVertexArrays(1, &vaoConvexHull);
        glBindVertexArray(vaoConvexHull);

        glGenBuffers(1, &vboConvexHull);
        glBindBuffer(GL_ARRAY_BUFFER, vboConvexHull);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              NULL);
    }

    virtual vec4 r(float t) { return wCtrlPoints[0]; }

    float tStart() { return ts[0]; }

    float tEnd() { return ts[wCtrlPoints.size() - 1]; }

    virtual void AddControlPoint(float cX, float cY) {
        vec4 wVertex = vec4(cX, cY, 0, 1) * camera.Pinv() * camera.Vinv();
        wCtrlPoints.push_back(wVertex);
    }

    int PickControlPoint(float cX, float cY) {
        vec4 wVertex = vec4(cX, cY, 0, 1) * camera.Pinv() * camera.Vinv();
        for (unsigned int p = 0; p < wCtrlPoints.size(); p++) {
            if (dot(wCtrlPoints[p] - wVertex, wCtrlPoints[p] - wVertex) < 0.1) return p;
        }
        return -1;
    }

    void MoveControlPoint(int p, float cX, float cY) {
        vec4 wVertex = vec4(cX, cY, 0, 1) * camera.Pinv() * camera.Vinv();
        wCtrlPoints[p] = wVertex;
    }

    void Draw() {
        mat4 VPTransform = camera.V() * camera.P();


        gpuProgram.setUniform(VPTransform, "MVP");

        int colorLocation = glGetUniformLocation(gpuProgram.getId(), "color");

        if (wCtrlPoints.size() > 0) {
            glBindVertexArray(vaoCtrlPoints);
            glBindBuffer(GL_ARRAY_BUFFER, vboCtrlPoints);
            glBufferData(GL_ARRAY_BUFFER, wCtrlPoints.size() * 4 * sizeof(float), &wCtrlPoints[0], GL_DYNAMIC_DRAW);
            if (colorLocation >= 0) glUniform3f(colorLocation, 1, 0, 0);
            glPointSize(5.0f);
            glDrawArrays(GL_POINTS, 0, wCtrlPoints.size());
        }

        if (wCtrlPoints.size() >= 2) {
            std::vector<float> vertexData;
            for (int i = 0; i < nTesselatedVertices; i++) {
                float tNormalized = (float) i / (nTesselatedVertices - 1);
                float t = tStart() + (tEnd() - tStart()) * tNormalized;
                vec4 wVertex = r(t);
                vertexData.push_back(wVertex.x);
                vertexData.push_back(wVertex.y);
            }

            glBindVertexArray(vaoCurve);
            glBindBuffer(GL_ARRAY_BUFFER, vboCurve);
            glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_DYNAMIC_DRAW);
            if (colorLocation >= 0) glUniform3f(colorLocation, 1.0f/102.0f, 1, 1);
            glDrawArrays(GL_LINE_LOOP, 0, nTesselatedVertices);
        }
    }
};

// A tanar ur gorbe szerkesztoje altal:
class Polynom : public Curve {

    //Sajat
    vec4 Hermite(vec4 p0, vec4 v0, float t0, vec4 p1, vec4 v1, float t1, float t) {
        vec4 a0 = p0;
        vec4 a1 = v0;
        vec4 a2 = (3 * (p1 - p0)) / pow(t1 - t0, 2) - (2 * v0 + v1) / (t1 - t0);
        vec4 a3 = (2 * (p0 - p1)) / pow(t1 - t0, 3) + (v0 + v1) / pow(t1 - t0, 2);

        return a3*pow(t - t0, 3) + a2*pow(t - t0, 2) + a1*(t - t0) + a0;
    }

public:
    void Add2(float cX, float cY){
        ts.push_back((float) wCtrlPoints.size());
        Curve::AddControlPoint(cX, cY);
    }

    //Sajat
    void AddControlPoint(float cX, float cY) {
        if (wCtrlPoints.size() < 3) {
            ts.push_back((float) wCtrlPoints.size());
            Curve::AddControlPoint(cX, cY);
        } else {
            vec4 click = vec4(cX, cY, 0, 1) * camera.Pinv() * camera.Vinv();
            float MinDistance;
            vec4 vertex;
            float location_t;

            ts.push_back((float) wCtrlPoints.size());
            wCtrlPoints.push_back(wCtrlPoints[0]);
            for (int i = 0; i < nTesselatedVertices; i++) {    // Tessellate
                float tNormalized = (float) i / (nTesselatedVertices - 1);

                float t = tStart() + (tEnd() - tStart()) * tNormalized;
                vec4 wVertex = r(t);

                float distance = dot(vec2(wVertex.x, wVertex.y) - vec2(click.x, click.y),
                                     vec2(wVertex.x, wVertex.y) - vec2(click.x, click.y));
                if (i == 0 || distance < MinDistance) {
                    MinDistance = distance;
                    vertex = wVertex;
                    location_t = t;
                }
            }

            ts.pop_back();
            wCtrlPoints.pop_back();

            bool found = false;
            for (int i = 0; i < ts.size(); i++) {
                if (ts[i] >= location_t) {
                    ts.push_back((float) wCtrlPoints.size());
                    wCtrlPoints = insertControlPoint(i, vertex, wCtrlPoints);
                    found = true;
                    break;
                }
            }
            if (!found) {
                ts.push_back((float) wCtrlPoints.size());
                wCtrlPoints.push_back(vertex);
            }
        }
    }

    //Sajat
    void RemoveControlPoint() {
        for (int i = 0; i < wCtrlPoints.size() / 2-1; ++i) {
            RemoveOneControlPoint();
        }
    }

    //Sajat
    void RemoveOneControlPoint() {
        if (wCtrlPoints.size() > 2) {
            float minDistance;
            int minIndex = 0;
            for (int i = 0; i < wCtrlPoints.size(); ++i) {
                if (i == 0) {
                    vec2 p1 = convertTo2D(wCtrlPoints[wCtrlPoints.size() - 1]);
                    vec2 p2 = convertTo2D(wCtrlPoints[i + 1]);
                    vec3 line = lineFromPoints(p1, p2);

                    vec2 p = convertTo2D(wCtrlPoints[i]);
                    vec2 AB_line = vec2(line.x, line.y);
                    minDistance = abs(dot(AB_line, p) + line.z)
                                  / sqrt(dot(AB_line, AB_line));
                } else if (i == wCtrlPoints.size() - 1) {
                    vec2 p1 = convertTo2D(wCtrlPoints[i - 1]);
                    vec2 p2 = convertTo2D(wCtrlPoints[0]);
                    vec3 line = lineFromPoints(p1, p2);

                    vec2 p = convertTo2D(wCtrlPoints[i]);
                    vec2 AB_line = vec2(line.x, line.y);
                    float d = abs(dot(AB_line, p) + line.z)
                              / sqrt(dot(AB_line, AB_line));

                    if (d < minDistance) {
                        minDistance = d;
                        minIndex = i;
                    }
                } else {
                    vec2 p1 = convertTo2D(wCtrlPoints[i - 1]);
                    vec2 p2 = convertTo2D(wCtrlPoints[i + 1]);
                    vec3 line = lineFromPoints(p1, p2);

                    vec2 p = convertTo2D(wCtrlPoints[i]);
                    vec2 AB_line = vec2(line.x, line.y);
                    float d = abs(dot(AB_line, p) + line.z)
                              / sqrt(dot(AB_line, AB_line));

                    if (d < minDistance) {
                        minDistance = d;
                        minIndex = i;
                    }
                }
            }

            wCtrlPoints = removeControlPoint(minIndex);
            ts = removeT(minIndex);
        }
    }

    //Sajat
    vec2 convertTo2D(vec4 v) {
        return vec2(v.x, v.y);
    }

    //Sajat
    vec3 lineFromPoints(vec2 p1, vec2 p2) {
        vec2 i = p2 - p1;
        vec2 n = vec2(-i.y, i.x);
        return vec3(n.x, n.y, -dot(n, p1));
    }

    //Sajat
    std::vector<vec4> removeControlPoint(int index) {
        std::vector<vec4> newCtrlPoints;
        for (int i = 0; i < wCtrlPoints.size(); i++) {
            if (i != index) {
                newCtrlPoints.push_back(wCtrlPoints[i]);
            }
        }

        return newCtrlPoints;
    }
    //Sajat
    std::vector<float> removeT(int index) {
        std::vector<float> newTs;
        for (int i = 0; i < ts.size(); i++) {
            if (i < index) {
                newTs.push_back(ts[i]);
            }else if(i > index){
                newTs.push_back(ts[i]-1);
            }
        }

        return newTs;
    }

    //Sajat
    void Smoothing() {
        unsigned size = wCtrlPoints.size();
        std::vector<vec4> newPoints = wCtrlPoints;
        std::vector<float> newTs = ts;
        int insert_delta = 0;
        for (int i = 0; i < size - 1; ++i) {
            float t0 = ts[i];
            float t1 = ts[i + 1];

            float middle_t = (t0 + t1) / 2;
            vec4 wVertex = r(middle_t, SmoothTension);

            newTs.push_back(newPoints.size());
            newPoints = insertControlPoint(i + 1 + insert_delta, wVertex, newPoints);
            insert_delta++;
        }
        ts.push_back(wCtrlPoints.size());
        wCtrlPoints.push_back(wCtrlPoints[0]);
        float middle_t = (ts[ts.size() - 2] + ts[ts.size() - 1]) / 2;
        vec4 wVertex = r(middle_t, SmoothTension);
        newTs.push_back(newPoints.size());
        newPoints.push_back(wVertex);

        ts = newTs;
        wCtrlPoints = newPoints;
    }

    std::vector<vec4> insertControlPoint(unsigned int i, vec4 newPoint, std::vector<vec4> oldPoints) {
        std::vector<vec4> newCtrlPoints;
        for (int j = 0; j < oldPoints.size(); ++j) {
            if (j == i) {
                newCtrlPoints.push_back(newPoint);
            }
            newCtrlPoints.push_back(oldPoints[j]);
        }
        return newCtrlPoints;
    }

    //Sajat
    vec4 r(float t) {
        return r(t, 1);
    }

    //Sajat
    vec4 v(vec4 r, vec4 r0, vec4 r1, float t, float t0, float t1, float tension) {
        float alpha = (1-tension)/2;
        return alpha * (((r1 - r) / (t1 - t)) + ((r - r0) / (t - t0)));
    }

    // Sajat
    vec4 r(float t, float tension) {
        for (int i = 0; i < wCtrlPoints.size() - 1; ++i) {
            if (ts[i] <= t && t <= ts[i + 1]) {
                int prev_i = (i > 0) ? i-1 : wCtrlPoints.size()-1;
                int next_i = i+1;
                int next_i2 = (i < wCtrlPoints.size()-2) ? i+2 : 0;

                vec4 v0 = v(wCtrlPoints[i], wCtrlPoints[prev_i], wCtrlPoints[next_i], ts[i], ts[i]-1, ts[i]+1, tension);
                vec4 v1 = v(wCtrlPoints[next_i], wCtrlPoints[i], wCtrlPoints[next_i2], ts[i]+1, ts[i], ts[i]+2, tension);

                return Hermite(wCtrlPoints[i], v0, ts[i], wCtrlPoints[i+1], v1, ts[i+1], t);
            }
        }
        return wCtrlPoints[0];
    }
};


Polynom *polygon;


void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    glLineWidth(2.0f);

    polygon = new Polynom();


    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}


void onDisplay() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    polygon->Draw();
    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
    if (key == 's') {
        polygon->Smoothing();
        glutPostRedisplay();
    }
    if (key == 'd') {
        polygon->RemoveControlPoint();
        glutPostRedisplay();
    }
}

void onKeyboardUp(unsigned char key, int pX, int pY) {

}

int pickedControlPoint = -1;

void onMouse(int button, int state, int pX, int pY) {
    if (button == GLUT_RIGHT_BUTTON &&
        state == GLUT_DOWN) {
        float cX = 2.0f * pX / windowWidth - 1;
        float cY = 1.0f - 2.0f * pY / windowHeight;

        pickedControlPoint = polygon->PickControlPoint(cX, cY);

        if(pickedControlPoint == -1) {
            polygon->AddControlPoint(cX, cY);
            pickedControlPoint = polygon->PickControlPoint(cX, cY);
        }
        glutPostRedisplay();
    }
    if (button == GLUT_RIGHT_BUTTON &&
        state == GLUT_UP) {
        pickedControlPoint = -1;
    }

    if(button == GLUT_LEFT_BUTTON &&
       state == GLUT_DOWN) {
        float cX = 2.0f * pX / windowWidth - 1;
        float cY = 1.0f - 2.0f * pY / windowHeight;

        polygon->Add2(cX, cY);
        pickedControlPoint = polygon->PickControlPoint(cX, cY);

        glutPostRedisplay();
    }
}

void onMouseMotion(int pX, int pY) {
    float cX = 2.0f * pX / windowWidth - 1;
    float cY = 1.0f - 2.0f * pY / windowHeight;
    if (pickedControlPoint >= 0) polygon->MoveControlPoint(pickedControlPoint, cX, cY);
}

void onIdle() {
    glutPostRedisplay();
}