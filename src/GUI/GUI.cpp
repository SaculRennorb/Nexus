#include "GUI.h"

namespace GUI
{
	void SetupWindowsAndKeybinds(); // forward declare

	bool					IsMenuVisible = true;
	bool					IsSetup = false;

	std::mutex				WindowsMutex;
	std::vector<IWindow*>	Windows;

	void Initialize()
	{
		// create imgui context
		if (!Renderer::GuiContext) { Renderer::GuiContext = ImGui::CreateContext(); }
		ImGuiIO& io = ImGui::GetIO();

		std::filesystem::path userFont = std::filesystem::path(Path::F_FONT);

		if (std::filesystem::exists(userFont))
		{
			io.Fonts->AddFontFromFileTTF(Path::F_FONT, 16.0f);
		}
		else
		{
			io.Fonts->AddFontDefault();
		}

		bool initializedFonts = false;
		HRSRC hResource = FindResourceA(AddonHostModule, MAKEINTRESOURCE(IDR_FONT1), RT_FONT);
		if (hResource)
		{
			HGLOBAL hLoadedResource = LoadResource(AddonHostModule, hResource);

			if (hLoadedResource)
			{
				LPVOID pLockedResource = LockResource(hLoadedResource);

				if (pLockedResource)
				{
					DWORD dwResourceSize = SizeofResource(AddonHostModule, hResource);

					if (0 != dwResourceSize)
					{
						io.Fonts->AddFontFromMemoryTTF(pLockedResource, dwResourceSize, 16.0f);
						io.Fonts->AddFontFromMemoryTTF(pLockedResource, dwResourceSize, 22.0f);
						initializedFonts = true;
					}
				}
			}
		}

		if (!initializedFonts)
		{
			io.Fonts->AddFontDefault();
			io.Fonts->AddFontDefault();
		}

		//io.Fonts->AddFontFromFileTTF(gw2uifont, 16.0f);
		//io.Fonts->AddFontFromFileTTF(gw2uifont, 22.0f);
		io.Fonts->Build();

		// Init imgui
		ImGui_ImplWin32_Init(Renderer::WindowHandle);
		ImGui_ImplDX11_Init(Renderer::Device, Renderer::DeviceContext);
		ImGui::GetIO().ImeWindowHandle = Renderer::WindowHandle;

		// create buffers
		ID3D11Texture2D* pBackBuffer;
		Renderer::SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		Renderer::Device->CreateRenderTargetView(pBackBuffer, NULL, &Renderer::RenderTargetView);
		pBackBuffer->Release();

		if (!IsSetup) { SetupWindowsAndKeybinds(); }

		State::IsImGuiInitialized = true;
	}

	void Shutdown()
	{
		if (State::IsImGuiInitialized)
		{
			State::IsImGuiInitialized = false;

			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			if (Renderer::RenderTargetView) { Renderer::RenderTargetView->Release(); Renderer::RenderTargetView = NULL; }
		}
	}

	bool WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (State::IsImGuiInitialized)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (io.WantCaptureMouse)
			{
				switch (uMsg)
				{
				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONDOWN:	if (!GetAsyncKeyState(VK_RBUTTON)) { io.MouseDown[0] = true; return true; }		break;
				case WM_RBUTTONDBLCLK:
				case WM_RBUTTONDOWN:	io.MouseDown[1] = true;															return true;

				case WM_LBUTTONUP:		io.MouseDown[0] = false;														break;
				case WM_RBUTTONUP:		io.MouseDown[1] = false;														break;

				case WM_MOUSEWHEEL:		io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;	return true;
				case WM_MOUSEHWHEEL:	io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;	return true;
				}
			}

			if (io.WantTextInput)
			{
				switch (uMsg)
				{
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					if (wParam < 256)
						io.KeysDown[wParam] = 1;
					return true;
				case WM_KEYUP:
				case WM_SYSKEYUP:
					if (wParam < 256)
						io.KeysDown[wParam] = 0;
					return true;
				case WM_CHAR:
					// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
					if (wParam > 0 && wParam < 0x10000)
						io.AddInputCharacterUTF16((unsigned short)wParam);
					return true;
				}
			}
		}

		return false;
	}

	void SetupWindowsAndKeybinds()
	{
		LogWindow* logWnd = new LogWindow();

		RegisterLogger(logWnd);
		logWnd->SetLogLevel(ELogLevel::ALL);

		AddWindow(logWnd);
		AddWindow(new AddonsWindow());
		AddWindow(new KeybindsWindow());
		AddWindow(new MumbleOverlay());
		AddWindow(new DebugWindow());
		AddWindow(new AboutBox());

		Keybinds::Register("ADDONHOST_MENU", ProcessKeybind, "CTRL+O");

		IsSetup = true;
	}

	void ProcessKeybind(std::string aIdentifier)
	{
		if (aIdentifier == "ADDONHOST_MENU")
		{
			IsMenuVisible = !IsMenuVisible;
			return;
		}

	}

	void Render()
	{
		if (State::AddonHost >= ggState::UI_READY && !State::IsImGuiInitialized)
		{
			GUI::Initialize();
		}

		if (State::IsImGuiInitialized)
		{
			/* new frame */
			ImGui_ImplWin32_NewFrame();
			ImGui_ImplDX11_NewFrame();
			ImGui::NewFrame();
			/* new frame end */

			/* draw overlay */

			/* draw menu */
			RenderMenu();

			/* draw windows */
			WindowsMutex.lock();
			for (IWindow* wnd : Windows)
			{
				wnd->Render();
			}
			WindowsMutex.unlock();

			/* draw addons*/
			Loader::AddonsMutex.lock();
			for (const auto& [path, addon] : Loader::AddonDefs)
			{
				if (addon.Definitions->Render) { addon.Definitions->Render(); }
			}
			Loader::AddonsMutex.unlock();

			/* TODO: RENDER UNDER UI */
			/* TODO: RENDER OVER UI */

			/* draw overlay end */

			/* end frame */
			ImGui::EndFrame();
			ImGui::Render();
			Renderer::DeviceContext->OMSetRenderTargets(1, &Renderer::RenderTargetView, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			/* end frame end*/
		}
	}

	void RenderMenu()
	{
		if (!IsMenuVisible) { return; }

		if (ImGui::Begin("Menu", &IsMenuVisible, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			WindowsMutex.lock();

			for (IWindow* wnd : Windows) { wnd->MenuOption(0); }
			//ImGui::Button("Layout", ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));
			//ImGui::Button("Options", ImVec2(ImGui::GetFontSize() * 13.75f, 0.0f));

			if (State::IsDeveloperMode)
			{
				ImGui::Separator();

				for (IWindow* wnd : Windows) { wnd->MenuOption(1); }
			}

			ImGui::Separator();

			for (IWindow* wnd : Windows) { wnd->MenuOption(2); }

			WindowsMutex.unlock();
		}
		ImGui::End();
	}

	void AddWindow(IWindow* aWindowPtr)
	{
		WindowsMutex.lock();

		Windows.push_back(aWindowPtr);

		WindowsMutex.unlock();
	}
}