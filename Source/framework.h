#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>
#include <map>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <ctime>
#include <thread>
#include <unordered_set>

namespace fs = std::filesystem;
namespace ch = std::chrono;
typedef unsigned int uint;
typedef unsigned short int ushort;