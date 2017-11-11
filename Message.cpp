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

/**
 * Realiza la copia desde el puntero src, hasta dst, indicando cuantos bytes se
 * quieren copiar. Mientras se realiza la copia, se busca en src un terminador 
 * 0. En caso de que no se encuentre se coloca en dst un terminador 0 al inicio.
 */ 
void static strcpy_s(void *dst, const void *src, uint8_t dstSize)
{
	#warning No implementado.
	//TODO tiene que hacer todo.
}

const uint8_t Message::TEXT_SIZE = 200;
const uint8_t Message::PASSWORD_SIZE = 200;
const uint8_t Message::WIFI_SSID_SIZE = 64;
const uint8_t Message::WIFI_PASSWORD_SIZE = 32;

struct Text {
	uint8_t brate;
	uint8_t srate;
	char text[Message::TEXT_SIZE];
}__attribute__((packed));

struct WifiConfig {
	char SSID[Message::WIFI_SSID_SIZE];
	char password[Message::WIFI_PASSWORD_SIZE];
	uint32_t ip;
	uint32_t subnetMask;
}__attribute__((packed));

struct Error {
	uint8_t errorCode;
}__attribute__((packed));

struct Password {
	char password[Message::PASSWORD_SIZE];
}__attribute__((packed));

struct BaseMessage {
	uint8_t version = Message::SUPPORTED_PROTOCOL_VERSION;
	uint8_t type;
	uint8_t signature[4] = {'A', 'N', 'R', 'S'};
	union {
		Text text;
		WifiConfig wifiConfig;
		Error error;
		Password password;
	};
}__attribute__((packed));

Message::Message(Type type)
: mType(type)
{
}

uint8_t Message::version() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->version;
}

Message::Type Message::type() const
{
	return mType;
}

const char *Message::text() const
{
	if (mType == GetTextResponse || mType == SetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.text;
	else
		return nullptr;
}

void Message::setText(const char *text)
{
	if (mType == GetTextResponse || mType == SetText)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->text.text, text, Message::TEXT_SIZE);
}

uint8_t Message::blinkRate() const
{
	if (mType == GetTextResponse || mType == SetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.brate;
	else
		return 0;
}

void Message::setBlinkRate(uint8_t brate)
{
	if (mType == GetTextResponse || mType == SetText)
		reinterpret_cast<BaseMessage*>(mRaw)->text.brate = brate;
}

uint8_t Message::slideRate() const
{
	if (mType == GetTextResponse || mType == SetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.srate;
	else
		return 0;
}

void Message::setSlideRate(uint8_t srate)
{
	if (mType == GetTextResponse || mType == SetText)
		reinterpret_cast<BaseMessage*>(mRaw)->text.srate = srate;
}

const char *Message::wifiSSID() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.SSID;
	else
		return nullptr;
}

void Message::setWiFiSSID(const char *str)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.SSID, str, Message::WIFI_SSID_SIZE);
}

const char *Message::wifiPassword() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.password;
	else
		return nullptr;
}

void Message::setWiFiPassword(const char *password)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.password, password, Message::WIFI_SSID_SIZE);
}

uint32_t Message::wifiIP() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.ip);
	else
		return 0;
}

void Message::setWiFiIP(uint32_t ip)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.ip = htonl(ip);
}

uint32_t Message::wifiSubnet() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.subnetMask);
	else
		return 0;
}

void Message::setWiFiSubnet(uint32_t mask)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.subnetMask = htonl(mask);
}

const char *Message::password() const
{
	if (mType == Auth)
		return reinterpret_cast<const BaseMessage*>(mRaw)->password.password;
	else
		return nullptr;
}

void Message::setPassword(const char *password)
{
	if (mType == Auth)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->password.password, password, Message::PASSWORD_SIZE);
}

Message::ErrorCode Message::errorCode() const
{
	if (mType == Invalid)
		return static_cast<ErrorCode>(reinterpret_cast<const BaseMessage*>(mRaw)->error.errorCode);
	else
		return static_cast<ErrorCode>(0);
}

void Message::setErrorCode(Message::ErrorCode errorCode)
{
	if (mType == Invalid) {
		uint8_t errorSource = static_cast<uint8_t>(errorCode);
		reinterpret_cast<const BaseMessage*>(mRaw)->error.errorCode = errorSource;
	}
}

// Request
Message Message::createAuthRequest(const char *password)
{
	Message msg(Auth);
	msg.setPassword(password);
	return msg;
}

Message Message::createSetTextRequest(uint8_t blinkRate, uint8_t slideRate, const char *text)
{
	Message msg(SetText);
	msg.setBlinkRate(blinkRate);
	msg.setSlideRate(slideRate);
	msg.setText(text);
	return msg;
}

Message Message::createGetTextRequest()
{
	Message msg(GetText);
	return msg;
}

Message Message::createSetWifiConfigRequest(const char *ssid, const char* password, uint32_t ip, uint32_t mask)
{
	Message msg(SetWiFiConfig);
	msg.setWiFiSSID(ssid);
	msg.setWiFiPassword(password);
	msg.setWiFiIP(ip);
	msg.setWiFiSubnet(mask);
	return msg;
}

Message Message::createGetWifiConfigRequest()
{
	Message msg(GetWiFiConfig);
	return msg;
}

Message Message::createSetPasswordRequest(const char *password)
{
	Message msg(SetPassword);
	msg.setPassword(password);
	return msg;
}

// Response
Message Message::createOKResponse()
{
	Message msg(OK);
	return msg;
}

Message Message::createInvalidResponse(ErrorCode errorCode)
{
	Message msg(Invalid);
	msg.setErrorCode(errorCode);
	return msg;
}

Message Message::createGetTextResponse(uint8_t blinkRate, uint8_t slideRate, const char *text)
{
	Message msg(GetTextResponse);
	msg.setBlinkRate(blinkRate);
	msg.setSlideRate(slideRate);
	msg.setText(text);
	return msg;
}

Message Message::createGetWiFiConfigResponse(const char *ssid, const char* password, uint32_t ip, uint32_t mask)
{
	Message msg(GetWiFiConfig);
	msg.setWiFiSSID(ssid);
	msg.setWiFiPassword(password);
	msg.setWiFiIP(ip);
	msg.setWiFiSubnet(mask);
	return msg;
}

Message Message::createMessage(const void *rawData)
{
	const BaseMessage *base = reinterpret_cast<const BaseMessage*>(rawData);
	
	if (base->version < SUPPORTED_PROTOCOL_VERSION)
		return createInvalidResponse(NoSupportedProtocol);
	if (base->signature[0] != 'A' || base->signature[1] != 'N' || base->signature[2] != 'R' || base->signature[3] != 'S')
		return createInvalidResponse(InvalidSignature);
	if (base->type < OK || base->type > SetPassword)
		return createInvalidResponse(InvalidType);
	
	switch(base->type) {
		case OK:
			return createOKResponse();
		case Invalid:
			if(base->error.errorCode < NoSupportedProtocol || base->error.errorCode > InvalidMessage)
				return createInvalidResponse(InvalidErrorCode);
			else
				return createInvalidResponse(static_cast<const ErrorCode>(base->error.errorCode));
		case Auth:
			return createAuthRequest(base->password.password);
		case GetText:
			return createGetTextRequest();
		case GetTextResponse:
			return createGetTextResponse(base->text.brate, base->text.srate, base->text.text);
		case SetText:
			return createSetTextRequest(base->text.brate, base->text.srate, base->text.text);
		case GetWiFiConfig:
			return createGetWifiConfigRequest();
		case GetWiFiConfigResponse:
			return createGetWiFiConfigResponse(base->wifiConfig.SSID, base->wifiConfig.password, base->wifiConfig.ip, base->wifiConfig.subnetMask);
		case SetWiFiConfig:
			return createSetWifiConfigRequest(base->wifiConfig.SSID, base->wifiConfig.password, base->wifiConfig.ip, base->wifiConfig.subnetMask);
		case SetPassword:
			return createSetPasswordRequest(base->password.password);
	}

	return createInvalidResponse(InvalidMessage);

}