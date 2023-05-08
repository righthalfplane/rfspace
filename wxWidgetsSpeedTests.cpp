/*
  The BasicGLPane sections are covered by -
  
                    GNU GENERAL PUBLIC LICENSE
                       Version 3, 29 June 2007
                       
   The remainder of the code is covered by -
                     
                       MIT License
			Copyright (c) 2023 RightHalfPlane
*/


//c++ -std=c++11 -o mainSpeed.x wxWidgetsSpeedTests.cpp -lGLEW `/usr/local/bin/wx-config --cxxflags --libs --gl-libs` -lGLU -lGL

//c++ -std=c++11 -o mainSpeed.x wxWidgetsSpeedTests.cpp -lGLEW `/usr/bin/wx-config --cxxflags --libs --gl-libs` -lGLU -lGL

//c++ -std=c++11 -o mainSpeed.x wxWidgetsSpeedTests.cpp -lGLEW `/opt/local/Library/Frameworks/wxWidgets.framework/Versions/wxWidgets/3.1/bin/wx-config --cxxflags --libs --gl-libs` -Wno-deprecated-declarations


#define GLEW_IN
#ifdef GLEW_IN
#include <GL/glew.h>
#endif

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/glcanvas.h>

#include <string.h>

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif


// include OpenGL
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif
 
#include <sys/timeb.h>


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

struct Color{
    float r;
    float g;
    float b;
};

struct Scene{
	float xmin,xmax,ymin,ymax,zmin,zmax;
	struct P Eye;
	double WorldSize;
	double WorldStepSize;
	struct P Target;
	GLuint polyList;
	long nt;
	long nq;
	double Degrees;
	GLfloat nearv;
	GLfloat farv;
	struct P Lamp;
	int pointClick;
};

int buildtriangles(struct Scene *scene,struct Nodes *trin,struct Norm  *norm,struct Colors *color,GLuint *faces,long *trianglemax,long size);

double rtime(void);

extern struct P Norm(struct P *p1);
extern struct P CrossN(struct P *v1,struct P *v2);
extern struct P Sub(struct P *p1,struct P *p2);
extern struct P Add(struct P *p1,struct P *p2);
extern struct P Mult(struct P *p1,double Factor);
extern double Len(struct P *v1);
extern double Dot(struct P *v1,struct P *v2);
extern struct P p(double x,double y,double z);
extern struct P Subf(struct Pf *p1,struct Pf *p2);


 enum {
    ID_INSERT = 10004,
    ID_DELETE,
    ID_RENDER,
    TEXT_CLIPBOARD_COPY,
    TEXT_CLIPBOARD_PASTE,
};


class BasicGLPane : public wxGLCanvas
{
    wxGLContext*	m_context;

public:
	BasicGLPane(wxFrame* parent, int* args);
	virtual ~BasicGLPane();
    
	void resized(wxSizeEvent& evt);
    
	int getWidth();
	int getHeight();
	float angle;
    
	void render(wxPaintEvent& evt);
	void render2();
	void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	void InitOpenGl();
	int dotriangles(long n);
	int doTestSpeed(struct Scene *scene);
    
	// events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
};

class BasicGLPane2 : public wxFrame
{
    wxGLContext*	m_context;

public:
	BasicGLPane2(wxFrame *frame, const wxString& title, const wxPoint& pos,
            const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	virtual ~BasicGLPane2();
    
	void resized(wxSizeEvent& evt);
    
	int getWidth();
	int getHeight();
	float angle;
	
	wxRadioBox *radioBoxp;
	wxTextCtrl *text;
    
	void render(wxPaintEvent& evt);
	
	// events
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    void OnButton( wxCommandEvent& event );
    void OnButton2( wxCommandEvent& event );
    void radioBox( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
}; 
 
 
 
 enum
{
    ButtonPage_Button = wxID_HIGHEST,
    ButtonPage_ChangeNote,
    ID_RADIOBOX,
    ID_TEXTCTRL
};

 struct Scene sceneG;

char WarningBuff[256];


struct Nodes *trin;
struct Norm  *norm;
struct Colors *color;
GLuint *faces1;

long int triangle=100;

long trianglemax;

GLuint vboId = 0;                   // ID of VBO for vertex arrays

#define ImmediateMode     0
#define CompiliedMode     1
#define VertexArrayMode   2
#define VboMode			  3  /* glDrawArrays */
#define VboMode2		  4  /* glDrawElements */

int runMode = ImmediateMode;

static char *mode[] =
			{
				(char *)"Immediate",
				(char *)"Compilied",
				(char *)"VertexArray",
				(char *)"glDrawArrays",
				(char *)"glDrawElements",
			};


int buildList(struct Scene *scene);


 

 void SetLighting(unsigned int mode);

BasicGLPane *glPane;
 
class MyApp: public wxApp
{
    virtual bool OnInit();
    
    wxFrame *frame;
    wxFrame *frame2;
    BasicGLPane2 *glPane2;
public:
    
};
 
bool MyApp::OnInit()
{

    if ( !wxApp::OnInit() )
        return false;
        

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    
     frame = new BasicGLPane2( (wxFrame*) NULL,  "Controls",
				  wxPoint(20,20), wxSize(400,350));
				 
     frame2 = new wxFrame((wxFrame *)NULL, -1,  wxT("Hello GL World"), wxPoint(50,50), wxSize(800,600));
	
    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};
    
    glPane = new BasicGLPane( (wxFrame*) frame2, args);
    sizer->Add(glPane, 1, wxEXPAND);
    frame2->SetSizer(sizer);
    frame2->SetAutoLayout(true);
   
    frame2->Show();  
    frame2->Raise();

    glPane->InitOpenGl();
 
    return true;
} 

IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(BasicGLPane2, wxFrame)
EVT_MOTION(BasicGLPane2::mouseMoved)
EVT_LEFT_DOWN(BasicGLPane2::mouseDown)
EVT_BUTTON(ButtonPage_Button, BasicGLPane2::OnButton)
EVT_BUTTON(ButtonPage_ChangeNote, BasicGLPane2::OnButton2)
EVT_RADIOBOX(ID_RADIOBOX, BasicGLPane2::radioBox)

END_EVENT_TABLE()
 
void BasicGLPane2::mouseMoved(wxMouseEvent& event) {}
void BasicGLPane2::mouseDown(wxMouseEvent& event) {/*printf("mouseDown 2\n");*/}
void BasicGLPane2::OnButton(wxCommandEvent& event) 
{
	exit(0);
}
void BasicGLPane2::OnButton2(wxCommandEvent& event) 
{    

    wxString value=text->GetValue();
    int nt=atof(value.mb_str().data());
    //printf("triangle %ld\n",triangle);
	runMode=radioBoxp->GetSelection();
	if(nt != triangle){
	    triangle=nt;
		glPane->dotriangles(triangle);
	}
	buildList(&sceneG);
    glPane->doTestSpeed(&sceneG);
}
void BasicGLPane2::radioBox(wxCommandEvent& event) 
{
	runMode=radioBoxp->GetSelection();
	//printf("radioBox runMode %d\n",radioBoxp->GetSelection());
}

BasicGLPane2::BasicGLPane2(wxFrame *frame, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, title, pos, size, style)
{


    wxArrayString strings;
    
    strings.Add(wxT("Immediate"));
    strings.Add(wxT("Compilied"));
    strings.Add(wxT("VertexArray"));
    strings.Add(wxT("glDrawArrays"));
    strings.Add(wxT("glDrawElements"));
    
    
    radioBoxp = new wxRadioBox(this,ID_RADIOBOX,wxT("Render Mode"),
                            wxDefaultPosition, wxDefaultSize,strings,1,wxRA_SPECIFY_COLS);

    
  	wxButton *button=new wxButton(this,ButtonPage_ChangeNote,wxT("Run"));
   	
    wxButton* cancel = new wxButton (this, ButtonPage_Button,wxT("Quit"));
    
    text=new wxTextCtrl(this,ID_TEXTCTRL,wxT("1000"),
          wxDefaultPosition, wxDefaultSize);
  
     wxStaticText *stext=new wxStaticText(this,wxID_STATIC,wxT("Triangle Groups:"),
          wxDefaultPosition, wxDefaultSize,wxALIGN_LEFT);
 
          
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
 	 sizer->Add(stext, 0, wxEXPAND|wxALL, 5);
 	 sizer->Add(text, 0, wxEXPAND|wxALL, 5);
   
     wxBoxSizer* sizer3 = new wxBoxSizer(wxVERTICAL);
	 sizer3->Add(radioBoxp, 0, wxEXPAND|wxALL, 5);
 	 sizer3->Add(sizer, 0, wxEXPAND|wxALL, 5);
	 sizer3->Add(button, 0, wxEXPAND|wxALL, 5);
	 sizer3->Add(cancel, 0, wxEXPAND|wxALL, 5);
     SetSizer(sizer3);
     SetAutoLayout(true);
     Show();
     

    
}
BasicGLPane2::~BasicGLPane2()
{
	//delete m_context;
}


BEGIN_EVENT_TABLE(BasicGLPane, wxGLCanvas)
EVT_MOTION(BasicGLPane::mouseMoved)
EVT_LEFT_DOWN(BasicGLPane::mouseDown)
EVT_LEFT_UP(BasicGLPane::mouseReleased)
EVT_RIGHT_DOWN(BasicGLPane::rightClick)
EVT_LEAVE_WINDOW(BasicGLPane::mouseLeftWindow)
EVT_SIZE(BasicGLPane::resized)
EVT_KEY_DOWN(BasicGLPane::keyPressed)
EVT_KEY_UP(BasicGLPane::keyReleased)
EVT_MOUSEWHEEL(BasicGLPane::mouseWheelMoved)
EVT_PAINT(BasicGLPane::render)
EVT_CHAR(BasicGLPane::OnChar)    
EVT_BUTTON(ID_DELETE,BasicGLPane::DeleteRow )
END_EVENT_TABLE()
 
 
// some useful events to use
void BasicGLPane::mouseMoved(wxMouseEvent& event) {}
void BasicGLPane::mouseDown(wxMouseEvent& event) {/*printf("mouseDown\n");*/}
void BasicGLPane::mouseWheelMoved(wxMouseEvent& event) {}
void BasicGLPane::mouseReleased(wxMouseEvent& event) {}
void BasicGLPane::rightClick(wxMouseEvent& event) {}
void BasicGLPane::mouseLeftWindow(wxMouseEvent& event) {}
void BasicGLPane::keyReleased(wxKeyEvent& event) {}
void BasicGLPane::OnChar(wxKeyEvent& event) {}
  
int InitGL=0;



void BasicGLPane::InitOpenGl()
{
    if(InitGL) return;
    //
    while ( !IsShown() ) {};  // Force the Shown
    
    SetCurrent(*m_context);

#ifdef GLEW_IN

    GLenum err = glewInit();
    if(err!=GL_NO_ERROR)
    {
        wxMessageBox(
            wxString("GLEW Error: ") +
            wxString(glewGetErrorString(err)),
            _("OpenGl ERROR"),
            wxOK | wxICON_EXCLAMATION
        );
   }
#endif
   InitGL=1;
   
   
    dotriangles(triangle);
    
    printf("Groups of triangles %ld\n",triangle);

}



BasicGLPane::BasicGLPane(wxFrame* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
	m_context = new wxGLContext(this);    
	
	

    // To avoid flashing on MSW
    
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    
    memset((char *)&sceneG,0,sizeof(struct Scene));

    struct Scene *scene=&sceneG;
    
    trin=NULL;
    norm=NULL;
    color=NULL;
    color=NULL;
	
    
    angle=50;
    
  //  printf("Groups of triangles %ld vender %s\n",triangle,glGetString(GL_VENDOR));
  

    
}
void BasicGLPane::DeleteRow( wxCommandEvent& WXUNUSED(ev) )
{
    printf("DeleteRow\n");
}


void BasicGLPane::keyPressed(wxKeyEvent& event) 
{
    wxString key;
    long keycode = event.GetKeyCode();
    
    //printf("keycode %ld\n",keycode);
    
    if(keycode == 84){
        buildList(&sceneG);
    	doTestSpeed(&sceneG);
    }
  

}
int BasicGLPane::doTestSpeed(struct Scene *scene)
{
    buildList(&sceneG);    
    
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
		angle += 2;
		render2();
	}
	
	for(n=0;n<25;++n){
	    count++;
		scene->Eye.x += scene->WorldStepSize*0.1;
		//doCommands(NewScene);
		angle += 2;
		render2();
	}
	
	
	for(n=0;n<25;++n){
	    count++;
		scene->Eye.z += scene->WorldStepSize*0.1;
		//doCommands(NewScene);
		angle += 2;
		render2();
	}
	
	for(n=0;n<25;++n){
	    count++;
		scene->Eye.x -= scene->WorldStepSize*0.1;
		//doCommands(NewScene);
		angle += 2;
		render2();
	}
	
	
	end=rtime();
	den=end-start;
	if(den <= 0)den=1;
	printf("%.2f Seconds %ld  Frames  %.2f (Frames/sec) %.2f (Millon triangles/sec) xsize %d  ysize %d Mode \"%s\" triangles %ld\n",
		   den,count,((double)count)/(den),((double)count*(scene->nt+scene->nq))/(1.0e6*den),
		   getWidth(),getHeight(),mode[runMode],trianglemax);
	
	return 0;

}

BasicGLPane::~BasicGLPane()
{
	delete m_context;
}

void BasicGLPane::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
   	 Refresh();
}
 
/** Inits the OpenGL viewport for drawing in 3D. */
void BasicGLPane::prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
    glEnable(GL_COLOR_MATERIAL);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    float ratio_w_h = (float)(bottomrigth_x-topleft_x)/(float)(bottomrigth_y-topleft_y);
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
}
 
/** Inits the OpenGL viewport for drawing in 2D. */
void BasicGLPane::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
 
int BasicGLPane::getWidth()
{
    return GetSize().x;
}
 
int BasicGLPane::getHeight()
{
    return GetSize().y;
}
 

int mDisplay2(struct Scene *scene)
{
	
	if(!scene)return 1;
  		
	if((scene->polyList) && (runMode == CompiliedMode) )
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
	
        glDrawElements(GL_TRIANGLES, trianglemax*3, GL_UNSIGNED_INT, faces1);
	
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
	}
	
	  
    glDisable(GL_COLOR_MATERIAL);

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);

    glDisable(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0, 0, 0);

  return 0;
}
 
void BasicGLPane::render2( )
{
    if(!IsShown()) return;
    
     SetCurrent(*m_context);
   
    //wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
	
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    glLoadIdentity();
	

    prepare3DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
    
    
    glColor4f(0,0,1,1);
    glTranslatef(0,0,-0.25);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    
    glColor4f(1, 0, 0, 1);
    mDisplay2(&sceneG);
    glFlush();
    SwapBuffers();
}
 
 
void BasicGLPane::render( wxPaintEvent& evt )
{
 	render2( );
}
int BasicGLPane::dotriangles(long size)
{

	//printf("runMode %d\n",runMode);
	
	trianglemax=size*size*2;
	
	if(trin)free(trin);
	if(norm)free(norm);
	if(color)free(color);
	if(faces1)free(faces1);
	
	trin=NULL;
    norm=NULL;
    color=NULL;
    faces1=NULL;

	
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

	if((faces1=(GLuint *)malloc(trianglemax*sizeof(GLuint)*3)) == NULL){
	    printf("Out of memory Requested (%ld)\n",trianglemax*sizeof(GLuint)*3);
	    exit(1);
	}

	buildtriangles(&sceneG,trin,norm,color,faces1,&trianglemax,size);
	
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
	   // printf("scene->polyList %d\n",scene->polyList);
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
	
	//printf("buildtriangles Count %ld vender %s\n",n,glGetString(GL_VENDOR));
	printf("buildtriangles Count %ld \n",n);
	
	
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
