//
// Created by ZQD on 25-9-2.
//

#ifndef ELEMENT_H
#define ELEMENT_H
#include <AIS_Shape.hxx>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <process.h>
#include <Standard_Handle.hxx>
#include <thread>

using  ElementId=uint64_t;

class UniqueIDGenerator {
private:
    using TimePoint = std::chrono::system_clock::time_point;
    static constexpr TimePoint kEpoch = TimePoint(std::chrono::milliseconds(1704067200000));  // 2024-01-01 00:00:00 UTC
    std::atomic<uint64_t> last_timestamp_ = 0;  // 上次生成ID的时间戳（微秒级）
    std::atomic<uint16_t> counter_ = 0;         // 同一时间戳内的计数器（12位，最大值4095）
    uint16_t process_id_;                 // 进程ID（12位，最大值4095）

public:
    UniqueIDGenerator(){
        // 获取进程ID并截断为12位（仅保留低12位，避免ID冲突）
#ifdef _WIN32
        process_id_ = static_cast<uint16_t>(_getpid() & 0x0FFF); // Windows：_getpid() 返回进程ID
#else
        process_id_ = static_cast<uint16_t>(getpid() & 0x0FFF);   // Linux/macOS：getpid()
#endif
    }

    // 生成64位唯一ID
    uint64_t generate() {
        // 1. 获取当前微秒级时间戳（相对于自定义Epoch）
        const auto now = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - kEpoch);
        uint64_t current_timestamp = duration.count();

        // 2. 原子更新计数器（同一时间戳内自增，跨时间戳重置）
        uint64_t last_ts = last_timestamp_.load(std::memory_order_relaxed);
        if (current_timestamp == last_ts) {
            // 同一时间戳：计数器自增，若超过4095则等待下一微秒（理论上极少见）
            counter_.fetch_add(1, std::memory_order_relaxed);
            if (counter_.load(std::memory_order_relaxed) > 0x0FFF) {
                // 极端情况：同一微秒内生成超过4096个ID，等待下一微秒
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                return generate();  // 递归重试（重新获取时间戳）
            }
        } else {
            // 新时间戳：重置计数器为0
            counter_.store(0, std::memory_order_relaxed);
            last_timestamp_.store(current_timestamp, std::memory_order_relaxed);
        }

        // 3. 组合64位ID：[时间戳(40位)] | [进程ID(12位)] | [计数器(12位)]
        const uint64_t id =
            (current_timestamp << 24) |           // 时间戳（左移24位，让出后24位给进程ID+计数器）
            (static_cast<uint64_t>(process_id_) << 12) |  // 进程ID（左移12位，让出后12位给计数器）
            static_cast<uint64_t>(counter_.load(std::memory_order_relaxed));  // 计数器

        return id;
    }
};

class Element {
public:
    explicit Element(const ElementId& id):m_Id(id){}

    ElementId GetId() const {return m_Id;}

    bool operator==(const Element & e) const {
        return m_Id==e.GetId();
    }
private:
    ElementId m_Id;
};

// 特化 std::hash<MyType>，提供哈希函数
namespace std {
    template<> struct hash<Element> {
        size_t operator()(const Element& obj) const {
            return std::hash<ElementId>{}(obj.GetId());  // 组合哈希值（避免冲突）
        }
    };
}


#endif //ELEMENT_H
