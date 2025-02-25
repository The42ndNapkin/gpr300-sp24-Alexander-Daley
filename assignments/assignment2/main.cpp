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


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

static float quad_vertices[] = {
	// pos (x, y) texcoord (u, v)
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f,  1.0f, 1.0f, 1.0f,
};

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
	GLuint depth;

	void initialize()
	{
		//initialize framebuffer
		glGenFramebuffers(1, &framebuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

		//color attachment
		glGenTextures(1, &framebuffer.color0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

		//Depth buffer attachment
		glGenTextures(1, &framebuffer.depth);
		glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

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
		//initialize fullscreen quad
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // positions
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
		glEnableVertexAttribArray(1); // texcoords
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

	}
};

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//cache
ew::Camera newCamera;
ew::Transform suzanneTransform;
ew::CameraController cameraController;
float bias = 0.005;
float suzanneX = 0.0f;
float suzanneY = 0.0f;
float suzanneZ = 0.0f;
float scaleX = 1.0f;
float scaleY = 1.0f;
float scaleZ = 1.0f;
float lightX = 0.0f;
float lightY = 3.0f;
float lightZ = 0.0f;

void Render(ew::Shader& shader, ew::Model& model, GLuint texture, float deltaTime, ew::Mesh plane, ew::Shader& shadowPass)
{
	const auto lightProj = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.1f,100.0f);
	const auto lightView = glm::lookAt(glm::vec3(-2.0f,4.0f,-1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const auto lightViewProj = lightProj * lightView;

	suzanneTransform.rotation = glm::rotate(suzanneTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
	suzanneTransform.position = glm::vec3(suzanneX, suzanneY, suzanneZ);
	suzanneTransform.scale = glm::vec3(scaleX,scaleY,scaleZ);

	glBindFramebuffer(GL_FRAMEBUFFER, depthbuffer.fbo);
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, 1024, 1024);

		//begin pass
		glClear(GL_DEPTH_BUFFER_BIT);
		shadowPass.use();
		shadowPass.setMat4("model", suzanneTransform.modelMatrix());
		shadowPass.setMat4("light_viewproj", lightViewProj);

		model.draw();
		glCullFace(GL_BACK);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Render lighting
	glViewport(0, 0, screenWidth, screenHeight);

	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);


	shader.use();
	
	shader.setMat4("transform_model", suzanneTransform.modelMatrix());
	shader.setMat4("camera_viewproj", newCamera.projectionMatrix() * newCamera.viewMatrix());
	shader.setMat4("light_viewproj", lightViewProj);

	shader.setVec3("light.position", glm::vec3(lightX,lightY,lightZ));
	shader.setVec3("light.color", glm::vec3(1.0f,1.0f,1.0f));
	shader.setInt("shadowMap", 0);
	shader.setVec3("cameraPosition", newCamera.position);
	shader.setFloat("material.ambient", material.ambientK);
	shader.setFloat("material.specular", material.specularK);
	shader.setFloat("material.diffuse", material.diffuseK);
	shader.setFloat("material.shininess", material.shininess);
	shader.setFloat("bias", bias);
	model.draw(); //Draws suzanne model using current shader

	shader.setMat4("transform_model", glm::translate(glm::vec3(0.0,-2.0,0.0)));
	plane.draw(); //Draws plane

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

int main() {

	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//initialize resources
	ew::Shader blinnPhong = ew::Shader("assets/blinnphong.vert", "assets/blinnphong.frag");
	ew::Shader shadowPass = ew::Shader("assets/shadowPass.vert", "assets/shadowPass.frag");

	ew::Model suzanne = ew::Model("assets/Suzanne.obj");
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	ew::Mesh plane;
	plane.load(ew::createPlane(50, 50, 1));

	//initialize camera
	newCamera.position = { 0.0f, 0.0f, 5.0f };
	newCamera.target = { 0.0f, 0.0f, 0.0f };
	newCamera.aspectRatio = (float)screenWidth / screenHeight;
	newCamera.fov = 60.0f;

	//framebuffer.initialize();
	depthbuffer.initialize();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		Render(blinnPhong, suzanne, brickTexture, deltaTime, plane, shadowPass);
		cameraController.move(window, &newCamera, deltaTime);

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
		ImGui::SliderFloat("Suzanne X", &suzanneX, -10.0f, 10.0f);
		ImGui::SliderFloat("Suzanne Y", &suzanneY, -10.0f, 10.0f);
		ImGui::SliderFloat("Suzanne Z", &suzanneZ, -10.0f, 10.0f);
		ImGui::SliderFloat("Scale X", &scaleX, 0.0f, 5.0f);
		ImGui::SliderFloat("Scale Y", &scaleY, 0.0f, 5.0f);
		ImGui::SliderFloat("Scale Z", &scaleZ, 0.0f, 5.0f);
	}
	//Light properties
	if (ImGui::CollapsingHeader("Directional Light"))
	{
		ImGui::SliderFloat("Light X", &lightX, -10.0f, 10.0f);
		ImGui::SliderFloat("Light Y", &lightY, -10.0f, 10.0f);
		ImGui::SliderFloat("Light Z", &lightZ, -10.0f, 10.0f);
	}
	ImGui::SliderFloat("Shadow Bias", &bias, 0.002f, 0.01f);

	ImGui::Begin("OpenGL Texture Test");
	ImGui::Image((ImTextureID)(intptr_t)depthbuffer.depth, ImVec2(800, 600));
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

