// Files_archiver.cpp: определяет точку входа для приложения.
//

#include "Files_archiver.h"

using namespace std;
namespace fs = filesystem;

const string OUTPUT_DIR = "Zip files";

//Перевод получаемого из консоли параметра в UTF-8 для корректной работы с кириллицей
vector<string> get_utf8_argv() {
	vector<string> args;
#ifdef _WIN32
	int argcW = 0;
	LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argcW);
	if (!argvW) return args;
	for (int i = 0; i < argcW; ++i) {
		int len = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
		string utf8(len - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, utf8.data(), len - 1, nullptr, nullptr);
		args.push_back(move(utf8));
	}
	LocalFree(argvW);
#endif
	return args;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
	setlocale(LC_ALL, "rus");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	vector<string> args = get_utf8_argv();
#endif

	string strDir;
	fs::path pathDir;
	ThreadQueue filesQueue;
	unsigned int numCores;
	unsigned int numThreads;
	vector<thread> handlerThreads;

	// Если нет параметра, то предлагается ввод, в противном случае считывается из параметра
	if (argc < 2) {
		cout << "Укажите директорию: ";
		getline(cin, strDir);
#ifdef _WIN32
		pathDir = fs::u8path(strDir);
#else
		pathDir = fs::path(strDir);
#endif
	}
	else
	{
#ifdef _WIN32
		strDir = args[1];
		pathDir = fs::u8path(strDir);
#else
		strDir = argv[1];
		pathDir = fs::path(strDir);
#endif
	}
	
	if (!fs::exists(pathDir))
	{
		cerr << "Указанная директория " << pathDir << " не найдена!\n";
		return 0;
	}

	try 
	{
		if (!fs::exists(OUTPUT_DIR)) 
		{
			fs::create_directory(OUTPUT_DIR);
		}
	}
	catch (const fs::filesystem_error& e) 
	{
		cerr << "Ошибка при создании директории zip файлов: " << e.what() << "\n";
		return 0;
	}

	// Определение количества потоков
	numCores = thread::hardware_concurrency();
	numThreads = (numCores == 0) ? 4 : numCores;
	cout << "Обнаружено " << numCores << " потоков процессора. Запущено " << numThreads << " рабочих потоков." << "\n";

	// Запуск потока для обхода директории
	thread collThread(CollectorTask, ref(pathDir), ref(filesQueue));

	// Запуск потоков для обработки
	for (unsigned int i = 0; i < numThreads; ++i) {
		handlerThreads.emplace_back(HandlerTask, ref(filesQueue));
	}

	// Ожидание завершения потока
	if (collThread.joinable()) {
		collThread.join();
	}

	// Завершение потоков обработчиков
	for (auto& handler : handlerThreads) {
		if (handler.joinable()) {
			handler.join();
		}
	}

#ifdef _WIN32
	system("pause");
#endif
	return 0;
}
