#include <cstdint>

class Uhid {
    int _fd;
public:
    Uhid() noexcept(false);
    ~Uhid();

    void Create(const char* name, uint16_t vendorId, uint16_t productId, const uint8_t* hidDesc, int hidDescLen);
    void Destroy();
    void Send(const uint8_t* data, int dataLen);
};
