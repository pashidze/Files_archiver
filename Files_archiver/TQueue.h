#pragma once

#include <queue>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <filesystem>
#include <Windows.h>

using namespace std;
namespace fs = filesystem;

class ThreadQueue {
private:
    queue<fs::path> files;
    mutex mtx;
    condition_variable cv;
    bool finish = false;
public:

    void push(const fs::path& file);

    fs::path pop();

    void close();
};