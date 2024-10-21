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

	// このクラスがコピーされるのを防ぐ
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;

	void Mainloop();
	// ウィンドウメッセージからの入力を処理する
	void HandleKeyInput(WPARAM wParam);
	void HandleMouseInput(UINT msg);

	Window window_ = {};
	Renderer renderer_ = {};
	//InputManager inputManager_;
};