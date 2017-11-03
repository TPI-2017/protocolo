#include "Message.h"
#include <string.h>

#if WIN32
#include <winsock2.h>
#elif POSIX
#include <arpa/inet.h>
#elif ESP
#include "network.h"
#else
	#warning No endianness conversion functions
#endif

uint8_t static strcpy_s(void *dst, const void *src, uint16_t dstSize)
{
	const char *csrc = reinterpret_cast<const char*>(src);
	char *cdst = reinterpret_cast<char*>(dst);

	while (dstSize && *csrc) {
		*(cdst++) = *(csrc++);
		dstSize--;
	}

	return !dstSize && *csrc;
}

uint16_t static memcpy_s(void *dst,
		uint16_t dstSize,
		const void *src,
		uint16_t srcSize)
{
	const char *csrc = reinterpret_cast<const char*>(src);
	char *cdst = reinterpret_cast<char*>(dst);

	while (dstSize-- && srcSize--)
		*(cdst++) = *(csrc++);

	return srcSize;
}

uint16_t static strnlen_s(const char *str, uint16_t maxSize)
{
	uint16_t length = 0;
	while (*str && maxSize--)
		length++;

	return length;
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
	char password[32];
	uint32_t ip;
	uint32_t subnetMask;
}__attribute__((packed));

uint8_t const Message::HeaderSize = 4;
uint8_t const Message::MinimumMessageSize = HeaderSize;
uint8_t const Message::MaximumMessageSize = BufferSize - MinimumMessageSize;
uint8_t const Message::SupportedProtocolVersion = 1;

struct BaseMessage {
	uint8_t version;
	uint8_t type;
	uint8_t size;
	uint8_t responseCode;
	union {
		Text text;
		WifiConfig wifiConfig;
		AnimParams animParams;
	};
}__attribute__((packed));

Message::Message(Type type)
: mType(type),
  mBufferDim(0)
{
}

Message::Message(const void *raw, uint16_t dim)
: mBufferDim(dim)
{
	memcpy_s(mRaw, BufferSize, raw, dim);
	const BaseMessage* base = reinterpret_cast<const BaseMessage*>(mRaw);
	mType = static_cast<Type>(base->type);
}

void Message::setResponseCode(enum ResponseCode responseCode)
{
	reinterpret_cast<BaseMessage*>(mRaw)->responseCode = static_cast<uint8_t>(responseCode);
}

Message::ResponseCode Message::responseCode() const
{

	return static_cast<ResponseCode>(reinterpret_cast<const BaseMessage*>(mRaw)->responseCode);
}

const char *Message::text() const
{
	if (mType == SetText || mType == GetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.text;
	else
		return nullptr;
}

void Message::setText(const char *text)
{
	if (mType == SetText)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->text.text, text, sizeof(Text));
}

const char *Message::wifiSSID() const
{
	if (mType == SetWifiConfig || mType == GetWifiConfig)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.SSID;
	else
		return nullptr;
}

void Message::setWifiSSID(const char *str)
{
	if (mType == SetWifiConfig)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.SSID, str, 64);
}

const char *Message::wifiPassword() const
{
	if (mType == SetWifiConfig || mType == GetWifiConfig)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.password;
	else
		return nullptr;
}

void Message::setWifiPassword(const char *password)
{
	if (mType == SetWifiConfig)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.password, password, 32);
}

uint32_t Message::wifiIP() const
{
	if (mType == SetWifiConfig || mType == GetWifiConfig)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.ip);
	else
		return 0;
}

void Message::setWifiIP(uint32_t ip)
{
	if (mType == SetWifiConfig)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.ip = htonl(ip);
}

uint32_t Message::wifiSubnet() const
{
	if (mType == SetWifiConfig || mType == GetWifiConfig)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.subnetMask);
	else
		return 0;
}

void Message::setWifiSubnet(uint32_t mask)
{
	if (mType == SetWifiConfig)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.subnetMask = htonl(mask);
}

uint8_t Message::blinkRate() const
{
	if (mType == SetAnimationParameters || mType == GetAnimationParameters)
		return reinterpret_cast<const BaseMessage*>(mRaw)->animParams.brate;
	else
		return 0;
}

void Message::setBlinkRate(uint8_t brate)
{
	if (mType == SetAnimationParameters)
		reinterpret_cast<BaseMessage*>(mRaw)->animParams.brate = brate;
}

uint8_t Message::slideRate() const
{
	if (mType == SetAnimationParameters || mType == GetAnimationParameters)
		return reinterpret_cast<const BaseMessage*>(mRaw)->animParams.srate;
	else
		return 0;
}

void Message::setSlideRate(uint8_t srate)
{
	if (mType == SetAnimationParameters)
		reinterpret_cast<BaseMessage*>(mRaw)->animParams.srate = srate;
}

const char *Message::password() const
{
	if (mType == Auth)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.text;
	else
		return nullptr;
}

void Message::setPassword(const char *password)
{
	if (mType == Auth)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->text.text, password, sizeof(Text));
}

uint8_t Message::size() const
{
	if (mBufferDim < MinimumMessageSize)
		return 0;
	else
		return reinterpret_cast<const BaseMessage*>(mRaw)->size;
}

void Message::prepare()
{
	computeSize();
}

uint16_t Message::addRawData(const void *raw, uint16_t dim)
{
	if (dim > BufferSize)
		return 0;

	if (dim + mBufferDim > BufferSize)
		dim -= mBufferDim;

	memcpy_s(mRaw + mBufferDim, mBufferDim, raw, BufferSize);

	return dim;
}

void Message::computeSize()
{
	BaseMessage *ptr = reinterpret_cast<BaseMessage*>(mRaw);
	switch (mType) {
	case Auth:
	case SetText:
	case GetText:
		setSize(HeaderSize + strnlen_s(ptr->text.text, sizeof(Text)));
		break;
	case SetAnimationParameters:
	case GetAnimationParameters:
		setSize(HeaderSize + sizeof(AnimParams));
		break;
	case SetWifiConfig:
	case GetWifiConfig:
		setSize(HeaderSize + sizeof(WifiConfig));
		break;
	}
}

void Message::setSize(uint8_t size)
{
	reinterpret_cast<BaseMessage*>(mRaw)->size = size;
}
