# CMakeList.txt : CMake project for SSProjekat, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (SSProjekat   "inc/assembler.h" "src/assembler.cpp" "src/symbolTable.cpp" "inc/symbolTable.h" "src/sectionTable.cpp" "inc/sectionTable.h" "inc/relocationTable.h" "src/relocationTable.cpp" "inc/literalTable.h" "src/literalTable.cpp" "inc/syntaxCheck.h" "src/syntaxCheck.cpp" "src/main.cpp" "inc/linker/linker.h" "src/linker/linker.cpp" "inc/emulator.h" "src/emulator.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SSProjekat PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
