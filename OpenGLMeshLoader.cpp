#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <sstream>
#include <windows.h>     
#include <mmsystem.h>   
#include <glut.h>

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
GLuint tex2;

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 500;

GLfloat speed = 0.7f;
bool isJumping = false;
bool ascend = false;
GLfloat jumpHeight = 0.0f;
GLfloat jumpSpeed = 0.3f * speed;
int level = 1;
bool lose = false;
int cameraMode = 0; // 0 for third-person, 1 for first-person
GLfloat thirdPersonDistance = 10.0f; // Distance behind the cat for third-person view
bool firstWin = false;
bool secondWin = false;
bool bird = false;
bool tree = false;
bool lily1 = false;
bool lily2 = false;
bool fish = false;
const char* obsSound = "C:/Users/Dell/Downloads/obs";
const char* colSound = "C:/Users/Dell/Downloads/coin";
const char* loseSound = "C:/Users/Dell/Downloads/lose";
const char* winSound = "C:/Users/Dell/Downloads/win";
bool isObsSound = false;
bool isColSound = false;
bool isWinSound = false;
int score = 0;
bool bird2=false;
bool bird3=false;

class Vector
{
public:
	GLdouble x, y, z;

	Vector(GLdouble _x, GLdouble _y, GLdouble _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector operator+(Vector& v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}

	Vector operator-(Vector& v) {
		return Vector(x - v.x, y - v.y, z - v.z);
	}

	Vector operator*(Vector& v) {
		return Vector(x * v.x, y * v.y, z * v.z);
	}

	Vector operator/(Vector& v) {
		return Vector(x / v.x, y / v.y, z / v.z);
	}

	bool isEqual(Vector& v) {
		return (x == v.x && y == v.y && -0.1 <= (z - v.z) && (z - v.z) <= 0.1);
	}

};

Vector Eye(40, 5, 0);
//Vector Eye(0, 5, -10);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

// Model Variables
Model_3DS model_house;
Model_3DS model_cat;
Model_3DS model_bird;
Model_3DS model_tree;
Model_3DS model_key;
Model_3DS model_duck;
Model_3DS model_fish;
Model_3DS model_lily;
Model_3DS model_trophy;

// Textures
GLTexture tex_ground;
GLTexture tex_sea;

void playMusic(const char* filePath) {
	PlaySoundA(filePath, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void stopMusic() {
	PlaySound(NULL, NULL, SND_ASYNC);
}

void playObsSound() {
	if (!isObsSound) {
		isObsSound = true;
		stopMusic();
		playMusic(obsSound);

		glutTimerFunc(1000, [](int) {
			stopMusic();
			isObsSound = false;
			}, 0);
	}
}

void playColSound() {
	if (!isColSound) {
		isColSound = true;
		stopMusic();
		playMusic(colSound);

		glutTimerFunc(1000, [](int) {
			stopMusic();
			isColSound = false;
			}, 0);
	}
}

void playWinSound() {
	if (!isWinSound) {
		isWinSound = true;
		stopMusic();
		playMusic(winSound);

		glutTimerFunc(1000, [](int) {
			stopMusic();
			isWinSound = false;
			}, 0);
	}
}

void gameLose() {
	lose = true;
	stopMusic();
	playMusic(loseSound);
}

void drawText(const char* text, int length, int x, int y) {
	glMatrixMode(GL_PROJECTION);
	double* matrix = new double[16];
	glGetDoublev(GL_PROJECTION_MATRIX, matrix);
	glLoadIdentity();
	glOrtho(0, WIDTH, 0, WIDTH, -5, 5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glLoadIdentity();
	glRasterPos2i(x, y);
	for (int i = 0; i < length; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, (int)text[i]);
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);
}

GLfloat ambBright = 1.0;
GLfloat difBright = 1.0;
GLfloat specBright = 1.0;

void InitLightSource2()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	GLfloat ambient[] = { 0.0f, 0.0f, 0.0, 1.0f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	GLfloat diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, light_position);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.6);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05);
	glEnable(GL_NORMALIZE);
}

void InitMaterial()
{
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspectRatio, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	InitLightSource2();
	InitMaterial();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
}

// Render Ground Function
void RenderGround()
{
	//glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	if (level == 1) {
		glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	}
	else {
		glBindTexture(GL_TEXTURE_2D, tex_sea.texture[0]);	// Bind the ground texture
	}

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-80, 0, -80);
	glTexCoord2f(5, 0);
	glVertex3f(80, 0, -80);
	glTexCoord2f(5, 5);
	glVertex3f(80, 0, 80);
	glTexCoord2f(0, 5);
	glVertex3f(-80, 0, 80);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

Vector catPosition(0, 0, -78); // Initial position of the cat
Vector birdPosition(0, 0, -10);
Vector birdPosition2(-10, 0, -10);
Vector birdPositiona(0, 0, -50);
Vector birdPositionb(0, 0, -30);
//Vector birdPositionc(0, 0, -10);
//Vector birdPositiond(0, 0, -10);
//Vector birdPositione(0, 0, -10);
Vector treePosition(0, 0, 10);
Vector treePosition2(10, 0, 10);
Vector treePositiona(0, 0, 30);
Vector treePositionb(0, 0, 50);
//Vector treePositionc(0, 0, 10);
//Vector treePositiond(0, 0, 10);
//Vector treePositione(0, 0, 10);
Vector keyPosition(0, 0, 78);
Vector duckPosition(0, 0, -40);
Vector lilyPosition1(0, 0, -25);
Vector lilyPosition2(0, 0, -10);
Vector fishPosition(0, 0, 10);
Vector trophyPosition(0, 0, 30);

bool checkCollisionCat(Vector& v1, Vector& v2) {
	Vector newCat(v1.x, v1.y, v1.z + 2.0);
	return newCat.isEqual(v2); // Return true if points are the same
}

bool checkCollisionDuck(Vector& v1, Vector& v2) {
	Vector newDuck(v1.x, v1.y, v1.z + 1.0);
	return newDuck.isEqual(v2); // Return true if points are the same
}

bool checkCollisionDuckLily(Vector& v1, Vector& v2) {
	Vector newDuck(v1.x, v1.y, v1.z + 2.5);
	return newDuck.isEqual(v2); // Return true if points are the same
}

void updatePlayerPosition() {
	// Automatically move forward
	if (level == 1) {
		if (!tree) {
			catPosition.z += 0.016 * 10.0f * speed; // Adjust speed as needed

			ambBright -= 0.001 * speed;
			difBright -= 0.001 * speed;
			specBright -= 0.001 * speed;

			GLfloat ambient[] = { ambBright, ambBright, ambBright, 1.0f };
			glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
			GLfloat diffuse[] = { difBright, difBright, difBright, 1.0f };
			glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
			GLfloat specular[] = { specBright, specBright, specBright, 1.0f };
			glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		}
	}
	else {
		if (!lily1 && !lily2 && !secondWin) {
			duckPosition.z += 0.016 * 10.0f * speed;
		}
	}

	if (checkCollisionCat(catPosition, birdPositiona) && !bird2) {
		birdPositiona.y += 100;
		bird2 = true;
		playColSound();
		score += 10;
	}

	if (checkCollisionCat(catPosition, birdPositionb) && !bird3) {
		birdPositionb.y += 100;
		bird3 = true;
		playColSound();
		score += 10;
	}

	if ((checkCollisionCat(catPosition, birdPosition) || checkCollisionCat(catPosition, birdPosition2)) && !bird) {
		if (checkCollisionCat(catPosition, birdPosition)) {
			birdPosition.y += 100;
		}
		else {
			birdPosition2.y += 100;
		}
		bird = true;
		playColSound();
		score += 10;
	}

	if ((checkCollisionCat(catPosition, treePosition) || checkCollisionCat(catPosition, treePosition2) || checkCollisionCat(catPosition, treePositiona) || checkCollisionCat(catPosition, treePositionb)) && !tree) {
		tree = true;
		playObsSound();
		glutTimerFunc(2000, [](int) {
			gameLose();
			}, 0);
	}

	if ((((catPosition.z + 3) - keyPosition.z) >= -0.1) && (((catPosition.z + 3) - keyPosition.z) <= 0.1) && !firstWin) {
		if (catPosition.x == 0) {
			keyPosition.y += 100;
			firstWin = true;
			level = 2;
			playWinSound();
			score += 50;
			glEnable(GL_LIGHT0);

			GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
			glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
			GLfloat diffuse[] = { 0.1f, 0.1f, 0.1f, 1.0f };
			glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
			GLfloat specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
			glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
			GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
			glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		}
		else {
			gameLose();
		}
	}

	if (checkCollisionDuckLily(duckPosition, lilyPosition1) && !lily1) {
		lily1 = true;
		playObsSound();
		glutTimerFunc(2000, [](int) {
			gameLose();
			}, 0);
	}

	if (checkCollisionDuckLily(duckPosition, lilyPosition2) && !lily2) {
		lily2 = true;
		playObsSound();
		glutTimerFunc(2000, [](int) {
			gameLose();
			}, 0);
	}

	if (checkCollisionDuck(duckPosition, fishPosition) && !fish) {
		fishPosition.y += 100;
		fish = true;
		playColSound();
		score += 10;
	}


	if ((((duckPosition.z + 1.5) - trophyPosition.z) >= -0.1) && (((duckPosition.z + 1.5) - trophyPosition.z) <= 0.1) && !secondWin) {
		trophyPosition.y += 100;
		secondWin = true;
		playWinSound();
		score += 50;
	}

	if (isJumping) {
		if (jumpHeight > 5.0f) {
			ascend = false;
		}
		if (ascend) { // Ascend
			jumpHeight += jumpSpeed;
			if (level == 1) {
				catPosition.y += jumpSpeed;
			}
			else {
				duckPosition.y += jumpSpeed;
			}
		}
		if (!ascend) { // Descend
			jumpHeight -= jumpSpeed;
			if (level == 1) {
				catPosition.y -= jumpSpeed;
			}
			else {
				duckPosition.y -= jumpSpeed;
			}
		}
		if ((level == 1 && catPosition.y <= 0) || (level == 2 && duckPosition.y <= 0)) {
			isJumping = false;
		}
	}
}

void updateGame(int value) {
	updatePlayerPosition();

	glutPostRedisplay();

	//if (!lose && !secondWin) {
	glutTimerFunc(1, updateGame, 0); // Call again after ~16 ms
	//}
}

void myDisplay(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update the camera position and target based on the mode
	if (level == 1) {
		if (cameraMode == 0) {
			// Third-person camera
			Eye.x = catPosition.x;
			Eye.y = catPosition.y + 8.0f; // Slightly above the cat
			Eye.z = catPosition.z - thirdPersonDistance;
			At.x = catPosition.x;
			At.y = catPosition.y + 1.0f;
			At.z = catPosition.z + 20.0;
		}
		else if (cameraMode == 1) {
			// First-person camera
			Eye.x = catPosition.x;
			Eye.y = catPosition.y + 1.5f; // Align with the cat's height
			Eye.z = catPosition.z + 2.0f; // Slightly ahead of the cat
			At.x = catPosition.x;
			At.y = catPosition.y + 1.5f;
			At.z = catPosition.z + 5.0f; // Look forward
		}
	}
	else {
		if (cameraMode == 0) {
			// Third-person camera
			Eye.x = duckPosition.x;
			Eye.y = duckPosition.y + 8.0f; // Slightly above the cat
			Eye.z = duckPosition.z - thirdPersonDistance;
			At.x = duckPosition.x;
			At.y = duckPosition.y + 1.0f;
			At.z = duckPosition.z + 20.0;
		}
		else if (cameraMode == 1) {
			// First-person camera
			Eye.x = duckPosition.x;
			Eye.y = duckPosition.y + 1.5f; // Align with the cat's height
			Eye.z = duckPosition.z + 2.0f; // Slightly ahead of the cat
			At.x = duckPosition.x;
			At.y = duckPosition.y + 1.5f;
			At.z = duckPosition.z + 5.0f; // Look forward
		}
	}

	// Update the camera view
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);

	// Draw Ground
	RenderGround();

	if (level == 1) {

		//Draw Cat Model
		glPushMatrix();
		glTranslatef(catPosition.x, catPosition.y, catPosition.z);
		glScalef(4.0, 4.0, 4.0);
		model_cat.Draw();
		glPopMatrix();

		//Draw Bird Model
		glPushMatrix();
		glTranslatef(birdPosition.x, birdPosition.y, birdPosition.z);
		glScalef(4.0, 4.0, 4.0);
		model_bird.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(birdPositiona.x, birdPositiona.y, birdPositiona.z);
		glScalef(4.0, 4.0, 4.0);
		model_bird.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(birdPositionb.x, birdPositionb.y, birdPositionb.z);
		glScalef(4.0, 4.0, 4.0);
		model_bird.Draw();
		glPopMatrix();

		//Draw Bird Model
		glPushMatrix();
		glTranslatef(-10, birdPosition.y, birdPosition.z);
		glScalef(4.0, 4.0, 4.0);
		model_bird.Draw();
		glPopMatrix();

		// Draw Tree Model
		glPushMatrix();
		glTranslatef(treePosition.x, treePosition.y, treePosition.z);
		glScalef(0.1, 0.1, 0.1);
		model_tree.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(treePositiona.x, treePositiona.y, treePositiona.z);
		glScalef(0.1, 0.1, 0.1);
		model_tree.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(treePositionb.x, treePositionb.y, treePositionb.z);
		glScalef(0.1, 0.1, 0.1);
		model_tree.Draw();
		glPopMatrix();

		// Draw Tree Model
		glPushMatrix();
		glTranslatef(10, treePosition.y, treePosition.z);
		glScalef(0.1, 0.1, 0.1);
		model_tree.Draw();
		glPopMatrix();

		// Draw Key Model
		glPushMatrix();
		glTranslatef(keyPosition.x, keyPosition.y + 3.3, keyPosition.z);
		glRotatef(90.f, 0, 0, 1);
		glScalef(1.0, 1.0, 1.0);
		model_key.Draw();
		glPopMatrix();
	}
	else {

		GLfloat pos[] = { duckPosition.x, duckPosition.y + 0.2 , duckPosition.z + 5.0, 1.0 };
		glLightfv(GL_LIGHT1, GL_POSITION, pos);

		// Draw Duck Model
		glPushMatrix();
		glTranslatef(duckPosition.x, duckPosition.y + 1.5, duckPosition.z);
		glScalef(4.0, 4.0, 4.0);
		model_duck.Draw();
		glPopMatrix();

		// Draw Fish Model
		glPushMatrix();
		glTranslatef(fishPosition.x, fishPosition.y, fishPosition.z);
		glScalef(100.0, 100.0, 100.0);
		model_fish.Draw();
		glPopMatrix();

		// Draw Lily 1 Model
		glPushMatrix();
		glTranslatef(lilyPosition1.x, lilyPosition1.y + 1, lilyPosition1.z);
		glScalef(1.0, 1.0, 1.0);
		model_lily.Draw();
		glPopMatrix();

		// Draw Lily 2 Model
		glPushMatrix();
		glTranslatef(lilyPosition2.x, lilyPosition2.y + 1, lilyPosition2.z);
		glScalef(1.0, 1.0, 1.0);
		model_lily.Draw();
		glPopMatrix();

		// Draw Trophy Model
		glPushMatrix();
		glTranslatef(trophyPosition.x, trophyPosition.y, trophyPosition.z);
		glScalef(0.2, 0.2, 0.2);
		model_trophy.Draw();
		glPopMatrix();
	}

	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	if (level == 1) {
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, tex2);
	}
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);

	glPopMatrix();

	glColor3f(1.0, 0.0, 0.0);

	if (lose) {
		GLfloat ambient[] = { 1.0f, 1.0f, 1.0, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

		glDisable(GL_DEPTH_TEST); // Disable depth test to ensure the black quad covers everything
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, WIDTH, 0, HEIGHT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glColor3f(1.0f, 1.0f, 1.0f); // Set color to black
		glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(WIDTH, 0);
		glVertex2f(WIDTH, HEIGHT);
		glVertex2f(0, HEIGHT);
		glEnd();

		//printf("howa ehh el habal da");
		glColor3f(1.0, 0.0, 0.0);
		std::string loseText = "You Lose!";
		drawText(loseText.c_str(), loseText.size(), WIDTH / 2, HEIGHT + 350);
		std::string scoreText = "Score: " + std::to_string(score);
		drawText(scoreText.c_str(), scoreText.size(), WIDTH / 2, HEIGHT + 300);

		glEnable(GL_DEPTH_TEST); // Re-enable depth test
		glutSwapBuffers();
		return;
	}

	if (secondWin) {
		std::string loseText = "You Win!";
		drawText(loseText.c_str(), loseText.size(), WIDTH / 2, HEIGHT + 350);
		std::string scoreText = "Score: " + std::to_string(score);
		drawText(scoreText.c_str(), scoreText.size(), WIDTH / 2, HEIGHT + 300);
	}

	if (!lose && !secondWin) {
		std::string scoreText = "Score: " + std::to_string(score);
		drawText(scoreText.c_str(), scoreText.size(), 10, HEIGHT + 350);
	}

	glutSwapBuffers();
}

void myKeyboard(unsigned char button, int x, int y)
{
	switch (button)
	{
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'a':
		if (catPosition.x < 10 && !lose) {
			catPosition.x += 10;
		}
		break;
	case 'd':
		if (catPosition.x > -10 && !lose) {
			catPosition.x -= 10;
		}
		break;
	case ' ':
		if (!isJumping) {
			isJumping = true;
			ascend = true;
			jumpHeight = 0.0f;
		}
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		cameraMode = (cameraMode + 1) % 2;
	}
}

// Assets Loading Function
void LoadAssets()
{
	// Loading Model files
	//model_house.Load("Models/house/house.3DS");
	model_cat.Load("Models/cat/cat.3ds");
	model_bird.Load("Models/bird/bird.3ds");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_key.Load("Models/key/key.3ds");
	model_duck.Load("Models/duck/duck.3ds");
	model_fish.Load("Models/fish/fish.3ds");
	model_lily.Load("Models/lily/lily.3ds");
	model_trophy.Load("Models/trophy/trophy.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_sea.Load("Textures/sea.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
	loadBMP(&tex2, "Textures/sky2.bmp", true);
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Animal Surfers");
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
	glutMouseFunc(myMouse);
	myInit();
	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	updateGame(0);
	glutMainLoop();
}