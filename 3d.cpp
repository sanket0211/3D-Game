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
#include <time.h>

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

GLuint programID;

struct Cube{
  float x;
  float y;
  float z;
  float side;
  float height;
  int rotation_angle;
  float rotation_direction;
  float translate_x;
  float translate_y;
  float translate_z;
  int visiblity;
  VAO *vao;
  int movement;
  int icord ;
  int jcord;
};
typedef struct Cube Cube;
Cube land[50][50];
Cube landskeleton[50][50];
Cube sea[100][100];
Cube man[10];
Cube objects[10][10];
Cube monster[10];
Cube destination[10];

int number_of_cubes=10;
int number_of_seacubes=50;
int x=1000;
float zoom=2.5;
int diffheight;
int helicopter_view=0;
int adventure_view=0;
int tower_view=0;
int freeview=0;
int follow_up_view=0;
int top_view=0;
int movement_flag=0;
int totchange=0;
int fall_view=0;
int score=0;

int timer =0;

int mani=0;
int manj=0;
int fall_flag=0;
//Movement Flags
int forward_flag=0,backward_flag=0,left_flag=0,right_flag=0;
int follow_up_forward_flag=0;
int camera_forward_flag=0,camera_backward_flag=0,camera_left_flag=0,camera_right_flag=0;
float translate_camera_z,translate_camera_x,translate_camera_y;
int camera_rotation_angle = 90;

float eye_x,eye_y=-200,eye_z;
float target_x=0,target_y=-400,target_z=100;
double mouse_x,mouse_y;
double xoffsety,yoffset;

float jump_velocity=0;
float jump_time=0;
int jump_flag=0;

float initial_pos=0;

int total_points=0;

float translate_man_x,translate_man_y,translate_man_z;


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
/*static functions*/
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
	//GL_line for triangle
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

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
//to be modified for the assignment
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
	//No diff btw samll and caps
	// to diff btw them then test the mods var
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_UP:

              if(freeview==0  && follow_up_view==0){
                  //forward_flag=1;
                  mani++;
                translate_man_z-=200;
                camera_forward_flag=0;
                }
                else if(freeview==1){
                  camera_forward_flag=1;
                }
                else if(follow_up_view==1 || adventure_view==1){
                  translate_man_x=translate_man_x - sin(man[2].rotation_angle*M_PI/180.0f)*200;
                  translate_man_z=translate_man_z - cos(man[2].rotation_angle*M_PI/180.0f)*200;
                  if(man[2].rotation_angle==0){
                    mani++;
                  }
                  else if(man[2].rotation_angle==90){
                    manj--;
                  }
                  else if(man[2].rotation_angle==180){
                    mani--;
                  }
                  else if(man[2].rotation_angle==270){
                    manj++;
                  }
                }
                //forward_flag=0;
                break;
            case GLFW_KEY_DOWN:
                if(freeview==0 && follow_up_view==0 && adventure_view==0){
                  //backward_flag=1;
                  mani--;
                  translate_man_z+=200;
                }
            	
                //backward_flag=0;
                break;
            case GLFW_KEY_RIGHT:
              if(freeview==0 && follow_up_view==0 && adventure_view==0){
                 // right_flag=1;
                  manj++;
                  translate_man_x+=200;
                  /*man[2].rotation_angle-=90;
                  man[2].rotation_angle+=360;
                  man[2].rotation_angle%=360;
                  man[3].rotation_angle-=90;
                  man[3].rotation_angle+=360;
                  man[3].rotation_angle%=360;*/
                }
            	
                //right_flag=0;
                break;
            case GLFW_KEY_LEFT:
                if(freeview==0 && follow_up_view==0 && adventure_view==0){
                  manj--;
                  translate_man_x-=200;
                }
                
                //left_flag=0;
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;

            case GLFW_KEY_Z:
                // do something ..
                zoom=zoom*1.2;
                break;

            case GLFW_KEY_X:
                // do something ..
                zoom=zoom*.8;

                break;

            case GLFW_KEY_A:
                helicopter_view=0;
                tower_view=0;
                freeview=0;
                follow_up_view=0;
                top_view=0;
                adventure_view=1;
                break;

            case GLFW_KEY_H:
                helicopter_view=1;
                tower_view=0;
                freeview=0;
                follow_up_view=0;
                top_view=0;
                adventure_view=0;
                break;

            case GLFW_KEY_O:
                adventure_view=0;
                helicopter_view=0;
                tower_view=0;
                freeview=0;
                follow_up_view=0;
                top_view=1;
                break;

            case GLFW_KEY_T:
                helicopter_view=0;
                adventure_view=0;
                tower_view=1;
                freeview=0;
                follow_up_view=0;
                top_view=0;
                break;
            case GLFW_KEY_F:
                adventure_view=0;
                helicopter_view=0;
                top_view=0;
                tower_view=0;
                freeview=1;
                translate_camera_x=man[1].x+man[1].translate_x;
                translate_camera_y=man[1].y+man[1].translate_y;
                translate_camera_z=man[1].x+man[1].translate_z; 
                camera_rotation_angle=0;
                break;
            case GLFW_KEY_U:
                adventure_view=0;
                top_view=0;
                helicopter_view=0;
                tower_view=0;
                freeview=0;
                follow_up_view=1;
                translate_camera_x=man[1].x+man[1].translate_x;
                translate_camera_y=man[1].y+man[1].translate_y;
                translate_camera_z=man[1].x+man[1].translate_z; 
                camera_rotation_angle=0;
                break;

            case GLFW_KEY_SPACE:
                jump_flag=1;
                jump_velocity=36;
                jump_time=0;
                initial_pos=man[1].y+man[1].translate_y;
                break;
            case GLFW_KEY_UP:
                if(freeview==0  && follow_up_view==0){
                  //forward_flag=1;
                  man[2].rotation_angle=0;
                  man[3].rotation_angle=0;
                }
                else if(freeview==1){
                  camera_forward_flag=1;
                }
                else if(follow_up_view==1){
                  follow_up_forward_flag=0;
                }
                break;
            case GLFW_KEY_DOWN:
                if(freeview==0 && follow_up_view==0){
                  //backward_flag=1;
                  man[2].rotation_angle=0;
                  man[3].rotation_angle=0;
                }
                /*else if(freeview==1){
                  camera_backward_flag=1;
                  camera_right_flag=0;
                  camera_left_flag=0;
                  camera_rotation_angle+=90;
                  camera_rotation_angle+=360;
                  camera_rotation_angle=camera_rotation_angle%360;
                }*/
                break;
            case GLFW_KEY_RIGHT:
                if(freeview==0 && follow_up_view==0 && adventure_view==0){
                 // right_flag=1;
                  man[2].rotation_angle=270;
                  man[3].rotation_angle=270;
                  /*man[2].rotation_angle-=90;
                  man[2].rotation_angle+=360;
                  man[2].rotation_angle%=360;
                  man[3].rotation_angle-=90;
                  man[3].rotation_angle+=360;
                  man[3].rotation_angle%=360;*/
                }
                else if(freeview==1){
                  camera_backward_flag=0;
                  camera_right_flag=1;
                  camera_left_flag=0;
                  camera_rotation_angle-=90;
                  camera_rotation_angle+=360;
                  camera_rotation_angle=camera_rotation_angle%360;
                }
                else if(follow_up_view==1){
                  man[2].rotation_angle-=90;
                  man[2].rotation_angle+=360;
                  man[2].rotation_angle%=360;
                  man[3].rotation_angle-=90;
                  man[3].rotation_angle+=360;
                  man[3].rotation_angle%=360;
                }
                else if(adventure_view==1){
                  man[2].rotation_angle-=90;
                  man[2].rotation_angle+=360;
                  man[2].rotation_angle%=360;
                  man[3].rotation_angle-=90;
                  man[3].rotation_angle+=360;
                  man[3].rotation_angle%=360;
                }
                break;
            case GLFW_KEY_LEFT:
                if(freeview==0 && follow_up_view==0 && adventure_view==0){
                  man[2].rotation_angle=90;
                  man[3].rotation_angle=90;
                 // left_flag=1;
                  /*man[2].rotation_angle+=90;
                  man[2].rotation_angle+=360;
                  man[2].rotation_angle%=360;
                  man[3].rotation_angle+=90;
                  man[3].rotation_angle+=360;
                  man[3].rotation_angle%=360;*/
                }
                else if(freeview==1){
                  camera_backward_flag=0;
                  camera_right_flag=0;
                  camera_left_flag=1;
                  camera_rotation_angle+=90;
                  camera_rotation_angle+=360;
                  camera_rotation_angle=camera_rotation_angle%360;
                }
                else if(follow_up_view==1){
                  man[2].rotation_angle+=90;
                  man[2].rotation_angle+=360;
                  man[2].rotation_angle%=360;
                  man[3].rotation_angle+=90;
                  man[3].rotation_angle+=360;
                  man[3].rotation_angle%=360;
                }
                else if(adventure_view==1){
                  man[2].rotation_angle+=90;
                  man[2].rotation_angle+=360;
                  man[2].rotation_angle%=360;
                  man[3].rotation_angle+=90;
                  man[3].rotation_angle+=360;
                  man[3].rotation_angle%=360;
                }
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
//ignore for rite now
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
            if (action == GLFW_PRESS){
                zoom=zoom*0.1;
                //cout << zoom << endl;

            }
            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                zoom=zoom*10.0;
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

     if(tower_view==1 || helicopter_view==1 || top_view==1){
      Matrices.projection = glm::ortho(-550.0f*zoom/*(left)*/, 550.0f*zoom, -350.0f*zoom, 350.0f*zoom, -5500.0f*zoom/*depth*/, +5500.0f*zoom);
     }
     else{
     Matrices.projection = glm::perspective(45.0f, 1.6f, 0.1f, 10000.0f);
   }
     //cout << "zoom"<< zoom << endl;

}

VAO* createCube(float x, float y, float z,float s,float height){
  
  GLfloat vertex_buffer_data [] = {
    x+(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),

    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),

    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z-(s/2),

  };

  GLfloat color_buffer_data [108];
  int i=0;
  while(i<18){
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
  }

  while(i<36){
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
  }

  while(i<54){
    color_buffer_data [i]=0.3;i++;
    color_buffer_data [i]=0.3;i++;
    color_buffer_data [i]=0.3;i++;
  }

  while(i<72){
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
  }

  while(i<90){
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
  }

  while(i<108){
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
    color_buffer_data [i]=0.15;i++;
  }

  return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

}

VAO* createCubeSkeleton(float x, float y, float z,float s,float height){
  
  GLfloat vertex_buffer_data [] = {
    x+(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),

    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),

    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z-(s/2),

  };

  GLfloat color_buffer_data [108];
  int i=0;
  while(i<18){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
  }

  while(i<36){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
  }

  while(i<54){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
  }

  while(i<72){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
  }

  while(i<90){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
  }

  while(i<108){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
  }

  return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_LINE);

}

VAO* createMan(float x, float y, float z,float s,float height,float red, float green, float blue){
  
  GLfloat vertex_buffer_data [] = {
    x+(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),

    x-(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z+(s/2),

    x-(s/2),y-(height/2),z+(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y-(height/2),z+(s/2),
    x-(s/2),y+(height/2),z+(s/2),
    x+(s/2),y+(height/2),z+(s/2),

    x-(s/2),y-(height/2),z-(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x+(s/2),y-(height/2),z-(s/2),
    x-(s/2),y+(height/2),z-(s/2),
    x+(s/2),y+(height/2),z-(s/2),

  };

  GLfloat color_buffer_data [108];
  int i=0;
  while(i<18){
    color_buffer_data [i]=red;i++;
    color_buffer_data [i]=green;i++;
    color_buffer_data [i]=blue;i++;
  }

  while(i<36){
    color_buffer_data [i]=red;i++;
    color_buffer_data [i]=green;i++;
    color_buffer_data [i]=blue;i++;
  }

  while(i<54){
    color_buffer_data [i]=red;i++;
    color_buffer_data [i]=green;i++;
    color_buffer_data [i]=blue;i++;
  }

  while(i<72){
    color_buffer_data [i]=red;i++;
    color_buffer_data [i]=green;i++;
    color_buffer_data [i]=blue;i++;
  }

  while(i<90){
    color_buffer_data [i]=red;i++;
    color_buffer_data [i]=green;i++;
    color_buffer_data [i]=blue;i++;
  }

  while(i<108){
    color_buffer_data [i]=red;i++;
    color_buffer_data [i]=green;i++;
    color_buffer_data [i]=blue;i++;
  }

  return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

}

VAO* createSea(float x, float y, float z,float s){
  
  GLfloat vertex_buffer_data [] = {
    x+(s/2),y-(s/2),z-(s/2),
    x+(s/2),y-(s/2),z+(s/2),
    x+(s/2),y+(s/2),z-(s/2),
    x+(s/2),y-(s/2),z+(s/2),
    x+(s/2),y+(s/2),z-(s/2),
    x+(s/2),y+(s/2),z+(s/2),

    x-(s/2),y-(s/2),z-(s/2),
    x-(s/2),y-(s/2),z+(s/2),
    x-(s/2),y+(s/2),z-(s/2),
    x-(s/2),y-(s/2),z+(s/2),
    x-(s/2),y+(s/2),z-(s/2),
    x-(s/2),y+(s/2),z+(s/2),

    x-(s/2),y+(s/2),z-(s/2),
    x-(s/2),y+(s/2),z+(s/2),
    x+(s/2),y+(s/2),z-(s/2),
    x-(s/2),y+(s/2),z+(s/2),
    x+(s/2),y+(s/2),z-(s/2),
    x+(s/2),y+(s/2),z+(s/2),

    x-(s/2),y-(s/2),z-(s/2),
    x-(s/2),y-(s/2),z+(s/2),
    x+(s/2),y-(s/2),z-(s/2),
    x-(s/2),y-(s/2),z+(s/2),
    x+(s/2),y-(s/2),z-(s/2),
    x+(s/2),y-(s/2),z+(s/2),

    x-(s/2),y-(s/2),z+(s/2),
    x+(s/2),y-(s/2),z+(s/2),
    x-(s/2),y+(s/2),z+(s/2),
    x+(s/2),y-(s/2),z+(s/2),
    x-(s/2),y+(s/2),z+(s/2),
    x+(s/2),y+(s/2),z+(s/2),

    x-(s/2),y-(s/2),z-(s/2),
    x+(s/2),y-(s/2),z-(s/2),
    x-(s/2),y+(s/2),z-(s/2),
    x+(s/2),y-(s/2),z-(s/2),
    x-(s/2),y+(s/2),z-(s/2),
    x+(s/2),y+(s/2),z-(s/2),

  };

  GLfloat color_buffer_data [108];
  int i=0;
  while(i<18){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=1;i++;
  }

  while(i<36){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=1;i++;
  }

  while(i<54){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=1;i++;
  }

  while(i<72){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=1;i++;
  }

  while(i<90){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=1;i++;
  }

  while(i<108){
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=0;i++;
    color_buffer_data [i]=1;i++;
  }

  return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

}




/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye (eye_x,eye_y,eye_z);
  //glm::vec3 eye ( 550*cos(camera_rotation_angle*M_PI/180.0f), eye_y, 350*sin(camera_rotation_angle*M_PI/180.0f) +1200);
  //glm::vec3 eye (0,0,1);
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (target_x,target_y, target_z);

  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  for(int i=0;i<number_of_seacubes;i++){
    for(int j=0;j<number_of_seacubes;j++){
      glm::mat4 MVP;  // MVP = Projection * View * Model
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 rotatesea = glm::rotate((float)(sea[i][j].rotation_angle*M_PI/180.0f), glm::vec3(0,0,1));
      glm::mat4 translatesea = glm::translate (glm::vec3(sea[i][j].x+sea[i][j].translate_x,sea[i][j].y+sea[i][j].translate_y, sea[i][j].z+sea[i][j].translate_z));
     // cout << land[i][j].x << " " << land[i][j].y << " "<<  land[i][j].z << endl;
      //cout << land[i][j].x+land[i][j].translate_x << " " << land[i][j].y+land[i][j].translate_y << " "<<  land[i][j].z+land[i][j].translate_z << endl;
      Matrices.model *= (translatesea*rotatesea);
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(sea[i][j].vao);
    }
  }


  for(int i=0;i<number_of_cubes;i++){
    for(int j=0;j<number_of_cubes;j++){
      glm::mat4 MVP;  // MVP = Projection * View * Model
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 rotateland = glm::rotate((float)(land[i][j].rotation_angle*M_PI/180.0f), glm::vec3(0,0,1));
      glm::mat4 translateland = glm::translate (glm::vec3(land[i][j].x+land[i][j].translate_x,land[i][j].y+land[i][j].translate_y, land[i][j].z+land[i][j].translate_z));
      ////cout << land[i][j].x << " " << land[i][j].y << " "<<  land[i][j].z << endl;
     // cout << land[i][j].x+land[i][j].translate_x << " " << land[i][j].y+land[i][j].translate_y << " "<<  land[i][j].z+land[i][j].translate_z << endl;
      Matrices.model *= (translateland*rotateland);
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      if(land[i][j].visiblity==1){
        draw3DObject(land[i][j].vao);
      }
    }
  }

  for(int i=0;i<number_of_cubes;i++){
    for(int j=0;j<number_of_cubes;j++){
      glm::mat4 MVP;  // MVP = Projection * View * Model
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 rotateobj = glm::rotate((float)(objects[i][j].rotation_angle*M_PI/180.0f), glm::vec3(0,0,1));
      glm::mat4 translateobj = glm::translate (glm::vec3(objects[i][j].x+objects[i][j].translate_x,objects[i][j].y+objects[i][j].translate_y, objects[i][j].z+objects[i][j].translate_z));
      ////cout << land[i][j].x << " " << land[i][j].y << " "<<  land[i][j].z << endl;
     // cout << land[i][j].x+land[i][j].translate_x << " " << land[i][j].y+land[i][j].translate_y << " "<<  land[i][j].z+land[i][j].translate_z << endl;
      Matrices.model *= (translateobj*rotateobj);
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      if(land[i][j].visiblity==1 and objects[i][j].visiblity==1){
        draw3DObject(objects[i][j].vao);
      }
    }
  }

  for(int i=0;i<number_of_cubes;i++){
    for(int j=0;j<number_of_cubes;j++){
      glm::mat4 MVP;  // MVP = Projection * View * Model
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 rotatelandskeleton = glm::rotate((float)(landskeleton[i][j].rotation_angle*M_PI/180.0f), glm::vec3(0,0,1));
      glm::mat4 translatelandskeleton = glm::translate (glm::vec3(landskeleton[i][j].x+landskeleton[i][j].translate_x,landskeleton[i][j].y+landskeleton[i][j].translate_y, landskeleton[i][j].z+landskeleton[i][j].translate_z));
      ////cout << land[i][j].x << " " << land[i][j].y << " "<<  land[i][j].z << endl;
     // cout << land[i][j].x+land[i][j].translate_x << " " << land[i][j].y+land[i][j].translate_y << " "<<  land[i][j].z+land[i][j].translate_z << endl;
      Matrices.model *= (translatelandskeleton*rotatelandskeleton);
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      if(land[i][j].visiblity==1){
        draw3DObject(landskeleton[i][j].vao);
      }
    }
  }

  for(int i=0;i<4;i++){
    
    glm::mat4 MVP;  // MVP = Projection * View * Model
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 rotateman = glm::rotate((float)(man[i].rotation_angle*M_PI/180.0f), glm::vec3(0,1,0));
    glm::mat4 translateman = glm::translate (glm::vec3(man[i].x+man[i].translate_x,man[i].y+man[i].translate_y,man[i].z+man[i].translate_z));
    Matrices.model *= (translateman*rotateman);
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(man[i].vao);
  }

  for(int i=0;i<3;i++){
    
    glm::mat4 MVP;  // MVP = Projection * View * Model
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 rotatemonster = glm::rotate((float)(monster[i].rotation_angle*M_PI/180.0f), glm::vec3(0,1,0));
    glm::mat4 translatemonster = glm::translate (glm::vec3(monster[i].x+monster[i].translate_x,monster[i].y+monster[i].translate_y,monster[i].z+monster[i].translate_z));
    Matrices.model *= (translatemonster*rotatemonster);
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(monster[i].vao);
  }

  if(destination[0].visiblity==1){
    glm::mat4 MVP;  // MVP = Projection * View * Model
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 rotatedestination = glm::rotate((float)(destination[0].rotation_angle*M_PI/180.0f), glm::vec3(0,1,0));
    glm::mat4 translatedestination = glm::translate (glm::vec3(destination[0].x+destination[0].translate_x,destination[0].y+destination[0].translate_y,destination[0].z+destination[0].translate_z));
    Matrices.model *= (translatedestination*rotatedestination);
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(destination[0].vao);
  }



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
    glfwSetKeyCallback(window, keyboard/*function name*/);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton/*function name*/);  // mouse button clicks

    return window;
}

void checkLand(){
  if(land[mani][manj].visiblity ==0 && jump_flag==0){
    fall_flag=1;

  }
}
void checkmonster(){
  for(int i=0;i<3;i++){
    if(monster[i].icord == mani && monster[i].jcord == manj){
      fall_flag=1;
      
    }
  }
}

void setCamera(int hv,int tv,int fv,int fu,int ov,int av,int fall){
  if(tv==1){
    //glm::vec3 eye ( 550*cos(camera_rotation_angle*M_PI/180.0f), eye_y, 350*sin(camera_rotation_angle*M_PI/180.0f) +1200);
    camera_rotation_angle = 90 + -1*((mouse_x*180)/550);
    eye_x=550*cos(camera_rotation_angle*M_PI/180.0f);
    eye_y=mouse_y;
    eye_z=350*sin(camera_rotation_angle*M_PI/180.0f) +200;
    target_x=0;target_y=-400;target_z=-1000;
  }
  else if(hv==1){
    camera_rotation_angle-=(.01*mouse_x);
    eye_x=550*cos(camera_rotation_angle*M_PI/180.0f);
    eye_y=mouse_y;
    eye_z=350*sin(camera_rotation_angle*M_PI/180.0f)-1000;
    target_x=0;target_y=0;target_z=-1000;
  }
  else if(fv==1){
    eye_x=translate_camera_x; //man[1].x+man[1].translate_x;
    eye_y=translate_camera_y+20; //man[1].y+man[1].translate_y+50;
    eye_z=translate_camera_z; //man[1].x+man[1].translate_z;
    target_y=translate_camera_y; //man[1].y+man[1].translate_y;
    target_x=translate_camera_x - sin(camera_rotation_angle*M_PI/180.0f)*50;
    target_z=translate_camera_z - cos(camera_rotation_angle*M_PI/180.0f)*50;
  }
  else if(fu==1){
    eye_x=man[1].x+man[1].translate_x+500*sin(man[2].rotation_angle*M_PI/180.0f);
    eye_y=man[1].y+man[1].translate_y+300;
    eye_z=man[1].z+man[1].translate_z+500*cos(man[2].rotation_angle*M_PI/180.0f);
    target_y=man[1].y+man[1].translate_y-100;
    target_x=man[1].x+man[1].translate_x - 300*sin(man[2].rotation_angle*M_PI/180.0f);
    target_z=man[1].z+man[1].translate_z - 300*cos(man[2].rotation_angle*M_PI/180.0f);
  }
  else if(ov==1){
    eye_x=-1*mouse_x;
    eye_y=50;
    eye_z=mouse_y-1000;
    target_x=-1*mouse_x;
    target_z=mouse_y-1-1000;
    target_y=-450;
  }
  else if(av==1){
    eye_x=man[1].x+man[1].translate_x-50*sin(man[2].rotation_angle*M_PI/180.0f);
    eye_y=man[1].y+man[1].translate_y+300;
    eye_z=man[1].z+man[1].translate_z-50*cos(man[2].rotation_angle*M_PI/180.0f);
    target_y=man[1].y+man[1].translate_y-100;
    target_x=man[1].x+man[1].translate_x - 400*sin(man[2].rotation_angle*M_PI/180.0f);
    target_z=man[1].z+man[1].translate_z - 400*cos(man[2].rotation_angle*M_PI/180.0f);
  }
  else if(fall==1){
    eye_x=man[1].x+man[1].translate_x-200*sin(man[2].rotation_angle*M_PI/180.0f);
    eye_y=man[1].y+man[1].translate_y;
    eye_z=man[1].z+man[1].translate_z-200*cos(man[2].rotation_angle*M_PI/180.0f);
    target_y=man[1].y+man[1].translate_y;
    target_x=man[1].x+man[1].translate_x;
    target_z=man[1].z+man[1].translate_z;
  }
}

void jump(){
  for(int i=0;i<4;i++){
    man[i].translate_y = (man[i].translate_y + (jump_velocity*jump_time) + (-1*jump_time*jump_time));
  }
  if((man[1].translate_y) < initial_pos){
    jump_velocity==0;
    jump_time=0;
    jump_flag=0;
  } 
}

void changeManPosition(int ff,int bf, int lf, int rf,int fu){
  if(ff==1){
    translate_man_z-=10;
  }
  else if(bf==1){
    translate_man_z+=10;
  }
  else if(lf==1){
    translate_man_x-=10;
  }
  else if(rf==1){
    translate_man_x+=10;
  }
  else if(fu==1){
    translate_man_x=translate_man_x - sin(man[2].rotation_angle*M_PI/180.0f)*10;
    translate_man_z=translate_man_z - cos(man[2].rotation_angle*M_PI/180.0f)*10;
  }
  for(int i=0;i<4;i++){
    man[i].translate_x=translate_man_x;
    man[i].translate_y=translate_man_y;
    man[i].translate_z=translate_man_z;
  }
}

void moveLand(int change, int randobs){
	for(int i=0;i<number_of_cubes;i++){
		for(int j=0;j<number_of_cubes;j++){
			if(land[i][j].visiblity!=0 && (i+j)==randobs){
				if(land[i][j].movement==1){
          if(j<9){
  					land[i][j].translate_x +=change;
  					landskeleton[i][j].translate_x = land[i][j].translate_x;
            if(totchange>=200){
              land[i][j].visiblity=0;
              j++;
              land[i][j].visiblity=1;
            }
          }
          
				}
				if(land[i][j].movement==2){
          if(i<9){
  					land[i][j].translate_z+=change;
  					landskeleton[i][j].translate_z = land[i][j].translate_z;
            if(totchange>=200){
              land[i][j].visiblity=0;
              i++;
              land[i][j].visiblity=1;
            }
          }
				}
				if(land[i][j].movement==3){
          if(j>0){
  					land[i][j].translate_x-=change;
  					landskeleton[i][j].translate_x = land[i][j].translate_x;
            if(totchange>=200){
              land[i][j].visiblity=0;
              j--;
              land[i][j].visiblity=1;
            }
          }
				}
				if(land[i][j].movement==4){
          if(i>0){
  					land[i][j].translate_z-=change;
  					landskeleton[i][j].translate_z = land[i][j].translate_z;
            if(totchange>=200){
              land[i][j].visiblity=0;
              i--;
              land[i][j].visiblity=1;
            }
          }
				}

				if(totchange>=200){
				  
				land[i][j].movement=land[i][j].movement%4+1;
				}
			}
		}
	}
}
void checkpoints(){
  if(objects[mani][manj].visiblity!=0){
    objects[mani][manj].visiblity=0;
    total_points--;
    score++;
  }
}

void changeCameraPosition(int cff, int clf, int crf){
  if(cff==1){
    translate_camera_x=translate_camera_x - sin(camera_rotation_angle*M_PI/180.0f)*10;
    translate_camera_z=translate_camera_z - cos(camera_rotation_angle*M_PI/180.0f)*10;
  }
}

void moveMonster(){
  srand(time(NULL));
  for(int i=0;i<3;i++){
    monster[i].movement=rand()%4;
    cout << monster[i].movement << endl;
    if(monster[i].movement==0){
      if(monster[i].icord!=9){
      monster[i].icord++;
      monster[i].translate_z=land[monster[i].icord][monster[i].jcord].translate_z;
      monster[i].translate_x=land[monster[i].icord][monster[i].jcord].translate_x;
      }
    }
    if(monster[i].movement==1){
      if(monster[i].icord!=0){
        monster[i].icord--;
        monster[i].translate_z=land[monster[i].icord][monster[i].jcord].translate_z;
        monster[i].translate_x=land[monster[i].icord][monster[i].jcord].translate_x;
      }
      
    }
    if(monster[i].movement==2){
      if(monster[i].jcord!=0){
        monster[i].jcord--;
        monster[i].translate_z=land[monster[i].icord][monster[i].jcord].translate_z;
        monster[i].translate_x=land[monster[i].icord][monster[i].jcord].translate_x;
      }
      
    }
    if(monster[i].movement==3){
      if(monster[i].jcord!=9){
        monster[i].jcord++;
        monster[i].translate_z=land[monster[i].icord][monster[i].jcord].translate_z;
        monster[i].translate_x=land[monster[i].icord][monster[i].jcord].translate_x;
      }
      
    }
  }

}


void fall(){
  translate_man_y-=5;
  follow_up_view=1;
  for(int i=0;i<4;i++){
    man[i].translate_x=translate_man_x;
    man[i].translate_y=translate_man_y;
    man[i].translate_z=translate_man_z;
  }
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here *///object creation
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
  srand (time(NULL));
  diffheight=(rand()%10)*20;
  //cout << diffheight << endl;
  //cout << diffheight << endl;
  Cube c;float side = 200;float translate_z=0;float translate_x=0;float translate_y=-400;int movement=1;
  for(int i=0;i<number_of_cubes;i++){
    translate_x=(number_of_cubes/2)*(-side)+(side/2)-side;
    translate_z-=(side);
    for(int j=0;j<number_of_cubes;j++){
      translate_x+=(side);
      c.height=200;c.x=0;c.y=0;c.z=0;c.side=side;c.translate_x=translate_x;c.translate_y=translate_y;c.translate_z=translate_z;c.visiblity=rand()%3;c.rotation_direction=1;
      c.rotation_angle=0;
      if((i==0 && j==0) || (i==9 && j==9)){
        c.visiblity=1;
      }
      if(c.visiblity!=0){
        c.visiblity=1;
        c.movement=movement%4+1;
        movement +=1;
      }
      c.vao=createCube(c.x,c.y,c.z,c.side,c.height);
     // cout << c.height << endl;
      land[i][j]=c;
    }
  }

  side = 200;translate_z=0;translate_x=0;translate_y=-400;
  for(int i=0;i<number_of_cubes;i++){
    translate_x=(number_of_cubes/2)*(-side)+(side/2)-side;
    translate_z-=(side);
    for(int j=0;j<number_of_cubes;j++){
      translate_x+=(side);
      c.height=200;c.x=0;c.y=0;c.z=0;c.side=side;c.translate_x=translate_x;c.translate_y=translate_y;c.translate_z=translate_z;c.visiblity=rand()%3;c.rotation_direction=1;
      c.rotation_angle=0;
      if(c.visiblity!=0){
        c.visiblity=1;
      }
      c.vao=createCubeSkeleton(c.x,c.y,c.z,c.side,c.height);
     // cout << c.height << endl;
      landskeleton[i][j]=c;
    }
  }

  side = 200;translate_z=0;translate_x=0;translate_y=-200;
  for(int i=0;i<number_of_cubes;i++){
    translate_x=(number_of_cubes/2)*(-side)+(side/2)-side;
    translate_z-=(side);
    for(int j=0;j<number_of_cubes;j++){
      translate_x+=(side);
      c.height=50;c.x=0;c.y=0;c.z=0;c.side=50;c.translate_x=translate_x;c.translate_y=translate_y;c.translate_z=translate_z;c.visiblity=rand()%2;c.rotation_direction=1;
      c.rotation_angle=0;
      if(c.visiblity!=0 && land[i][j].visiblity!=0){
        c.visiblity=1;
        total_points++;
      }
      c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,0,1);
     // cout << c.height << endl;
      objects[i][j]=c;
    }
  }

  side = 200;translate_z=(number_of_seacubes/2)*side;translate_x=0;translate_y=-450;
  for(int i=0;i<number_of_seacubes;i++){
    translate_x=(number_of_seacubes/2)*(-side)+(side/2)-side;
    translate_z-=(side);
    for(int j=0;j<number_of_seacubes;j++){
      translate_x+=(side);
      c.x=0;c.y=0;c.z=0;c.side=side;c.translate_x=translate_x;c.translate_y=translate_y;c.translate_z=translate_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
      c.vao=createSea(c.x,c.y,c.z,c.side);
      sea[i][j]=c;
    }
  }

  float monsterwidth=50;
  float body_height,leg_height,leg_width,head_height,head_width,body_width;
  body_height=100;leg_height=100;leg_width=20;head_width=50;head_height=50;body_width=100;
  translate_man_x=land[0][0].translate_x;translate_man_y=land[0][0].height/2 + man[2].y+leg_height/2 + body_height/2-400; translate_man_z=land[0][0].translate_z;

  side = body_width;translate_z=0;translate_x=0;translate_y=0;
  c.height=body_height;c.x=0;c.y=0;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,0,0);
  man[0]=c;

  side = head_width;translate_z=0;translate_x=0;translate_y=(body_height+head_height)/2;
  c.height=head_height;c.x=0;c.y=(head_height)/2+10;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,.5,0);
  man[1]=c;

  side = leg_width;translate_z=0;translate_x=-25;translate_y=(body_height+leg_height)/2;
  c.height=head_height;c.x=-25;c.y=-1*((body_height/2))+15;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,.5,0);
  man[2]=c;

  side = leg_width;translate_z=0;translate_x=25;translate_y=(body_height+leg_height)/2;
  c.height=head_height;c.x=25;c.y=-1*(body_height/2)+15;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,.5,0);
  man[3]=c;

  c.height=monsterwidth;c.x=0;c.y=0;c.z=0;c.side=monsterwidth;c.translate_x=land[9][9].translate_x;c.translate_y=land[9][9].translate_y+125;c.translate_z=land[9][9].translate_z;c.visiblity=1;c.rotation_angle=0;c.rotation_direction=1;c.icord=9;c.jcord=9;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,0,0,0);
  monster[0]=c;

  c.height=monsterwidth;c.x=0;c.y=0;c.z=0;c.side=monsterwidth;c.translate_x=land[9][9].translate_x;c.translate_y=land[9][9].translate_y+125;c.translate_z=land[9][9].translate_z;c.visiblity=1;c.rotation_angle=0;c.rotation_direction=1;c.icord=9;c.jcord=9;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,0,0,0);
  monster[1]=c;

  c.height=monsterwidth;c.x=0;c.y=0;c.z=0;c.side=monsterwidth;c.translate_x=land[9][9].translate_x;c.translate_y=land[9][9].translate_y+125;c.translate_z=land[9][9].translate_z;c.visiblity=1;c.rotation_angle=0;c.rotation_direction=1;c.icord=9;c.jcord=9;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,0,0,0);
  monster[2]=c;

  c.height = 600;c.x=0;c.y=0;c.z=0;c.side=200;c.translate_x=land[9][9].translate_x;c.translate_y=land[9][9].translate_y;c.translate_z=land[9][9].translate_z;c.visiblity=0;c.icord=9;c.jcord=9;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,0,1,0);
  destination[0]=c;
  /*body_height=100;leg_height=100;leg_width=20;head_width=50;head_height=50;body_width=100;
  translate_man_x=land[0][0].translate_x;translate_man_y=land[0][0].height/2 + monster[2].y+leg_height/2 + body_height/2-400; translate_man_z=land[0][0].translate_z;

  side = body_width;translate_z=0;translate_x=0;translate_y=0;
  c.height=body_height;c.x=0;c.y=0;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,0,0);
  monster[0]=c;

  side = head_width;translate_z=0;translate_x=0;translate_y=(body_height+head_height)/2;
  c.height=head_height;c.x=0;c.y=(head_height)/2+10;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,.5,0);
  monster[1]=c;

  side = leg_width;translate_z=0;translate_x=-25;translate_y=(body_height+leg_height)/2;
  c.height=head_height;c.x=-25;c.y=-1*((body_height/2))+15;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,.5,0);
  monster[2]=c;

  side = leg_width;translate_z=0;translate_x=25;translate_y=(body_height+leg_height)/2;
  c.height=head_height;c.x=25;c.y=-1*(body_height/2)+15;c.z=0;c.side=side;c.translate_x=translate_man_x;c.translate_y=translate_man_y;c.translate_z=translate_man_z;c.visiblity=1;c.rotation_direction=1;c.rotation_angle=0;
  c.vao=createMan(c.x,c.y,c.z,c.side,c.height,1,.5,0);
  monster[3]=c;*/





  //Creating Sea.
  
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void scroll(GLFWwindow* window,double x,double y){

  if(y>0){
    zoom*=.5;
  }
  else if(y<0){
    zoom*=2;
  }

}
int randobs=0;
int main (int argc, char** argv)
{
	int width = 1100;
	int height = 700;
  
  tower_view=1;
    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    double monster_last_update_time = glfwGetTime(),monster_current_time;
    double timer_last = glfwGetTime(), timer_current;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        changeManPosition(forward_flag,backward_flag,left_flag,right_flag,follow_up_forward_flag);
        checkpoints();
        checkmonster();
        changeCameraPosition(camera_forward_flag, camera_left_flag, camera_right_flag);
        checkLand();
        if(fall_flag==1){
          fall_view=1;
          helicopter_view=0;top_view=0;freeview=0;follow_up_view=0;adventure_view=0;tower_view=0;
          fall();
        }
        if(total_points==0){
          destination[0].visiblity=1;
          if(mani==9 && manj==9){ 
            cout << "congratulations, you completed the adventure in " << timer << " seconds" << endl;
            quit(window);
          }
        }
        if(movement_flag==1){
        	totchange=totchange+10;
        	moveLand(10,randobs);
        	if(totchange >=200){
        		movement_flag=0;
        		totchange=0;
        	}
        }
        
        reshapeWindow (window, width, height);
        if(jump_flag==1){
          jump_time+=2;
        }
        jump();
        draw();
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        glfwSetScrollCallback(window,scroll);
        //mousescroll(GLFWwindow* window, double xoffset, double yoffset);
        mouse_x-=550;
        mouse_y*=-1;
        mouse_y+=350;
        setCamera(helicopter_view,tower_view,freeview,follow_up_view,top_view,adventure_view,fall_view);

        if(man[1].translate_y<=-700){
          cout << "Sorry you lost!!" << endl;
          cout << "score = " << score << endl;
          quit(window);
        }
        //camera_rotation_angle = 90 + -1*((mouse_x*180)/550);
       // camera_rotation_angle=360;
        //eye_x=mouse_x;
       // eye_y=mouse_y;
        //cout << mouse_x << " " << mouse_y << endl;

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);//shows the frame you rendered 

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds,can be used to make animations smooth
        monster_current_time = glfwGetTime();
        timer_current =  glfwGetTime();
        if ((current_time - last_update_time) >= 5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
            srand (time(NULL));
  			randobs=(rand()%20)+1;
            movement_flag=1;
            
            //camera_rotation_angle+=10;
            //x=x-10;
           // zoom=zoom+10.0;

        }

        if((monster_current_time-monster_last_update_time ) >=0.5){
          monster_last_update_time = monster_current_time;
          moveMonster();
        }

        if((timer_last - timer_current) >=1){
          
          timer_last = timer_current;
          timer ++;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

