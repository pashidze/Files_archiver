// Files_archiver.cpp: определяет точку входа для приложения.
//

#include "Files_archiver.h"

using namespace std;
namespace fs = filesystem;

int main()
{
	setlocale(LC_ALL, "rus");

	string strDir;
	fs::path pathDir;
	ThreadQueue filesQueue;
	unsigned int numCores;
	unsigned int numThreads;

	cout << "Укажите директорию: ";
	getline(cin, strDir);

	pathDir = fs::u8path(strDir);
	
	if (!fs::exists(pathDir))
	{
		cerr << "Указанная директория " << pathDir << " не найдена!\n";
		return 0;
	}

	// Определение количества потоков
	numCores = thread::hardware_concurrency();
	numThreads = (numCores == 0) ? 4 : numCores;
	cout << "Обнаружено " << numCores << " потоков процессора. Запущено " << numThreads << " рабочих потоков." << "\n";

	// Запуск потока для обхода директории
	thread collThread(CollectorTask, ref(pathDir), ref(filesQueue));

	// Ожидание завершения потока
	if (collThread.joinable()) {
		collThread.join();
	}

	system("pause");
	return 0;
}
