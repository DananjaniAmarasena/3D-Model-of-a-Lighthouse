#include<windows.h>
#include<math.h>
#include <GL/glut.h>
#include<iostream>
#include<fstream>

using namespace std;

GLfloat x = 2.0f;
GLfloat y = 0.0f;
GLfloat z = 2.0f;

//variables to move
GLfloat moveBoat = 0.0f;
GLfloat moveX = 0.0f;
GLfloat moveY = 0.0f;
GLfloat moveZ = 0.0f;

//variables to rotate
GLfloat rot = 0.0f;
GLfloat rotX = 0.0f;
GLfloat rotY = 0.0f;
GLfloat rotZ = 0.0f;

//for animating the rotaion of objects
GLfloat animateRotation = 0.0f;

//variables to move the camera
GLfloat camX = 0.0f;
GLfloat camY = 0.0f;
GLfloat camZ = 0.0f;

//a quadratic object pointer used to draw the sides of the cylinder
GLUquadricObj* qobj;

//for camera 
GLfloat radius = 25.0;
GLfloat angle = 0.0;

struct BitMapFile
{
	int sizeX;
	int sizeY;
	unsigned char* data;
};

BitMapFile* getbmp(string filename) {
	int offset, headerSize;

	// Initialize bitmap files for RGB (input) and RGBA (output).
	BitMapFile* bmpRGB = new BitMapFile;
	BitMapFile* bmpRGBA = new BitMapFile;

	// Read input bmp file name.
	ifstream infile(filename.c_str(), ios::binary);

	// Get starting point of image data in bmp file.
	infile.seekg(10);
	infile.read((char*)&offset, 4);

	// Get header size of bmp file.
	infile.read((char*)&headerSize, 4);

	// Get image width and height values from bmp file header.
	infile.seekg(18);
	infile.read((char*)&bmpRGB->sizeX, 4);
	infile.read((char*)&bmpRGB->sizeY, 4);

	// Determine the length of zero-byte padding of the scanlines 
	// (each scanline of a bmp file is 4-byte aligned by padding with zeros).
	int padding = (3 * bmpRGB->sizeX) % 4 ? 4 - (3 * bmpRGB->sizeX) % 4 : 0;

	// Add the padding to determine size of each scanline.
	int sizeScanline = 3 * bmpRGB->sizeX + padding;

	// Allocate storage for image in input bitmap file.
	int sizeStorage = sizeScanline * bmpRGB->sizeY;
	bmpRGB->data = new unsigned char[sizeStorage];

	// Read bmp file image data into input bitmap file.
	infile.seekg(offset);
	infile.read((char*)bmpRGB->data, sizeStorage);

	// Reverse color values from BGR (bmp storage format) to RGB.
	int startScanline, endScanlineImageData, temp;
	for (int y = 0; y < bmpRGB->sizeY; y++)
	{
		startScanline = y * sizeScanline; // Start position of y'th scanline.
		endScanlineImageData = startScanline + 3 * bmpRGB->sizeX; // Image data excludes padding.
		for (int x = startScanline; x < endScanlineImageData; x += 3)
		{
			temp = bmpRGB->data[x];
			bmpRGB->data[x] = bmpRGB->data[x + 2];
			bmpRGB->data[x + 2] = temp;
		}
	}

	// Set image width and height values and allocate storage for image in output bitmap file.
	bmpRGBA->sizeX = bmpRGB->sizeX;
	bmpRGBA->sizeY = bmpRGB->sizeY;
	bmpRGBA->data = new unsigned char[4 * bmpRGB->sizeX * bmpRGB->sizeY];

	// Copy RGB data from input to output bitmap files, set output A to 1.
	for (int j = 0; j < 4 * bmpRGB->sizeY * bmpRGB->sizeX; j += 4)
	{
		bmpRGBA->data[j] = bmpRGB->data[(j / 4) * 3];
		bmpRGBA->data[j + 1] = bmpRGB->data[(j / 4) * 3 + 1];
		bmpRGBA->data[j + 2] = bmpRGB->data[(j / 4) * 3 + 2];
		bmpRGBA->data[j + 3] = 0xFF;
	}

	return bmpRGBA;

}

static unsigned int texture[4];

void loadExternalTextures() {
	BitMapFile* image[4];
	image[0] = getbmp("Images/water.bmp");
	image[1] = getbmp("Images/beach.bmp");
	image[2] = getbmp("Images/grass.bmp");

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->sizeX, image[1]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[2]->sizeX, image[2]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[2]->data);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void setupLighting() {
	GLfloat light_ambient[] = { 0.3,0.4,0.6,1.0 };
	GLfloat light_diffuse[] = { 0.7,0.7,0.7,1.0 };
	GLfloat light_specular[] = { 0.4,1.0,0.9,1.0 };
	GLfloat light_position[] = { 5,5,5,1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	GLfloat light_position2[] = { -5,-5,-5,1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);


	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
}



void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	GLfloat globalAmbient[] = { 0.1,0.1,0.1,1.0 };
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
	glEnable(GL_NORMALIZE);
	//initialization of qobj
	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluQuadricNormals(qobj, GLU_SMOOTH);
	setupLighting();
	glGenTextures(3, texture);
	loadExternalTextures();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void drawLightHouse() {
	//draw cylinders for light house 
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	//white
	glColor3f(1.0f, 1.0f, 1.0f);
	gluCylinder(qobj, 2.5, 2.5, 4, 60, 60);
	//red
	glTranslatef(0.0, 0.0, 4.0);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluCylinder(qobj, 2.5, 2.5, 4, 60, 60);
	//white
	glTranslatef(0.0, 0.0, 4.0);
	glColor3f(1.0f, 1.0f, 1.0f);
	gluCylinder(qobj, 2.5, 2.5, 4, 60, 60);
	//red
	glTranslatef(0.0, 0.0, 4.0);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluCylinder(qobj, 2.5, 2.5, 4, 60, 60);
	//bottom balcony
	glTranslatef(0.0, 0.0, 4.0);
	glColor3f(1.0f, 1.0f, 1.0f);
	gluDisk(qobj, 0.5, 3.5, 20, 50);
	//pillers around the balcony

	for (int i = 0; i < 361; i = i + 45)
	{

		glPushMatrix();
		glRotatef(-i, 0.0, 0.0, 1.0);
		glTranslatef(3.0, 0.0, 0.0);
		glColor3f(1.0f, 1.0f, 1.0f);
		gluCylinder(qobj, 0.1, 0.1, 1.2, 50, 50);
		glPopMatrix();

	}
	glTranslatef(0.0, 0.0, 1.0);
	glColor3f(1.0f, 1.0f, 1.0f);
	gluDisk(qobj, 3.4, 3.5, 50, 50);

	//middle pillers
	for (int i = 0; i < 361; i = i + 90)
	{

		glPushMatrix();
		glRotatef(-i, 0.0, 0.0, 1.0);
		glTranslatef(1.5, 0.0, 0.0);
		glColor4f(1.0f, 0.5f, 0.0f, 0.0f);
		gluCylinder(qobj, 0.2, 0.2, 1.2, 60, 60);
		glPopMatrix();

	}
	glTranslatef(0.0, 0.0, 1.5);
	glColor3f(1.0f, 1.0f, 1.0f);
	glutSolidCone(2.5, 2, 50, 50);


}

void drawEntryHouse() {
	//base
	glPushMatrix();
	glColor3f(0.1f, 0.2f, 0.1f);
	glScalef(1.6f, 0.0f, 1.6f);
	glutSolidCube(3.5);
	glPopMatrix();
	//house
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 2.0f, 0.0f);
	glutSolidCube(3);
	glPopMatrix();
	//roof
	glPushMatrix();
	glTranslatef(0.0f, 3.5f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.1f, 0.0f, 0.0f);
	glVertex3f(x - 1, y + 2, -z);
	glVertex3f(-x, y, -z);
	glVertex3f(-x, y, z);
	glVertex3f(x, y, z);
	glVertex3f(x - 1, y + 2, -z);
	glEnd();
	glPopMatrix();
	//door
	glPushMatrix();
	glTranslatef(0.0f, 0.55f, 4.0f);
	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(x - 1.5, y + 2, -z);
	glVertex3f(-(x - 1.5), y + 2, -z);
	glVertex3f(-(x - 1.5), y, -z);
	glVertex3f(x - 1.5, y, -z);
	glEnd();
	glPopMatrix();
}

void drawBridge() {
	glColor3f(0.1f, 0.0f, 0.0f);
	glPushMatrix();
	glScalef(2.0f, 0.0f, 5.0f);
	glutSolidCube(2.5);
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0);
	glTranslatef(-4.0f, 0.0f, -2.0f);
	for (int i = 0; i < 11; i = i + 2)
	{
		glPushMatrix();
		glRotatef(-90, 1.0, 0.0, 0.0);
		glTranslatef(i, 0.0, 0.0);
		glColor3f(0.1f, 0.0f, 0.0f);
		gluCylinder(qobj, 0.2, 0.2, 1.8, 50, 50);
		glPopMatrix();
	}
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0);
	glTranslatef(-4.0f, 0.0f, 2.0f);
	for (int i = 0; i < 11; i = i + 2)
	{
		glPushMatrix();
		glRotatef(-90, 1.0, 0.0, 0.0);
		glTranslatef(i, 0.0, 0.0);
		glColor3f(0.1f, 0.0f, 0.0f);
		gluCylinder(qobj, 0.2, 0.2, 1.8, 50, 50);
		glPopMatrix();
	}
	glTranslatef(10.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.5f, 0.0f);
	glutSolidTorus(0.5, 1, 50, 50);
	glPopMatrix();
}

void drawBoat() {
	glPushMatrix();
	glColor3f(0.2f, 0.3f, 0.0f);
	glTranslatef(1.0f, 1.0f, 0.0f);
	glScalef(5.0f, 0.5f, 1.0f);
	glutSolidCube(3.0);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.3f, 0.0f);
	glTranslatef(-0.0f, 2.0f, 0.0f);
	glScalef(5.0f, 0.5f, 1.0f);
	glutSolidCube(2.5);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.2f, 0.3f, 0.0f);
	glTranslatef(-1.0f, 3.0f, 0.0f);
	glScalef(5.0f, 0.5f, 1.0f);
	glutSolidCube(2.0);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	glRotatef(-90, 1.0, 0.0, 0.0);
	glTranslatef(-4.0f, 0.0f, 0.0f);
	gluCylinder(qobj, 0.3, 0.3, 5, 50, 50);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-90, 1.0, 0.0, 0.0);
	glTranslatef(-1.0f, 0.0f, 0.0f);
	gluCylinder(qobj, 0.3, 0.3, 5, 50, 50);
	glPopMatrix();
}

void drawStars() {
	glRotatef(-90, 1.0, 0.0, 0.0);
	for (int j = -30; j < 30; j = j + 2)
	{
		glPushMatrix();
		glTranslatef(j, 0.0f, 0.0f);
		for (int i = 0; i < 361; i = i + 35)
		{
			glPushMatrix();
			glRotatef(-i, 0.0, 0.0, 1.0);
			glTranslatef(10.0, 0.0, 0.0);
			glColor3f(1.0f, 0.9f, 0.5f);
			glutSolidSphere(0.07, 70, 20);
			glPopMatrix();
		}
		glPopMatrix();
	}

}


void drawLand() {
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y, -z);
	glTexCoord2f(0.0, 1.0); glVertex3f(-x, y, -z);
	glTexCoord2f(0.0, 0.0); glVertex3f(-x, y, z);
	glEnd();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	//gluLookAt(0.0+camX, 16.0 + camY,-25.0 + camZ, 0, 0, 0, 0, 1.0, 0);
	gluLookAt(radius * sin(angle), 8, radius * cos(angle), 0, 0, 0, 0, 1.0, 0);

	//light house
	glPushMatrix();
	glRotatef(-90, 1.0, 0.0, 0.0);
	drawLightHouse();
	glPopMatrix();
	glPushMatrix();
	//lightcylinder
	glRotatef(rot, 0, 1, 0);
	glTranslatef(0.0, 17.5, -1.0);
	glColor3f(1.0f, 0.7f, 0.5f);
	gluCylinder(qobj, 0.2, 1, 10, 50, 50);
	glPopMatrix();

	//entry house
	glPushMatrix();
	glTranslatef(0.0f, 1.0f, 4.0f);
	drawEntryHouse();
	glPopMatrix();

	//sea
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	//glColor3f(0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, 0.0f, -22.0f);
	glScalef(20.0f, 0.0f, 8.5f);
	drawLand();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//beach
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	//glColor3f(0.9f, 0.6f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	glScalef(20.0f, 0.0f, 3.0f);
	drawLand();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//grassLand
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	//glColor3f(0.2f, 0.6f, 0.0f);
	glTranslatef(0.0f, 0.0f, 8.0f);
	glScalef(20.0f, 0.0f, 1.0f);
	drawLand();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//rock1
	glPushMatrix();
	glTranslatef(-20.0f, 1.0f, 0.0f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glutSolidDodecahedron();
	glPopMatrix();

	//rock2
	glPushMatrix();
	glTranslatef(-23.0f, 1.0f, 0.0f);
	glScalef(1.5f, 1.5f, 1.5f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glutSolidDodecahedron();
	glPopMatrix();

	//rock3
	glPushMatrix();
	glTranslatef(-23.0f, 1.0f, 4.0f);
	glScalef(0.5f, 0.5f, 0.5f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glutSolidDodecahedron();
	glPopMatrix();

	//rock4
	glPushMatrix();
	glTranslatef(27.0f, 1.0f, 1.0f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glutSolidDodecahedron();
	glPopMatrix();

	//rock5
	glPushMatrix();
	glTranslatef(25.0f, 1.0f, -2.0f);
	glScalef(1.5f, 1.5f, 1.5f);
	glColor3f(0.1f, 0.1f, 0.1f);
	glutSolidDodecahedron();
	glPopMatrix();

	//bridge
	glPushMatrix();
	glTranslatef(0.0f, 1.5f, -8.0f);
	drawBridge();
	glPopMatrix();

	//boat1
	glPushMatrix();
	glTranslatef(moveBoat, 1.0f, -22.0f);
	drawBoat();
	glPopMatrix();

	//boat2
	glPushMatrix();
	glTranslatef(-moveBoat, 1.0f, -18.0f);
	drawBoat();
	glPopMatrix();

	//moon
	glPushMatrix();
	glTranslatef(8.0f, 25.0f, 0.0f);
	glColor3f(1.0f, 0.9f, 0.5f);
	glutSolidSphere(1, 70, 20);
	glPopMatrix();

	//stars
	glPushMatrix();
	glTranslatef(0.0f, 25.0f, -8.0f);
	drawStars();
	glPopMatrix();
	glPopMatrix();
	glutSwapBuffers();
	glDisable(GL_COLOR_MATERIAL);
}

void timer(int value) {
	rot += 1;
	moveBoat += 0.5;
	if (moveBoat > 20.0f)
	{
		moveBoat = -moveBoat;
	}
	glutTimerFunc(130, timer, 1);
	glutPostRedisplay();
}

void keyBoard(unsigned char key, int x, int y) {
	switch (key)
	{
	case '1':
		glEnable(GL_LIGHT0);
		break;
	case '!':
		glDisable(GL_LIGHT0);
		break;
	case '2':
		glEnable(GL_LIGHT1);
		break;
	case '@':
		glDisable(GL_LIGHT1);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void keyBoardSpecial(int key, int x, int y) {
	if (key == GLUT_KEY_RIGHT) {
		angle -= 1;
	}
	if (key == GLUT_KEY_LEFT) {
		angle += 1;
	}
	if (key == GLUT_KEY_UP) {

	}
	glutPostRedisplay();
}

void resize(GLsizei w, GLsizei h) {
	glViewport(0, 0, w, h);
	GLfloat aspect_ratio = h == 0 ? w / 1 : (GLfloat)w / (GLfloat)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(120.0, aspect_ratio, 1.0, 200.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(1500, 1500);
	glutCreateWindow("CS 308 Project");
	glutInitWindowPosition(100, 100);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutTimerFunc(230, timer, 1);
	//keyboard function activation
	glutKeyboardFunc(keyBoard);
	glutSpecialFunc(keyBoardSpecial);

	init();
	glutMainLoop();

	return 0;
}



