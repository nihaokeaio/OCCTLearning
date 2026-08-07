#include "GlobalUniqueId.h"

GlobalUniqueId& GlobalUniqueId::Instance()
{
    static GlobalUniqueId instance;
    return instance;
}


GlobalUniqueId::GlobalUniqueId()
{
    m_StartTimepoint = std::chrono::steady_clock::now();
    m_StartMillsecond = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

bool GlobalUniqueId::Reset(uint64_t workerId, uint64_t datacenterId)
{
    if (workerId > MAX_WORKER_ID)
    {
        printf("worker Id can't be greater than 31 or less than 0\n");
        return false;
    }

    if (datacenterId > MAX_DATACENTER_ID)
    {
        printf("datacenter Id can't be greater than 31 or less than 0\n");
        return false;
    }

    m_WorkerId = workerId;
    m_DataCenterId = datacenterId;
    return true;
}

uint64_t GlobalUniqueId::NextId()
{
    //std::chrono::steady_clock  cannot decrease as physical time moves forward
    auto timestamp = MillSecond();
    if (m_LastTimestamp == timestamp)
    {
        m_Sequence = (m_Sequence + 1) & m_SequenceMASK;
        if (m_Sequence == 0)
        {
            timestamp = WaitNextMillis(m_LastTimestamp);
        }
    }
    else
    {
        m_Sequence = 0;
    }

    m_LastTimestamp = timestamp;

    return ((timestamp - TWEPOCH) << TIMESTAMP_LEFT_SHIFT)
        | (m_DataCenterId << DATACENTER_ID_SHIFT)
        | (m_WorkerId << WORKER_ID_SHIFT)
        | m_Sequence;
}

uint64_t GlobalUniqueId::GetWorkerId() const
{
    return m_WorkerId;
}

uint64_t GlobalUniqueId::GetDataCenterId() const
{
    return m_DataCenterId;
}

uint64_t GlobalUniqueId::MillSecond() const noexcept
{
    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_StartTimepoint);
    return m_StartMillsecond + diff.count();
}

uint64_t GlobalUniqueId::WaitNextMillis(uint64_t last) const noexcept
{
    auto timestamp = MillSecond();
    while (timestamp <= last)
    {
        timestamp = MillSecond();
    }
    return timestamp;
}
