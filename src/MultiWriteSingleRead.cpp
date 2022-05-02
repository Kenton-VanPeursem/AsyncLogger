// MultiWriteSingleRead.cpp : This file contains the 'main' function. Program execution
// begins and ends there.

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <queue>
#include <semaphore>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>


enum LogLevel {
    DEBUG, INFO, WARN, ERROR
};

struct LogMessage {
    std::thread::id thread_id;
    LogLevel level;
    std::time_t ts;
    std::string msg;

    std::string formatted() {
        std::stringstream ss;
        ss << "{"
            << "level:" << level
            << ",thread_id:" << thread_id
            << ",ts:" << ts
            << ",msg:" << msg
            << "}" << std::endl;

        return ss.str();
    }

    LogMessage(LogLevel level, std::string msg) {
        this->thread_id = std::this_thread::get_id();
        this->level = level;
        this->ts = std::time(NULL);
        this->msg = msg;
    }
};


class Logger {
    public:
        std::atomic_long messageListSize;
        std::queue<LogMessage> messagesToLog;
        std::counting_semaphore<> dequeSema;
        std::mutex messageListMutex;
        std::vector<LogMessage> messageList;

        void popMessages(void) {
            std::string filename = "LogFile_";
            filename.append(std::to_string(time(NULL)));
            filename.append(".txt");
            std::ofstream logFile(filename);

            LogMessage item(INFO, "");
            while (true) {
                dequeSema.acquire(); // This either blocks or decrements the semaphore
                item = messagesToLog.front();

                messageListMutex.lock();
                messagesToLog.pop();
                messageListMutex.unlock();

                logFile << item.formatted();
                std::flush(logFile);
                messageListSize--;
            }
        }

        void spawnReader() {
            std::thread reader(&Logger::popMessages, this);
            reader.detach();
        }

        // This is the writer thread worker function.
        void pushMessage(LogLevel level, std::string msg) {  // Pushes the thread ID and the message into the queue.
            LogMessage logmsg(level, msg);

            messageListMutex.lock();
            messagesToLog.push(logmsg);
            messageListMutex.unlock();

            dequeSema.release(); // This increments the semaphore
        }

        void flush() {
            while (messageListSize) {
                std::cout << "Waiting for log messages to write (" << messageListSize << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

    public:
        Logger() : dequeSema(0) {
            spawnReader();
        }

        void debug(std::string msg) {
            messageListSize++;
            std::thread t1(&Logger::pushMessage, this, DEBUG, msg);
            t1.detach();
        }

        void info(std::string msg) {
            messageListSize++;
            std::thread t1(&Logger::pushMessage, this, DEBUG, msg);
            t1.detach();
        }

        void warn(std::string msg) {
            messageListSize++;
            std::thread t1(&Logger::pushMessage, this, DEBUG, msg);
            t1.detach();
        }

        void error(std::string msg) {
            messageListSize++;
            std::thread t1(&Logger::pushMessage, this, DEBUG, msg);
            t1.detach();
        }

        virtual ~Logger() {
            flush();
        }
};

int main() {
    Logger logger;

    for (long i = 0; i < 5; ++i) {
        logger.debug("debug");
        logger.info("info");
        logger.warn("warn");
        logger.error("error");
    }

    return 0;
}
