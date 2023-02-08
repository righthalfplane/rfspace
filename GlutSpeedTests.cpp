#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#define GLEW_IN
#ifdef GLEW_IN
#include <GL/glew.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>	// GLUT OpenGL includes
#include <OpenGL/glext.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>	// GLUT OpenGL includes
#include <stdarg.h>
#endif

// c++ -o GlutSpeedTests GlutSpeedTests.cpp -Wno-deprecated-declarations -L/opt/X11/lib -lX11 -lpthread  -lglut -lGL -lGLU -lGLEW 

//GlutSpeedTests -nt 2000 -mode 0 -mode 1 -mode 2 -mode 3 -mode 4 -end
//GlutSpeedTests -nt 3000 -mode 3 -mode 4 -end
//GlutSpeedTests -nt 4100 -mode 3 -mode 4 -end

//dotriangles(2130); /* 9063153 Triangles */
//dotriangles(1420); /* 4025703 Triangles */
//dotriangles(1065); /* 2263128 Triangles */
//dotriangles(225L); /* 100128  Triangles */
//dotriangles(75);   /* 10878   Triangles */

#define ImmediateMode     0
#define CompiliedMode     1
#define VertexArrayMode   2
#define VboMode			  3  /* glDrawArrays */
#define VboMode2		  4  /* glDrawElements */


//#define NTRIANGLE 3000L    /* 17985003 Triangles */
//#define NTRIANGLE 2000L     /* 7990003 Triangles */
//#define NTRIANGLE 1000L     /* 1995003 Triangles */
//#define NTRIANGLE 500L      /* 100128 Triangles */
//#define NTRIANGLE 75L       /* 10878 Triangles */


static char *mode[] =
			{
				(char *)"Immediate",
				(char *)"Compilied",
				(char *)"VertexArray",
				(char *)"VboMode",
				(char *)"VboMode2",
			};

int runMode = VertexArrayMode;

long int triangle=225L;



#define EXTERN 

//#include "Utilities.h"
//#include "Scene.h"
//#include "BuildTriangles.h"

GLuint vboId = 0;                   // ID of VBO for vertex arrays


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


struct P{
	double x;
	double y;
	double z;
};

struct Pf{
	float x;
	float y;
	float z;
};

struct Nodes{
    float x1,y1,z1;
    float x2,y2,z2;
    float x3,y3,z3;
};

struct Colors{
	struct Pf c1;
	struct Pf c2;
	struct Pf c3;
};

struct Norm{
    struct Pf n1;
    struct Pf n2;
    struct Pf n3;
};

int buildtriangles(struct Scene *scene,struct Nodes *trin,struct Norm  *norm,struct Colors *color,GLuint *faces,long *trianglemax,long size);
	
#define		NSLABS		(3)
#define		BUNCHINGFACTOR	(10)
#define		PQSIZE		(10000)


#define Point Point2
 
#ifndef HUGE
#define HUGE 1e60
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif
 
typedef double Flt ;
typedef Flt Vec[3] ;
	
typedef float Fltf ;
typedef Fltf Vecf[3] ;

typedef Vecf Colorf ;
typedef Vecf Pointf ;
	
typedef Vec Colorv ;
typedef Vec Point ;
typedef Vec ColorVEC ;
 
#define MakeVector(x, y, z, v)		(v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecNegate(a)	(a)[0]=0-(a)[0];\
			(a)[1]=0-(a)[1];\
			(a)[2]=0-(a)[2];
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecLen(a)	(sqrt(VecDot(a,a)))
#define VecCopy(a,b)	 (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];
#define VecAdd(a,b,c)	 (c)[0]=(a)[0]+(b)[0];\
			 (c)[1]=(a)[1]+(b)[1];\
			 (c)[2]=(a)[2]+(b)[2]
#define VecSub(a,b,c)	 (c)[0]=(a)[0]-(b)[0];\
			 (c)[1]=(a)[1]-(b)[1];\
			 (c)[2]=(a)[2]-(b)[2]
#define VecComb(A,a,B,b,c)	(c)[0]=(A)*(a)[0]+(B)*(b)[0];\
				(c)[1]=(A)*(a)[1]+(B)*(b)[1];\
			 	(c)[2]=(A)*(a)[2]+(B)*(b)[2]
#define VecAddS(A,a,b,c)	 (c)[0]=(A)*(a)[0]+(b)[0];\
				 (c)[1]=(A)*(a)[1]+(b)[1];\
				 (c)[2]=(A)*(a)[2]+(b)[2]
#define VecCross(a,b,c)	 (c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1];\
			 (c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2];\
			 (c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]
 
#define ColorToVec(a,b)	 (b)[0]=(a.red);(b)[1]=(a.green);(b)[2]=(a.blue);

#define PToVec(a,b)	 (b)[0]=(a.x);(b)[1]=(a.y);(b)[2]=(a.z);


 
/* #define max(a,b) 	((a)>(b)?(a):(b)) */
/* #define min(a,b) 	((a)<(b)?(a):(b)) */
  
 
#define RayPoint(ray,t,point)	VecAddS(t,(ray)->D,(ray)->P,point)

  	
extern struct P Norm(struct P *p1);
extern struct P CrossN(struct P *v1,struct P *v2);
extern struct P Sub(struct P *p1,struct P *p2);
extern struct P Add(struct P *p1,struct P *p2);
extern struct P Mult(struct P *p1,double Factor);
extern double Len(struct P *v1);
extern double Dot(struct P *v1,struct P *v2);
extern struct P p(double x,double y,double z);
extern struct P Subf(struct Pf *p1,struct Pf *p2);



	int GetTime2(long *Seconds,long *milliseconds);
	void *cMalloc(unsigned long r,int tag);
	int zerol(char *p,unsigned long n);	
	void WarningBatch(char *message);
	double rtime(void);
	int cFree(char *p);


#define uint32 uint3232

//#include <df.h>


#define ClipPlane_Off       0
#define ClipPlane_Fixed   	1
#define ClipPlane_Eye  		2



#define AlmostOne 0.99999
#define TwoPower16 65536

struct Color{
    float r;
    float g;
    float b;
};

struct FireBall{
	struct P startLocation;
	struct P location;
	double radius;
	int numberOfPointsDum;
	struct Color color;
	int timeSteps;
	int updateSteps;
	int fireBallPoints;
	double stepSize;
	int autoUpdate;
	int opaque;
	double time;
	double yield;
	double HOB;
	double tmin;
	double t2max;
	double P2max;
	double power;
	double *locations;
	volatile int iAmUpdating;
};

struct ScaleInfo{
	int updateTemperatureScale;
	int showPalette;
	double dmin,dmax;
	int logscale;
	double a,b;
	double sPmax;
	double sPmin;
};

struct GlobalData{
	unsigned char *dpp;
	int xResolution;
	int yResolution;
//	float32 *data;
	double xmin,xmax;
	double ymin,ymax;
	double zmin,zmax;
	double vmin,vmax;
	double odv;
	double diagonal;
	double step;
	int nx;
	int ny;
	int nz;
	int length;
	unsigned char palname[256];
	unsigned char palette[256*3];
	double opacity[TwoPower16];
	struct Color colors[TwoPower16];
	struct Color background;
	int ThreadCount;
	int FileType;
	double scaleVolumeStep;
	struct FireBall fireball;
	struct FireBall fireballNew;
	struct ScaleInfo scale;
	struct ScaleInfo scaleNew;
	
};

struct DisplayInfo{
	float Ambient;
	float xStart;
	float yStart;
	float zStart;
	float yRange2;
	float dx;
	float dy;
	struct P V1;
	struct P V2;
	struct P V3;
	struct P pScreen;
	struct P S1,S2,S3;
	struct P Eye;
	int xsize;
};

struct Scene{
//    struct KdTree ObjectKdTree;
    struct CObjectListStruct *ObjectList;
	float xmin,xmax,ymin,ymax,zmin,zmax;
	struct P EyeStart;
	struct P Eye;
	struct P Target;
	struct P Find;
	struct P UpStart;
	struct P Up;
    struct P EyeLeft;
    struct P EyeRight;
	struct P Lamp;
	double WalkAngle;
	int WalkMotion;
	int WalkMotionContinuous;	
	double AspectRatio;
    long xResolution;
    long yResolution;
    double EyeSeparation;
    long Jitter;
    long JitterCount;	
    double Height;
	GLuint polyList;
	GLfloat nearv;
	GLfloat farv;
	int pointClick;
	double WorldStepSize;
	double WorldSize;
	double Degrees;
	double theta1;
	double theta2;
	double dtheta1;
	double dtheta2;
	double theta1Start;
	double theta2Start;
	int xStart;
	int yStart;
	int Tool;
	long nt;
	long nq;
	long debug;
    struct DisplayInfo g;
	
    struct P ClipPlanePoint;
    struct P ClipPlaneNormal;
    double ClipPlaneDistance;
    double ClipPlaneAngle;
    long ClipPlane;
    
    long RayTracing;
    long FixRayObjects;
	
    long threads;
	
	struct Color BackGround;
	
    long *CellBuff;
	double *zbuff;
		
    int	nLights;

	int UpDateScene;
};



#define RayPoint(ray,t,point)	VecAddS(t,(ray)->D,(ray)->P,point)

	int AdjustEye(struct Scene *scene,double Length);
	int RotateToTheta(struct Scene *scene);
	int CenterScreen(struct Scene *scene);
	int DefaultScene(struct Scene *scene);
	int alineEye(struct Scene *scene);	
	int SetScreen(struct Scene *scene);
	int CheckSceneBuffer(struct Scene *scene);
	int SetScales(struct Scene *scene);

#ifndef EXTERN
#define EXTERN extern
#endif

	EXTERN struct GlobalData g;
	



#define KEY_OFFSET 1000

#define w_Joystick_Forward			1
#define w_Joystick_Backward			2
#define w_Rotate_Faster				3
#define w_Rotate_Slower				4
#define w_Rotate_Left				5
#define w_Rotate_Right				6
#define w_Rotate_Up					7
#define w_Rotate_Down				8
#define w_Move_Up					9
#define w_Move_Down					10
#define w_Move_Left					11
#define w_Move_Right				12
#define w_Move_Forward				14
#define w_Move_Backward				15
#define w_Move_Faster				16
#define w_Move_Slower				17
#define w_Go_Up						18
#define w_Go_Down					19


#define ControlFindPoint			4
#define ControlModeRotateWorld		5
#define ControlModeWalk				6
#define ControlRotateEye			7
#define ControlOpenFile				8
#define ControlQuit					9
#define ControlRangeData		   10
#define ControlFireBall			   11


#define NoCommand	0
#define NewScene	1
#define QuitProgram	2


struct Scene sceneG;

void SetLighting(unsigned int mode);

int mJoyStick(struct Scene *scene,unsigned int buttonMask,int x, int y, int z);
int mGetMouse(struct Scene *scene,int button, int state, int x, int y);
int mKeys(struct Scene *scene,unsigned int key, int x, int y);
void JoyStick(unsigned int buttonMask,int x, int y, int z);
int mReshape(struct Scene *scene,int w, int h);
int mMenuSelect(struct Scene *scene,int item);
int init(struct Scene *scene);

int mTimer(struct Scene *scene);
void timer(int value);

void output(GLfloat x, GLfloat y, GLfloat z, char *format,...);
void output2(GLfloat x, GLfloat y, GLfloat z, char *format,...);

int doTestSpeed(struct Scene *scene);


int JoyStickOn=0;

int doHelp(struct Scene *scene);

int doWalkCommands(struct Scene *scene,int WalkMotion);

int Width=400;

int   main_window;

int ReDrawScene(struct Scene *scene);

void reshape(struct Scene *scene,int wscr,int hscr);

int doCommands(int command);

char WarningBuff[256];

int FindPoint(struct Scene *scene,int x,int y);

double xmin,ymin,xmax,ymax;
int w=800,h=800;

struct Nodes *trin;
struct Norm  *norm;
struct Colors *color;
GLuint *faces;

long trianglemax;

int dotriangles(long size);

int buildList(struct Scene *scene);

void menu_select(int item);
void keys(unsigned char key, int x, int y);
void keyss(int key, int x, int y);
void myReshape(int w, int h);
void getMouse(int button, int state, int x, int y);
void moveMouse(int x, int y);
void display(void);


char **argv;
int argc;
int n=1;


int main(int argc, char *argv[])
{
	for(int n=1;n<argc;++n){
		if(!strcmp(argv[n],"-nt")){
			triangle=(long)atof(argv[++n]);
		}else if(!strcmp(argv[n],"-mode")){
			runMode=(int)atof(argv[++n]);
		}
	}
	
	::argc=argc;
	::argv=argv;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800,600);
	
	main_window = glutCreateWindow(argv[0]);

#ifdef GLEW_IN
	{
		GLenum err;
		err=glewInit();
		if(err != GLEW_OK)
		{
			fprintf(stderr,"glewInit Error : %s\n",glewGetErrorString(err));
			exit(1);
		}
	}
#endif
	
	glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
    glutIdleFunc(display);
	glutKeyboardFunc(keys);
	glutSpecialFunc( keyss );
	glutMouseFunc(getMouse);
	glutMotionFunc(moveMouse);
	//glutJoystickFunc(JoyStick, 200);
	glutTimerFunc(1000,timer,10);
	
	//glEnable(GL_DEPTH_TEST);

	//glDepthFunc(GL_LEQUAL);
	
	//glClearColor(1.0,1.0,1.0,0.);
	
	init(&sceneG);
	
	SetLighting(3);
	
	//glEnable(GL_LIGHTING);
	
	//glShadeModel(GL_SMOOTH);
	
	glutCreateMenu(menu_select);
	glutAddMenuEntry("Range Data", ControlRangeData);
	glutAddMenuEntry("Open File", ControlOpenFile);
	glutAddMenuEntry("Fireball", ControlFireBall);
	glutAddMenuEntry("------------", -1);
	glutAddMenuEntry("Rotate World", ControlModeRotateWorld);
	glutAddMenuEntry("Find Point", ControlFindPoint);
	glutAddMenuEntry("Rotate Eye", ControlRotateEye);
	glutAddMenuEntry("Walk", ControlModeWalk);
	glutAddMenuEntry("------------", -1);
	glutAddMenuEntry("Quit", ControlQuit);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	glutMainLoop();
	return 0;
}


void menu_select(int item)
{
	mMenuSelect(&sceneG,item);
}

int mMenuSelect(struct Scene *scene,int item)
{
	if(!scene)return 1;
	
	switch(item)
	{
		case ControlQuit:
			exit(0);
			break;
/*
		case ControlOpenFile:
			dialogFileOpen(&sceneG);
			break;
		case ControlRangeData:
			dialogRangeData(&sceneG);
			break;
		case ControlFireBall:
			dialogFireBall(&sceneG);
			break;
 */
		case ControlFindPoint:
		case ControlModeRotateWorld:
		case ControlModeWalk:
		case ControlRotateEye:
			scene->Tool=item;
			break;
	}
	return 0;
}
void EndFrame();
void BeginFrame(struct Scene *scene);
int mDisplay2(struct Scene *scene);
void display(void)
{
	
		BeginFrame(&sceneG);
		mDisplay2(&sceneG);
		EndFrame();

}
void BeginFrame(struct Scene *scene)
{
    // BASIC STATE
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // CULLING

    //glCullFace(GL_BACK);
   // glEnable(GL_CULL_FACE);

     glColor3f(0, 0, 0);
     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    

      glShadeModel(GL_SMOOTH);
    

    // LIGHTING

	GLfloat light0pos[4];

	light0pos[0] = scene->Eye.x;
    light0pos[1] = scene->Eye.y;
    light0pos[2] = scene->Eye.z;
    light0pos[3] = 0.0f;


	GLfloat ambient[4]  = {0.2f,0.2f,0.2f,1.0f};
	GLfloat diffuse[4]  = {1.0f,1.0f,1.0f,1.0f};
	GLfloat specular[4] = {1.0f,1.0f,1.0f,1.0f};

	glLightfv(GL_LIGHT0,GL_POSITION,light0pos);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {90.0};

	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_COLOR_MATERIAL);
	
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	gluLookAt(scene->Eye.x,scene->Eye.y,scene->Eye.z,
			  scene->Target.x, scene->Target.y, scene->Target.z, scene->Up.x, scene->Up.y, scene->Up.z);

}

void EndFrame()
{
    glDisable(GL_COLOR_MATERIAL);

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);

    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0, 0, 0);
}

int mDisplay2(struct Scene *scene)
{
	
	if(!scene)return 1;
  		
	if(scene->polyList && (runMode == CompiliedMode) )
	{
		glCallList (scene->polyList);  
	}
	else if(runMode == VertexArrayMode)
	{
	
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, &trin[0].x1);
		glNormalPointer(GL_FLOAT, 0, &norm[0].n1.x);
		glColorPointer(3, GL_FLOAT, 0, &color[0].c1.x);
	
		glDrawArrays(GL_TRIANGLES, 0, trianglemax*3);
	
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	
	}
	else if(runMode == VboMode)
	{
	
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboId);
 		glVertexPointer(3, GL_FLOAT, 0,(void *)(0));
		glNormalPointer(GL_FLOAT, 0, (void *)((sizeof(struct Nodes))*trianglemax));
		glColorPointer(3, GL_FLOAT, 0, (void *)((sizeof(struct Norm)+sizeof(struct Nodes))*trianglemax));
	
		glDrawArrays(GL_TRIANGLES, 0, trianglemax*3);
	
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
 	}
	else if(runMode == VboMode2)
	{
	
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 3);
        glColorPointer(3, GL_FLOAT, 0, 0);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 2);
        glNormalPointer(GL_FLOAT, 0, 0);
            
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 1);
        glVertexPointer(3, GL_FLOAT, 0, 0);
	
		// glDrawArrays(GL_TRIANGLES, 0, trianglemax*3);

        glDrawElements(GL_TRIANGLES, trianglemax*3, GL_UNSIGNED_INT, faces);
	
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
 	}
	else
	{
		long n;
		
		glBegin (GL_TRIANGLES);
		
		for(n=0;n<trianglemax;++n){		
			glColor3fv(&color[n].c1.x);
			glNormal3fv(&norm[n].n1.x);
			glVertex3fv(&trin[n].x1);
			
			glColor3fv(&color[n].c2.x);
			glNormal3fv(&norm[n].n2.x);
			glVertex3fv(&trin[n].x2);
			
			glColor3fv(&color[n].c3.x);
			glNormal3fv(&norm[n].n3.x);
			glVertex3fv(&trin[n].x3);
			
		}
		glEnd();
		//runMode = ImmediateMode;
	}
	
	glutSwapBuffers();
  
    glDisable(GL_COLOR_MATERIAL);

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);

    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0, 0, 0);

  return 0;
}
int doTestSpeed(struct Scene *scene)
{
	double start,end;
	long count,n;
	double den;
	
	if(!scene)return 1;
	count=0;
	
	start=rtime();
	
	for(n=0;n<25;++n){
	    count++;
		scene->Eye.z -= scene->WorldStepSize*0.1;
		//doCommands(NewScene);
		display();
	}
	
	for(n=0;n<25;++n){
	    count++;
		scene->Eye.x += scene->WorldStepSize*0.1;
		//doCommands(NewScene);
		display();
	}
	
	
	for(n=0;n<25;++n){
	    count++;
		scene->Eye.z += scene->WorldStepSize*0.1;
		//doCommands(NewScene);
		display();
	}
	
	for(n=0;n<25;++n){
	    count++;
		scene->Eye.x -= scene->WorldStepSize*0.1;
		//doCommands(NewScene);
		display();
	}
	
	
	end=rtime();
	den=end-start;
	if(den <= 0)den=1;
	printf("%.2f Seconds %ld  Frames  %.2f (Frames/sec) %.2f (Million triangles/sec) xsize %d  ysize %d Mode \"%s\" triangles %ld\n",
		   den,count,((double)count)/(den),((double)count*(scene->nt+scene->nq))/(1.0e6*den),
		   glutGet(GLUT_WINDOW_WIDTH),glutGet(GLUT_WINDOW_HEIGHT),mode[runMode],trianglemax);
	
	return 0;

}
void keyss(int key, int x, int y)
{
     mKeys(&sceneG,key+KEY_OFFSET,x,y);
}
void keys(unsigned char key, int x, int y)
{
     mKeys(&sceneG,key,x,y);
}

void output(GLfloat x, GLfloat y,GLfloat z, char *format,...)
{
	va_list args;
	char buffer[200], *p;
	
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	glColor3f(0.0, 0.0, 0.0);
	glPushMatrix();
	
	//glScalef(0.05, 0.05, 1.0);
	
	glTranslatef(x, y, z);
	
	for (p = buffer; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, *p);
    glPopMatrix();
}
int doHelp(struct Scene *scene)
{
    int y,yinc;
/*
	int n,nstep;
*/
	
	if(!scene)return 1;
	
	
	
  glClear(GL_COLOR_BUFFER_BIT);
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 3000, 0, 3000);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  y=2800;
  yinc=150;
  output(80, y, 0, (char *)"     Fireball 36");
  y -= yinc;
  output(80, y, 0, (char *)"a  - aline eye with target");
  y -= yinc;
  output(80, y, 0, (char *)"l  - center objects");
  y -= yinc;
  output(80, y, 0, (char *)"c  - start continuous");
  y -= yinc;
  output(80, y, 0, (char *)"s  - stop continuous");
  y -= yinc;
  output(80, y, 0, (char *)"x  - set x-axis up");
  y -= yinc;
  output(80, y, 0, (char *)"y  - set y-axis up");
  y -= yinc;
  output(80, y, 0, (char *)"z  - set z-axis up");
  y -= yinc;
  output(80, y, 0, (char *)"+  - move faster");
  y -= yinc;
  output(80, y, 0, (char *)"-  - move slower");
  y -= yinc;
  output(80, y, 0, (char *)"f  - move forward");
  y -= yinc;
  output(80, y, 0, (char *)"b  - move backward");
  y -= yinc;
  output(80, y, 0, (char *)"r  - refresh screen");
  y -= yinc;
  output(80, y, 0, (char *)"d  - disable joystick");
  y -= yinc;
  output(80, y, 0, (char *)"h  - help");
  y -= yinc;
  output(80, y, 0, (char *)"left  arrow  - move left");
  y -= yinc;
  output(80, y, 0, (char *)"right arrow  - move right");
  y -= yinc;
  output(80, y, 0, (char *)"up    arrow  - move up");
  y -= yinc;
  output(80, y, 0, (char *)"down  arrow  - move down");
  y -= yinc;

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glutSwapBuffers();
	
  glMatrixMode(GL_MODELVIEW);
	
	
	return 0;
}
int mKeys(struct Scene *scene,unsigned int key, int x, int y)
{
	int reDraw;
	
	if(!scene)return 1;
	
	// fprintf(stderr,"key = %d\n",key);
	
	reDraw=FALSE;
	
	switch(key)
	{
		case 27:
		case 'q':
			//PrintHit(scene);
			doCommands(QuitProgram);
			break;
			//	case 'd':
			//	    JoyStickOn = !JoyStickOn;
			//		break;
		case 'i':
			// WriteJpeg(scene);
			reDraw=TRUE;
			break;
		case 'l':
			CenterScreen(scene);
			reDraw=TRUE;
			break;
		case 'h':
			doHelp(scene);
			return 0;
		case 'c':
			scene->WalkMotionContinuous=1;
			break;
		case 's':
			scene->WalkMotionContinuous=0;
			break;
		case 'a':
			alineEye(scene);
			reDraw=TRUE;
			break;
		case 'r':
			reDraw=TRUE;
			break;
		case 'x':
			scene->Up=p(1.,0.,0.);
			reDraw=TRUE;
			break;
		case 'y':
			scene->Up=p(0.,1.,0.);
			reDraw=TRUE;
			break;
		case 'z':
			scene->Up=p(0.,0.,1.);
			reDraw=TRUE;
			break;
		case 'o':
			/*
			g.fireball.location=g.fireball.startLocation;
			if(g.fireball.autoUpdate)
			{
				printf("Waiting For Update To Finish\n");
				g.fireball.autoUpdate=FALSE;
				while(g.fireball.iAmUpdating);
				UpdateTemperatures(scene);
				g.fireball.autoUpdate=TRUE;
			}
			else 
			{
				UpdateTemperatures(scene);
					
			}
			*/
			reDraw=TRUE;
			break;
		case '+':
			if(scene->Tool == ControlRotateEye){
				doWalkCommands(scene,w_Rotate_Faster);
			}else{
				doWalkCommands(scene,w_Move_Faster);
			}
			break;
		case '-':
			if(scene->Tool == ControlRotateEye){
				doWalkCommands(scene,w_Rotate_Slower);
			}else{
				doWalkCommands(scene,w_Move_Slower);
			}
			break;
		case 't':
			doTestSpeed(scene);
			//doTestPoints(scene);
			break;
		case 'f':
			doWalkCommands(scene,w_Move_Forward);
			break;
		case 'b':
			doWalkCommands(scene,w_Move_Backward);
			break;
		case 'u':
			doWalkCommands(scene,w_Go_Up);
			break;
		case 'd':
			doWalkCommands(scene,w_Go_Down);
			break;
		case GLUT_KEY_LEFT+KEY_OFFSET:
			if(scene->Tool == ControlRotateEye){
				doWalkCommands(scene,w_Rotate_Left);
			}else{
				doWalkCommands(scene,w_Move_Left);
			}
			break;
		case GLUT_KEY_RIGHT+KEY_OFFSET:
			if(scene->Tool == ControlRotateEye){
				doWalkCommands(scene,w_Rotate_Right);
			}else{
				doWalkCommands(scene,w_Move_Right);
			}
			break;
		case GLUT_KEY_UP+KEY_OFFSET:
			if(scene->Tool == ControlRotateEye){
				doWalkCommands(scene,w_Rotate_Up);
			}else{
				doWalkCommands(scene,w_Move_Up);
			}
			break;
		case GLUT_KEY_DOWN+KEY_OFFSET:
			if(scene->Tool == ControlRotateEye){
				doWalkCommands(scene,w_Rotate_Down);
			}else{
				doWalkCommands(scene,w_Move_Down);
			}
			break;
			
			
		default:
			return 0;
			
	}
	
	if(reDraw == TRUE)
	{
		doCommands(NewScene);
	}
	
	return 0;
	
}

void myReshape(int w, int h)
{
		mReshape(&sceneG,w,h);

}

void reshape(struct Scene *scene,int wscr,int hscr)
{
	w=wscr/4; h=hscr/4;
	wscr=w*4;
	hscr=h*4;
	w=wscr; h=hscr;
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	xmin=ymin=0.0; xmax=ymax=1.0;
	if(w<=h)
		ymax=1.0*(GLfloat)h/(GLfloat)w;
	else
		xmax=1.0*(GLfloat)w/(GLfloat)h;
	
	gluOrtho2D(xmin,xmax,ymin,ymax);
    glMatrixMode(GL_MODELVIEW);
    
    scene->xResolution=wscr;
    scene->yResolution=hscr;
    if(g.dpp != NULL)doCommands(NewScene);
}

int  mReshape(struct Scene *scene,int wscr, int hscr)
{

  double len;

	w=wscr/4; h=hscr/4;
	wscr=w*4;
	hscr=h*4;
	w=wscr; h=hscr;
	
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
    
	len=scene->nearv*tan(0.5*scene->Degrees*atan(1.0)/45.);

    glFrustum(-len*(GLfloat)w/(GLfloat)h, len*(GLfloat) w/(GLfloat)h, -len,
              len, scene->nearv, scene->farv);
			  	  
	glMatrixMode(GL_MODELVIEW);
	
	return 0;
}
void getMouse(int button, int state, int x, int y)
{
	mGetMouse(&sceneG,button,state,x,y);
}
int mGetMouse(struct Scene *scene,int button, int state, int x, int y)
{
	
	
	if(!scene)return 1;
	
	scene->dtheta1=0;
	scene->dtheta2=0;
	scene->theta1Start=scene->theta1;
	scene->theta2Start=scene->theta2;
	
	scene->xStart=x;
	scene->yStart=y;
	scene->EyeStart=scene->Eye;
	scene->UpStart=scene->Up;
	
	if(state == GLUT_UP)
	{
		if(g.scaleVolumeStep < 1.0)
		{
			g.scaleVolumeStep=1.0;
			doCommands(NewScene);
		}
	}
	
	
	if(scene->Tool == ControlModeRotateWorld){
		
		;
		
	}else if(scene->Tool == ControlModeWalk){
		
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
			
			if(!FindPoint(scene,x,y))
			{
				scene->Target=scene->Find;
				glutPostRedisplay();
			}
		}
		
	}else if(scene->Tool == ControlFindPoint){
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
			/*
			 printf("down button %d state %d x %d y %d\n",button,state,x,y);
			 */
			if(!FindPoint(scene,x,y))
			{
				GLfloat pos[3];
				printf("\n");
				printf("x %g y %g z %g\n",scene->Find.x,scene->Find.y,scene->Find.z);
				glGetLightfv(GL_LIGHT0,GL_POSITION,pos);
				printf("E lamp.x %g lamp.y %g lamp.z %g\n",pos[0],pos[1],pos[2]);
				printf("eye.x %g eye.y %g eye.z %g\n",scene->Eye.x,scene->Eye.y,scene->Eye.z);
				printf("Target.x %g Target.y %g Target.z %g\n",scene->Target.x,scene->Target.y,scene->Target.z);
				printf("G lamp.x %g lamp.y %g lamp.z %g\n",pos[0]+scene->Eye.x,pos[1]+scene->Eye.y,pos[2]+scene->Eye.z);
			}
		}
	}
	
	return  0;
}
int FindPoint(struct Scene *scene,int x,int y)
{
	
 	double modelview[16], projection[16];
	GLint viewport[4];
	float z = 1;
	
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetIntegerv( GL_VIEWPORT, viewport );
	
	glReadPixels( x, viewport[3]-y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );	
	if(z == 1.0){
		scene->pointClick=0;
		return 1;
	}else{
		gluUnProject( x, viewport[3]-y, z, modelview, projection, viewport, &scene->Find.x, &scene->Find.y, &scene->Find.z);
	}
	return 0;
}
int mMoveMouse(struct Scene *scene, int x, int y)
 {
 	if(!scene)return 1;
	
	scene->dtheta1= 0.25*(scene->xStart-x);
	scene->dtheta2= 0.25*(y-scene->yStart);
/*
   printf("dtheta1 %g dtheta2 %g\n",scene->dtheta1,scene->dtheta2);
*/ 	
	RotateToTheta(scene);
	 
	g.scaleVolumeStep=0.05;
	 
	doCommands(NewScene);
	 
   return 0;
 }
void moveMouse(int x, int y)
{
	mMoveMouse(&sceneG, x, y);
}
void JoyStick(unsigned int buttonMask,int x, int y, int z)
{
	mJoyStick(&sceneG,buttonMask,x,y,z);
}
int mJoyStick(struct Scene *scene,unsigned int buttonMask,int x, int y, int z)
{
	double factor=50;
	int reDraw=FALSE;
	struct P diff;
	struct P xl,yl;
	double step;
	double len;
	double yy;	
	double pi=4.0*atan(1.0);
	
	
	if(!scene)return 1;
	
	
	if(!JoyStickOn)return 0;
	
	if(buttonMask & 1){
	    scene->WalkMotion=w_Joystick_Forward;
		return 0;
	}

	if(buttonMask & 2){
	    scene->WalkMotion= w_Joystick_Backward;
		return 0;
	}
	
	/*
	 printf("buttonMask %d x %d y %d z %d \n",buttonMask,x,y,z); 
	*/
	if(x < -200){
	    step=x/(5*factor);
		step *= (pi/360.);
		step = -step;
	    xl=Sub(&scene->Target,&scene->Eye);
		len=Len(&xl);
		xl=Norm(&xl);
		yl=CrossN(&xl,&scene->Up);
		
		scene->Target.x=scene->Eye.x+len*(cos(step)*xl.x+sin(step)*yl.x);
		scene->Target.y=scene->Eye.y+len*(cos(step)*xl.y+sin(step)*yl.y);
		scene->Target.z=scene->Eye.z+len*(cos(step)*xl.z+sin(step)*yl.z);
		reDraw=TRUE;
	}
	if(x > 200){
	    step=x/(5*factor);
		step *= (pi/360.);
		step = -step;
	    xl=Sub(&scene->Target,&scene->Eye);
		len=Len(&xl);
		xl=Norm(&xl);
		yl=CrossN(&xl,&scene->Up);
		
		scene->Target.x=scene->Eye.x+len*(cos(step)*xl.x+sin(step)*yl.x);
		scene->Target.y=scene->Eye.y+len*(cos(step)*xl.y+sin(step)*yl.y);
		scene->Target.z=scene->Eye.z+len*(cos(step)*xl.z+sin(step)*yl.z);
		reDraw=TRUE;
	}
	
	yy=(y-127)*1000.0/128.0;
	
	if(yy < -200){
	    diff=Sub(&scene->Target,&scene->Eye);
		diff=Norm(&diff);
	    step=-(yy*scene->WorldStepSize)/(10*factor);
	    scene->Eye.x += step*diff.x;
	    scene->Eye.y += step*diff.y;
	    scene->Eye.z += step*diff.z;
	    scene->Target.x += step*diff.x;
	    scene->Target.y += step*diff.y;
	    scene->Target.z += step*diff.z;
		reDraw=TRUE;
	}
	if(yy > 200){
	    diff=Sub(&scene->Target,&scene->Eye);
		diff=Norm(&diff);
	    step=(yy*scene->WorldStepSize)/(10*factor);
	    scene->Eye.x -= step*diff.x;
	    scene->Eye.y -= step*diff.y;
	    scene->Eye.z -= step*diff.z;
	    scene->Target.x -= step*diff.x;
	    scene->Target.y -= step*diff.y;
	    scene->Target.z -= step*diff.z;
		reDraw=TRUE;
	}

	if(z < -200){
	    step=-(z*scene->WorldStepSize)/(50*factor);
	    scene->Eye.x += step*scene->Up.x;
	    scene->Eye.y += step*scene->Up.y;
	    scene->Eye.z += step*scene->Up.z;
	    scene->Target.x += step*scene->Up.x;
	    scene->Target.y += step*scene->Up.y;
	    scene->Target.z += step*scene->Up.z;
		reDraw=TRUE;
	}
	if(z > 200){
	    step=(z*scene->WorldStepSize)/(50*factor);
	    scene->Eye.x -= step*scene->Up.x;
	    scene->Eye.y -= step*scene->Up.y;
	    scene->Eye.z -= step*scene->Up.z;
	    scene->Target.x -= step*scene->Up.x;
	    scene->Target.y -= step*scene->Up.y;
	    scene->Target.z -= step*scene->Up.z;
		reDraw=TRUE;
	}
	
	if(reDraw == TRUE)
	{
		doCommands(NewScene);
	}
	
	return 0;
	
}
int mTimer2(struct Scene *scene)
{
	if(!scene)return 1;
	
	if(n >= argc)return 0;
	
	//printf("argv[n] %s ",argv[n]);
	
	if(!strcmp(argv[n],"-nt")){
		triangle=(long)atof(argv[++n]);
	}else if(!strcmp(argv[n],"-mode")){
		runMode=(int)atof(argv[++n]);
		buildList(&sceneG);
		//printf("n1 %d mTimer2 triangle %ld runMode %ld\n",n,(long)triangle,(long)runMode);
		doTestSpeed(scene);
		//printf("n2 %d mTimer2 triangle %ld runMode %ld\n",n,(long)triangle,(long)runMode);
	}else if(!strcmp(argv[n],"-end")){
		doCommands(QuitProgram);;
	}	
	

	
	n++;

	return 0;
}

void timer(int value)
{
	mTimer2(&sceneG);
	glutTimerFunc(1000,timer,10);
}
int mTimer(struct Scene *scene)
{
	if(!scene)return 1;
	
	if((scene->WalkMotion > 0) && scene->WalkMotionContinuous){
		doWalkCommands(scene,scene->WalkMotion);
	}
	
	return 0;
}
int doWalkCommands(struct Scene *scene,int WalkMotion)
{
	struct P EyeToTarget;
	struct P Vx;
	struct P Vy;
	struct P Vz;
	double WalkAngle;
	double Length;
	struct P diff;
	double step;
	int reDraw;
	
	if(!scene)return 1;

  EyeToTarget=Sub(&scene->Target,&scene->Eye);
  Length=Len(&EyeToTarget);
  EyeToTarget=Norm(&EyeToTarget);
  Vx=CrossN(&EyeToTarget,&scene->Up);
  Vy=scene->Up;
  Vz=Mult(&EyeToTarget,-1.0);
 	WalkAngle=scene->WalkAngle;	
	if(WalkAngle <= 0){
	    WalkAngle=5;
		scene->WalkAngle=WalkAngle;
	}
	WalkAngle *= atan(1.0)/45;

	reDraw=FALSE;
	
	switch(WalkMotion)
	{
		case w_Rotate_Faster:
			scene->WalkAngle *= 2;
		break;
		case w_Rotate_Slower:
			scene->WalkAngle /= 2;
		break;
		case w_Rotate_Left:
        	scene->Target.x = scene->Eye.x-Length*(cos(WalkAngle)*Vz.x+sin(WalkAngle)*Vx.x);
        	scene->Target.y = scene->Eye.y-Length*(cos(WalkAngle)*Vz.y+sin(WalkAngle)*Vx.y);
        	scene->Target.z = scene->Eye.z-Length*(cos(WalkAngle)*Vz.z+sin(WalkAngle)*Vx.z);
			scene->WalkMotion=w_Rotate_Left;
			reDraw=TRUE;
		break;
		case w_Rotate_Right:
        	scene->Target.x = scene->Eye.x-Length*(cos(-WalkAngle)*Vz.x+sin(-WalkAngle)*Vx.x);
        	scene->Target.y = scene->Eye.y-Length*(cos(-WalkAngle)*Vz.y+sin(-WalkAngle)*Vx.y);
        	scene->Target.z = scene->Eye.z-Length*(cos(-WalkAngle)*Vz.z+sin(-WalkAngle)*Vx.z);
			scene->WalkMotion=w_Rotate_Right;
			reDraw=TRUE;
		break;
		case w_Rotate_Up:		
        	scene->Target.x = scene->Eye.x-Length*(cos(-WalkAngle)*Vz.x+sin(-WalkAngle)*Vy.x);
        	scene->Target.y = scene->Eye.y-Length*(cos(-WalkAngle)*Vz.y+sin(-WalkAngle)*Vy.y);
        	scene->Target.z = scene->Eye.z-Length*(cos(-WalkAngle)*Vz.z+sin(-WalkAngle)*Vy.z);
			scene->WalkMotion=w_Rotate_Up;
			reDraw=TRUE;
		break;
		case w_Rotate_Down:
        	scene->Target.x = scene->Eye.x-Length*(cos(WalkAngle)*Vz.x+sin(WalkAngle)*Vy.x);
        	scene->Target.y = scene->Eye.y-Length*(cos(WalkAngle)*Vz.y+sin(WalkAngle)*Vy.y);
        	scene->Target.z = scene->Eye.z-Length*(cos(WalkAngle)*Vz.z+sin(WalkAngle)*Vy.z);
			scene->WalkMotion=w_Rotate_Down;
			reDraw=TRUE;
		break;
		case w_Go_Up:
			scene->Eye.x += scene->Up.x*scene->WorldStepSize;
			scene->Eye.y += scene->Up.y*scene->WorldStepSize;
			scene->Eye.z += scene->Up.z*scene->WorldStepSize;
			scene->Target.x += scene->Up.x*scene->WorldStepSize;
			scene->Target.y += scene->Up.y*scene->WorldStepSize;
			scene->Target.z += scene->Up.z*scene->WorldStepSize;
			scene->WalkMotion=w_Go_Up;
			reDraw=TRUE;
			break;
		case w_Go_Down:
			scene->Eye.x -= scene->Up.x*scene->WorldStepSize;
			scene->Eye.y -= scene->Up.y*scene->WorldStepSize;
			scene->Eye.z -= scene->Up.z*scene->WorldStepSize;
			scene->Target.x -= scene->Up.x*scene->WorldStepSize;
			scene->Target.y -= scene->Up.y*scene->WorldStepSize;
			scene->Target.z -= scene->Up.z*scene->WorldStepSize;
			scene->WalkMotion=w_Go_Down;
			reDraw=TRUE;
			break;
		case w_Move_Up:
			scene->Eye.x += scene->Up.x*scene->WorldStepSize;
			scene->Eye.y += scene->Up.y*scene->WorldStepSize;
			scene->Eye.z += scene->Up.z*scene->WorldStepSize;
			AdjustEye(scene,Length);
			scene->WalkMotion=w_Move_Up;
			reDraw=TRUE;
		break;
		case w_Move_Down:
			scene->Eye.x -= scene->Up.x*scene->WorldStepSize;
			scene->Eye.y -= scene->Up.y*scene->WorldStepSize;
			scene->Eye.z -= scene->Up.z*scene->WorldStepSize;
			AdjustEye(scene,Length);
			scene->WalkMotion=w_Move_Down;
			reDraw=TRUE;
		break;
		case w_Move_Left:
			scene->Eye.x -= Vx.x*scene->WorldStepSize;
			scene->Eye.y -= Vx.y*scene->WorldStepSize;
			scene->Eye.z -= Vx.z*scene->WorldStepSize;
			AdjustEye(scene,Length);
			scene->WalkMotion=w_Move_Left;
			reDraw=TRUE;
		break;
		case w_Move_Right:
			scene->Eye.x += Vx.x*scene->WorldStepSize;
			scene->Eye.y += Vx.y*scene->WorldStepSize;
			scene->Eye.z += Vx.z*scene->WorldStepSize;
			AdjustEye(scene,Length);
			scene->WalkMotion=w_Move_Right;
			reDraw=TRUE;
		break;
		case w_Move_Forward:
			scene->Eye.x += EyeToTarget.x*scene->WorldStepSize;
			scene->Eye.y += EyeToTarget.y*scene->WorldStepSize;
			scene->Eye.z += EyeToTarget.z*scene->WorldStepSize;
			scene->WalkMotion=w_Move_Forward;
			reDraw=TRUE;
		break;
		case w_Move_Backward:
			scene->Eye.x -= EyeToTarget.x*scene->WorldStepSize;
			scene->Eye.y -= EyeToTarget.y*scene->WorldStepSize;
			scene->Eye.z -= EyeToTarget.z*scene->WorldStepSize;
			scene->WalkMotion=w_Move_Backward;
			reDraw=TRUE;
		break;
		case w_Move_Faster:
			scene->WorldStepSize *= 2;
		break;
		case w_Move_Slower:
			scene->WorldStepSize /= 2;
		break;
		case w_Joystick_Forward:
			diff=Sub(&scene->Target,&scene->Eye);
			diff=Norm(&diff);
			step=scene->WorldStepSize;
			scene->Eye.x += step*diff.x;
			scene->Eye.y += step*diff.y;
			scene->Eye.z += step*diff.z;
			scene->Target.x += step*diff.x;
			scene->Target.y += step*diff.y;
			scene->Target.z += step*diff.z;
			reDraw=TRUE;
		break;
		
		case w_Joystick_Backward:
			diff=Sub(&scene->Target,&scene->Eye);
			diff=Norm(&diff);
			step=scene->WorldStepSize;
			scene->Eye.x -= step*diff.x;
			scene->Eye.y -= step*diff.y;
			scene->Eye.z -= step*diff.z;
			scene->Target.x -= step*diff.x;
			scene->Target.y -= step*diff.y;
			scene->Target.z -= step*diff.z;
			reDraw=TRUE;
		break;
		default:
			return 0;
	}
	
	if(reDraw)
	{
	    doCommands(NewScene);
	}
	return 0;
}
void SetLighting(unsigned int mode)
{
/*
	GLfloat position[4] = {7.0,-7.0,-12.0,0.0};
*/

	GLfloat position[4] = {700.0,700.0,1200.0,0.0};
	GLfloat ambient[4]  = {0.2,0.2,0.2,1.0};
	GLfloat diffuse[4]  = {1.0,1.0,1.0,1.0};
	GLfloat specular[4] = {1.0,1.0,1.0,1.0};
	
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {90.0};

	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	switch (mode) {
		case 0:
			break;
		case 1:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 2:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
		case 3:
			//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			//glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 4:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
	}
	
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glEnable(GL_LIGHT0);
}
int mJoyStick2(struct Scene *scene,unsigned int buttonMask,int x, int y, int z)
{
	double factor=50;
	int reDraw=FALSE;
	struct P diff;
	struct P xl,yl;
	double step;
	double len;
	double yy;	
	double pi=4.0*atan(1.0);
	
	
	if(!scene)return 1;
	/*
	 printf("buttonMask %d x %d y %d z %d \n",buttonMask,x,y,z); 
	*/
	if(x < -200){
	    step=x/(1*factor);
		step *= (pi/360.);
		step = -step;
	    xl=Sub(&scene->Target,&scene->Eye);
		len=Len(&xl);
		xl=Norm(&xl);
		yl=CrossN(&xl,&scene->Up);
		
		scene->Target.x=scene->Eye.x+len*(cos(step)*xl.x+sin(step)*yl.x);
		scene->Target.y=scene->Eye.y+len*(cos(step)*xl.y+sin(step)*yl.y);
		scene->Target.z=scene->Eye.z+len*(cos(step)*xl.z+sin(step)*yl.z);
		reDraw=TRUE;
	}
	if(x > 200){
	    step=x/(1*factor);
		step *= (pi/360.);
		step = -step;
	    xl=Sub(&scene->Target,&scene->Eye);
		len=Len(&xl);
		xl=Norm(&xl);
		yl=CrossN(&xl,&scene->Up);
		
		scene->Target.x=scene->Eye.x+len*(cos(step)*xl.x+sin(step)*yl.x);
		scene->Target.y=scene->Eye.y+len*(cos(step)*xl.y+sin(step)*yl.y);
		scene->Target.z=scene->Eye.z+len*(cos(step)*xl.z+sin(step)*yl.z);
		reDraw=TRUE;
	}
	
	yy=(y-127)*1000.0/128.0;
	
	if(yy < -200){
	    diff=Sub(&scene->Target,&scene->Eye);
		diff=Norm(&diff);
	    step=-(yy*scene->WorldStepSize)/(10*factor);
	    scene->Eye.x += step*diff.x;
	    scene->Eye.y += step*diff.y;
	    scene->Eye.z += step*diff.z;
	    scene->Target.x += step*diff.x;
	    scene->Target.y += step*diff.y;
	    scene->Target.z += step*diff.z;
		reDraw=TRUE;
	}
	if(yy > 200){
	    diff=Sub(&scene->Target,&scene->Eye);
		diff=Norm(&diff);
	    step=(yy*scene->WorldStepSize)/(10*factor);
	    scene->Eye.x -= step*diff.x;
	    scene->Eye.y -= step*diff.y;
	    scene->Eye.z -= step*diff.z;
	    scene->Target.x -= step*diff.x;
	    scene->Target.y -= step*diff.y;
	    scene->Target.z -= step*diff.z;
		reDraw=TRUE;
	}

	if(z < -200){
	    step=-(z*scene->WorldStepSize)/(50*factor);
	    scene->Eye.x += step*scene->Up.x;
	    scene->Eye.y += step*scene->Up.y;
	    scene->Eye.z += step*scene->Up.z;
	    scene->Target.x += step*scene->Up.x;
	    scene->Target.y += step*scene->Up.y;
	    scene->Target.z += step*scene->Up.z;
		reDraw=TRUE;
	}
	if(z > 200){
	    step=(z*scene->WorldStepSize)/(50*factor);
	    scene->Eye.x -= step*scene->Up.x;
	    scene->Eye.y -= step*scene->Up.y;
	    scene->Eye.z -= step*scene->Up.z;
	    scene->Target.x -= step*scene->Up.x;
	    scene->Target.y -= step*scene->Up.y;
	    scene->Target.z -= step*scene->Up.z;
		reDraw=TRUE;
	}
	
	if(reDraw)glutPostRedisplay();
	
	return 0;
	
}

int SendNewScene(struct Scene *scene)
{
	SetScreen(scene);
	
	return 0;
}

int doCommands(int command)
{
	
	int reDraw=0;
	
	switch(command)
	{
		case NewScene:
			SendNewScene(&sceneG);
			reDraw=1;
			break;
		case NoCommand:
			break;
		case QuitProgram:
			exit(0);
			break;
	}
	
	if(reDraw)
	{
		ReDrawScene(&sceneG);
		glutPostRedisplay();
	}
	return 0;
}

int ReDrawScene(struct Scene *scene)
{
	
	return 0;
}
int init(struct Scene *scene)
{
	
	//struct FireBall fireball={{-38383.,2*30000.,69297.},20000.,1,{1.0,0.0,0.0},0,100};// 1,6,14
	struct FireBall fireball={{-38383.0+4000.0,18926.,69297.0+8000.0},{-38383.0+4000.0,18926.,69297.0+8000.0},
		                       20000.,1,{1.0,0.0,0.0},0,100,8,5000.,0,1,1.0,1.0,1.0};
	
	if(!scene)return 1;
	
	zerol((char *)scene,sizeof(struct Scene));
	
	scene->Up=p(0.,1.,0.);
	
	scene->Eye=p(0.0, 5.0, 10.0);
	
	scene->Tool=ControlModeWalk;
	
	scene->debug=0;
	
	g.fireball=fireball;
	
	g.scale.showPalette=1;
	
	g.FileType = -1;
		
	//dotriangles(3*710L); /* 9063153 Triangles */
	//dotriangles(2*710L); /* 4025703 Triangles */
	//dotriangles(3*710L/2); /* 1004653 Triangles */
	dotriangles(triangle); /* 1004653 Triangles */
	//dotriangles(225L); /* 100128  Triangles */
	//dotriangles(75); /* 10878   Triangles */

	doCommands(NewScene);
	
	return 0;
}
int dotriangles(long size)
{
	
	trianglemax=size*size*2;
	
	if((trin=(struct Nodes *)malloc(trianglemax*sizeof(struct Nodes))) == NULL){
	    printf("Out of memory Requested (%ld)\n",trianglemax*sizeof(struct Nodes));
	    exit(1);
	}
	
	if((norm=(struct Norm *)malloc(trianglemax*sizeof(struct Norm))) == NULL){
	    printf("Out of memory Requested (%ld)\n",trianglemax*sizeof(struct Norm));
	    exit(1);
	}
	
	if((color=(struct Colors *)malloc(trianglemax*sizeof(struct Colors))) == NULL){
	    printf("Out of memory Requested (%ld)\n",trianglemax*sizeof(struct Colors));
	    exit(1);
	}

	if((faces=(GLuint *)malloc(trianglemax*sizeof(GLuint)*3)) == NULL){
	    printf("Out of memory Requested (%ld)\n",trianglemax*sizeof(GLuint)*3);
	    exit(1);
	}

	buildtriangles(&sceneG,trin,norm,color,faces,&trianglemax,size);
	
	buildList(&sceneG);
	
	/* printf("Triangles Created %ld in %ld Seconds\n",trianglemax,t1-start); */
	
	return 0;	
}
int buildList(struct Scene *scene)
{
	long n;

	if (scene->polyList)
		glDeleteLists (scene->polyList, 1);

	scene->polyList=0;

	if(runMode == ImmediateMode)return 0;


	if(runMode == VboMode)
	{

        glGenBuffersARB(1, &vboId);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboId);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, (sizeof(struct Colors)+sizeof(struct Norm)+sizeof(struct Nodes))*trianglemax, 0, GL_STATIC_DRAW_ARB);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0,(sizeof(struct Nodes))*trianglemax, &trin[0].x1);                             // copy vertices starting from 0 offest
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(sizeof(struct Nodes))*trianglemax, (sizeof(struct Norm))*trianglemax, &norm[0].n1.x);                // copy normals after vertices
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(sizeof(struct Norm)+sizeof(struct Nodes))*trianglemax, (sizeof(struct Norm))*trianglemax, &color[0].c1.x);  // copy colours after normals
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	else if(runMode == VboMode2)
	{

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 1);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,(sizeof(struct Nodes))*trianglemax, &trin[0].x1, GL_STATIC_DRAW);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 2);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, (sizeof(struct Norm))*trianglemax, &norm[0].n1.x, GL_STATIC_DRAW);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 3);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, (sizeof(struct Colors))*trianglemax, &color[0].c1.x, GL_STATIC_DRAW);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	}
	else if(runMode == CompiliedMode)
	{
	    scene->polyList = glGenLists (1);
	    glNewList(scene->polyList, GL_COMPILE);			
	
	    glBegin (GL_TRIANGLES);
	    for(n=0;n<trianglemax;++n){		

		    glColor3fv(&color[n].c1.x);
		    glNormal3fv(&norm[n].n1.x);
		    glVertex3fv(&trin[n].x1);
		
		   glColor3fv(&color[n].c2.x);
		    glNormal3fv(&norm[n].n2.x);
		    glVertex3fv(&trin[n].x2);
		
		    glColor3fv(&color[n].c3.x);
		    glNormal3fv(&norm[n].n3.x);
		    glVertex3fv(&trin[n].x3);
		
	    }
		
	    glEnd();
	
	    glEndList ();
	}
	return 0;
}


struct P Norm(struct P *p1)
{
    struct P p;
    double sum;

    sum=1./pow(p1->x*p1->x+p1->y*p1->y+p1->z*p1->z,.5);
    p.x=(p1->x*sum);
    p.y=(p1->y*sum);
    p.z=(p1->z*sum);
    return p;
}
struct P CrossN(struct P *v1,struct P *v2)
{
    static struct P Zero;
    struct P Vcross;
    double Length;

    Vcross.x =   v1->y*v2->z-v2->y*v1->z;
    Vcross.y = -(v1->x*v2->z-v2->x*v1->z);
    Vcross.z =   v1->x*v2->y-v2->x*v1->y;
    Length=Len(&Vcross);
    if(Length <= 0.)return Zero;
    Vcross.x/=Length;
    Vcross.y/=Length;
    Vcross.z/=Length;

    return  Vcross;
}
struct P Sub(struct P *p1,struct P *p2)
{
    struct P psub;
	
    psub.x=p1->x-p2->x;
    psub.y=p1->y-p2->y;
    psub.z=p1->z-p2->z;
    return psub;
	
}
struct P Subf(struct Pf *p1,struct Pf *p2)
{
    struct P psub;
	
    psub.x=p1->x-p2->x;
    psub.y=p1->y-p2->y;
    psub.z=p1->z-p2->z;
    return psub;
	
}
struct P Add(struct P *p1,struct P *p2)
{
    struct P padd;

    padd.x=p1->x+p2->x;
    padd.y=p1->y+p2->y;
    padd.z=p1->z+p2->z;
    return padd;
 
}
struct P Mult(struct P *p1,double Factor)
{
    struct P p;

    p.x=(p1->x*Factor);
    p.y=(p1->y*Factor);
    p.z=(p1->z*Factor);
    return p;
}
double Len(struct P *v1)
{
    double Lenr;

    Lenr=pow((v1->x)*(v1->x)+
            (v1->y)*(v1->y)+
            (v1->z)*(v1->z),.5);

    return  Lenr;
}
double Dot(struct P *v1,struct P *v2)
{
    double Length;

    Length=(v1->x*v2->x)+(v1->y*v2->y)+(v1->z*v2->z);

    return  Length;
}
struct P p(double x,double y,double z)
{
	struct P ret;
	
	ret.x=x;
	ret.y=y;
	ret.z=z;
	
	return ret;
}

int colorit(long count,double *level,double value,int *ic);

int getrainbow(double *red,double *green,double *blue);

double red[256],green[256],blue[256],table[256];

int buildtriangles(struct Scene *scene,struct Nodes *trin,struct Norm  *norm,struct Colors *color,GLuint *faces,long *trianglemax,long size)
{
	float xmin,xmax,ymin,ymax,zmin,zmax;
	double xc,yc,dr,range;
	double x1,x2,x3,x4,y1,y2,y3,y4,r1,r2,s1,s2,dt;
	double theta,sintheta1,costheta1,sintheta2,costheta2;
	double scale=0.1;
	long i,j,n;
	struct P v1,v2,normt;
	struct Pf normtf;
	GLuint np;
	int ic;
	
	for(n=0;n<256;++n){
		table[n]=(13.0*n/255.)-3.;
	}
	
	getrainbow(red,green,blue);
	
	
	xmin=1e30;
	xmax=-1e30;
	
	ymin=1e30;
	ymax=-1e30;
	
	zmin=1e30;
	zmax=-1e30;
	
	
	xc=0.;
	yc=0.;	
	theta=0;
	dr=scale/(size-1);
	dt=360./(size-1);
	dt *= (3.1415926/180.);
	n=0;
	for(j=0;j<size-1;++j){
	    sintheta1=sin(theta);
	    costheta1=cos(theta);
	    theta += dt;
	    sintheta2=sin(theta);
	    costheta2=cos(theta);
	    range=0;
	    for(i=0;i<size-1;++i){
	        x1=xc+range*costheta1;
	        y1=yc+range*sintheta1;
	        x4=xc+range*costheta2;
	        y4=yc+range*sintheta2;
	        range += dr;
	        x2=xc+range*costheta1;
	        y2=yc+range*sintheta1;
	        x3=xc+range*costheta2;
	        y3=yc+range*sintheta2;
	        r1=sqrt((xc-x1)*(xc-x1)+(yc-y1)*(yc-y1));
      		r1=10*r1*2./scale;
      		if(r1 == 0.){
      		    s1=scale;
      		}else{
				s1=scale*sin(r1)/(r1);
      		}
			
	        r2=sqrt((xc-x2)*(xc-x2)+(yc-y2)*(yc-y2));
      		r2=10*r2*2./scale;
      		if(r2 == 0.){
      		    s2=scale;
      		}else{
				s2=scale*sin(r2)/(r2);
      		}
			
			{
				float x,y,z;
				
				x=x1;
				y=y1;
				z=s1;
				if(x > xmax)xmax=x;
				if(x < xmin)xmin=x;
				if(y > ymax)ymax=y;
				if(y < ymin)ymin=y;
				if(z > zmax)zmax=z;
				if(z < zmin)zmin=z;
				x=x2;
				y=y2;
				z=s2;
				if(x > xmax)xmax=x;
				if(x < xmin)xmin=x;
				if(y > ymax)ymax=y;
				if(y < ymin)ymin=y;
				if(z > zmax)zmax=z;
				if(z < zmin)zmin=z;
				x=x3;
				y=y3;
				z=s2;
				if(x > xmax)xmax=x;
				if(x < xmin)xmin=x;
				if(y > ymax)ymax=y;
				if(y < ymin)ymin=y;
				if(z > zmax)zmax=z;
				if(z < zmin)zmin=z;
				x=x4;
				y=y4;
				z=s2;
				if(x > xmax)xmax=x;
				if(x < xmin)xmin=x;
				if(y > ymax)ymax=y;
				if(y < ymin)ymin=y;
				if(z > zmax)zmax=z;
				if(z < zmin)zmin=z;
			}
			
			if(i != 0){
				trin[n].x1=x1;
				trin[n].x2=x2;
				trin[n].x3=x4;
				trin[n].y1=y1;
				trin[n].y2=y2;
				trin[n].y3=y4;
				trin[n].z1=s1;
				trin[n].z2=s2;
				trin[n].z3=s1;
				
				colorit(256,table,trin[n].z1*100,&ic);
				
				color[n].c1.x=red[ic];
				color[n].c1.y=green[ic];
				color[n].c1.z=blue[ic];
				
				colorit(256,table,trin[n].z2*100,&ic);
				
				color[n].c2.x=red[ic];
				color[n].c2.y=green[ic];
				color[n].c2.z=blue[ic];
				
				colorit(256,table,trin[n].z3*100,&ic);
				
				color[n].c3.x=red[ic];
				color[n].c3.y=green[ic];
				color[n].c3.z=blue[ic];
				
				v1=Subf((struct Pf *)&trin[n].x2,(struct Pf *)&trin[n].x1);
				v2=Subf((struct Pf *)&trin[n].x3,(struct Pf *)&trin[n].x1);
				normt=CrossN(&v1,&v2);
				normtf.x=normt.x;
				normtf.y=normt.y;
				normtf.z=normt.z;
				
				
				if(normt.z < 0)
				{
					
					normtf.x= -normt.x;
					normtf.y= -normt.y;
					normtf.z= -normt.z;
					/*
					 printf("%ld %f %f %f\n",n,v1.x,v1.y,v1.z);
					 printf("%ld %f %f %f\n",n,v2.x,v2.y,v2.z);
					 printf("%ld %f %f %f\n",n,normtf.x,normtf.y,normtf.z);
					 printf("%ld %f %f %f\n",n,normtf.x,normtf.y,normtf.z);
					 */
				}
				
				
	            norm[n].n1=normtf;
	            norm[n].n2=normtf;
	            norm[n].n3=normtf;
				++n;
			}
			trin[n].x1=x3;
			trin[n].x2=x4;
			trin[n].x3=x2;
			trin[n].y1=y3;
			trin[n].y2=y4;
			trin[n].y3=y2;
			trin[n].z1=s2;
			trin[n].z2=s1;
			trin[n].z3=s2;
			
			colorit(256,table,trin[n].z1*100,&ic);
			
			color[n].c1.x=red[ic];
			color[n].c1.y=green[ic];
			color[n].c1.z=blue[ic];
			
			colorit(256,table,trin[n].z2*100,&ic);
			
			color[n].c2.x=red[ic];
			color[n].c2.y=green[ic];
			color[n].c2.z=blue[ic];
			
			colorit(256,table,trin[n].z3*100,&ic);
			
			color[n].c3.x=red[ic];
			color[n].c3.y=green[ic];
			color[n].c3.z=blue[ic];
			
			
			v1=Subf((struct Pf *)&trin[n].x2,(struct Pf *)&trin[n].x1);
			v2=Subf((struct Pf *)&trin[n].x3,(struct Pf *)&trin[n].x1);
			normt=CrossN(&v1,&v2);
			normtf.x=normt.x;
			normtf.y=normt.y;
			normtf.z=normt.z;
			if(normt.z < 0)
			{
				
				normtf.x= -normt.x;
				normtf.y= -normt.y;
				normtf.z= -normt.z;
				/*
			    printf("%ld %f %f %f\n",n,v1.x,v1.y,v1.z);
			    printf("%ld %f %f %f\n",n,v2.x,v2.y,v2.z);
			    printf("%ld %f %f %f\n",n,normtf.x,normtf.y,normtf.z);
			    printf("%ld %f %f %f\n",n,normtf.x,normtf.y,normtf.z);
				*/
			}
			norm[n].n1=normtf;
			norm[n].n2=normtf;
			norm[n].n3=normtf;
			++n;
	    }
	}
	
	scene->xmin=xmin;
	scene->xmax=xmax;
	scene->ymin=ymin;
	scene->ymax=ymax;
	scene->zmin=zmin;
	scene->zmax=zmax;
	
	CenterScreen(scene);
	
	scene->nt=n;
	scene->nq=0;
	scene->pointClick=1;
	
	for(np=0;np<n;++np)
	{
		faces[3*np+0]=3*np+0;
		faces[3*np+1]=3*np+1;
		faces[3*np+2]=3*np+2;
	}
	
	*trianglemax=n;
	
	printf("buildtriangles Count %ld vender %s\n",n,glGetString(GL_VENDOR));
	return 0;	
}
int colorit(long count,double *level,double value,int *ic)
{
	long ib,it,ns;
	
	if(value <= *level){
		*ic=0;
		return 0;
	}
	it=count-1;
	if(value >= level[it]){
		*ic=it;
		return 0;
	}
	ib=0;
	while(it > ib+1){
		ns=(it+ib)/2;
		if(value > level[ns]){
			ib=ns;
		}else{
			it=ns;
		}
	}
	*ic=ib;
	return 0;
}
int getrainbow(double *red,double *green,double *blue)
{
	
	static unsigned char r[256]={
		255,124,120,115,111,106,102,97,93,88,
		84,79,75,70,66,61,57,52,48,43,
		39,34,30,25,21,16,12,7,3,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,1,5,10,14,19,23,28,32,
		37,41,46,50,55,59,64,68,73,77,
		82,86,91,95,100,104,109,113,118,123,
		126,132,135,141,144,150,153,159,162,168,
		171,177,180,186,189,195,198,204,207,213,
		216,222,225,231,234,240,243,249,252,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,255,0};
	
	static unsigned char g[256]={
		255,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,2,6,11,15,20,
		24,29,33,38,42,47,51,56,60,65,
		69,74,78,83,87,92,96,101,105,110,
		114,119,122,128,131,137,140,146,149,155,
		158,164,167,173,176,182,185,191,194,200,
		203,209,212,218,221,227,230,236,240,245,
		249,254,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,252,247,243,238,234,
		229,225,220,216,211,207,202,198,193,189,
		184,180,175,171,166,162,157,153,148,144,
		139,135,131,126,122,117,113,108,104,99,
		95,90,86,81,77,72,68,63,59,54,
		50,45,41,36,32,27,23,18,14,9,
		5,0};
	
	static unsigned char b[256]={
		255,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,254,255,
		254,255,254,255,254,255,254,255,250,247,
		241,238,232,229,223,220,214,211,205,202,
		196,193,187,184,178,175,169,166,160,157,
		151,148,142,139,133,130,125,121,116,112,
		107,103,98,94,89,85,80,76,71,67,
		62,58,53,49,44,40,35,31,26,22,
		17,13,8,4,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0};
	
	double base;
	int k;
	
	base=1.0/255.;
	
	for(k=0;k<256;++k){
		red[k]=((double)(r[k]))*base;
		green[k]=((double)(g[k]))*base;
		blue[k]=((double)(b[k]))*base;
	}
	return 0;
}
int CenterScreen(struct Scene *scene)
{
	struct P EyeToTarget;
	struct P pScreen;
	double thetaHalf;
	double xc,yc,zc;
	double pi;
	
	if(!scene)return 1;
	
	
	pi=4.0*atan(1.0);
	
	xc=0.5*(scene->xmin+scene->xmax);
	yc=0.5*(scene->ymin+scene->ymax);
	zc=0.5*(scene->zmin+scene->zmax);
	
	scene->WorldSize=sqrt( (scene->xmin-scene->xmax)*(scene->xmin-scene->xmax)+
						   (scene->ymin-scene->ymax)*(scene->ymin-scene->ymax)+
						   (scene->zmin-scene->zmax)*(scene->zmin-scene->zmax));
	
	scene->WorldStepSize=scene->WorldSize*0.1;
	
	scene->Target.x=xc;
	scene->Target.y=yc;
	scene->Target.z=zc;
	
	scene->Eye.x=scene->Target.x+1.001;
	scene->Eye.y=scene->Target.y+1.002;
	scene->Eye.z=scene->Target.z+1.003;
	
	scene->Degrees=30;
	
	thetaHalf=0.5*scene->Degrees*pi/180.;
	
	EyeToTarget=Sub(&scene->Target,&scene->Eye);
	EyeToTarget=Norm(&EyeToTarget);
	
	pScreen=Mult(&EyeToTarget,-(0.5*scene->WorldSize/tan(thetaHalf)));
	
	/*
	 pScreen=Mult(&EyeToTarget,-(scene->WorldSize*0.75));
	 */
	scene->Eye=Add(&pScreen,&scene->Target);
	
	EyeToTarget=Sub(&scene->Target,&scene->Eye);
	
	scene->nearv=scene->WorldSize/100;
	
	/*
	 scene->farv=1.5*scene->WorldSize;
	 */
	
	
	scene->farv=Len(&EyeToTarget)+0.5*scene->WorldSize;
	
	scene->Lamp=scene->Eye;
		
	/*
	 printf("w %f  z %f\n",scene->WorldSize*4,scene->farv);
	 printf("x %f  y %f z %f\n",scene->Eye.x,scene->Eye.y,scene->Eye.z);
	 scene->Eye.x=xc+scene->WorldSize;
	 scene->Eye.y=yc+scene->WorldSize;
	 scene->Eye.z=zc+scene->WorldSize;
	 printf("x %f  y %f z %f\n",scene->Eye.x,scene->Eye.y,scene->Eye.z);
	 */
	
	
	return 0;
}
int SetScreen(struct Scene *scene)
{
	struct P EyeToTarget,V1,V2,V3,V4,pScreen;
	
	if(!scene)return 1;
	
	EyeToTarget=Sub(&scene->Target,&scene->Eye);
	EyeToTarget=Norm(&EyeToTarget);
	
	/* Local x Direction */
	V1=CrossN(&EyeToTarget,&scene->Up);
	
	/* Local y Direction */
	V2=CrossN(&V1,&EyeToTarget);
	
	/* Local z Direction */
	V3=Mult(&EyeToTarget,-1.0);
	
	pScreen=Mult(&EyeToTarget,scene->Height);
	
	/* Point At Center of Screen */
	pScreen=Add(&pScreen,&scene->Eye);
	if(scene->debug >2){
		WarningBatch((char *)"\n");
	    sprintf(WarningBuff,"V1 %f %f %f\n",V1.x,V1.y,V1.z);
		WarningBatch(WarningBuff);
	    sprintf(WarningBuff,"V2 %f %f %f\n",V2.x,V2.y,V2.z);
		WarningBatch(WarningBuff);
	    sprintf(WarningBuff,"V3 %f %f %f\n",V3.x,V3.y,V3.z);
		WarningBatch(WarningBuff);
	    sprintf(WarningBuff,"pScreen %f %f %f\n",pScreen.x,pScreen.y,pScreen.z);
		WarningBatch(WarningBuff);
	}
   	scene->g.V1=V1;
   	scene->g.V2=V2;
   	scene->g.V3=V3;
   	scene->g.pScreen=pScreen;
   	
   	V4=Mult(&V1,0.5*scene->EyeSeparation);
   	scene->EyeRight=Add(&V4,&scene->Eye);
   	
   	V4=Mult(&V1,-0.5*scene->EyeSeparation);
   	scene->EyeLeft=Add(&V4,&scene->Eye);
	if(scene->debug > 2){
	    sprintf(WarningBuff,"EyeRight %g %g %g\n",scene->EyeRight.x,scene->EyeRight.y,scene->EyeRight.z);
		WarningBatch(WarningBuff);
	    sprintf(WarningBuff,"EyeLeft %g %g %g\n",scene->EyeLeft.x,scene->EyeLeft.y,scene->EyeLeft.z);
		WarningBatch(WarningBuff);
		sprintf(WarningBuff,"Target %g %g %g\n",scene->Target.x,scene->Target.y,scene->Target.z);
		WarningBatch(WarningBuff);
	}   	 	
   	
	return 0;
}

int RotateToTheta(struct Scene *scene)
{
	double DegreesToRadians=atan(1.0)/45.0;
	struct P EyeToTarget,TargetToEye;
	struct P x,y,z;
	struct P Target;
	double vertDegree,hortDegree;
	double radius;
	double rOut;
	double a,b,c;
	
 	if(!scene)return 1;
	
	if((scene->dtheta1 == 0.0) && (scene->dtheta2 == 0.0))return 0;
	
	EyeToTarget=Sub(&scene->Target,&scene->EyeStart);
	radius=Len(&EyeToTarget);
	
	y=Norm(&scene->Up);
	z=Sub(&scene->EyeStart,&scene->Target);
	z=Norm(&z);
	
	x=CrossN(&y,&z);
	
	Target=scene->Target;
	
	vertDegree = scene->dtheta2;
	
	if(vertDegree >= 90){
		vertDegree = 89.001;
	}else if(vertDegree <= -90){
		vertDegree = -89.001;
	}
	
	b=radius*sin(vertDegree*DegreesToRadians);	
	rOut=radius*cos(vertDegree*DegreesToRadians);
	
	hortDegree=scene->dtheta1;
	
	c=rOut*cos(hortDegree*DegreesToRadians);
	a=rOut*sin(hortDegree*DegreesToRadians);
	
	
	TargetToEye.x=a*x.x+b*y.x+c*z.x;
	TargetToEye.y=a*x.y+b*y.y+c*z.y;
	TargetToEye.z=a*x.z+b*y.z+c*z.z;
	TargetToEye=Norm(&TargetToEye);	
	TargetToEye=Mult(&TargetToEye,radius);
	
	scene->Eye.x=Target.x+TargetToEye.x;
	scene->Eye.y=Target.y+TargetToEye.y;
	scene->Eye.z=Target.z+TargetToEye.z;
	
	EyeToTarget=Sub(&scene->Target,&scene->Eye);
	radius=Len(&EyeToTarget);
	
	return 0;
}
int AdjustEye(struct Scene *scene,double Length)
{
	struct P diff;
	
	if(!scene || Length <= 0)return 1;
	
	diff=Sub(&scene->Eye,&scene->Target);	
	diff=Norm(&diff);
	diff=Mult(&diff,Length);
	
	scene->Eye=Add(&scene->Target,&diff);
	
	return 0;
}

int alineEye(struct Scene *scene)
{
	/* move Eye to plane defined with normal Up at point Target */
	
	struct P diff;
	double off;
	
	if(!scene)return 1;
	
	diff=Sub(&scene->Eye,&scene->Target);
	
	off=Dot(&diff,&scene->Up);
	
	scene->Eye.x -= off*scene->Up.x;
	scene->Eye.y -= off*scene->Up.y;
	scene->Eye.z -= off*scene->Up.z;
	
	return 0;
}

#include <sys/timeb.h>

int zerol(char *p,unsigned long n)
{
	if(!p || !n)return 1;
	
	while(n-- > 0)*p++ = 0;
	
	return 0;
}

int GetTime(long *Seconds,long *milliseconds)
{
	struct timeb t;
	
	if(!Seconds || !milliseconds)return 1;
	

	ftime(&t);

	*Seconds=(long)t.time;
	*milliseconds=t.millitm;
	
	return 0;
}

double rtime(void)
{
	long milliseconds;
	long Seconds;
	double ret;
	
	
	GetTime(&Seconds,&milliseconds);
	
	ret=(double)Seconds+(double)milliseconds/1000.;
	
	return ret;

}
void WarningBatch(char *message)
{
	printf("%s",message);
}



