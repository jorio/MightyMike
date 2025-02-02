// MIGHTY MIKE ENTRY POINT
// (C) 2025 Iliyas Jorio
// This file is part of Mighty Mike. https://github.com/jorio/mightymike

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <thread>

#include "Pomme.h"
#include "PommeFiles.h"
#include "PommeInit.h"

extern "C"
{
	#include "externs.h"
	#include "renderdrivers.h"
	#include "framebufferfilter.h"
	#include "version.h"

	SDL_Window* gSDLWindow = nullptr;
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
	gDataSpec = Pomme::Files::HostPathToFSSpec(dataPath / "System");

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

static void Boot(int argc, char** argv)
{
	const char* executablePath = argc > 0 ? argv[0] : NULL;

	SDL_SetAppMetadata(GAME_FULL_NAME, GAME_VERSION, GAME_IDENTIFIER);
#if _DEBUG
	SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
#else
	SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif

	gNumThreads = (int) std::thread::hardware_concurrency();
	if (gNumThreads >= MAX_RENDER_THREADS)
		gNumThreads = MAX_RENDER_THREADS;
	else if (gNumThreads <= 0)
		gNumThreads = 1;

	// Start our "machine"
	Pomme::Init();

	// Initialize SDL video subsystem
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		throw std::runtime_error("Couldn't initialize SDL video subsystem.");
	}

#if GLRENDER
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif // GLRENDER

	// Create window
	int windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if GLRENDER
	windowFlags |= SDL_WINDOW_OPENGL;
#endif
	gSDLWindow = SDL_CreateWindow(GAME_FULL_NAME " " GAME_VERSION, VISIBLE_WIDTH, VISIBLE_HEIGHT, windowFlags);
	if (!gSDLWindow)
		throw std::runtime_error("Couldn't create SDL window.");

#if GLRENDER
	GLRender_Init();
#else
	if (!SDLRender_Init())
		throw std::runtime_error("Couldn't create SDL renderer.");
#endif // GLRENDER

	// Find path to game data folder
	fs::path dataPath = FindGameData(executablePath);

	// Init joystick subsystem
	{
		SDL_Init(SDL_INIT_GAMEPAD);
		auto gamecontrollerdbPath8 = (dataPath / "System" / "gamecontrollerdb.txt").u8string();
		if (-1 == SDL_AddGamepadMappingsFromFile((const char*)gamecontrollerdbPath8.c_str()))
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, GAME_FULL_NAME, "Couldn't load gamecontrollerdb.txt!", gSDLWindow);
		}
	}
}

static void Shutdown()
{
	// Always restore the user's mouse acceleration before exiting.
	// SetMacLinearMouse(false);

	Pomme::Shutdown();

	if (gSDLWindow)
	{
		SDL_DestroyWindow(gSDLWindow);
		gSDLWindow = NULL;
	}

	SDL_Quit();
}

int main(int argc, char** argv)
{
	bool success = true;
	std::string uncaught = "";

	try
	{
		Boot(argc, argv);
		GameMain();
	}
	catch (Pomme::QuitRequest&)
	{
		// no-op, the game may throw this exception to shut us down cleanly
	}
#if !(_DEBUG)
	// In release builds, catch anything that might be thrown by GameMain
	// so we can show an error dialog to the user.
	catch (std::exception& ex)		// Last-resort catch
	{
		success = false;
		uncaught = ex.what();
	}
	catch (...)						// Last-resort catch
	{
		success = false;
		uncaught = "unknown";
	}
#endif

	Shutdown();

	if (!success)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Uncaught exception: %s", uncaught.c_str());
		SDL_ShowSimpleMessageBox(0, GAME_FULL_NAME, uncaught.c_str(), nullptr);
	}

	return success ? 0 : 1;
}
