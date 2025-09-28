#include "ProcessingaData.h"

using namespace std;
namespace fs = filesystem;

mutex globMtx;
extern const string OUTPUT_DIR;

//Перевод строк в UTF-8 для корректной работы с кириллицей
#ifdef _WIN32
static string utf16_to_utf8(const wstring& wPath)
{
    if (wPath.empty())
    {
        return {};
    }
    int size = ::WideCharToMultiByte(CP_UTF8, 0, wPath.data(), (int)wPath.size(), nullptr, 0, nullptr, nullptr);
    if (size == 0) return {};
    string out(size, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, wPath.data(), (int)wPath.size(), &out[0], size, nullptr, nullptr);
    return out;
}

static string path_to_utf8(const fs::path& p)
{
    return utf16_to_utf8(p.wstring());
}

#else

static string path_to_utf8(const fs::path& p)
{
    return p.string();
}
#endif

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
    string zipFileName = path_to_utf8(zipPath.string());
    string outFileName = zipPath.string();

    {
        lock_guard<mutex> lock(globMtx);
        cout << "Обработчик [" << this_thread::get_id() << "] сжимает файл " << pathFile << " в \"" << outFileName << "\"\n";
    }

    int err;
    // Открытие ZIP-архива
    zip_t* zip = zip_open(zipFileName.c_str(), ZIP_CREATE | ZIP_EXCL, &err);

    if (zip == nullptr) {
        lock_guard<mutex> lock(globMtx);
        if (fs::exists(zipPath))
        {
            cerr << "Файл " << outFileName << " уже существует!\n";
            return;
        }
        zip_error_t ziperr;
        zip_error_init_with_code(&ziperr, err);
        cerr << "Ошибка открытия zip-архива " << zip_error_strerror(&ziperr) << " для файла: " << outFileName << "\n";
        zip_error_fini(&ziperr);
        return;
    }

    // Создание источника данных из файла
    string tempPath = path_to_utf8(pathFile.string());
    zip_source_t* source = zip_source_file(zip, tempPath.c_str(), 0, 0);
    if (source == nullptr) {
        lock_guard<mutex> lock(globMtx);
        cerr << "Ошибка создания источника данных " << zip_strerror(zip) << " для файла: " << pathFile << "\n";
        zip_close(zip);
        return;
    }

    // Добавление файла в архив
    const string fileName = path_to_utf8(dirPath.filename().string());
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
        cerr << "Ошибка чтения/записи архива " << outFileName << ": " << zip_strerror(zip) << "\n";
    }
}