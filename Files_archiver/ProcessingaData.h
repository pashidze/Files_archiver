#pragma once

#include "TQueue.h"
#include <zip.h>

using namespace std;
namespace fs = filesystem;

extern const string OUTPUT_DIR;

// Функция для обхода директории и формирования очереди
void CollectorTask(const fs::path& pathDir, ThreadQueue& files);

// Функция для извлечения файлов из очереди
void HandlerTask(ThreadQueue& files);

// Функция для сжатия файлов
void ZipFile(const fs::path& pathFile);