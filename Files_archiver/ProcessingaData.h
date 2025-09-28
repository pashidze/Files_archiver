#pragma once

#include "TQueue.h"

using namespace std;
namespace fs = filesystem;

// Функция для обхода директории и формирования очереди
void CollectorTask(const fs::path& pathDir, ThreadQueue& files);

// Функция для извлечения файлов из очереди
void HandlerTask(ThreadQueue& files);

// Функция для сжатия файлов
void ZipFile(const fs::path& pathFile);