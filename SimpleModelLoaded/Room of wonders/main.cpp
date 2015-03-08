#define FREEGLUT_STATIC

#include<iostream>
#include<GLTools.h>
#include<GL/glut.h>
#include<GLMatrixStack.h>
#include<GLGeometryTransform.h>
#include<stopWatch.h>

#include <GLFrustum.h>
#include "header.h"
#include<iostream>
#include<conio.h>
#include <stdio.h>

#define MY_PORT_NUMBER 9797


SOCKET listenSocket,connectSocket;
sockaddr_in serverAddr, clientAddr;	//vars for storing addresses
int clientLength=0,msg_length;
char dataToSend[2048],dataRecieved[2048];
float prev_yaw,prev_roll,prev_pitch;

using namespace std;

// vertices
GLBatch heron_batch;

// shaders
GLuint shader;

// vertex shader uniforms
GLuint locNM, locMVP, locMV, locLight, locSign;    

// pixel shader uniforms
GLuint locTexture, locAmb, locDiff, 
locMaterial, locShine, locNoShine;

GLGeometryTransform transformPipeline;
GLMatrixStack modelViewMatrix, projectionMatrix;

GLuint Tex_heron;

GLFrame worldFrame;

float len=1280, wide=1024, turn_y=0, turn_z =0, turn_x=-90, inc = 5;


// vertex arrays read from obj file (containing only distinct vertices)
component *v, *vn, *vt;

// the final vertics given to openGL
M3DVector3f *Ver;
M3DVector3f *Normals;
M3DVector2f *vTexCoords; 	

// light position 
// in which space? normally this is given in in world space, but the vertex shader expects this in view space :(
M3DVector3f lightPosition = {-450, 240, 1335};  // world space
M3DVector3f viewSpaceLightPos;  // view space

struct Rotation {
	float yaw,pitch,roll;
};

class object
{	
public:

    GLfloat ambColor;
    GLfloat diffColor;
    GLfloat material;
    GLfloat translation1[3];
    GLfloat translation2[3];
    GLfloat translation3[3];
    GLfloat translation4[3];

    GLfloat rotation_x[4];
    GLfloat rotation_y[4];
    GLfloat rotation_z[4];
    GLfloat scale[3];
    GLfloat translateO;
    int noShine;
    float shininess;


    object()
    {

        ambColor = 0.01;
        diffColor = 1;
        material = 1;


        rotation_x[0]=0;
        rotation_x[1]=1;
        rotation_x[2]=0;
        rotation_x[3]=0;


        rotation_y[0]=0;
        rotation_y[1]=0;
        rotation_y[2]=1;
        rotation_y[3]=0;


        rotation_z[0]=0;
        rotation_z[1]=0;
        rotation_z[2]=0;
        rotation_z[3]=1;


        translation1[0]=0;
        translation1[1]=0;
        translation1[2]=0;			 

        translation2[0]=0;
        translation2[1]=0;
        translation2[2]=0;



        translation3[0]=0;
        translation3[1]=0;
        translation3[2]=0;			 

        translation4[0]=0;
        translation4[1]=0;
        translation4[2]=0;


        scale[0]=1;
        scale[1]=1;
        scale[2]=1;

        shininess=50;
        noShine=0;
        translateO=0;
    }
};

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    GLbyte *pBits;
    int nWidth, nHeight, nComponents;
    GLenum eFormat;

    // Read the texture bits
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if(pBits == NULL) 
        return false;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
        eFormat, GL_UNSIGNED_BYTE, pBits);

    free(pBits);

    if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
        minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_NEAREST_MIPMAP_NEAREST)
        glGenerateMipmap(GL_TEXTURE_2D);    
    return true;
}

void freePointers()
{
    free(v);
    free(vn);
    free(vt);
    free(Ver);
    free(Normals);
    free(vTexCoords);
}


void fillBuffer(char fname[40],GLBatch *batch)
{

    FILE *fp;
    fp=fopen(fname,"r+");
    int total_ver=loadMesh(fp);	
    fclose(fp);

    batch->Begin(GL_TRIANGLES,total_ver,1);
    batch->CopyVertexData3f(Ver);
    batch->CopyTexCoordData2f(vTexCoords,0);
    batch->CopyNormalDataf(Normals);	
    batch->End();

    freePointers();
}

void setup()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glClearColor(0.4,0.4,0.9,1);
    worldFrame.RotateWorld(-0.3,1,0,0);	
    worldFrame.MoveForward(-3000);
    worldFrame.MoveUp(60);

    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);

    GLFrustum viewFrustum;	        
    viewFrustum.SetPerspective(35.0f, float(len)/float(wide),1, 5000);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());

    int total_ver;

    fillBuffer("girl.obj",&heron_batch);	
   

    shader=gltLoadShaderPairWithAttributes("ver.vp", "frag.fp", 3, 
        GLT_ATTRIBUTE_VERTEX, "vPosition", 
        GLT_ATTRIBUTE_NORMAL, "vNormal", 
        GLT_ATTRIBUTE_TEXTURE0, "vTexture");

    // vertex shader's uniforms
    locNM=glGetUniformLocation(shader,"normalMatrix");
    locMVP=glGetUniformLocation(shader,"mvpMatrix");
    locMV=glGetUniformLocation(shader,"mvMatrix");
    locLight=glGetUniformLocation(shader,"lightPosition");
    locSign=glGetUniformLocation(shader,"sign");

    // fragment shader uniforms    
    locTexture=glGetUniformLocation(shader,"colorMap2D");
    locDiff=glGetUniformLocation(shader,"diffLight");
    locAmb=glGetUniformLocation(shader,"ambLight");
    locShine=glGetUniformLocation(shader,"shininess");
    locMaterial=glGetUniformLocation(shader,"material");
    locNoShine=glGetUniformLocation(shader,"noShine");


    glGenTextures(1,&Tex_heron);
    glBindTexture(GL_TEXTURE_2D,Tex_heron);	
    LoadTGATexture("copper.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

}

void updateMatrices()
{
    glUniformMatrix3fv(locNM,1,GL_FALSE,transformPipeline.GetNormalMatrix());
    glUniformMatrix4fv(locMV,1,GL_FALSE,transformPipeline.GetModelViewMatrix());
    glUniformMatrix4fv(locMVP,1,GL_FALSE,transformPipeline.GetModelViewProjectionMatrix());
}

void bindDraw(GLBatch *batch,GLuint texName)
{
    glBindTexture(GL_TEXTURE_2D,texName);	
    batch->Draw();
}


void setUniforms(object obj)
{
    modelViewMatrix.PushMatrix();

    //glUniform3fv(locLight, 1, lightPosition);
    glUniform3fv(locLight, 1, viewSpaceLightPos);
    glUniform1f(locAmb, obj.ambColor);
    glUniform1f(locDiff, obj.diffColor);
    glUniform1f(locShine, obj.shininess);
    glUniform1f(locMaterial, obj.material);
    glUniform1i(locTexture, 0);
    glUniform1i(locSign, 1);	
    glUniform1f(locNoShine, obj.noShine);

    modelViewMatrix.Scale(obj.scale[0],obj.scale[1],obj.scale[2]);

	modelViewMatrix.Rotate(obj.rotation_y[0],obj.rotation_y[1],obj.rotation_y[2],obj.rotation_y[3]);		
	modelViewMatrix.Rotate(obj.rotation_z[0],obj.rotation_z[1],obj.rotation_z[2],obj.rotation_z[3]);
	modelViewMatrix.Rotate(obj.rotation_x[0],obj.rotation_x[1],obj.rotation_x[2],obj.rotation_x[3]);	
    

	
    updateMatrices();
    modelViewMatrix.PopMatrix();
}

char* mystrsep(char** stringp, const char* delim)
{
  char* start = *stringp;
  char* p;

  p = (start != NULL) ? strpbrk(start, delim) : NULL;

  if (p == NULL)
  {
    *stringp = NULL;
  }
  else
  {
    *p = '\0';
    *stringp = p + 1;
  }

  return start;

}
Rotation parseDataRecieved(string data) {
	char x,y,z;
	string currentWord =  "";
	Rotation rotn;

	int counter = 0;
	for(int i = 0; data[i]!='\0' ; i++) {
		if(data[i] != '\n')
			currentWord.push_back(data[i]);
		else {
			float temp = ::atof(currentWord.c_str());
			if(counter == 0) {
				rotn.yaw = temp;
				break;
			}
			else if(counter == 1) {
				rotn.pitch = temp;
			}
			else if(counter == 2) {
				rotn.roll = temp;
			}
			else {
				break;
			}
			counter++;
			currentWord = "";
		}
	}
	
	return rotn;
}
float rotnMatrix[16];
float transpose[16];
void parseRotnMatrix(string data) {
	string currentWord =  "";
	Rotation rotnrender;

	int counter = 0;
	int i = 0;
	// while start of matrix not got 
	while(data[i] != 'S') {
		i++;
		if(data[i] == '\0')
			break;
	}
	i++;

	for(;data[i]!='X' ; i++) {

		if(data[i] == '\0')
			break;

		if(data[i] != '\n')
			currentWord.push_back(data[i]);
		else {
			float temp = ::atof(currentWord.c_str());
			rotnMatrix[counter] = temp;
			counter++;
			currentWord = "";
		}
	}
	

}

float mod(float num) {
	if(num < 0) 
		return num*-1;
	else return num;
}
Rotation currentRotn;
FILE *fp = fopen("d:\\rotatationMatrix.txt", "r+");
void render()
{
	float yaw, roll, pitch;
	char *token = " ";
	
	msg_length = recv(connectSocket, dataRecieved, 2048, 0);
	if (dataRecieved[0] != NULL) {
		printf("%s\n", dataRecieved);
	 	parseRotnMatrix(dataRecieved);
	}
	//fprintf(fp, "%s\n", "=============================");
	
	Sleep(100);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glRotated(-90.0,1.0,0.0,0.0); 

    // This puts the view matrix (camera matrix) into the stack
    M3DMatrix44f  mViewMatrix;     // the view matrix
    worldFrame.GetCameraMatrix(mViewMatrix);    
    modelViewMatrix.PushMatrix();
    modelViewMatrix.MultMatrix(mViewMatrix);
	
	
    m3dTransformVector3(viewSpaceLightPos, lightPosition, mViewMatrix);

    glUseProgram(shader);
	
    // the light
    updateMatrices();
	
	
    //the herons
    object heron;
	modelViewMatrix.MultMatrix(rotnMatrix);
    heron.noShine=0;	
    heron.scale[0]=120;
    heron.scale[1]=120;
    heron.scale[2]=120;
	//heron.rotation_y[0] = 180;
	//heron.rotation_z[0] = 90;
//heron.rotation_z[0] = currentRotn.yaw;
//	heron.rotation_y[0] = currentRotn.pitch + 90;
//	heron.rotation_x[0] = currentRotn.roll ;

    setUniforms(heron);

    bindDraw(&heron_batch,Tex_heron);
    
    //end of herons
    // pops out the view matrix
    modelViewMatrix.PopMatrix();

    glutSwapBuffers();
    glutPostRedisplay();
}

int time=0;
float rotn=0.01,forw=0;
void keys(int key,int x,int y)
{

    worldFrame.RotateWorld(+0.3,1,0,0);	
    if(key==GLUT_KEY_UP)
    {
      turn_x=turn_x-inc;
    }


    if(key==GLUT_KEY_DOWN)
    {
		turn_x=turn_x+inc;
    }
    worldFrame.RotateWorld(-0.3,1,0,0);	


    if(key==GLUT_KEY_RIGHT)
    {
       turn_y=turn_y+inc;
    }
    if(key==GLUT_KEY_LEFT)
    {   		
		turn_y=turn_y-inc;
    }

}



void keyPressed (unsigned char key, int x, int y)
{
  

}

void keyReleased(unsigned char key, int x, int y)
{


}
void dispError(char *err)
{
	cout<<err;
}

void init()
{

	WSADATA wsadata;

	int iResult = WSAStartup (MAKEWORD(2,2), &wsadata );
	if (iResult !=NO_ERROR )
		dispError("\nerror at WSAStartup()\n");

	if((listenSocket= socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET)	//create a socket for listening
		dispError("Error Creating Socket for listening");

	//set the address structure
	
	serverAddr.sin_family = AF_INET;					// TCP/IP v4 protocol family
	serverAddr.sin_port = htons(MY_PORT_NUMBER);		// port address		
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);		// ANY Address


	// bind the newly created socket with server's address
	if(bind(listenSocket,(struct sockaddr *) &serverAddr,sizeof(serverAddr))==SOCKET_ERROR)
		dispError("Error in binding");

	// Make the socket listenSocket listen for connections
	if(listen(listenSocket,SOMAXCONN)==SOCKET_ERROR)
		dispError("Error in Listening");

	clientLength=sizeof(sockaddr_in);
	cout<<"Waiting for friends";

}

void main(int argc,char **argv)
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(len,wide);
    glutInitWindowPosition(0,0);
    glutCreateWindow("table");
    glutDisplayFunc(render);
    glutSpecialFunc(keys);
    glutKeyboardFunc(keyPressed);
    glutKeyboardUpFunc(keyReleased);

	GLenum err=glewInit();
    if(GLEW_OK!=  err)
    {
        fprintf(stderr,"glew errot: %s\n",glewGetErrorString(err));
    }
	init();
	//accept a connection (creates a new socket file descriptor)
	if ((connectSocket = accept(listenSocket, (struct sockaddr *) &clientAddr, &clientLength)) == SOCKET_ERROR)
	{
		dispError("error accepting connection");
	}
	else
		cout << "\n\nyou are connected to a friend\n";

	// say hello to friend
	send(connectSocket, "hello", 2048, 0);


    setup();
    glutMainLoop();
	closesocket(connectSocket);
	
}
