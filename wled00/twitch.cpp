#include "wled.h"


#ifdef WLED_ENABLE_TWITCH

#include <IRCClient.h>
#include "twitchconfig.h"

// TODO: MOVE TO CONFIG FILE!!11!1111one


const String botName = "STREAMELEMENTS";

#define defaultPreset   16
#define eventNum        6
const uint8_t eventTime[] = {10, 10, 10, 30, 20, 20};

  
  //"Monaten dabei und immer noch nicht gelangweilt.",



uint8_t twitchNamewallTimeout = 15;
uint8_t normalBrightness = 0;
uint8_t highlightBrightness = 255;

const uint8_t names = 2;
String userNames[names] = {
  "JVPEEK",
  "LUIGIPLAYERTWO"
};

// local variables, do not eat
long alertTimeout = -1;
long lastSeen[names] = {1000, 1000};

#define IRC_SERVER   "irc.chat.twitch.tv"
#define IRC_PORT     6667
int lastConnectionAttempt = millis();

const String ircChannel = "#" + twitchChannelName;





WiFiClient wiFiClient;
IRCClient client(IRC_SERVER, IRC_PORT, wiFiClient);


void sendTwitchMessage(String message) {
  client.sendMessage(ircChannel, message);
}
float twitchGetMulti(uint8_t index) {
  uint8_t multiplier = 0;
  if (millis() < lastSeen[index] + (twitchNamewallTimeout * 1000)) {
    multiplier = constrain(map(millis(), lastSeen[index], (lastSeen[index] + (twitchNamewallTimeout * 1000)), highlightBrightness, normalBrightness), normalBrightness, highlightBrightness);
  }

  return (float)multiplier / 255.0;
}

void twitchBeforeDraw() {
  for (int userNum = 0; userNum < names; userNum++) {
    float multiplier = twitchGetMulti(userNum);

    for (int i = userNum * 9; i < (userNum * 9) + 9; i++) {
      uint32_t in = strip.getPixelColor(i); // time to get the colors for the individual fixtures as suggested by AirCoookie at issue #462
      byte w = in >> 24 & 0xFF;
      byte r = in >> 16 & 0xFF;
      byte g = in >> 8  & 0xFF;
      byte b = in       & 0xFF;

      strip.setPixelColor(i, r * (1.0 - multiplier) + 255 * multiplier, g * (1.0 - multiplier) + 255 * multiplier, b * (1.0 - multiplier) + 255 * multiplier, w * (1.0 - multiplier) + 255 * multiplier);
    }
  }
}
void twitchCallback(IRCMessage ircMessage) {
  if (ircMessage.command == "PRIVMSG" && ircMessage.text[0] != '\001') {
    String username = ircMessage.nick;
    username.toUpperCase();
    for (int i = 0; i < names; i++) {
      if (userNames[i] == username) {

        lastSeen[i] = millis();
      }
    }

    if (ircMessage.text == "!syscheck" && username == "JVPEEK") {
      sendTwitchMessage("/me Userwall is ready");

    }
    for (int i = 0; i < eventNum; i++) {
      if (ircMessage.text.indexOf(eventMessage[i]) > -1 && ((username == botName) || (username == "JVPEEK"))) {
        applyPreset(i + 1);
        colorUpdated(NOTIFIER_CALL_MODE_PRESET_CYCLE);
        alertTimeout = millis() + (eventTime[i] * 1000);
      }
    }
  }
}

void handleTwitch() {

  if (!client.connected()) {
    if (lastConnectionAttempt + 5000 < millis()) {
      lastConnectionAttempt = millis();
      // Attempt to connect
      // Second param is not needed by Twtich
      if (client.connect(TWITCH_BOT_NAME, "", TWITCH_OAUTH_TOKEN)) {
        client.sendRaw("JOIN " + ircChannel);
        sendTwitchMessage(" / me WLED ready to do WLED things");
      } else {

      }
      return;
    }

  }
  client.loop();
  // does things. i don't care. i'm drunk.
  if (alertTimeout != -1 && alertTimeout < millis()) {
    applyPreset(defaultPreset);
    colorUpdated(NOTIFIER_CALL_MODE_PRESET_CYCLE);
    alertTimeout = -1;
  }
}


void initTwitch() {
  client.setCallback(twitchCallback);
  //TODO: remove hard coded values
  strcpy(eventMessage[0], "Folgen");
  strcpy(eventMessage[1], "rausgehauen");
  strcpy(eventMessage[2], "Mitten auf den");
  strcpy(eventMessage[3], "jetzt den Laden hier");
  strcpy(eventMessage[4], "EINER VON UNS! EINER VON UNS!");
  
}

#else
void handleTwitch() {}
void initTwitch() {}
void twitchBeforeDraw() {}
#endif
