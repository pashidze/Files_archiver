# CMakeList.txt: проект CMake для Files_archiver; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Files_archiver "Files_archiver.cpp" "Files_archiver.h" "TQueue.h" "TQueue.cpp" "ProcessingaData.h" "ProcessingData.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Files_archiver PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
find_package(libzip CONFIG REQUIRED)
target_link_libraries(Files_archiver PRIVATE libzip::zip)