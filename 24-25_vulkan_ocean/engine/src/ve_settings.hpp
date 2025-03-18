/*
    Copyright (c) Arnaud BANNIER and Nicolas BODIN.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include <cassert>
#include <cmath>
#include <cstdbool>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <memory>
#include <limits>
#include <optional>

#include <algorithm>
#include <array>
#include <set>
#include <vector>
#include <map>
#include <unordered_map>


//#define FHD
//#define FULLSCREEN

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_to_string.hpp>

#ifdef _WIN32
#  include "SDL.h"
#  include "SDL_vulkan.h"
//#  include "SDL_image.h"
//#  include "SDL_ttf.h"
//#  include "SDL_mixer.h"
#else
#  include <SDL2/SDL.h>
#  include <SDL2/SDL_vulkan.h>
//#  include <SDL2/SDL_image.h>
//#  include <SDL2/SDL_ttf.h>
//#  include <SDL2/SDL_mixer.h>
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

#ifdef _MSC_VER
#  define INLINE inline
#else
#  define INLINE static inline
#endif

#ifdef _MSC_VER
#  define strdup(str) _strdup(str)
#endif

#define AssertNew(ptr) { if (ptr == NULL) { assert(false); abort(); } }

#define HD_WIDTH 1280
#define HD_HEIGHT 720
#define FHD_WIDTH 1920
#define FHD_HEIGHT 1080

#ifdef FHD
#define WINDOW_WIDTH   FHD_WIDTH
#define WINDOW_HEIGHT  FHD_HEIGHT
#define LOGICAL_WIDTH  FHD_WIDTH
#define LOGICAL_HEIGHT FHD_HEIGHT
#else
#define WINDOW_WIDTH   HD_WIDTH
#define WINDOW_HEIGHT  HD_HEIGHT
#define LOGICAL_WIDTH  HD_WIDTH
#define LOGICAL_HEIGHT HD_HEIGHT
#endif
