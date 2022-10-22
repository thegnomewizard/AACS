#include "Uhid.h"
#include <memory.h>
#include <linux/uhid.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

namespace {

class Requester {
    int _fd;
public:
    uhid_event ev;

    Requester(int fd)
    :_fd(fd)
    {
        memset(&ev, 0, sizeof(ev));
    }
    ~Requester() noexcept(false)
    {
        ssize_t ret = write(_fd, &ev, sizeof(ev));
        if (ret < 0)
            throw std::runtime_error("Cannot write to UHID driver");
        if (ret != sizeof(ev))
            throw std::runtime_error("Write to UHID driver size incorrect");
    }
};

}

Uhid::Uhid() noexcept(false)
{
    _fd = open("/dev/uhid", O_RDWR | O_CLOEXEC);
    if (_fd < 0)
        throw std::runtime_error("Failed to open UHID driver");
}

Uhid::~Uhid()
{
    close(_fd);
}

void Uhid::Create(const char* name, uint16_t vendorId, uint16_t productId, const uint8_t* hidDesc, int hidDescLen)
{
    Requester req(_fd);
    req.ev.type = UHID_CREATE2;
    strcpy((char*)req.ev.u.create2.name, name);
    memcpy(req.ev.u.create2.rd_data, hidDesc, hidDescLen);
    req.ev.u.create2.rd_size = hidDescLen;
    req.ev.u.create2.bus = BUS_USB;
    req.ev.u.create2.vendor = vendorId;
    req.ev.u.create2.product = productId;
}

void Uhid::Destroy()
{
    Requester req(_fd);
    req.ev.type = UHID_DESTROY;
}

void Uhid::Send(const uint8_t* data, int dataLen)
{
    Requester req(_fd);
    req.ev.type = UHID_INPUT;
    req.ev.u.input.size = dataLen;
    memcpy(req.ev.u.input.data, data, dataLen);
}
