#include "ProcessingaData.h"

using namespace std;
namespace fs = filesystem;

mutex globMtx;

void CollectorTask(const fs::path& pathDir, ThreadQueue& files)
{
    int count = 0;
    cout << "Начат обход директории! " << pathDir << "\n";
    try {
        // Рекурсивный обход директории и добавление в очередь
        for (const auto& file : fs::recursive_directory_iterator(pathDir)) {
            if (file.is_regular_file()) {
                fs::path filePath = file.path().string();
                files.push(filePath);
                count++;
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
        cout << "Обход директории завершён! Обнаружено " << count << " файлов!\n";
    }
}

void HandlerTask(ThreadQueue& files)
{
    while (true) {
        fs::path file = files.pop();

        if (file == "") {
            break;
        }

        ZipFile(file);
    }
}

void ZipFile(const fs::path& pathFile)
{
    {
        lock_guard<mutex> lock(globMtx);
        cout << "Обработчик [" << this_thread::get_id() << "] взял файл " << pathFile << "\n";
    }
}