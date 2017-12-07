#include "Message.h"


// Declaraciones de funciones de manejo de string y conversión de endiannness
// Estas funciones no son parte de C99, con lo que están declaradas en distintos
// headers dependiendo de la plataforma.
#if WIN32
	#include <winsock2.h>
	#include <string.h>
#elif LINUX
	#include <arpa/inet.h>
	#include <string.h>
	// En GNU no hay strlcpy, con lo que necesitamos uno en base a strncpy que
	// sí está en el estándar C99.
	size_t strlcpy(char *dst, const	char *src, size_t dstSize)
	{
		if (!dstSize)
			return 1;
		strncpy(dst, src, dstSize);
		if (dst[dstSize - 1])
			dst[0] = 0;
	}
	#define strcpy_s(dest, dmax, src) strlcpy(dest, src, dmax)
#elif BSD
	#include <arpa/inet.h>
	#include <string.h>
	#define strcpy_s(dest, dmax, src) strlcpy(dest, src, dmax)
#elif ESP
	#include "../network.h"
	#include "../strings.h"
	#include "c_types.h"
#else
	#warning No endianness conversion functions
#endif


#ifndef ESP
	#undef ICACHE_FLASH_ATTR
	#define ICACHE_FLASH_ATTR
#endif
// Constantes internas. =======================================================

static const uint8_t INTERNAL_TEXT_SIZE = Message::TEXT_SIZE + 1;
static const uint8_t INTERNAL_PASSWORD_SIZE = Message::PASSWORD_SIZE + 1;
static const uint8_t INTERNAL_WIFI_SSID_SIZE = Message::WIFI_SSID_SIZE + 1;
static const uint8_t INTERNAL_WIFI_PASSWORD_SIZE = Message::WIFI_PASSWORD_SIZE + 1;

// Rangos de velocidad de desplazamiento y frecuencia de parpadeo. ============

static constexpr uint8_t SLIDE_RATE_FRAC = 3;
static constexpr uint8_t BLINK_RATE_FRAC = 3;
static_assert(BLINK_RATE_FRAC <= 8, "Demasiados bits fraccionarios.");
static_assert(SLIDE_RATE_FRAC <= 8, "Demasiados bits fraccionarios.");
const float Message::BLINK_RATE_RESOLUTION = 1.0 / (1 << BLINK_RATE_FRAC);
const float Message::SLIDE_RATE_RESOLUTION = 1.0 / (1 << SLIDE_RATE_FRAC);
const float Message::MAX_SLIDE_RATE = 127.0 / (1 << SLIDE_RATE_FRAC);
const float Message::MIN_SLIDE_RATE = -128.0 / (1 << SLIDE_RATE_FRAC);
const float Message::MAX_BLINK_RATE = 255.0 / (1 << SLIDE_RATE_FRAC);

// Funciones de conversión

ICACHE_FLASH_ATTR
static float srate_floating(int8_t num)
{
	return static_cast<float>(num) / static_cast<float>(1 << SLIDE_RATE_FRAC);
}

ICACHE_FLASH_ATTR
static float brate_floating(uint8_t num)
{
	return static_cast<float>(num) / static_cast<float>(1 << BLINK_RATE_FRAC);
}

ICACHE_FLASH_ATTR
static int8_t srate_fixed(float num)
{
	if (num < static_cast<float>(Message::MIN_SLIDE_RATE))
		num = static_cast<float>(Message::MIN_SLIDE_RATE);
	if (num > static_cast<float>(Message::MAX_SLIDE_RATE))
		num = static_cast<float>(Message::MAX_SLIDE_RATE);

	return static_cast<int8_t>((float) num * (float) (1 << SLIDE_RATE_FRAC));
}

ICACHE_FLASH_ATTR
static uint8_t brate_fixed(float num)
{
	if (num < 0.0)
		num = 0.0;
	if (num > static_cast<float>(Message::MAX_SLIDE_RATE))
		num = Message::MAX_SLIDE_RATE;

	return static_cast<uint8_t>((float) num * (float) (1 << SLIDE_RATE_FRAC));
}

// Estructuras internas. ======================================================

struct text_t {
	uint8_t brate;
	int8_t srate;
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
		responseCode_t responseCode;
		newPassword_t newPassword;
	};
}__attribute__((packed));

static_assert(sizeof(BaseMessage) <= Message::MESSAGE_SIZE, "Tamaño incorrecto del paquete.");


// Constructores. =============================================================

ICACHE_FLASH_ATTR
Message::Message()
: mEmpty(true)
{
}

ICACHE_FLASH_ATTR
Message::Message(Type type, uint8_t version)
: mType(type),
  mEmpty(false)
{
	BaseMessage *base = reinterpret_cast<BaseMessage*>(mRaw);
	base->version = version;
	base->signature[0] = 'A';
	base->signature[1] = 'N';
	base->signature[2] = 'R';
	base->signature[3] = 'S';
	base->type = type;
}


// Getters. ===================================================================

ICACHE_FLASH_ATTR
bool Message::empty() const
{
	return mEmpty;
}

ICACHE_FLASH_ATTR
uint8_t Message::version() const
{
	return reinterpret_cast<const BaseMessage*>(mRaw)->version;
}

ICACHE_FLASH_ATTR
Message::Type Message::type() const
{
	return mType;
}

ICACHE_FLASH_ATTR
const char *Message::password() const
{
	if (mType >= Auth && mType < GenericResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->password;
	else
		return nullptr;
}

ICACHE_FLASH_ATTR
const char *Message::newPassword() const
{
	if (mType == SetPassword)
		return reinterpret_cast<const BaseMessage*>(mRaw)->newPassword.newPassword;
	else
		return nullptr;
}

ICACHE_FLASH_ATTR
float Message::blinkRate() const
{
	if (mType == GetTextResponse || mType == SetText)
		return brate_floating(reinterpret_cast<const BaseMessage*>(mRaw)->text.brate);
	else
		return 0.0;
}

ICACHE_FLASH_ATTR
float Message::slideRate() const
{
	if (mType == GetTextResponse || mType == SetText)
		return srate_floating(reinterpret_cast<const BaseMessage*>(mRaw)->text.srate);
	else
		return 0.0;
}

ICACHE_FLASH_ATTR
const char *Message::text() const
{
	if (mType == GetTextResponse || mType == SetText)
		return reinterpret_cast<const BaseMessage*>(mRaw)->text.text;
	else
		return nullptr;
}

ICACHE_FLASH_ATTR
const char *Message::wifiSSID() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.SSID;
	else
		return nullptr;
}

ICACHE_FLASH_ATTR
const char *Message::wifiPassword() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.password;
	else
		return nullptr;
}

ICACHE_FLASH_ATTR
uint32_t Message::wifiIP() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.ip);
	else
		return 0;
}

ICACHE_FLASH_ATTR
uint32_t Message::wifiSubnet() const
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		return ntohl(reinterpret_cast<const BaseMessage*>(mRaw)->wifiConfig.subnetMask);
	else
		return 0;
}

ICACHE_FLASH_ATTR
uint8_t Message::responseCode() const
{
	if (mType == GenericResponse)
		return reinterpret_cast<const BaseMessage*>(mRaw)->responseCode.responseCode;
	else
		return 0;
}

ICACHE_FLASH_ATTR
const void *Message::data() const
{
	return mRaw;
}


// Setters ====================================================================

ICACHE_FLASH_ATTR
void Message::setPassword(const char *password)
{
	if (mType >= Auth && mType < GenericResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->password, INTERNAL_PASSWORD_SIZE, password);
}

ICACHE_FLASH_ATTR
void Message::setNewPassword(const char *newPassword)
{
	if (mType == SetPassword)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->newPassword.newPassword, INTERNAL_PASSWORD_SIZE, newPassword);
}

ICACHE_FLASH_ATTR
void Message::setBlinkRate(float brate)
{
	if (mType == GetTextResponse || mType == SetText)
		reinterpret_cast<BaseMessage*>(mRaw)->text.brate = brate_fixed(brate);
}

ICACHE_FLASH_ATTR
void Message::setSlideRate(float srate)
{
	if (mType == GetTextResponse || mType == SetText)
		reinterpret_cast<BaseMessage*>(mRaw)->text.srate = srate_fixed(srate);
}

ICACHE_FLASH_ATTR
void Message::setText(const char *text)
{
	if (mType == GetTextResponse || mType == SetText)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->text.text, INTERNAL_TEXT_SIZE, text);
}

ICACHE_FLASH_ATTR
void Message::setWiFiSSID(const char *ssid)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.SSID, INTERNAL_WIFI_SSID_SIZE, ssid);
}

ICACHE_FLASH_ATTR
void Message::setWiFiPassword(const char *password)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		strcpy_s(reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.password, INTERNAL_WIFI_PASSWORD_SIZE, password);
}

ICACHE_FLASH_ATTR
void Message::setWiFiIP(uint32_t ip)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.ip = htonl(ip);
}

ICACHE_FLASH_ATTR
void Message::setWiFiSubnet(uint32_t mask)
{
	if (mType == SetWiFiConfig || mType == GetWiFiConfigResponse)
		reinterpret_cast<BaseMessage*>(mRaw)->wifiConfig.subnetMask = htonl(mask);
}

ICACHE_FLASH_ATTR
void Message::setResponseCode(uint8_t responseCode)
{
	if (mType == GenericResponse) {
		reinterpret_cast<BaseMessage*>(mRaw)->responseCode.responseCode = responseCode;
	}
}


// Request. ===================================================================

ICACHE_FLASH_ATTR
Message Message::createAuthRequest(const char *password, uint8_t version)
{
	Message msg(Auth, version);
	msg.setPassword(password);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createSetPasswordRequest(const char *password, const char *newPassword, uint8_t version)
{
	Message msg(SetPassword, version);
	msg.setPassword(password);
	msg.setNewPassword(newPassword);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createGetTextRequest(const char *password, uint8_t version)
{
	Message msg(GetText, version);
	msg.setPassword(password);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createSetTextRequest(const char *password, float blinkRate, float slideRate, const char *text, uint8_t version)
{
	Message msg(SetText, version);
	msg.setPassword(password);
	msg.setBlinkRate(blinkRate);
	msg.setSlideRate(slideRate);
	msg.setText(text);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createGetWifiConfigRequest(const char *password, uint8_t version)
{
	Message msg(GetWiFiConfig, version);
	msg.setPassword(password);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createSetWifiConfigRequest(const char *password, const char *ssid, const char* wifiPassword, uint32_t ip, uint32_t mask, uint8_t version)
{
	Message msg(SetWiFiConfig, version);
	msg.setPassword(password);
	msg.setWiFiSSID(ssid);
	msg.setWiFiPassword(wifiPassword);
	msg.setWiFiIP(ip);
	msg.setWiFiSubnet(mask);
	return msg;
}


// Response. ==================================================================

ICACHE_FLASH_ATTR
Message Message::createGenericResponse(uint8_t responseCode, uint8_t version)
{
	Message msg(GenericResponse, version);
	msg.setResponseCode(responseCode);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createGetTextResponse(float blinkRate, float slideRate, const char *text, uint8_t version)
{
	Message msg(GetTextResponse, version);
	msg.setBlinkRate(blinkRate);
	msg.setSlideRate(slideRate);
	msg.setText(text);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createGetWiFiConfigResponse(const char *ssid, const char* password, uint32_t ip, uint32_t mask, uint8_t version)
{
	Message msg(GetWiFiConfigResponse, version);
	msg.setWiFiSSID(ssid);
	msg.setWiFiPassword(password);
	msg.setWiFiIP(ip);
	msg.setWiFiSubnet(mask);
	return msg;
}

ICACHE_FLASH_ATTR
Message Message::createMessage(const void *rawData)
{
	const BaseMessage *base = reinterpret_cast<const BaseMessage*>(rawData);

	if (base->signature[0] != 'A' || base->signature[1] != 'N' || base->signature[2] != 'R' || base->signature[3] != 'S')
		return Message();
	if (base->type < Auth || base->type > GetWiFiConfigResponse)
		return Message();

	switch(base->type) {
		case Auth:
			return createAuthRequest(base->password, base->version);
		case SetPassword:
			return createSetPasswordRequest(base->password, base->newPassword.newPassword, base->version);
		case GetText:
			return createGetTextRequest(base->password, base->version);
		case SetText:
			return createSetTextRequest(base->password, brate_floating(base->text.brate), srate_floating(base->text.srate), base->text.text, base->version);
		case GetWiFiConfig:
			return createGetWifiConfigRequest(base->password, base->version);
		case SetWiFiConfig:
			return createSetWifiConfigRequest(base->password, base->wifiConfig.SSID, base->wifiConfig.password, ntohl(base->wifiConfig.ip), ntohl(base->wifiConfig.subnetMask), base->version);
		case GenericResponse:
			return createGenericResponse(base->responseCode.responseCode, base->version);
		case GetTextResponse:
			return createGetTextResponse(brate_floating(base->text.brate), srate_floating(base->text.srate), base->text.text, base->version);
		case GetWiFiConfigResponse:
			return createGetWiFiConfigResponse(base->wifiConfig.SSID, base->wifiConfig.password, ntohl(base->wifiConfig.ip), ntohl(base->wifiConfig.subnetMask), base->version);
	}

	return Message();
}
