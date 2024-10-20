#pragma once

#include "Window.h"
#include "Renderer.h"
#include "InputManager.h"

class Application
{
public:
		Application();
		~Application();

		bool Initialize(HINSTANCE hInstance, int nShowCmd);
		void Run();
		Window window_ = {};
private:

	// ���̃N���X���R�s�[�����̂�h��
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;

	void Mainloop();

	//Renderer renderer_;
	//InputManager inputManager_;
};