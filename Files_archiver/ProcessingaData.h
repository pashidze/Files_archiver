#pragma once

#include "TQueue.h"

using namespace std;
namespace fs = filesystem;

// Функция для обхода директории и формирования очереди
void CollectorTask(const std::filesystem::path& pathDir, ThreadQueue& files);

// Функция для извлечения файлов из очереди
void HandlerTask(ThreadQueue& files);

// Функция для сжатия файлов
void ZipFile(const std::filesystem::path& pathFile);