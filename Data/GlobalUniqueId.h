
#pragma once

#include <chrono>

class GlobalUniqueId
{
public:
    bool Reset(uint64_t workerId, uint64_t datacenterId);

    uint64_t NextId();

private:
    GlobalUniqueId();
    GlobalUniqueId(const GlobalUniqueId&) = delete;
    GlobalUniqueId& operator=(const GlobalUniqueId&) = delete;

public:
    static GlobalUniqueId& Instance();

private:
    uint64_t GetWorkerId() const;

    uint64_t GetDataCenterId() const;

    uint64_t MillSecond() const noexcept;

    uint64_t WaitNextMillis(uint64_t last) const noexcept;

public:
    static constexpr uint64_t TWEPOCH = 1644290979702L;
    static constexpr uint64_t WORKER_ID_BITS = 5L;
    static constexpr uint64_t DATACENTER_ID_BITS = 5L;
    static constexpr uint64_t MAX_WORKER_ID = (1 << WORKER_ID_BITS) - 1;
    static constexpr uint64_t MAX_DATACENTER_ID = (1 << DATACENTER_ID_BITS) - 1;
    static constexpr uint64_t m_SequenceBITS = 12L;
    static constexpr uint64_t WORKER_ID_SHIFT = m_SequenceBITS;
    static constexpr uint64_t DATACENTER_ID_SHIFT = m_SequenceBITS + WORKER_ID_BITS;
    static constexpr uint64_t TIMESTAMP_LEFT_SHIFT = m_SequenceBITS + WORKER_ID_BITS + DATACENTER_ID_BITS;
    static constexpr uint64_t m_SequenceMASK = (1 << m_SequenceBITS) - 1;

private:
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    uint64_t m_StartMillsecond;
    uint64_t m_LastTimestamp = -1;
    uint64_t m_WorkerId = 0;
    uint64_t m_DataCenterId = 0;
    uint64_t m_Sequence = 0;
};

