// Distributed under GPLv3 only as specified in repository's root LICENSE file

#pragma once

#include "ChannelHandler.h"
#include "VidCapture.h"

class VideoChannelHandler : public ChannelHandler {
  bool gotSetupResponse;
  std::mutex m;
  std::condition_variable cv;
  bool channelOpened;

  void sendSetupRequest();
  void expectSetupResponse();
  void sendStartIndication();

  VidCapture vidCapture;

  void openChannel();

public:
  VideoChannelHandler(uint8_t channelId);
  virtual void disconnected(int clientId);
  virtual bool handleMessageFromHeadunit(const Message &message);
  virtual bool handleMessageFromClient(int clientId, uint8_t channelId,
                                       bool specific,
                                       const std::vector<uint8_t> &data);
  virtual ~VideoChannelHandler();
};
