cmake_minimum_required(VERSION 3.15)
project("PaperCube" LANGUAGES CXX)

# Enforce Modern C++ globally (Setting to 20 since your MSVC is already using it!)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Cross-platform protection: Only apply Windows/MSVC debugging features if on Windows
if(MSVC)
    if (POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
    endif()
endif()

# Delegate tasks to the subdirectories
add_subdirectory("core")
add_subdirectory("tests")