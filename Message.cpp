#include "Message.h"

struct Text {
	char text[252];
}__attribute__((packed));

struct WifiConfig {
	char SSID[64];
	char password[64];
	uint32_t ip;
	uint32_t subnetMask;
}__attribute__((packed));

struct ServerResponse {
	uint8_t responseCode;
	union {
		WifiConfig wifiConfig;
		Text text;
	};
}__attribute__((packed));

struct BaseMessage {
	uint8_t version;
	uint8_t type;
	uint8_t size;
	union {
		Text text;
		WifiConfig wifiConfig;
		ServerResponse response;
	};
}__attribute__((packed));

Message::Message(const void *raw)
{
}

Message::Message(Type type)
: mType(type)
{
}

void Message::prepare()
{
	#warning Not implemented.
}

// Getters
const char *Message::text() const
{
	#warning Not implemented.
}

const char *Message::wifiSSID() const
{
	#warning Not implemented.
}

const char *Message::wifiPassword() const
{
	#warning Not implemented.
}

uint32_t Message::wifiIP() const
{
	#warning Not implemented.
}

uint32_t Message::wifiSubnet() const
{
	#warning Not implemented.
}

uint8_t Message::blinkRate() const
{
	#warning Not implemented.
}

uint8_t Message::slideRate() const
{
	#warning Not implemented.
}

const char *Message::password() const
{
	#warning Not implemented.
}

// Setters
void setWifiSSID(const char *str)
{
	#warning Not implemented.
}

void setWifiPassword(const char *password)
{
	#warning Not implemented.
}

void setWifiIP(uint32_t ip)
{
	#warning Not implemented.
}

void setWifiSubnet(uint32_t mask)
{
	#warning Not implemented.
}

void setBlinkRate(uint8_t brate)
{
	#warning Not implemented.
}

void setSlideRate(uint8_t srate)
{
	#warning Not implemented.
}

void setPassword(const char *password)
{
	#warning Not implemented.
}
