#include "Message.h"
#include <string.h>


#if WIN32
	#include <winsock2.h>
#elif POSIX
	#include <arpa/inet.h>
#elif ESP
	#include "../network.h"
#else
	#warning No endianness conversion functions
#endif


// Realiza la copia desde el puntero src, hasta dst, indicando cuantos bytes se
// quieren copiar. Mientras se realiza la copia, se busca en src un terminador 
// 0. En caso de que no se encuentre se coloca en dst un terminador 0 al inicio.
void static strcpy_s(void *dst, const void *src, uint8_t dstSize)
{
	const char *csrc = reinterpret_cast<const char*>(src);
	char *cdst = reinterpret_cast<char*>(dst);

	if (dstSize)
		dstSize--;

	while (*csrc && dstSize) {
		*(cdst++) = *(csrc++);
		dstSize--;
	}

	if (*csrc)
		reinterpret_cast<char*>(dst)[0] = '\0';
}


// Constantes internas. =======================================================

const uint8_t INTERNAL_TEXT_SIZE = Message::TEXT_SIZE + 1;
const uint8_t INTERNAL_PASSWORD_SIZE = Message::PASSWORD_SIZE + 1;
const uint8_t INTERNAL_WIFI_SSID_SIZE = Message::WIFI_SSID_SIZE + 1;
const uint8_t INTERNAL_WIFI_PASSWORD_SIZE = Message::WIFI_PASSWORD_SIZE + 1;


// Estructuras internas. ======================================================

struct text_t {
	uint8_t brate;
	uint8_t srate;
	char text[INTERNAL_TEXT_SIZE];
}__attribute__((packed));

struct wifiConfig_t {
	char SSID[INTERNAL_WIFI_SSID_SIZE];
	char password[INTERNAL_WIFI_PASSWORD_SIZE];
	uint32_t ip;
	uint32_t subnetMask;
}__attribute__((packed));

struct responseCode_t {
	uint8_t responseCode;
}__attribute__((packed));

struct newPassword_t {
	char newPassword[INTERNAL_PASSWORD_SIZE];
}__attribute__((packed));

struct BaseMessage {
	uint8_t version = Message::SUPPORTED_PROTOCOL_VERSION;
	uint8_t type;
	uint8_t signature[4] = {'A', 'N', 'R', 'S'};
	char password[INTERNAL_PASSWORD_SIZE];
	union {
		text_t text;
		wifiConfig_t wifiConfig;
		responseCode_t resposeCode;
		newPassword_t newPassword;
	};
}__attribute__((packed));


// Constructores. =============================================================

Message::Message()
: mEmpty(true)
{
}

Message::Message(Type type)
: mType(type),
  mEmpty(false)
{
	BaseMessage *base = reinterpret_cast<BaseMessage*>(mRaw);
	base->version = Message::SUPPORTED_PROTOCOL_VERSION;
	base->signature[0] = 'A';
	base->signature[1] = 'N';
	base->signature[2] = 'R';
	base->signature[3] = 'S';
	base->type = type;
}


// Getters. ===================================================================

bool Message::empty() const
{
	return mEmpty;
}

uint8_t Message::version() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->version;
}

Message::Type Message::type() const
{
	return mType;
}

const char *Message::password() const
{
	if (mType >= Auth && mType < GenericResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->password;
	else
		return nullptr;
}

const char *Message::newPassword() const
{
	if (mType == SetPassword)
		return reinterpret_cast<const BaseMessage*>(mRaw)->newPassword.newPassword;
	else
		return nullptr;
}

uint8_t Message::blinkRate() const
{
	if (mType == GetTextResponse || mType == SetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.brate;
	else
		return 0;
}

uint8_t Message::slideRate() const
{
	if (mType == GetTextResponse || mType == SetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.srate;
	else
		return 0;
}

const char *Message::text() const
{
	if (mType == GetTextResponse || mType == SetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.text;
	else
		return nullptr;
}

const char *Message::wifiSSID() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.SSID;
	else
		return nullptr;
}

const char *Message::wifiPassword() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.password;
	else
		return nullptr;
}

uint32_t Message::wifiIP() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.ip);
	else
		return 0;
}

uint32_t Message::wifiSubnet() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.subnetMask);
	else
		return 0;
}

uint8_t Message::responseCode() const
{
	if (mType == GenericResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->resposeCode.responseCode;
	else
		return 0;
}

const void *Message::data() const
{
	return mRaw;
}


// Setters ====================================================================

void Message::setPassword(const char *password)
{
	if (mType >= Auth && mType < GenericResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->password, password, INTERNAL_PASSWORD_SIZE);
}

void Message::setNewPassword(const char *newPassword)
{
	if (mType == SetPassword)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->newPassword.newPassword, newPassword, INTERNAL_PASSWORD_SIZE);
}

void Message::setBlinkRate(uint8_t brate)
{
	if (mType == GetTextResponse || mType == SetText)
		reinterpret_cast<BaseMessage*>(mRaw)->text.brate = brate;
}

void Message::setSlideRate(uint8_t srate)
{
	if (mType == GetTextResponse || mType == SetText)
		reinterpret_cast<BaseMessage*>(mRaw)->text.srate = srate;
}

void Message::setText(const char *text)
{
	if (mType == GetTextResponse || mType == SetText)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->text.text, text, INTERNAL_TEXT_SIZE);
}

void Message::setWiFiSSID(const char *ssid)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.SSID, ssid, INTERNAL_WIFI_SSID_SIZE);
}

void Message::setWiFiPassword(const char *password)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.password, password, INTERNAL_WIFI_SSID_SIZE);
}

void Message::setWiFiIP(uint32_t ip)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.ip = htonl(ip);
}

void Message::setWiFiSubnet(uint32_t mask)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.subnetMask = htonl(mask);
}

void Message::setResponseCode(uint8_t responseCode)
{
	if (mType == GenericResponse) {
		reinterpret_cast<BaseMessage*>(mRaw)->resposeCode.responseCode = responseCode;
	}
}


// Request. ===================================================================

Message Message::createAuthRequest(const char *password)
{
	Message msg(Auth);
	msg.setPassword(password);
	return msg;
}

Message Message::createSetPasswordRequest(const char *password, const char *newPassword)
{
	Message msg(SetPassword);
	msg.setPassword(password);
	msg.setNewPassword(newPassword);
	return msg;
}

Message Message::createGetTextRequest(const char *password)
{
	Message msg(GetText);
	msg.setPassword(password);
	return msg;
}

Message Message::createSetTextRequest(const char *password, uint8_t blinkRate, uint8_t slideRate, const char *text)
{
	Message msg(SetText);
	msg.setPassword(password);
	msg.setBlinkRate(blinkRate);
	msg.setSlideRate(slideRate);
	msg.setText(text);
	return msg;
}

Message Message::createGetWifiConfigRequest(const char *password)
{
	Message msg(GetWiFiConfig);
	msg.setPassword(password);
	return msg;
}

Message Message::createSetWifiConfigRequest(const char *password, const char *ssid, const char* wifiPassword, uint32_t ip, uint32_t mask)
{
	Message msg(SetWiFiConfig);
	msg.setPassword(password);
	msg.setWiFiSSID(ssid);
	msg.setWiFiPassword(password);
	msg.setWiFiIP(ip);
	msg.setWiFiSubnet(mask);
	return msg;
}


// Response. ==================================================================

Message Message::createGenericResponse(uint8_t responseCode)
{
	Message msg(GenericResponse);
	msg.setResponseCode(responseCode);
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
	
	if (base->signature[0] != 'A' || base->signature[1] != 'N' || base->signature[2] != 'R' || base->signature[3] != 'S')
		return Message();
	if (base->type < Auth || base->type > GetWiFiConfigResponse)
		return Message();
	
	switch(base->type) {
		case Auth:
			return createAuthRequest(base->password);
		case SetPassword:
			return createSetPasswordRequest(base->password, base->newPassword.newPassword);
		case GetText:
			return createGetTextRequest(base->password);
		case SetText:
			return createSetTextRequest(base->password, base->text.brate, base->text.srate, base->text.text);
		case GetWiFiConfig:
			return createGetWifiConfigRequest(base->password);
		case SetWiFiConfig:
			return createSetWifiConfigRequest(base->password, base->wifiConfig.SSID, base->wifiConfig.password, base->wifiConfig.ip, base->wifiConfig.subnetMask);
		case GenericResponse:
			return createGenericResponse(base->resposeCode.responseCode);
		case GetTextResponse:
			return createGetTextResponse(base->text.brate, base->text.srate, base->text.text);
		case GetWiFiConfigResponse:
			return createGetWiFiConfigResponse(base->wifiConfig.SSID, base->wifiConfig.password, base->wifiConfig.ip, base->wifiConfig.subnetMask);
	}

	return Message();
}
