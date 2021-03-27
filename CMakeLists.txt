cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)

# macOS options
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Minimum macOS deployment version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Target macOS architectures")

project(MightyMike
	VERSION 3.0.0
	LANGUAGES C CXX
)

set(GAME_TARGET ${PROJECT_NAME})
set(GAME_ICNS cmake/${GAME_TARGET}.icns)
get_filename_component(GAME_ICNS_FILENAME ${GAME_ICNS} NAME)

#------------------------------------------------------------------------------
# GLOBAL OPTIONS (BEFORE ADDING SUBDIRECTORIES)
#------------------------------------------------------------------------------

if(MSVC)
	# This lets us detect that MSVC supports C++20 features via the __cplusplus macro
	# (See: https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus)
	add_compile_options(/Zc:__cplusplus)
	add_compile_definitions(UNICODE _UNICODE)

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${GAME_TARGET})
endif()

set(POMME_NO_VIDEO true)
set(POMME_NO_QD3D true)

#------------------------------------------------------------------------------
# DEPENDENCIES
#------------------------------------------------------------------------------

add_subdirectory(extern/Pomme)

find_package(SDL2 REQUIRED COMPONENTS main)

# OpenMP is not mandatory to compile the game. If it's not available, framebuffer
# filtering in Window.c will be single-threaded.
# Disabled by default because on some systems (including Windows),
# the default wait policy is "active", causing high CPU load.
if (USE_OPENMP)
	find_package(OpenMP)
endif()

#------------------------------------------------------------------------------
# GAME SOURCES
#------------------------------------------------------------------------------

set(GAME_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Write header file containing version info
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in ${GAME_SRCDIR}/Headers/version.h)

set(GAME_SOURCES
	${GAME_SRCDIR}/Heart/Cinema.c
	${GAME_SRCDIR}/Heart/Collision.c
	${GAME_SRCDIR}/Heart/Infobar.c
	${GAME_SRCDIR}/Heart/input.c
	${GAME_SRCDIR}/Heart/IO.c
	${GAME_SRCDIR}/Heart/LZSS.c
	${GAME_SRCDIR}/Heart/Misc.c
	${GAME_SRCDIR}/Heart/Picture.c
	${GAME_SRCDIR}/Heart/Spin.c
	${GAME_SRCDIR}/Heart/Window.c
	${GAME_SRCDIR}/Heart/Main.c
	${GAME_SRCDIR}/Heart/SettingsScreen.c
	${GAME_SRCDIR}/Drivers/Animation.c
	${GAME_SRCDIR}/Drivers/ObjectManager.c
	${GAME_SRCDIR}/Drivers/Palette.c
	${GAME_SRCDIR}/Drivers/Shape.c
	${GAME_SRCDIR}/Drivers/Sound.c
	${GAME_SRCDIR}/MeAndMo/Enemy.c
	${GAME_SRCDIR}/MeAndMo/MyGuy.c
	${GAME_SRCDIR}/MeAndMo/MyGuy2.c
	${GAME_SRCDIR}/MeAndMo/Weapon.c
	${GAME_SRCDIR}/MeAndMo/Weapon2.c
	${GAME_SRCDIR}/Misc/Bonus.c
	${GAME_SRCDIR}/Misc/MiscAnims.c
	${GAME_SRCDIR}/Misc/Traps.c
	${GAME_SRCDIR}/Misc/Triggers.c
	${GAME_SRCDIR}/Playfield/Playfield.c
	${GAME_SRCDIR}/Enemies/Bargain/Enemy_8Ball.c
	${GAME_SRCDIR}/Enemies/Bargain/Enemy_Battery.c
	${GAME_SRCDIR}/Enemies/Bargain/Enemy_doggy.c
	${GAME_SRCDIR}/Enemies/Bargain/Enemy_Robot.c
	${GAME_SRCDIR}/Enemies/Bargain/Enemy_Slinky.c
	${GAME_SRCDIR}/Enemies/Bargain/Enemy_Top.c
	${GAME_SRCDIR}/Enemies/Bargain/RaceCar.c
	${GAME_SRCDIR}/Enemies/Candy/Enemy_Carmel.c
	${GAME_SRCDIR}/Enemies/Candy/Enemy_ChocBunny.c
	${GAME_SRCDIR}/Enemies/Candy/Enemy_GBread.c
	${GAME_SRCDIR}/Enemies/Candy/Enemy_GummyBear.c
	${GAME_SRCDIR}/Enemies/Candy/Enemy_LemonDrop.c
	${GAME_SRCDIR}/Enemies/Candy/Enemy_Mint.c
	${GAME_SRCDIR}/Enemies/Clown/Enemy_Clown.c
	${GAME_SRCDIR}/Enemies/Clown/Enemy_ClownCar.c
	${GAME_SRCDIR}/Enemies/Clown/Enemy_FlowerClown.c
	${GAME_SRCDIR}/Enemies/Clown/Enemy_HatBunny.c
	${GAME_SRCDIR}/Enemies/Fairy/Enemy_BBWolf.c
	${GAME_SRCDIR}/Enemies/Fairy/Enemy_Dragon.c
	${GAME_SRCDIR}/Enemies/Fairy/Enemy_Giant.c
	${GAME_SRCDIR}/Enemies/Fairy/Enemy_Soldier.c
	${GAME_SRCDIR}/Enemies/Fairy/Enemy_Spider.c
	${GAME_SRCDIR}/Enemies/Fairy/Enemy_Witch.c
	${GAME_SRCDIR}/Enemies/Jurassic/Enemy_BabyDino.c
	${GAME_SRCDIR}/Enemies/Jurassic/Enemy_CaveMan.c
	${GAME_SRCDIR}/Enemies/Jurassic/Enemy_DinoEgg.c
	${GAME_SRCDIR}/Enemies/Jurassic/Enemy_Rex.c
	${GAME_SRCDIR}/Enemies/Jurassic/Enemy_Triceratops.c
	${GAME_SRCDIR}/Enemies/Jurassic/Enemy_Turtle.c
	${GAME_SRCDIR}/Main.cpp
)

#set(GAME_HEADERS
#)

#set(GAME_GLUE_SOURCES
#	${GAME_SRCDIR}/Main.cpp
#)

set(GAME_ALL_SOURCES
#	${GAME_GLUE_SOURCES}
	${GAME_SOURCES}
#	${GAME_HEADERS}
)

#------------------------------------------------------------------------------
# SOURCE GROUPS
#------------------------------------------------------------------------------

source_group(TREE ${GAME_SRCDIR} PREFIX "" FILES ${GAME_ALL_SOURCES})

#------------------------------------------------------------------------------
# SOURCE FILE PROPERTIES
#------------------------------------------------------------------------------

if(NOT MSVC)
	set_source_files_properties(${GAME_SOURCES} PROPERTIES COMPILE_FLAGS "-include Pomme.h")
else()
	set_source_files_properties(${GAME_SOURCES} PROPERTIES COMPILE_FLAGS /FIPomme.h)
endif()

#------------------------------------------------------------------------------
# EXECUTABLE TARGET
#------------------------------------------------------------------------------

if(WIN32)
	# "WIN32" here is equivalent to /SUBSYSTEM:WINDOWS for MSVC
	add_executable(${GAME_TARGET} WIN32
        ${GAME_ALL_SOURCES}
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/WindowsResources.rc"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${GAME_TARGET}.exe.manifest"
    )
elseif(APPLE)
	add_executable(${GAME_TARGET} MACOSX_BUNDLE
		${GAME_ALL_SOURCES}
		${GAME_ICNS}
		${SDL2_LIBRARIES}
	)
else()
	add_executable(${GAME_TARGET} ${GAME_ALL_SOURCES})
endif()

target_include_directories(${GAME_TARGET} PRIVATE
	${SDL2_INCLUDE_DIRS}
	extern/Pomme/src
	${GAME_SRCDIR}
	${GAME_SRCDIR}/Headers
)

target_link_libraries(${GAME_TARGET}
	Pomme
	${SDL2_LIBRARIES}
	"$<$<BOOL:${APPLE}>:-framework Foundation>"		# Required for KillMacMouseAcceleration
	"$<$<BOOL:${APPLE}>:-framework IOKit>"			# Required for KillMacMouseAcceleration
)

#------------------------------------------------------------------------------
# DEFINES
#------------------------------------------------------------------------------

add_compile_definitions(
	"$<$<CONFIG:DEBUG>:_DEBUG>"
	__powerc		# TODO: REMOVEME once all 68k asm is gone
)

if(WIN32)
	# On Win32, we need NOGDI and NOUSER to be able to define some Mac functions
	# whose names are otherwise taken by Windows APIs.
	target_compile_definitions(${GAME_TARGET} PRIVATE WIN32_LEAN_AND_MEAN NOGDI NOUSER)
endif()

#------------------------------------------------------------------------------
# COMPILER OPTIONS
#------------------------------------------------------------------------------

if(OpenMP_FOUND)
	message("OpenMP found; framebuffer filtering will be multithreaded")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

if(NOT MSVC)
	target_compile_options(${GAME_TARGET} PRIVATE
		-fexceptions
		-Wall
		-Wextra
		-Wshadow
		-Wno-multichar
		-Wno-unknown-pragmas
		-Wno-unused-parameter

		$<$<CXX_COMPILER_ID:GNU>:-Wno-unused-but-set-variable>

		-Wno-missing-braces						# TODO: REMOVEME
		-Wno-parentheses						# TODO: REMOVEME
#		-Werror=implicit-function-declaration	# TODO: UNCOMMENTME
		-Wno-implicit-fallthrough				#TODO: REMOVEME  (for ILD macro)
		-Wno-unused-variable					#TODO: REMOVEME
		-Wno-implicit-int						#TODO: REMOVEME

		-Werror=return-type
	)
else()
	# By default, MSVC may add /EHsc to CMAKE_CXX_FLAGS, which we don't want (we use /EHs below)
	string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

	# By default, MSVC may add /W3 to CMAKE_CXX_FLAGS, which we don't want (we use /W4 below)
	# Note that this is not required with "cmake_minimum_required(VERSION 3.15)" or later
	string(REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	string(REPLACE "/W3" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})

	target_compile_options(${GAME_TARGET} PRIVATE
		/EHs	# synchronous exceptions; also, extern "C" functions may throw exceptions
		/W4
		/wd4068 # ignore unrecognized pragmas
		/wd4100 # unreferenced formal parameters
		/wd4200 # nonstandard extension: zero-sized array in struct
		/wd4201 # nonstandard extension: nameless struct
		/wd4244 # conversion from double to float
		/wd4305 # truncation from double to float
	)
endif()

#------------------------------------------------------------------------------
# PLATFORM-SPECIFIC PACKAGING
#------------------------------------------------------------------------------

if(APPLE)
	set_target_properties(${GAME_TARGET} PROPERTIES
		# Set framework search path to (App bundle)/Contents/Frameworks so the game can use its embedded SDL2.framework
		XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../Frameworks"

		# Explicitly turn off code signing, otherwise downloaded app will be quarantined forever
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""

		RESOURCE ${GAME_ICNS}
		MACOSX_BUNDLE_ICON_FILE ${GAME_ICNS_FILENAME}
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
		MACOSX_BUNDLE_COPYRIGHT "© 1995 Pangea Software, Inc."
	)

	# Copy stuff to app bundle contents
	set_source_files_properties(${GAME_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	set_source_files_properties(${SDL2_LIBRARIES} PROPERTIES MACOSX_PACKAGE_LOCATION "Frameworks")

	set(BUNDLE_CONTENTS_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/..")
	set(APP_PARENT_DIR "${BUNDLE_CONTENTS_DIR}/../..")

	add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
		# Copy assets to app bundle
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Data ${BUNDLE_CONTENTS_DIR}/Resources

		# Nuke SDL headers in embedded framework
		COMMAND rm -rf ${BUNDLE_CONTENTS_DIR}/Frameworks/SDL2.framework/Headers
		COMMAND rm -rf ${BUNDLE_CONTENTS_DIR}/Frameworks/SDL2.framework/Versions/Current/Headers

		# High-DPI support in Info.plist
#		plutil -replace NSHighResolutionCapable -bool true ${BUNDLE_CONTENTS_DIR}/Info.plist
	)
else()
	set(APP_PARENT_DIR "$<TARGET_FILE_DIR:${GAME_TARGET}>")

	# Copy assets besides executable
	add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Data ${APP_PARENT_DIR}/Data)
endif()

# Copy SDL2 DLLs to output folder on Windows
if(WIN32)
	foreach(DLL ${SDL2_DLLS})
		add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL} ${APP_PARENT_DIR})
	endforeach()

	set_target_properties(${GAME_TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# Copy documentation to output folder
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ReadMe.txt.in ${CMAKE_CURRENT_BINARY_DIR}/ReadMe.txt)
add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/ReadMe.txt ${APP_PARENT_DIR}/ReadMe.txt
#	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md ${APP_PARENT_DIR}/License.txt
#	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/docs ${APP_PARENT_DIR}/Documentation
)

