CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT("VoxelVolution")

# Set the directory of cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

# Put the executable in the bin folder
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# find all source files in the src directory
FILE(GLOB_RECURSE VV_SRC "src/*.cpp")

# find all the files in the includes directory
FILE(GLOB_RECURSE VV_INCLUDES "include/*.h" "include/*.hpp")

#Check if we can build with the current tool chain
INCLUDE(SetCompilerFlags)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/include/)
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/")

# If we are on windows add in the local search directories as well.
IF (WIN32 AND NOT MINGW) # Windows
	SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/lib/include/)
	INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/include/")
	LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/")
	SET(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/lib/)
ENDIF (WIN32 AND NOT MINGW)

FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLFW3 REQUIRED)

# Give these some dummy values and if the platform is LINUX or OSX they will be set accordingly.
SET(X11_LIBRARIES "")
SET(OSX_LIBRARIES "")

IF (NOT APPLE) # X11 and GLEW are not needed on OSX.
	FIND_PACKAGE(X11)
	FIND_PACKAGE(GLEW REQUIRED) # We find GLEW here as OSX doesn't need it.
ENDIF (NOT APPLE)

IF (APPLE) # Mac OSX
	SET(GLEW_LIBRARY "") # Set a dummy value for GLEW.

	SET(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS macosx)

	# Need the 10.7 SDK or later.
	EXECUTE_PROCESS(COMMAND xcodebuild -sdk macosx -version SDKVersion OUTPUT_VARIABLE OSX_SDK_VERSION)
	IF (NOT (OSX_SDK_VERSION VERSION_GREATER 10.7 OR OSX_SDK_VERSION VERSION_EQUAL 10.7))
		MESSAGE(FATAL_ERROR "The installed version of Xcode does not support the 10.7 SDK or later.  Please upgrade Xcode and try again.")
	ENDIF (NOT (OSX_SDK_VERSION VERSION_GREATER 10.7 OR OSX_SDK_VERSION VERSION_EQUAL 10.7))

	# Configure the project to use the correct SDK.
	IF (XCODE_VERSION)
		SET(CMAKE_OSX_SYSROOT macosx)
	ELSE (XCODE_VERSION)
		# Non-Xcode generators need the full path.
		EXECUTE_PROCESS(COMMAND xcodebuild -sdk macosx -version Path | head -n 1 OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
		STRING(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
	ENDIF (XCODE_VERSION)

	# Can deploy back to 10.7, the first OS X to support the GL Core.
	SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)

	# Need Core Foundation and libobjc.
	SET(OSX_LIBRARIES "-framework CoreFoundation /usr/lib/libobjc.dylib")
ENDIF (APPLE)

# define all required external libraries
set(VV_ALL_LIBS
	${OPENGL_LIBRARIES}
	${GLFW3_LIBRARIES}
	${X11_LIBRARIES}
	${OSX_LIBRARIES}
	${GLEW_LIBRARIES}
)

ADD_EXECUTABLE("VoxelVolution"
	${VV_SRC}
	${VV_INCLUDES}
)

TARGET_LINK_LIBRARIES("VoxelVolution" ${VV_ALL_LIBS})
