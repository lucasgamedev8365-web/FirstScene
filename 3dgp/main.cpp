	#include <iostream>
	#include <GL/glew.h>
	#include <3dgl/3dgl.h>
	#include <GL/glut.h>
	#include <GL/freeglut_ext.h>
#define _USE_MATH_DEFINES
	#include <math.h>

	// Include GLM core features
	#include "glm/glm.hpp"
	#include "glm/gtc/matrix_transform.hpp"

	#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

C3dglProgram program;

// 3D models
C3dglModel chairAndTable, vase, chicken, room, lamp, sphere, teapot, ceilingLamp;

//bitmaps
C3dglBitmap bm;

//texture buffers
GLuint idTexWood, idTexNone;

// The View Matrix
mat4 matrixView;

// Buffers
unsigned buf, ind;

//triangle vertices and normals
// Vertex Data:
float vertices[] = {
  -4, 14,-4, 0, 4,-7, 4, 14,-4, 0, 4,-7, 0, 0, 0, 0, 4,-7,
  -4, 14, 4, 0, 4, 7, 4, 14, 4, 0, 4, 7, 0, 0, 0, 0, 4, 7,
  -4, 14,-4,-7, 4, 0,-4, 14, 4,-7, 4, 0, 0, 0, 0,-7, 4, 0,
   4, 14,-4, 7, 4, 0, 4, 14, 4, 7, 4, 0, 0, 0, 0, 7, 4, 0,
  -4, 14,-4, 0,-1, 0,-4, 14, 4, 0,-1, 0, 4, 14,-4, 0,-1, 0,
   4, 14, 4, 0,-1, 0 };

// Index Data
unsigned indices[] = {
  0, 1, 2,	  // side triangle
  3, 4, 5,	  // side triangle
  6, 7, 8,	  // side triangle
  9, 10, 11,	  // side triangle
  12, 13, 14,	  // one of the base triangles
  13, 14, 15 };	  // the other one reuses two out of the three vertices


//chicken rotation
float rotation = 0;

//red Value of the light
float redValue = 0.5f;
bool redGoingUp = true;

//lamps on/off bool value
bool lamp1On = true;	
bool lamp2On = true;
bool spotLightOn = true;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

bool init()
{
	//glut setup
	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));

	// Initialise Shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;

	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_FLAT);
	//glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// setup lighting
	glEnable(GL_LIGHTING);									// --- DEPRECATED
	glEnable(GL_LIGHT0);									// --- DEPRECATED

	// load your 3D models here!
	if (!vase.load("models\\vase.obj")) return false;
	if (!chicken.load("models\\chicken.obj")) return false;
	if (!chairAndTable.load("models\\table.obj")) return false;
	if (!room.load("models\\LivingRoomObj\\LivingRoom.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;
	if (!sphere.load("models\\sphere.obj")) return false;
	if (!teapot.load("models\\utah_teapot_hires.obj")) return false;
	if (!ceilingLamp.load("models\\ceilinglamp.3ds")) return false;
	room.loadMaterials("models\\LivingRoomObj\\");

	// Generate 1 buffer name
	glGenBuffers(1, &buf);
	// Bind (activate) the buffer
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	// Send data to the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// GL_ARRAY_BUFFER – informs OGL that this is an array buffer
	// GL_STATIC_DRAW – informs OGL that the buffer is written once, read many times

	// prepare indices array
	glGenBuffers(1, &ind);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ind);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// load textures/ bitmaps
	bm.load("models/oak.bmp", GL_RGBA);
	if (!bm.getBits()) return false;
	// texture buffer
	//wood
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA,
		GL_UNSIGNED_BYTE, bm.getBits());

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);


	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0.0f, 10.0f, 10.0f),
		vec3(0.0f, 10.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f));

	// setup the screen background colour
	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);   // deep grey background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift to speed up your movement" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	program.sendUniform("texture0", 0);

	mat4 m;

	// setup View Matrix
	program.sendUniform("matrixView", matrixView);

	//specular light
	program.sendUniform("materialSpecular", vec3(0.8f, 0.8f, 0.8f)); //brightness of the bright-spots
	program.sendUniform("shininess",  50.0f); // lower levels of shininess means larger and sharper glare

	//directional light setup
	program.sendUniform("lightDir.direction", vec3(1.0f, 1.0f, 0.1f));
	program.sendUniform("lightDir.diffuse", vec3(0.5f, 0.5f, 0.5f));// dimmed white light
	
	//ambient light setup
	program.sendUniform("lightAmbient.color", vec3(0.0f, 0.0f, 0.0f)); // set quite dark
	program.sendUniform("materialAmbient", vec3(1.0f, 1.0f, 1.0f));

	// setup materials - brown

	//program.sendUniform("materialDiffuse", vec3(1.0f, 0.5f, 0.0f));

	glBindTexture(GL_TEXTURE_2D, idTexWood);
	//table
	m = matrixView;
	m = translate(m, vec3(-3.0f, -6.0f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.016f, 0.016f, 0.016f));
	chairAndTable.render(2, m);

	glBindTexture(GL_TEXTURE_2D, idTexWood);
	// setup materials - white

	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));

	//chair1 - furtherest middle from starting camera
	m = matrixView;
	m = translate(m, vec3(-3.0f, -6.0f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.016f, 0.016f, 0.016f));
	chairAndTable.render(0, m);
	chairAndTable.render(1, m);

	//chair2 - right of the starting camera
	m = matrixView;
	m = translate(m, vec3(1.0f, -6.0f, 0.0f));
	m = rotate(m, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.016f, 0.016f, 0.016f));
	chairAndTable.render(0, m);
	chairAndTable.render(1, m);

	//chair3 - left of the starting camera
	m = matrixView;
	m = translate(m, vec3(-7.0f, -6.0f, 0.0f));
	m = rotate(m, radians(270.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.016f, 0.016f, 0.016f));
	chairAndTable.render(0, m);
	chairAndTable.render(1, m);

	//chair4 - closest middle from starting camera
	m = matrixView;
	m = translate(m, vec3(-3.0f, -6.0f, 0.0f));
	m = rotate(m, radians(0.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.016f, 0.016f, 0.016f));
	chairAndTable.render(0, m);
	chairAndTable.render(1, m);

	//shininess of vase
	program.sendUniform("shininess", 2.0f);

	// setup materials - darker yellow

	program.sendUniform("materialDiffuse", vec3(0.8f, 0.8f, 0.0f));

	//lamp1 - top-left corner
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = matrixView;
	m = translate(m, vec3(-12.0f, 6.2f, -5.0f));
	m = rotate(m, radians(-45.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.07f, 0.07f, 0.07f));
	lamp.render(m);

	//lamp2 - bottom-right
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	m = matrixView;
	m = translate(m, vec3(4.0f, 6.2f, 5.0f));
	m = rotate(m, radians(-25.0f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.07f, 0.07f, 0.07f));
	lamp.render(m);
	
	// setup materials - yellow

	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 0.0f));

	//chicken
	m = matrixView;
	m = translate(m, vec3(-5.0f, 7.43f, -5.0f));
	m = rotate(m, radians(90.0f + rotation), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	chicken.render(m);

	//triangle
	m = matrixView;
	m = translate(m, vec3(-5.0f, 7.43f, -5.0f));
	m = rotate(m, radians(90.0f + rotation), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	// Bind (activate) the buffer
	glBindBuffer(GL_ARRAY_BUFFER, buf);

	// render nearly as usually
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 6 * sizeof(float), 0);
	glNormalPointer(GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	// Bind (activate) index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ind);
	// Draw triangles using 18 indices (unsigned int), starting at number 0
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	// Pendulum mechanics for ceiling lamp
	static float alpha = 0; // angular position (swing)
	static float omega = 0.7f; // angular velocity
	deltaTime = glm::min(deltaTime, 0.2f); // remove time distortions (longer than 0.2s)
	omega -= alpha * 0.05f * deltaTime; // Hooke's law: acceleration proportional to swing
	alpha += omega * deltaTime * 50; // motion equation: swing += velocity * delta-time

	// setup materials - black

	program.sendUniform("materialDiffuse", vec3(0.1f, 0.1f, 0.01));

	// Ceiling lamp
	m = matrixView;
	m = translate(m, vec3(-3.0f, 30.0f, 0.0f));
	m = scale(m, vec3(2.0f, 2.0f, 2.0f));
	m = rotate(m, radians(alpha), vec3(0.5, 0, 1));
	m = translate(m, vec3(0, -9, 0));
	mat4 m1 = m;
	program.sendUniform("spotlight1.matrix", m);
	m = translate(m, vec3(0, 9, 0));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	ceilingLamp.render(m);

	// setup materials - blue

	program.sendUniform("materialDiffuse", vec3(0.0f, 0.2f, 0.8f));

	// vase
	m = matrixView;
	m = translate(m, vec3(-3.0f, 6.15f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.4f, 0.4f, 0.4f));
	vase.render(m);
	
	// setup materials - green

	program.sendUniform("materialDiffuse", vec3(0.2f, 0.8f, 0.2f));

	//teapot
	m = matrixView;
	m = translate(m, vec3(3.0f, 6.0f, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(2.0f, 2.0f, 2.0f));
	program.sendUniform("matrixModelView", m);
	teapot.render(m);

	//blank texture
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	// setup materials - grey
	program.sendUniform("materialDiffuse", vec3(0.3f, 0.3f, 0.3f));

	vec3 pulsatingRed = vec3(redValue * float(lamp1On), 0.0f, 0.0f);
	// emissive light - point 1
	program.sendUniform("lightAmbient.color", pulsatingRed);
	program.sendUniform("lightPoint1.diffuse", pulsatingRed);
	program.sendUniform("lightPoint1.specular", pulsatingRed);

	//point light setup
	m = matrixView;
	m = translate(m, vec3(-13.8f, 10.3f, -6.85f));
	m = scale(m, vec3(0.013f, 0.013f, 0.013f));
	program.sendUniform("matrixModelView", m);
	sphere.render(m);
	program.sendUniform("lightPoint1.position", vec3(-13.8f, 10.3f, -6.85f));

	//emissive light - point 2
	program.sendUniform("lightAmbient.color", vec3(0.0f, 0.0f, float(lamp2On)));
	program.sendUniform("lightPoint2.diffuse", vec3(0.0f, 0.0f, float(lamp2On)));
	program.sendUniform("lightPoint2.specular", vec3(0.0f, 0.0f, float(lamp2On)));

	m = matrixView;
	m = translate(m, vec3(1.65f, 10.3f, 3.95f));
	m = scale(m, vec3(0.013f, 0.013f, 0.013f));
	program.sendUniform("matrixModelView", m);
	sphere.render(m);
	program.sendUniform("lightPoint2.position", vec3(1.65f, 10.3f, 3.95f));

	vec3 spotLightColour = vec3(float(spotLightOn), float(spotLightOn), float(spotLightOn));
	// spotlight set up
	program.sendUniform("lightAmbient.color", spotLightColour); //emissive
	program.sendUniform("spotlight1.diffuse", spotLightColour);
	program.sendUniform("spotlight1.specular", spotLightColour);
	program.sendUniform("spotlight1.cutoff", 0.4);
	program.sendUniform("spotlight1.direction", vec3(0.0f, -1.0f, 0.0f));
	program.sendUniform("spotlight1.attenuation", 5.0f);

	// light bulb
	m = m1;
	m = translate(m, vec3(0.0f, 3.96f, 0.0f));
	m = scale(m, vec3(0.004f, 0.004f, 0.004f));
	program.sendUniform("matrixModelView", m);
	sphere.render(m);
	program.sendUniform("spotlight1.position", vec3(0.0f, 3.96f, 0.0f));

	// essential for double-buffering technique
	//glutSwapBuffers();

	// the GLUT objects require the Model View Matrix setup
	glMatrixMode(GL_MODELVIEW);								// --- DEPRECATED
	glLoadIdentity();										// --- DEPRECATED
	glMultMatrixf((GLfloat*)&m);							// --- DEPRECATED
}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//rotate chicken
	rotation += 45 * deltaTime;
	if (redValue > 1) redGoingUp = false;
	else if (redValue < 0) redGoingUp = true;

	if (redGoingUp) redValue += 1 * deltaTime;
	else if (!redGoingUp) redValue -= 1 * deltaTime;

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	program.sendUniform("matrixProjection", matrixProjection);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	case '1': lamp1On = !lamp1On; redValue = 0.0f; break;
	case '2': lamp2On = !lamp2On; break;
	case '3': spotLightOn= !spotLightOn; break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene: First Example");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

