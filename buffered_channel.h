#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex sLocker;
std::mutex rLocker;
std::condition_variable newsignal;

template <class T>
struct Channel{
private:
std::queue<T> buff;
int buffer_size;
bool closed;
public:
    Channel(int size){
      buffer_size = size;
    }

    void send(T value){
        if (closed) {
            throw std::runtime_error("channel is closed");
        }
        std::unique_lock<std::mutex> locker(sLocker);
        while (buff.size() >= buffer_size) {
            newsignal.wait(locker);
        }
        buff.push(value);
        newsignal.notify_one();
    }

    std::pair<T, bool> Recv() {
        if (closed && buff.size() >= buffer_size) {
            return {T(), false};
        }
        std::unique_lock<std::mutex> locker(rLocker);
        while (buff.empty()) {
            newsignal.wait(locker);
        }
        T value = buff.front();
            buff.pop();
        newsignal.notify_one();
        return {value, false};
    }

    void Close() {
        closed = true;
    }

};
