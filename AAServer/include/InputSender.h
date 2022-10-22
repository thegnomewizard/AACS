#include "Uhid.h"

class InputSender {
    Uhid _driver;
    int _width, _height;
public:
    InputSender();

    void SetScreenSize(int width, int height);
    void SendEvent(bool active, int x, int y);
};
