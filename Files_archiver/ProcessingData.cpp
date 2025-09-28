#include "ProcessingaData.h"

mutex globMtx;

void CollectorTask(const std::filesystem::path& pathDir, ThreadQueue& files)
{
    cout << "Начат обход директории! " << pathDir << "\n";
    try {
        // Рекурсивный обход директории и добавление в очередь
        for (const auto& file : fs::recursive_directory_iterator(pathDir)) {
            if (file.is_regular_file()) {
                fs::path filePath = file.path().string();
                files.push(filePath);
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        {
            lock_guard<mutex> lock(globMtx);
            cerr << "Ошибка при обходе директории: " << e.what() << "\n";
        }
    }

    files.close();
    {
        lock_guard<mutex> lock(globMtx);
        cout << "Обход директории завершён! Обнаружено " << files.size() << " файлов!\n";
    }
}

void HandlerTask(ThreadQueue& files)
{

}

void ZipFile(const std::filesystem::path& pathFile)
{

}