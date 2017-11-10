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

	if (dst == nullptr)
		return 0;

	if (dstSize)
		dstSize--;

	while (dstSize && csrc != nullptr && *csrc) {
		*(cdst++) = *(csrc++);
		dstSize--;
	}

	*cdst = 0;

	return !dstSize && *csrc;
}

uint16_t static memcpy_s(void *dst,
		uint16_t dstSize,
		const void *src,
		uint16_t srcSize)
{
	const char *csrc = reinterpret_cast<const char*>(src);
	char *cdst = reinterpret_cast<char*>(dst);

	if (!dst)
		return 0;

	while (dstSize-- && srcSize-- && src)
		*(cdst++) = *(csrc++);

	return srcSize;
}

uint16_t static strnlen_s(const char *str, uint16_t maxSize)
{
	uint16_t length = 0;
	while (*(str++) && maxSize--)
		length++;

	return length;
}

bool isStringValid(const char *str, uint8_t size)
{
	if (size)
		size--;

	while (*str && size) {
		str++;
		size--;
	}

	return !size && *str == 0;
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
	uint8_t statusCode;
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
	repair();
}

void Message::setStatusCode(enum StatusCode statusCode)
{
	reinterpret_cast<BaseMessage*>(mRaw)->statusCode = static_cast<uint8_t>(statusCode);
}

Message::StatusCode Message::statusCode() const
{

	return static_cast<StatusCode>(reinterpret_cast<const BaseMessage*>(mRaw)->statusCode);
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
	if (mType == SetText || mType == GetText)
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
	if (mType == SetWifiConfig || mType == GetWifiConfig)
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
	if (mType == SetWifiConfig || mType == GetWifiConfig)
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
	if (mType == SetWifiConfig || mType == GetWifiConfig)
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
	if (mType == SetWifiConfig || mType == GetWifiConfig)
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
	if (mType == SetAnimationParameters || mType == GetAnimationParameters)
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
	if (mType == SetAnimationParameters || mType == GetAnimationParameters)
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

uint16_t Message::addRawData(const void *raw, uint16_t dim)
{
	if (dim > BufferSize)
		return 0;

	if (dim + mBufferDim > BufferSize)
		dim -= mBufferDim;

	memcpy_s(mRaw + mBufferDim, mBufferDim, raw, BufferSize);

	return dim;
}

void Message::updateSize()
{
	BaseMessage *ptr = reinterpret_cast<BaseMessage*>(mRaw);
	uint8_t size;

	switch (mType) {
	case Auth:
	case SetText:
	case GetText:
		size = strnlen_s(ptr->text.text, sizeof(Text)) + 1;
		break;
	case SetAnimationParameters:
	case GetAnimationParameters:
		size = sizeof(AnimParams);
		break;
	case SetWifiConfig:
	case GetWifiConfig:
		size = sizeof(WifiConfig);
		break;
	default:
		size = 0;
		break;
	}
	
	setSize(size);
	mBufferDim = HeaderSize + size;
}

void Message::setSize(uint8_t size)
{
	reinterpret_cast<BaseMessage*>(mRaw)->size = size;
}

void Message::setType(Type type)
{
	reinterpret_cast<BaseMessage*>(mRaw)->type = type;
}

// Request
Message Message::createAuthRequest(const char *str)
{
	Message msg(Auth);
	msg.setText(str);
	msg.updateSize();
	return msg;
}

Message Message::createSetTextRequest(const char *str)
{
	Message msg(SetText);
	msg.setText(str);
	msg.updateSize();
	return msg;
}

Message Message::createGetTextRequest()
{
	Message msg(GetText);
	msg.updateSize();
	return msg;
}

Message Message::createSetWifiConfigRequest(const char *ssid, const char* password, uint32_t ip, uint32_t mask)
{
	Message msg(SetWifiConfig);
	msg.setWifiSSID(ssid);
	msg.setWifiPassword(password);
	msg.setWifiIP(ip);
	msg.setWifiSubnet(mask);
	msg.updateSize();
	return msg;
}

Message Message::createGetWifiConfigRequest()
{
	Message msg(GetWifiConfig);
	msg.updateSize();
	return msg;
}

Message Message::createSetAnimationParametersRequest(uint8_t blinkRate, uint8_t slideRate)
{
	Message msg(SetAnimationParameters);
	msg.setBlinkRate(blinkRate);
	msg.setSlideRate(slideRate);
	msg.updateSize();
	return msg;
}

Message Message::createGetAnimationParametersRequest()
{
	Message msg(GetAnimationParameters);
	msg.updateSize();
	return msg;
}


// Response
Message Message::createAuthResponse(StatusCode statusCode)
{
	Message msg(Auth);
	msg.setStatusCode(statusCode);
	msg.updateSize();
	return msg;
}

Message Message::createSetTextResponse(StatusCode statusCode)
{
	Message msg(SetText);
	msg.setStatusCode(statusCode);
	msg.updateSize();
	return msg;
}

Message Message::createGetTextResponse(StatusCode statusCode, const char *str)
{
	Message msg(GetText);
	msg.setStatusCode(statusCode);
	msg.setText(str);
	msg.updateSize();
	return msg;
}

Message Message::createSetWifiConfigResponse(StatusCode statusCode)
{
	Message msg(SetWifiConfig);
	msg.setStatusCode(statusCode);
	msg.updateSize();
	return msg;
}

Message Message::createGetWifiConfigResponse(StatusCode statusCode, const char *ssid, const char* password, uint32_t ip, uint32_t mask)
{
	Message msg(GetWifiConfig);
	msg.setStatusCode(statusCode);
	msg.setWifiSSID(ssid);
	msg.setWifiPassword(password);
	msg.setWifiIP(ip);
	msg.setWifiSubnet(mask);
	msg.updateSize();
	return msg;
}

Message Message::createSetAnimationParametersResponse(StatusCode statusCode)
{
	Message msg(SetAnimationParameters);
	msg.setStatusCode(statusCode);
	msg.updateSize();
	return msg;
}

Message Message::createGetAnimationParametersResponse(StatusCode statusCode, uint8_t blinkRate, uint8_t slideRate)
{
	Message msg(GetAnimationParameters);
	msg.setStatusCode(statusCode);
	msg.setBlinkRate(blinkRate);
	msg.setSlideRate(slideRate);
	msg.updateSize();
	return msg;
}

void Message::repair()
{
	const BaseMessage* base = reinterpret_cast<const BaseMessage*>(mRaw);

	if (statusCode() == Request) {
		switch (mType) {
		case Auth:
		case SetText:
			if (!isStringValid(base->text.text, size()))
				setType(NoType);
			break;
		case SetWifiConfig:
			if (!isStringValid(base->wifiConfig.SSID, 64) || !isStringValid(base->wifiConfig.password, 32))
				setType(NoType);
			break;
		case default:
			break;
		}
	} else if (statusCode() == ResponseOK) {
		switch (mType) {
		case GetText:
			if (!isStringValid(base->text.text, size()))
				setType(NoType);
			break;
		case GetWifiConfig:
			if (!isStringValid(base->wifiConfig.SSID, 64) || !isStringValid(base->wifiConfig.password, 32))
				setType(NoType);
			break;
		case default:
			break;
		}
	} else {

	}
}
