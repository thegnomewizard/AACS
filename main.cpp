// Distributed under GPLv3 only as specified in repository's root LICENSE file

#include "AaCommunicator.h"
#include "Library.h"
#include "ManualResetEvent.h"
#include "ModeSwitcher.h"
#include "Udc.h"
#include <iostream>

using namespace std;

string configFsBasePath = "/sys/kernel/config";

int main() {
  try {
    ManualResetEvent mre;
    Library lib(configFsBasePath);
    ModeSwitcher::handleSwitchToAccessoryMode(lib);
    AaCommunicator aac(lib);
    aac.setup(Udc::getUdcById(lib, 0));
    aac.error.connect([&](std::exception &ex) {
      cout << "Exception:" << endl;
      cout << ex.what() << endl;
      mre.set();
    });
    mre.wait();
  } catch (exception &ex) {
    cout << "Exception:" << endl;
    cout << ex.what() << endl;
    return 1;
  }
  return 0;
}
