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



void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

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
}framebuffer;

struct FullscreenQuad
{
	GLuint vao;
	GLuint vbo;
}fullscreenQuad;

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//cache
ew::Camera newCamera;
ew::Transform suzanneTransform;
ew::CameraController cameraController;
static float quad_vertices[] = {
	// pos (x, y) texcoord (u, v)
	-1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f,  1.0f, 1.0f, 1.0f,
};
static int effectIndex = 0;
static std::vector<std::string> post_processing_effects = {
	"None",
	"Greyscale",
	"Kernel Blur",
	"Inverse",
	"Chromatic Aberration",
	"Gamma Correction",
	"Edge Detections",
};

void Render(ew::Shader& shader, ew::Model& model, GLuint texture, float deltaTime)
{
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	//Pipeline definition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, texture);

	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Pipeline definition
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, texture);

	

	shader.use();
	suzanneTransform.rotation = glm::rotate(suzanneTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
	shader.setMat4("transform_model", suzanneTransform.modelMatrix());
	shader.setMat4("camera_viewproj", newCamera.projectionMatrix() * newCamera.viewMatrix());
	shader.setVec3("_eyePos", newCamera.position);
	shader.setInt("_MainTexture", 0);
	shader.setFloat("_Material.ambientK", material.ambientK);
	shader.setFloat("_Material.specularK", material.specularK);
	shader.setFloat("_Material.diffuseK", material.diffuseK);
	shader.setFloat("_Material.shininess", material.shininess);
	model.draw(); //Draws monkey model using current shader

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
}

int main() {

	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//initialize resources
	ew::Shader newShade = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader fullscreen = ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag");
	ew::Shader inverse = ew::Shader("assets/inverse.vert", "assets/inverse.frag");
	ew::Shader greyscale = ew::Shader("assets/greyscale.vert", "assets/greyscale.frag");
	ew::Shader blur = ew::Shader("assets/blur.vert", "assets/blur.frag");
	ew::Shader chromatic = ew::Shader("assets/chromatic.vert", "assets/chromatic.frag");
	ew::Shader gamma = ew::Shader("assets/gamma.vert", "assets/gamma.frag");
	ew::Shader edge = ew::Shader("assets/edgeDetect.vert", "assets/edgeDetect.frag");
	ew::Model suzanne = ew::Model("assets/Suzanne.obj");
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	//initialize camera
	newCamera.position = { 0.0f, 0.0f, 5.0f };
	newCamera.target = { 0.0f, 0.0f, 0.0f };
	newCamera.aspectRatio = (float)screenWidth / screenHeight;
	newCamera.fov = 60.0f;

	//initialize fullscreen quad
	glGenVertexArrays(1, &fullscreenQuad.vao);
	glGenBuffers(1, &fullscreenQuad.vbo);
	glBindVertexArray(fullscreenQuad.vao);

	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuad.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0); // positions
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
	glEnableVertexAttribArray(1); // texcoords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

	glBindVertexArray(0);

	//initialize framebuffer
	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	//color attachment
	glGenTextures(1, &framebuffer.color0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

	//Depth buffer attachment
	glGenTextures(1, &framebuffer.depth);
	glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

	//check completeness
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("BEEP FAIL");
		return 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		Render(newShade, suzanne, brickTexture, deltaTime);
		cameraController.move(window, &newCamera, deltaTime);
		
		//fullscreen quad pipeline
		glDisable(GL_DEPTH_TEST);

		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Render quad
		switch (effectIndex)
		{
			case 1:
				greyscale.use();
				greyscale.setInt("texture0", 0);
				break;
			case 2:
				blur.use();
				blur.setInt("texture0", 0);
				break;
			case 3:
				inverse.use();
				inverse.setInt("texture0", 0);
				break;
			case 4:
				chromatic.use();
				chromatic.setInt("texture0", 0);
				break;
			case 5:
				gamma.use();
				gamma.setInt("texture0", 0);
				break;
			case 6:
				edge.use();
				edge.setInt("texture0", 0);
				break;
			default:
				fullscreen.use();
				fullscreen.setInt("texture0", 0);
				break;
		}
		glBindVertexArray(fullscreenQuad.vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,framebuffer.color0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		
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

	if (ImGui::BeginCombo("Effect", post_processing_effects[effectIndex].c_str()))
	{
		for (auto n = 0; n < post_processing_effects.size(); ++n)
		{
			auto is_selected = (post_processing_effects[effectIndex] == post_processing_effects[n]);
			if (ImGui::Selectable(post_processing_effects[n].c_str(), is_selected))
			{
				effectIndex = n;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Begin("OpenGL Texture Test");
	ImGui::Text("pointer = %x", framebuffer.color0);
	ImGui::Text("size = %d x %d", 800, 600);
	ImGui::Image((ImTextureID)(intptr_t)framebuffer.color0, ImVec2(800, 600));
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

