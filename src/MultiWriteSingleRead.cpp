// MultiWriteSingleRead.cpp : This file contains the 'main' function. Program execution
// begins and ends there.

#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <utility>
#include <atomic>
#include <mutex>
#include <semaphore>
#include <chrono>

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

void prop(void) {
}

class Logger {
private:
    // friend void ::prop();
    // Globals used by all parties
    static std::atomic_long messageListSize;
    static std::queue<LogMessage> messagesToLog;
    static std::counting_semaphore<> dequeSema;
    static std::mutex messageListMutex;
    static std::vector<LogMessage> messageList;

    // This is the reader thead worker function.
    static void popMessages(void) { // Note: this thread never returns, it just sits and waits for messages.
        // Create the end desired log file (to where all the messages are being put)
        std::string filename = "LogFile_";
        filename.append(std::to_string(time(NULL)));
        filename.append(".txt");
        std::ofstream logFile(filename);

        // PUll each message out of the queue and tell the log file which thread sent the message.
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
        std::thread reader(popMessages);
        reader.detach();
    }

    // This is the writer thread worker function.
    static void pushMessage(LogLevel level, std::string msg) {  // Pushes the thread ID and the message into the queue.
        messageListSize++;
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
    Logger() {
        messageListSize = 0;

        spawnReader();
    }

    void debug(std::string msg) {
        std::thread t1(prop);
        // std::thread t1(pushMessage, DEBUG, msg);
        t1.detach();
    }

    void info(std::string msg) {
        std::thread t1(prop);
        // std::thread t1(pushMessage, DEBUG, msg);
        t1.detach();
    }

    void warn(std::string msg) {
        std::thread t1(prop);
        // std::thread t1(&Logger::pushMessage, DEBUG, msg);
        t1.detach();
    }

    void error(std::string msg) {
        std::thread t1(prop);
        // std::thread t1(&Logger::pushMessage, DEBUG, msg);
        t1.detach();
    }

    virtual ~Logger() {
        // flush();
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
