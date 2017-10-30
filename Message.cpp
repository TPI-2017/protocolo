#include "Message.h"
#include <string.h>


uint8_t strcpy_s(void *dst, const void *src, uint16_t dstSize)
{
	const char *csrc = reinterpret_cast<const char*>(src);
	char *cdst = reinterpret_cast<char*>(dst);

	while (dstSize && *csrc) {
		*(cdst++) = *(csrc++);
		dstSize--;
	}

	return !dstSize && *csrc;
}

struct Text {
	char text[252];
}__attribute__((packed));

struct AnimParams {
	uint8_t brate;
	uint8_t srate;
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
		AnimParams animParams;
	};
}__attribute__((packed));

struct BaseMessage {
	uint8_t version;
	uint8_t type;
	uint8_t size;
	union {
		Text text;
		WifiConfig wifiConfig;
		AnimParams animParams;
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
	return reinterpret_cast<const BaseMessage*>(mRaw)->text.text;
}

const char *Message::wifiSSID() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.SSID;
}

const char *Message::wifiPassword() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.password;
}

uint32_t Message::wifiIP() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.ip;
}

uint32_t Message::wifiSubnet() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.subnetMask;
}

uint8_t Message::blinkRate() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->animParams.brate;
}

uint8_t Message::slideRate() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->animParams.srate;
}

const char *Message::password() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->text.text;
}

// Setters
void Message::setWifiSSID(const char *str)
{
	strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.SSID, static_cast<const void*>(str), 64);
}

void Message::setWifiPassword(const char *password)
{
	strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.password, static_cast<const void*>(password), 32);
}

void Message::setWifiIP(uint32_t ip)
{
	reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.ip = ip;
}

void Message::setWifiSubnet(uint32_t mask)
{
	reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.subnetMask = mask;
}

void Message::setBlinkRate(uint8_t brate)
{
	reinterpret_cast<BaseMessage*>(mRaw)->animParams.brate = brate;
}

void Message::setSlideRate(uint8_t srate)
{
	reinterpret_cast<BaseMessage*>(mRaw)->animParams.srate = srate;
}

void Message::setPassword(const char *password)
{
	strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->text.text, static_cast<const void*>(password), sizeof(Text));
}
