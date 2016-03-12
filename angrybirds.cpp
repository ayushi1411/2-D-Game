#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;
void projectile();
double y_initial=0,x_initial=0,speed=7,g=9.8,d=0,angle=90;
GLuint programID;

class ball_obj{
public:
  float radius;
  double c_x;
  double c_y;
}b;

class wall_obj{
public:
  float width_wall;
  float height_wall;
  double c_x;
  double c_y;
}w;

class objcircle_obj{
  public:
    float radius;
    double c_x;
    double c_y;

}obj1;
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
float rangex1=0,rangex2=0, rangey1=0,rangey2=0,cameradepth=1;
float triangle_rot_dir = 1;
float cannon_rot_dir = 1;
bool triangle_rot_status = true;
bool cannon_rot_status = false;
bool shoot=false;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if(action == GLFW_REPEAT || action==GLFW_PRESS)
    {
      switch (key)
      {
        case GLFW_KEY_A :
          cannon_rot_status=1;
          cannon_rot_dir=1;
          projectile();
          break;
        case GLFW_KEY_B :
          cannon_rot_status=1;
          cannon_rot_dir=-1;
          projectile();
          break;
        case GLFW_KEY_F:
          if(speed<15)
          speed+=1;break;
        case GLFW_KEY_S:
          if(speed>3)
            speed--;break;
        case GLFW_KEY_SPACE :
          shoot=true;
          break;

        case GLFW_KEY_LEFT:
          if(rangex1>-4)
          {
            rangex1-=1;
            rangex2-=1;
           // rangey1-=1;
            //rangey2+=1;
           // reshapeWindow();
          }
          break;

        case GLFW_KEY_RIGHT:
          if(rangex1<0)
          {
            rangex1+=1;
            rangex2+=1;
            //rangey1+=1;
            //rangey2-=1;
           // reshapeWindow();
          }
          break;

        case GLFW_KEY_UP:
          if(cameradepth<1.5)
            cameradepth+=0.1;
          cout<<"cameradepth : "<<cameradepth<<endl;
          break;

          

        case GLFW_KEY_DOWN:
          if(cameradepth>0.5)
            cameradepth-=0.1;
                    cout<<"cameradepth : "<<cameradepth<<endl;

          break;
      }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                cannon_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    //Matrices.projection = glm::ortho(cameradepth*(-4.0f+rangex1), cameradepth*(4.0f+rangex2), cameradepth*(-4.0f+rangey1) ,cameradepth*(4.0f+rangey2), 0.1f, 500.0f);
Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f ,4.0f, 0.1f, 500.0f);
}
VAO *triangle, *rectangle, *ground, *cannon, *stand,* ball,*wall,*obj;

// Creates the triangle object used in this sample code
VAO* createTriangle (float x1,float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3,float r, float g, float b)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
VAO* createRectangle (float width, float height,float r, float g, float b)
{
  // GL3 accepts only Triangles. Quads are not supported
    GLfloat vertex_buffer_data [] = {
    -width/2,-height/2,0, // vertex 1
    width/2,-height/2,0, // vertex 2
    width/2, height/2,0, // vertex 3

    width/2, height/2,0, // vertex 3
    -width/2, height/2,0, // vertex 4
    -width/2,-height/2,0  // vertex 1
  };

    GLfloat color_buffer_data [] = {
    r,g,b, // color 1
    r,g,b, // color 2
    r,g,b, // color 3

    r,g,b,// color 3
    r,g,b, // color 4
    r,g,b,  // color 1
  };


  // create3DObject creates and returns a handle to a VAO that can be used later
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO* createCircle(float radius,float r, float g, float b)
{
  GLfloat vertex_buffer_data[1500];
  GLfloat color_buffer_data [1500];
  int index=0;

  for(int i=0;i<360;i++)
  {
    vertex_buffer_data[i*3]=(radius * cos(i*M_PI/180.0f));
    vertex_buffer_data[i*3+1]=(radius * sin(i*M_PI/180.0f));
    vertex_buffer_data[i*3+2]=0;    


    color_buffer_data[i*3]=r;
    color_buffer_data[i*3+1]=g;
    color_buffer_data[i*3+2]=b;
  }
  return create3DObject(GL_TRIANGLE_FAN, 360, vertex_buffer_data, color_buffer_data, GL_FILL);

}
void createCannon()
 {
  GLfloat vertex_buffer_data[]={
    -0.125,0.25,0,
    0.125,0.25,0,
    -0.25,-0.25,0,

    -0.25,-0.25,0,
    0.25,-0.25,0,
    0.125,0.25,0,
  };
  GLfloat color_buffer_data []={
    0.25,0.25,0.25,
    0.25,0.25,0.25,
    0.25,0.25,0.25,
    0.25,0.25,0.25,
    0.25,0.25,0.25,
    0.25,0.25,0.25,
  };
  cannon=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
 }
 


float camera_rotation_angle = 90;
float cannon_rotation = 0;
float triangle_rotation = 0;

void projectile()
 {
  angle=90+cannon_rotation;
  cout<<"angle-> "<<angle<<endl;
  double x=0.5/sin(angle*M_PI/180.0f);
  x+=0.25;
  y_initial=x*sin(angle*M_PI/180.0f);
 // cout<<"y0-> "<<y_initial<<endl;
  double a=speed*speed*sin(angle*M_PI/180.0f)*sin(angle*M_PI/180.0f);
  a+=(2*g*y_initial);
  a=sqrt(a);
  a+=(speed*sin(angle*M_PI/180.0f));
  a*=(speed*cos(angle*M_PI/180.0f)/g);
  d=a;
  cout<<"dist-> "<<d<<endl;
  x_initial=-3+(y_initial/tan(angle*M_PI/180.0f))-(0.5/tan(angle*M_PI/180.0f));
  y_initial+=-3.5;
  b.c_x=x_initial;
  b.c_y=y_initial;

 }
int collide =1, collideobj=0;
double objspeed=0;
 double i=0;
void collision()
{
  cout<<"ballx> "<<b.c_x<<" bally-> "<<b.c_y<<endl;
  y_initial=b.c_y;
  x_initial=b.c_x;
  speed=0.7*speed;
  collide=-1;
  i=0;
}
void collision_obj()
{
  collideobj=1;

}
int incx=1,incy=1;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
      Matrices.projection = glm::ortho(cameradepth*(-4.0f+rangex1), cameradepth*(4.0f+rangex2), cameradepth*(-4.0f+rangey1) ,cameradepth*(4.0f+rangey2), 0.1f, 500.0f);

  float increments = 3;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  cannon_rotation = cannon_rotation + increments*cannon_rot_dir*cannon_rot_status;
  cannon_rot_status=0;
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
// Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

 /* glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef

  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);*/

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  //GROUND
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateGround = glm::translate (glm::vec3(0, -3.75f, 0));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 // Matrices.model *= (translateRectangle * rotateRectangle);
  Matrices.model=translateGround;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(ground);

  //CANNON
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCannon = glm::translate (glm::vec3(-3, -3.0f, 0));        // glTranslatef
  glm::mat4 rotateCannon = glm::rotate((float)(cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model*=(translateCannon*rotateCannon);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(cannon);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateStand = glm::translate (glm::vec3(-3, -3.25f, 0));        // glTranslatef
  Matrices.model=translateStand;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(stand);
  double y=0;
  if(shoot==true)
   // for(double i=0; i<=d || y>-3.5;)
    //{
      //if(c_time-last_time>=0.1)
      {
        y=y_initial+((i*tan(angle*M_PI/180.0f))-((g*i*i)/(2*speed*speed*cos(angle*M_PI/180.0f)*cos(angle*M_PI/180.0f))));
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateBall = glm::translate (glm::vec3(x_initial+i, y, 0));        // glTranslatef
        Matrices.model=translateBall ;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      //  cout<<"x init -> "<<x_initial<<" y -> "<<y<<" i-> "<<i<<endl;
        draw3DObject(ball);
        b.c_x+=(collide*0.01);
        b.c_y=y;
        i+=(collide*0.01);
  //      last_time=c_time;
      }
    //}
  if(y<-3.5)
  {
    shoot=false;
    collide=1;
    y=0;
    i=0;
  }

   Matrices.model = glm::mat4(1.0f);
   w.c_x=2;
   w.c_y=(-3.5-w.height_wall)/2;
  glm::mat4 translateWall = glm::translate (glm::vec3(w.c_x,w.c_y , 0));        // glTranslatef
  Matrices.model=translateWall;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(wall);

   Matrices.model = glm::mat4(1.0f);
   obj1.c_x=w.c_x;
   obj1.c_y=w.c_y+(w.height_wall/2)+obj1.radius;
   if(collideobj==1)
    {
      if(obj1.c_x+(incx*0.05)<=2.25)
      {
        obj1.c_x+=(incx*0.05);
        incx++;
        cout<<"objx-> "<<obj1.c_x<<" objy-> "<<obj1.c_y<<endl;
      }
      else if(obj1.c_y-(incy*0.05)>-3.5)
      {
        obj1.c_x+=(incx*0.05);

        obj1.c_y-=(incy*0.05);
        incy++;
      }
      else
      {
        obj1.c_x+=(incx*0.05);

        obj1.c_y=-3.5+obj1.radius;
      }

     // cout<<"enter collideobj-> "<<collideobj<<endl;
    }
   glm::mat4 translateObj = glm::translate (glm::vec3(obj1.c_x,obj1.c_y , 0));  
  Matrices.model=translateObj;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obj);


  if((b.radius+obj1.radius)*(b.radius+obj1.radius)>=((b.c_x-obj1.c_x)*(b.c_x-obj1.c_x))+((b.c_y-obj1.c_y)*(b.c_y-obj1.c_y)))
    {
      collision_obj();
     // cout<<"collideobj-> "<<collideobj<<endl;
     
    }
  if(collide==1 && ((b.c_x+w.width_wall/2+b.radius>=w.c_x && b.c_y<-3.5+w.height_wall && b.c_x<=w.c_x-w.width_wall/2)|| ( b.c_x<=w.c_x+w.width_wall/2 && b.c_x>= (w.c_x-(w.width_wall/2)) && -3.5+w.height_wall+b.radius>=b.c_y )))
    collision();
  // Increment angles
  
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	ground=createRectangle (8,0.5,0,1,0);
	createCannon();
  stand=createRectangle(0.1,0.5,0.4,0.2,0);
  //if(shoot==true)
  b.radius=0.1;
  b.c_x=b.c_y=0;
  ball=createCircle(b.radius,0,0,0);

  w.width_wall=0.3;
  w.height_wall=1.75;
  wall=createRectangle(w.width_wall,w.height_wall,0.4,0.4,0);

  obj1.radius=0.1;
  obj1.c_x=0;
  obj1.c_y=0;
  obj=createCircle(obj1.radius,1,0,0.5);
  //shoot=false;  
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.6f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 1) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
