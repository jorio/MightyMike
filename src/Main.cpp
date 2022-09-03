#include "Pomme.h"
#include "PommeInit.h"
#include "PommeFiles.h"
#include "PommeGraphics.h"

#include <SDL.h>
#include <iostream>
#include <thread>

#include "version.h"

extern "C"
{
	// Satisfy externs in game code
	SDL_Window*			gSDLWindow		= nullptr;
	SDL_Renderer*		gSDLRenderer	= nullptr;
	SDL_Texture*		gSDLTexture		= nullptr;

	// Lets the game know where to find its asset files
	FSSpec gDataSpec;

	void GameMain(void);

	int gNumThreads = 0;
}

static fs::path FindGameData(const char* executablePath)
{
	fs::path dataPath;

	int attemptNum = 0;

#if !(__APPLE__)
	attemptNum++;		// skip macOS special case #0
#endif

	if (!executablePath)
		attemptNum = 2;

tryAgain:
	switch (attemptNum)
	{
		case 0:			// special case for macOS app bundles
			dataPath = executablePath;
			dataPath = dataPath.parent_path().parent_path() / "Resources";
			break;

		case 1:
			dataPath = executablePath;
			dataPath = dataPath.parent_path() / "Data";
			break;

		case 2:
			dataPath = "Data";
			break;

		default:
			throw std::runtime_error("Couldn't find the Data folder.");
	}

	attemptNum++;

	dataPath = dataPath.lexically_normal();

	// Set data spec -- Lets the game know where to find its asset files
	gDataSpec = Pomme::Files::HostPathToFSSpec(dataPath / "Shapes");

	// Use application resource file
	auto applicationSpec = Pomme::Files::HostPathToFSSpec(dataPath / "System" / "Application");
	short resFileRefNum = FSpOpenResFile(&applicationSpec, fsRdPerm);

	if (resFileRefNum == -1)
	{
		goto tryAgain;
	}

	UseResFile(resFileRefNum);

	return dataPath;
}

static void Boot(const char* executablePath)
{
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

#if OSXPPC
	gNumThreads = 1;
#else
	gNumThreads = (int) std::thread::hardware_concurrency();
	if (gNumThreads >= 32)
		gNumThreads = 32;
	else if (gNumThreads <= 0)
		gNumThreads = 1;
#endif

	// Start our "machine"
	Pomme::Init();

	// Initialize SDL video subsystem
	if (0 != SDL_Init(SDL_INIT_VIDEO))
		throw std::runtime_error("Couldn't initialize SDL video subsystem.");

	// Create window
	gSDLWindow = SDL_CreateWindow(
			"Mighty Mike " PROJECT_VERSION,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			640,
			480,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!gSDLWindow)
		throw std::runtime_error("Couldn't create SDL window.");

	gSDLRenderer = SDL_CreateRenderer(gSDLWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!gSDLRenderer)
		throw std::runtime_error("Couldn't create SDL renderer.");
	// The texture bound to the renderer is created in-game after loading the prefs.

	SDL_RenderSetLogicalSize(gSDLRenderer, 640, 480);

	fs::path dataPath = FindGameData(executablePath);
#if !(__APPLE__)
//	Pomme::Graphics::SetWindowIconFromIcl8Resource(gSDLWindow, 400);
#endif

#if !(NOJOYSTICK)
	// Init joystick subsystem
	SDL_Init(SDL_INIT_JOYSTICK);
	SDL_Init(SDL_INIT_HAPTIC);
	{
		auto gamecontrollerdbPath8 = (dataPath / "System" / "gamecontrollerdb.txt").u8string();
		if (-1 == SDL_GameControllerAddMappingsFromFile((const char*)gamecontrollerdbPath8.c_str()))
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Mighty Mike", "Couldn't load gamecontrollerdb.txt!", gSDLWindow);
		}
	}
#endif
}

static void Shutdown()
{
	Pomme::Shutdown();

	if (gSDLRenderer)
	{
		SDL_DestroyRenderer(gSDLRenderer);
		gSDLRenderer = nullptr;
	}

	if (gSDLWindow)
	{
		SDL_DestroyWindow(gSDLWindow);
		gSDLWindow = nullptr;
	}
	
	SDL_Quit();
}

int main(int argc, char** argv)
{
	int				returnCode				= 0;
	std::string		finalErrorMessage		= "";
	bool			showFinalErrorMessage	= false;

	const char* executablePath = argc > 0 ? argv[0] : NULL;

	// Start the game
	try
	{
		Boot(executablePath);
		GameMain();
	}
	catch (Pomme::QuitRequest&)
	{
		// no-op, the game may throw this exception to shut us down cleanly
	}
#if !(_DEBUG)
	// In release builds, catch anything that might be thrown by CommonMain
	// so we can show an error dialog to the user.
	catch (std::exception& ex)		// Last-resort catch
	{
		returnCode = 1;
		finalErrorMessage = ex.what();
		showFinalErrorMessage = true;
	}
	catch (...)						// Last-resort catch
	{
		returnCode = 1;
		finalErrorMessage = "unknown";
		showFinalErrorMessage = true;
	}
#endif

	// Clean up!
	Shutdown();

	if (showFinalErrorMessage)
	{
		std::cerr << "Uncaught exception: " << finalErrorMessage << "\n";
		SDL_ShowSimpleMessageBox(0, "Uncaught exception", finalErrorMessage.c_str(), nullptr);
	}

	return returnCode;
}
