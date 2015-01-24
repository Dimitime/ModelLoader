#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/glm.hpp>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define NUMBER_MODELS 3

//OGL Buffer objects
GLuint phongshader,flatshader;
GLuint vao;
GLuint vbo[2*NUMBER_MODELS];

std::vector<glm::vec3> vertices[NUMBER_MODELS];
std::vector<glm::vec3> normals[NUMBER_MODELS];
std::vector<GLushort> faces[NUMBER_MODELS];

glm::mat4 Projection = glm::ortho(-0.35,0.35,-0.35,0.35);
glm::mat4 View = glm::lookAt(glm::vec3(0.0f,0.0f,1.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
glm::mat4 Model[4] = { glm::mat4(1.f), glm::mat4(1.f),  glm::mat4(1.f),  glm::mat4(1.f) };

glm::vec3 center(std::vector<glm::vec3> pos);
static void disp(void) {
	glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (size_t i=0; i<NUMBER_MODELS; i++) {
		//Vertex positions
		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[NUMBER_MODELS+i]);	
		//Vertex normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
		glEnableVertexAttribArray(1);

		GLuint m,v,p;
		//Draw the models
		if (i==0) {
			glUseProgram(phongshader);
			m = glGetUniformLocation(phongshader, "model");
			v = glGetUniformLocation(phongshader, "view");
			p = glGetUniformLocation(phongshader, "projection");
		}
		else {
			glUseProgram(flatshader);
			m = glGetUniformLocation(flatshader, "model");
			v = glGetUniformLocation(flatshader, "view");
			p = glGetUniformLocation(flatshader, "projection");
		}

		glUniformMatrix4fv(m, 1, GL_FALSE, &Model[i][0][0]);
		glUniformMatrix4fv(v, 1, GL_FALSE, &View[0][0]);
		glUniformMatrix4fv(p, 1, GL_FALSE, &Projection[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());
	}
	//unbind everything
	//glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glutSwapBuffers();
}

static void keyboard(unsigned char key, int x, int y) {
    switch (key) {
		case 27:
			exit(0);
    }
}

glm::vec3 center(std::vector<glm::vec3> pos) {
	float maxx=pos[0].x;
	float minx=pos[0].x;
	float maxy=pos[0].y;
	float miny=pos[0].y;
	float maxz=pos[0].z;
	float minz=pos[0].z;

	for (size_t i=0; i<pos.size(); i++) {
		if (pos[i].x > maxx)
			maxx = pos[i].x;
		if (pos[i].x < minx)
			minx = pos[i].x;
		if (pos[i].y > maxy)
			maxy = pos[i].y;
		if (pos[i].y < miny)
			miny = pos[i].y;
		if (pos[i].z > maxz)
			maxz = pos[i].z;
		if (pos[i].z < minz)
			minz = pos[i].z;
	}

	glm::vec3 ret(0.0f);
	ret.x = (maxx + minx) / 2.0f;
	ret.y = (maxy + miny) / 2.0f;
	ret.z = (maxz + minz) / 2.0f;

	std::cout << "Center: " << ret.x << "," << ret.y << "," << ret.z << std::endl;

	return ret;
}

void loadObj(const char* filename, std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<GLushort> &faces) {
	std::ifstream in(filename, std::ios::in);
	if (!in)
		std::cerr << "Cannot open " << filename << std::endl;
 
	//Temporary containers
	std::vector<glm::vec3> tv;
	std::vector<glm::vec3> tn;

	//an array of indices
	std::vector<int> vi;
	std::vector<int> ni;

	std::string line;
	while (std::getline(in, line)) {
		//We have vertex data
		if (line.substr(0,2) == "v ") {
			std::istringstream s(line.substr(2));
			glm::vec3 v; s >> v.x; s >> v.y; s >> v.z;
			tv.push_back(v);
		}
		//We have vertex normals
		else if (line.substr(0,2) == "vn") {
			std::istringstream s(line.substr(2));
			glm::vec3 n; s >> n.x; s >> n.y; s >> n.z;
			tn.push_back(n);
		}
		//Face info. A little hacky, I'm assuming no texture coordinates.
		else if (line.substr(0,2) == "f ") {
			std::istringstream s(line.substr(2));
			//std::cout << s.str() << std::endl;

			GLushort v1,v2,v3,n1,n2,n3;
			char s1;
			//format is v1//n1 v2//n2 v3//n3
			s>>v1; s>>s1; s>>s1; s>>n1; 
			s>>v2; s>>s1; s>>s1; s>>n2;
			s>>v3; s>>s1; s>>s1; s>>n3;
			
			faces.push_back(v1);
			vi.push_back(v1); vi.push_back(v2); vi.push_back(v3);
			ni.push_back(n1); ni.push_back(n2); ni.push_back(n3);
		}
	}

	//Now we rearrange the vertex and normal arrays based on the indices found in the faces
	for (size_t i=0; i<vi.size(); i++) {
		 glm::vec3 v = tv[vi[i]-1];
		 vertices.push_back(v);
		 glm::vec3 n = tn[ni[i]-1];
		 normals.push_back(n);
	}
}

//Loading shelders
GLuint loadShaders(const char * vertex_file_path, const char * fragment_file_path) {
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(std::getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
 
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(std::getline(FragmentShaderStream, Line))
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
    std::vector<char> ProgramErrorMessage( glm::max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
 
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

//initialize everything
static void init(void) {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	//Create the VAO	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Load the shaders
	phongshader = loadShaders("Shaders/phong.vertexshader", "Shaders/phong.fragmentshader");
	flatshader = loadShaders("Shaders/fphong.vertexshader", "Shaders/fphong.fragmentshader");

	//Load the models
	loadObj("Meshes/me100k.obj",vertices[0],normals[0],faces[0]);
	std::cout << "First mesh loaded" << std::endl;
	loadObj("Meshes/me1k.obj"  ,vertices[1],normals[1],faces[1]);
	std::cout << "Second mesh loaded" << std::endl;
	loadObj("Meshes/me100.obj" ,vertices[2],normals[2],faces[2]);
	std::cout << "Third mesh loaded" << std::endl;
	//loadObj("Meshes/me10.obj"  ,vertices[3],normals[3],faces[3]);
	std::cout << "Fourth mesh loaded" << std::endl;
	
	Model[0] = glm::scale(Model[0], glm::vec3(1.5f,1.5f,1.5f));

	Model[0] = glm::translate(Model[0], glm::vec3(0.0f,0.13f,0.0f));
	Model[1] = glm::translate(Model[1], glm::vec3(-0.2f,-0.13f,0.0f));
	Model[2] = glm::translate(Model[2], glm::vec3(0.2f,-0.13f,0.0f));
	//Model[3] = glm::translate(Model[3], glm::vec3(0.2f,-0.15f, 0.0f));

	//Orient the meshes properly
	for (size_t i=0; i<NUMBER_MODELS; i++) {
		Model[i] = glm::translate(Model[i], center(vertices[i]));
		Model[i] = glm::rotate(Model[i], 1.45f, glm::vec3(-1.0f, 0.0f, 0.0f));
		Model[i] = glm::rotate(Model[i], 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
		Model[i] = glm::translate(Model[i], -center(vertices[i]));
	}
	//Create vertex buffer object
	glGenBuffers(2*NUMBER_MODELS, vbo);

	for (size_t i=0; i<NUMBER_MODELS; i++) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
	
		//Initialize the vbo to the initial positions
		glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(glm::vec3), &vertices[i][0], GL_STREAM_DRAW);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[NUMBER_MODELS+i]);
		glBufferData(GL_ARRAY_BUFFER, normals[i].size() * sizeof(glm::vec3), &normals[i][0], GL_STREAM_DRAW);
		glEnableVertexAttribArray(1);
	}

	std::cout <<"Buffer setup complete" << std::endl;
}

int main(int argc, char** argv) {
	// init glut:
	glutInit (&argc, argv);
    //glutInitContextVersion(3,3);
    //glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB| GLUT_MULTISAMPLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0,0);
	glutCreateWindow("SPH");	
	//glutFullScreen();
    //glutGameModeString("1280x720:16@60"); glutEnterGameMode();
	printf("OpenGL Version:%s\n",glGetString(GL_VERSION));
	printf("GLSL Version  :%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));

	glutDisplayFunc(&disp);
    //glutIdleFunc(&idle);
    glutKeyboardFunc(&keyboard);

	glewExperimental=GL_TRUE;
	GLenum err=glewInit();
	if(err!=GLEW_OK)
		printf("glewInit failed, aborting.\n");
    if (!glewIsSupported("GL_VERSION_3_3 ")) {
        fprintf(stderr, "ERROR: Support for necessary OpenGL extensions missing.");
        fflush(stderr);
		exit(0);
    }

	init();

	// enter tha main loop and process events:
	glutMainLoop();
	return 0;
}