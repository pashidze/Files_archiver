#include "TQueue.h"

using namespace std;
namespace fs = filesystem;

void ThreadQueue::push(const fs::path& file)
{
    {
        lock_guard<mutex> lock(mtx);

        // Провеврка что еще есть файлы
        if (finish)
        {
            return;
        }

        files.push(file);
    }

    // Оповещение одного ждущего потока о появлении задачи
    cv.notify_one();
}

fs::path ThreadQueue::pop()
{
    unique_lock<mutex> lock(mtx);

    // Блокирование потока и разблокирование мьютекса
    cv.wait(lock, [this]
        {
            return !files.empty() || finish;
        });

    // Если очередь пуста и получен сигнал завершения - значит, задач больше нет
    if (files.empty() && finish)
    {
        // Оповещение всех обработчиков для корректного завершения работы
        cv.notify_all();
        return "";
    }

    fs::path file = files.front();
    files.pop();

    return file;
}

void ThreadQueue::close()
{
    {
        lock_guard<mutex> lock(mtx);
        finish = true;
    }

    // Оповещение всех ожидающих обработчиков для завершения работы
    cv.notify_all();
}