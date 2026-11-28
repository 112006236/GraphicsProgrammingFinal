#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <exception>
#include <cmath>

Indoor room;
bool showPanel=true;
void on_gui(){
    if(!showPanel)return;
    ImGui::SetNextWindowSize(ImVec2(320,0),ImGuiCond_FirstUseEver);
    ImGui::Begin("Indoor architecture | Lighting studio", &showPanel);
    ImGui::TextWrapped("GPU: %s", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    ImGui::Text("%.1f FPS | %.2f ms", ImGui::GetIO().Framerate,1000.f/std::max(1.f,ImGui::GetIO().Framerate));
    ImGui::TextWrapped("WASD move / Q E height / right drag look / R reset / Tab panel");
    if(ImGui::Button("Room view")){room.eye={4,1,-1.5f};room.yaw=180;room.pitch=0;room.lookDistance=1;}
    ImGui::SameLine();
    if(ImGui::Button("Table detail")){room.eye={2.52f,1.00f,-1.38f};room.yaw=188;room.pitch=-19;}
    glm::vec3 center=room.eye+room.forward()*room.lookDistance;
    bool eyeChanged=ImGui::DragFloat3("Camera eye",&room.eye.x,.02f);
    bool centerChanged=ImGui::DragFloat3("Look-at center",&center.x,.02f);
    if(eyeChanged||centerChanged){auto d=center-room.eye;float len=glm::length(d);if(len>.001f){room.lookDistance=len;d/=len;room.yaw=glm::degrees(std::atan2(d.z,d.x));room.pitch=glm::degrees(std::asin(d.y));}}
    if(ImGui::CollapsingHeader("Basic techniques",ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::Checkbox("Blinn-Phong shading", &room.shading);
        ImGui::Checkbox("Directional light", &room.directional);
        ImGui::Checkbox("Directional shadow", &room.dirShadow);
        ImGui::TextDisabled("Deferred shading: G-buffer active");
        ImGui::Checkbox("Normal mapping", &room.normal);
        ImGui::Checkbox("Bloom", &room.bloom);
    }
    if(ImGui::CollapsingHeader("Advanced techniques",ImGuiTreeNodeFlags_DefaultOpen)){
        ImGui::Checkbox("SSAO", &room.ao); ImGui::Checkbox("Point light", &room.point);
        ImGui::Checkbox("Real-time point shadow", &room.pointShadow);
        ImGui::Checkbox("Screen space reflection", &room.ssr);
        ImGui::Checkbox("Non-photorealistic rendering", &room.npr);
        ImGui::Checkbox("FXAA", &room.fxaa); ImGui::Checkbox("Area light", &room.area);
        ImGui::Checkbox("Volumetric light", &room.volume);
        ImGui::Checkbox("Show area emitter", &room.areaPanel);
    }
    if(ImGui::CollapsingHeader("Lighting controls")){
        ImGui::SliderFloat("Exposure",&room.exposure,.4f,2.f);
        ImGui::SliderFloat("Point intensity",&room.pointPower,0.f,12.f);
        ImGui::DragFloat3("Point position",&room.light.x,.02f);
        ImGui::DragFloat3("Directional eye",&room.sunEye.x,.02f);
        ImGui::DragFloat3("Volumetric position",&room.volumePos.x,.1f);
        ImGui::SliderFloat("Directional intensity",&room.sunPower,0.f,3.f);
        ImGui::SliderFloat("Area intensity",&room.areaPower,0.f,8.f);
        ImGui::DragFloat3("Area position",&room.areaCenter.x,.02f);
        ImGui::SliderFloat("Area yaw",&room.areaYaw,-180,180);
        ImGui::SliderFloat("Area pitch",&room.areaPitch,-85,85);
        ImGui::SliderFloat("Scattering density",&room.fog,0.f,1.5f);
    }
    const char* views[]={"Final image","World position","World normal","Ambient","Diffuse","Specular","Ambient occlusion","Directional visibility","Point visibility"};
    ImGui::Combo("View",&room.debug,views,9);
    ImGui::End();
}
void input(GLFWwindow* window,float dt){
    auto& io=ImGui::GetIO();static bool previousTab=false,drag=false;static double lastX=0,lastY=0;
    bool tab=glfwGetKey(window,GLFW_KEY_TAB)==GLFW_PRESS;if(tab&&!previousTab)showPanel=!showPanel;previousTab=tab;
    if(!io.WantCaptureKeyboard){glm::vec3 f=room.forward();f.y=0;f=glm::normalize(f);glm::vec3 r=glm::normalize(glm::cross(f,glm::vec3(0,1,0)));float speed=1.5f*dt;
        if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS)room.eye+=f*speed;
        if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS)room.eye-=f*speed;
        if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)room.eye+=r*speed;
        if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS)room.eye-=r*speed;
        if(glfwGetKey(window,GLFW_KEY_Q)==GLFW_PRESS)room.eye.y-=speed;
        if(glfwGetKey(window,GLFW_KEY_E)==GLFW_PRESS)room.eye.y+=speed;
        if(glfwGetKey(window,GLFW_KEY_R)==GLFW_PRESS){room.eye={4,1,-1.5f};room.yaw=180;room.pitch=0;room.lookDistance=1;}
        
    }
    double x,y;glfwGetCursorPos(window,&x,&y);bool down=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS&&!io.WantCaptureMouse;
    if(down&&drag){room.yaw+=float(x-lastX)*.16f;room.pitch=std::clamp(room.pitch-float(y-lastY)*.16f,-85.f,85.f);}drag=down;lastX=x;lastY=y;
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	const char* glsl_version = "#version 410";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	// Create window with graphics context
	float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "GPA 2026 | Indoor architecture", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		return -1;
	}
	glfwSwapInterval(0);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	try { room.init(); } catch(const std::exception& e) { std::cerr << e.what() << std::endl; glfwTerminate(); return 1; }
	double lastTime=glfwGetTime();
	// Main loop
	while (!glfwWindowShouldClose(window))

	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		double now=glfwGetTime();
        input(window,float(std::min(now-lastTime,.1))); lastTime=now;
        on_gui();
        int rw,rh; glfwGetFramebufferSize(window,&rw,&rh);
        try { room.draw(rw,rh); } catch(const std::exception& e) { std::cerr << e.what() << std::endl; break; }
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	room.release();
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}