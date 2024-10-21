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
private:

	// ���̃N���X���R�s�[�����̂�h��
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;

	void Mainloop();
	// �E�B���h�E���b�Z�[�W����̓��͂���������
	void HandleKeyInput(WPARAM wParam);
	void HandleMouseInput(UINT msg);

	Window window_ = {};
	Renderer renderer_ = {};
	//InputManager inputManager_;
};