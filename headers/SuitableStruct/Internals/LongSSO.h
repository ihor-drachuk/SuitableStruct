#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <utility>

namespace SuitableStruct {

using ExternalSSOBuffer = std::vector<uint8_t>;

template<size_t sso_limit = 80>
class LongSSO {
public:
    inline LongSSO() {
    }

    inline LongSSO(ExternalSSOBuffer& cache) {
        m_longBuf = &cache;
    }

    inline LongSSO(const LongSSO& rhs) {
        allocate_copy(rhs.size(), rhs.cdata());
    }

    inline LongSSO(LongSSO&& rhs) noexcept {
        *this = std::move(rhs);
    }

    inline ~LongSSO() {
        if (m_deleteLongBuf)
            delete m_longBuf;
    }

    inline LongSSO& operator=(const LongSSO& rhs) {
        if (this == &rhs) return *this;

        LongSSO temp(rhs);
        swap(temp);

        return *this;
    }

    inline LongSSO& operator=(LongSSO&& rhs) noexcept {
        if (this == &rhs) return *this;

        if (m_deleteLongBuf)
            delete m_longBuf;

        m_isShortBuf = rhs.m_isShortBuf;
        m_deleteLongBuf = rhs.m_deleteLongBuf;
        m_longBuf = rhs.m_longBuf;

        if (m_isShortBuf) {
            m_sz = rhs.m_sz;
            memcpy(m_buf, rhs.m_buf, m_sz);
        }

        rhs.m_deleteLongBuf = false;
        return *this;
    }

    inline operator bool() const { return size() != 0; }
    inline bool operator==(const LongSSO& rhs) {
        if (this == &rhs) return true;

        if (rhs.size() != size())
            return false;

        return (memcmp(data(), rhs.data(), size()) == 0);
    }
    inline bool operator!=(const LongSSO& rhs) { return !(*this == rhs); }

    inline uint8_t* allocate_copy(size_t sz, const uint8_t* buffer = nullptr) {
        if (m_isShortBuf) {
            // Short buffer
            size_t newSz = m_sz + sz;

            int64_t limitDelta = sso_limit;
            limitDelta -= newSz;

            if (limitDelta >= 0) {
                uint8_t* target = m_buf + m_sz;
                memcpy(target, buffer, buffer ? sz : 0); // It's 20x faster, than doing memcpy in `appendData`
                m_sz += sz;
                return target;
            } else {
                assert(sz > 0);
                assert(limitDelta < 0);
                auto target = makeLong(sz);
                memcpy(target, buffer, buffer ? sz : 0);
                return target;
            }
        } else {
            // Long buffer
            size_t oldSz = m_longBuf->size();
            size_t newSz = oldSz + sz;
            m_longBuf->resize(newSz);
            uint8_t* target = m_longBuf->data() + oldSz;
            memcpy(target, buffer, buffer ? sz : 0);
            return target;
        }
    }

    inline void appendData(const uint8_t* buffer, size_t sz) {
        allocate_copy(sz, buffer);
    }

    inline void reduceSize(size_t amount) {
        if (m_isShortBuf) {
            assert(m_sz >= amount || !"Not enough data!");
            m_sz -= amount;
        } else {
            const auto sz = m_longBuf->size();
            assert(sz >= amount || !"Not enough data!");
            m_longBuf->resize(sz - amount);
        }
    }

    inline uint8_t* data() { return reinterpret_cast<uint8_t*>(m_isShortBuf ? m_buf : m_longBuf->data()); }
    inline const uint8_t* data() const { return reinterpret_cast<const uint8_t*>(m_isShortBuf ? m_buf : m_longBuf->data()); }
    inline const uint8_t* cdata() const { return data(); }
    inline size_t size() const { return m_isShortBuf ? m_sz : (m_longBuf->size()); }

    static constexpr size_t getSsoLimit() { return sso_limit; }
    inline bool isShortBuf() const { return m_isShortBuf; }

    inline void clear() {
        m_sz = 0;
        m_buf[0] = 0;
        m_isShortBuf = true;
    }

private:
    inline uint8_t* makeLong(size_t addSz) {
        assert(m_isShortBuf);

        size_t newSz = m_sz + addSz;

        if (m_longBuf) {
            m_longBuf->resize(newSz);
        } else {
            m_longBuf = new ExternalSSOBuffer(newSz);
            m_deleteLongBuf = true;
        }

        memcpy(m_longBuf->data(), m_buf, m_sz);

        m_isShortBuf = false;

        return m_longBuf->data() + m_sz;
    }

    void swap(LongSSO& rhs) {
        std::swap(m_buf, rhs.m_buf);
        std::swap(m_sz, rhs.m_sz);
        std::swap(m_longBuf, rhs.m_longBuf);
        std::swap(m_isShortBuf, rhs.m_isShortBuf);
        std::swap(m_deleteLongBuf, rhs.m_deleteLongBuf);
    }

private:
    uint8_t m_buf[sso_limit];
    size_t m_sz { 0 };

    ExternalSSOBuffer* m_longBuf { nullptr };

    bool m_isShortBuf { true };
    bool m_deleteLongBuf { false };
};

} // namespace SuitableStruct
