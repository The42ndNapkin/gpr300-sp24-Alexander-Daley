#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <ew/transform.h>
#include <ew/procGen.h>
#include <ew/mesh.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();


int screenWidth = 1080;
int screenHeight = 720;
struct Material {
	float diffuseK = 1; //diffuse light coeficcient (0-1)
	float specularK = 0.5; //Specular light coeficcient (0-1)
	float ambientK = 0.5; //Ambient light coeficcient (0-1)
	float shininess = 128;  //Size of specular highlight
}material;

struct Framebuffer {
	GLuint fbo;
	GLuint color0;
	GLuint color1;
	GLuint color2;
	GLuint depth;
	
	void initialize()
	{
		//initialize framebuffer
		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		//color attachment
		glGenTextures(1, &framebuffer.color0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

		//color attachment
		glGenTextures(1, &framebuffer.color1);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer.color1, 0);

		//color attachment
		glGenTextures(1, &framebuffer.color2);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, framebuffer.color2, 0);

		//Depth buffer attachment
		glGenTextures(1, &framebuffer.depth);
		glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

		GLuint array[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, array);

		//check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("BEEP FAIL");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}framebuffer;

struct Depthbuffer {
	GLuint fbo;
	GLuint depth;

	void initialize() {
		//initialize framebuffer
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		//Depth buffer attachment
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		//check completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("BEEP FAIL");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}depthbuffer;

struct FullscreenQuad
{
	GLuint vao;
	GLuint vbo;

	void initialize()
	{

		float quad_vertices[] = {
			// pos (x, y) texcoord (u, v)
			-1.0f,  1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
				1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f,  1.0f, 0.0f, 1.0f,
				1.0f, -1.0f, 1.0f, 0.0f,
				1.0f,  1.0f, 1.0f, 1.0f,
		};

		//initialize fullscreen quad
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // positions
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1); // texcoords
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

		glBindVertexArray(0);

	}
}fullscreenQuad;

struct PointLight
{
	glm::vec3 position;
	float radius;
	glm::vec4 color;

	void init(glm::vec3 pos, float r, glm::vec4 c)
	{
		position = pos;
		r = radius;
		color = c;
	}
};
const int MAX_POINT_LIGHTS = 64;
PointLight pLights[MAX_POINT_LIGHTS];

//Global state

float prevFrameTime;
float deltaTime;

//cache
ew::Camera newCamera;
ew::Transform suzanneTransform;
ew::CameraController cameraController;
//Variables for imgui modification
float suzanneX = 0.0f;
float suzanneY = 0.0f;
float suzanneZ = 0.0f;
float scaleX = 1.0f;
float scaleY = 1.0f;
float scaleZ = 1.0f;
float lightX = 0.0f;
float lightY = 3.0f;
float lightZ = 0.0f;
float lightR = 1.0f;
float lightG = 1.0f;
float lightB = 1.0f;
int numSuzannes = 8;

void initLights()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			float r = (rand() % 256) / 255.0f;
			float g = (rand() % 256) / 255.0f;
			float b = (rand() % 256) / 255.0f;
			pLights[i * 8 + j] = PointLight();
			pLights[i * 8 + j].init(glm::vec3(i * 2.0f, 5.0f, j * 2.0f), 1.0f, glm::vec4(r, g, b, 1.0f));
		}
	}
}

void postProcess(ew::Shader shader)
{
	shader.use();
	shader.setInt("gAlbedo", 0);
	shader.setInt("gPosition", 1);
	shader.setInt("gNormal", 2);
	shader.setVec3("cameraPosition", newCamera.position);
	shader.setVec3("lightPos", glm::vec3(lightX, lightY, lightZ));
	shader.setVec3("lightColor", glm::vec3(lightR, lightG, lightB));
	shader.setFloat("mDiffuse", material.diffuseK);
	shader.setFloat("mShininess", material.shininess);
	shader.setFloat("mSpecular", material.specularK);
	for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
		std::string prefix = "_PointLights[" + std::to_string(i) + "].";
		shader.setVec3(prefix + "position", pLights[i].position);
		shader.setFloat(prefix + "radius", pLights[i].radius);
		shader.setVec4(prefix + "color", pLights[i].color);
	}

	glDisable(GL_DEPTH_TEST);

	//Clear defualt buffer
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, screenWidth, screenHeight);

	glBindVertexArray(fullscreenQuad.vao);

	//draw fullscreen quad
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color2);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Render(ew::Shader& shader, ew::Model& model, GLuint texture, float deltaTime)
{
	suzanneTransform.position = glm::vec3(suzanneX, suzanneY, suzanneZ);
	suzanneTransform.scale = glm::vec3(scaleX,scaleY,scaleZ);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	glViewport(0, 0, screenWidth, screenHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	//Render suzannes
	shader.use();
	shader.setMat4("camera_viewproj", newCamera.projectionMatrix() * newCamera.viewMatrix());
	shader.setInt("_MainTexture", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	for (int i = 0; i < numSuzannes; i++)
	{
		for (int k = 0; k < numSuzannes; k++)
		{
			shader.setMat4("transform_model", glm::translate(glm::vec3(i * 2.0f, 0, k * 2.0f)));
			model.draw(); //Draws suzanne model using current shader
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderPointLights(ew::Shader shader, ew::Mesh sphere)
{
    //Blit gBuffer depth to same framebuffer as fullscreen quad
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo); //Read from gBuffer 
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); //Write to current fbo
	glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	//Draw all light orbs
	shader.use();
	shader.setMat4("_ViewProjection", newCamera.projectionMatrix() * newCamera.viewMatrix());
	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		glm::mat4 m = glm::mat4(1.0f);
		m = glm::translate(m, pLights[i].position);
		m = glm::scale(m, glm::vec3(1.0f));

		shader.setMat4("_Model", m);
		shader.setVec3("_Color", pLights[i].color);
		sphere.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {

	GLFWwindow* window = initWindow("Assignment 3", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//initialize resources
	ew::Shader blinnPhong = ew::Shader("assets/blinnphong.vert", "assets/blinnphong.frag");
	ew::Shader geoShader = ew::Shader("assets/geoShader.vert", "assets/geoShader.frag");
	ew::Shader pointLights = ew::Shader("assets/lights.vert", "assets/lights.frag");
	ew::Model suzanne = ew::Model("assets/Suzanne.obj");
	ew::Mesh sphereMesh = ew::Mesh(ew::createSphere(1.0f, 8));
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	//initialize camera
	newCamera.position = { 0.0f, 0.0f, 5.0f };
	newCamera.target = { 0.0f, 0.0f, 0.0f };
	newCamera.aspectRatio = (float)screenWidth / screenHeight;
	newCamera.fov = 60.0f;

	framebuffer.initialize();
	depthbuffer.initialize();
	fullscreenQuad.initialize();
	initLights();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &newCamera, deltaTime);

		//RENDER geo information
		Render(geoShader, suzanne, brickTexture, deltaTime);

		// RENDER light information
		postProcess(blinnPhong);

		// render all lights as spheres
		renderPointLights(pointLights, sphereMesh);

		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	//button to reset camera
	if (ImGui::Button("Reset Camera"))
	{
		resetCamera(&newCamera, &cameraController);
	}
	//Material properties
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.ambientK, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.diffuseK, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.specularK, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.shininess, 2.0f, 1024.0f);
	}
	//Suzanne properties
	if (ImGui::CollapsingHeader("Suzanne"))
	{
		ImGui::SliderInt("Suzannes", &numSuzannes, 0, 100);
		ImGui::SliderFloat("Suzanne X", &suzanneX, -5.0f, 5.0f);
		ImGui::SliderFloat("Suzanne Y", &suzanneY, -4.0f, 4.0f);
		ImGui::SliderFloat("Suzanne Z", &suzanneZ, -5.0f, 5.0f);
		ImGui::SliderFloat("Scale X", &scaleX, 0.0f, 5.0f);
		ImGui::SliderFloat("Scale Y", &scaleY, 0.0f, 5.0f);
		ImGui::SliderFloat("Scale Z", &scaleZ, 0.0f, 5.0f);
	}
	
	ImGui::Begin("OpenGL Texture Test");
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color1, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color2, ImVec2(800, 600));
	ImGui::End();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

