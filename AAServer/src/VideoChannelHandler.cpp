// Distributed under GPLv3 only as specified in repository's root LICENSE file

#include "VideoChannelHandler.h"
#include "ChannelHandler.h"
#include "enums.h"
#include "utils.h"
#include <boost/range/algorithm/max_element.hpp>
#include <iostream>

#include <linux/types.h>

using namespace std;

VideoChannelHandler::VideoChannelHandler(uint8_t channelId)
    : ChannelHandler(channelId) {
  cout << "VideoChannelHandler: " << (int)channelId << endl;
  channelOpened = false;

  vidCapture.start([=](int64_t pts, const uint8_t* buf, int size){
    static bool firstSample = true;
    vector<uint8_t> msgToHeadunit;
    if (firstSample) {
      openChannel();
      firstSample = false;
    }
//    pts=-1;
    if (pts == -1) {
      pushBackInt16(msgToHeadunit, MediaMessageType::MediaIndication);
    } else {
      pushBackInt16(msgToHeadunit,
                    MediaMessageType::MediaWithTimestampIndication);
      pushBackInt64(msgToHeadunit, pts);
    }
    copy(buf, buf + size, back_inserter(msgToHeadunit));
//    printf("Sending %i bytes of video\n", msgToHeadunit.size());
    sendToHeadunit(channelId,
                  EncryptionType::Encrypted | FrameType::Bulk,
                  msgToHeadunit);
  });
}

VideoChannelHandler::~VideoChannelHandler() {}

void VideoChannelHandler::openChannel() {
  channelOpened = true;
  ChannelHandler::openChannel();
  gotSetupResponse = false;
  sendSetupRequest();
  expectSetupResponse();
}

void VideoChannelHandler::disconnected(int clientId) {
}

void VideoChannelHandler::sendSetupRequest() {
  std::vector<uint8_t> plainMsg;
  pushBackInt16(plainMsg, MediaMessageType::SetupRequest);
  plainMsg.push_back(0x08);
  plainMsg.push_back(0x03);
  sendToHeadunit(channelId, FrameType::Bulk | EncryptionType::Encrypted,
                 plainMsg);
}

void VideoChannelHandler::expectSetupResponse() {
  std::unique_lock<std::mutex> lk(m);
  cv.wait(lk, [=] { return gotSetupResponse; });
}

void VideoChannelHandler::sendStartIndication() {
  std::vector<uint8_t> plainMsg;
  pushBackInt16(plainMsg, MediaMessageType::StartIndication);
  plainMsg.push_back(0x08);
  plainMsg.push_back(0x00);
  plainMsg.push_back(0x10);
  plainMsg.push_back(0x00);
  sendToHeadunit(channelId, FrameType::Bulk | EncryptionType::Encrypted,
                 plainMsg);
}

bool VideoChannelHandler::handleMessageFromHeadunit(const Message &message) {
  if (!channelOpened) {
    ChannelHandler::sendToClient(-1, message.channel,
                                 message.flags & MessageTypeFlags::Specific,
                                 message.content);
    return true;
  }
  if (ChannelHandler::handleMessageFromHeadunit(message))
    return true;
  bool messageHandled = false;
  {
    std::unique_lock<std::mutex> lk(m);
    auto msg = message.content;
    const __u16 *shortView = (const __u16 *)(msg.data());
    auto messageType = be16_to_cpu(shortView[0]);
    if (messageType == MediaMessageType::SetupResponse) {
      gotSetupResponse = true;
      messageHandled = true;
    } else if (messageType == MediaMessageType::VideoFocusIndication) {
      sendStartIndication();
      messageHandled = true;
    } else if (messageType == MediaMessageType::MediaAckIndication) {
      messageHandled = true;
    }
  }
  cv.notify_all();
  return messageHandled;
}

bool VideoChannelHandler::handleMessageFromClient(int clientId,
                                                  uint8_t channelId,
                                                  bool specific,
                                                  const vector<uint8_t> &data) {
  // Video is routed through snowmix
  return false;
}
