#include "ProcessingaData.h"

using namespace std;
namespace fs = filesystem;

mutex globMtx;
extern const string OUTPUT_DIR;

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
    fs::path dirPath(pathFile);

    // Формирование имени ZIP-файла
    fs::path zipPath = fs::path(OUTPUT_DIR) / (dirPath.filename().string() + ".zip");
    string zipFileName = zipPath.string();

    {
        lock_guard<mutex> lock(globMtx);
        cout << "Обработчик [" << this_thread::get_id() << "] сжимает файл " << pathFile << " в \"" << zipFileName << "\"\n";
    }

    int err;
    // Открытие ZIP-архива
    zip_t* zip = zip_open(zipFileName.c_str(), ZIP_CREATE | ZIP_EXCL, &err);

    if (zip == nullptr) {
        lock_guard<mutex> lock(globMtx);
        if (fs::exists(zipPath))
        {
            cerr << "Файл " << zipFileName << " уже существует!\n";
            return;
        }
        zip_error_t ziperr;
        zip_error_init_with_code(&ziperr, err);
        cerr << "Ошибка открытия zip-архива " << zip_error_strerror(&ziperr) << " для файла: " << zipFileName << "\n";
        zip_error_fini(&ziperr);
        return;
    }

    // Создание источника данных из файла
    string tempPath = pathFile.string();
    zip_source_t* source = zip_source_file(zip, tempPath.c_str(), 0, 0);
    if (source == nullptr) {
        lock_guard<mutex> lock(globMtx);
        cerr << "Ошибка создания источника данных " << zip_strerror(zip) << " для файла: " << pathFile << "\n";
        zip_close(zip);
        return;
    }

    // Добавление файла в архив
    const string fileName = dirPath.filename().string();
    zip_int64_t fileIdx = zip_file_add(zip, fileName.c_str(), source, ZIP_FL_ENC_UTF_8);

    if (fileIdx < 0) {
        lock_guard<mutex> lock(globMtx);
        cerr << "Ошибка добавления файла в архив " << zip_strerror(zip) << " для файла: " << pathFile << "\n";
        zip_source_free(source);
    }
    else {
        // Сжатие файла, метод сжатия - стандартный, уровень сжатия - средний
        zip_set_file_compression(zip, (zip_uint64_t)fileIdx, ZIP_CM_DEFLATE, 5);
    }

    // Закрытие архива
    if (zip_close(zip) < 0) {
        lock_guard<mutex> lock(globMtx);
        cerr << "Ошибка чтения/записи архива " << zipFileName << ": " << zip_strerror(zip) << "\n";
    }
}