#pragma once
#include <stdint.h>

class MessageTest;

class Message {
public:
	enum Type {
		NoType = 0,
		Auth,
		SetText,
		SetAnimationParameters,
		SetWifiConfig,
		GetText,
		GetAnimationParameters,
		GetWifiConfig
	};

	enum StatusCode {
		Request = 1,
		ResponseOK = 0,
		ResponseFailure = -1,
		ResponseMalformedPackage = -2,
		ResponseIncompatiblePackage = -3,
		ResponseIllegalWiFiConfig = -4,
		ResponseBadPassword = -5
	};

	Message(const void *raw, uint16_t dim);

	// Requests
	static Message createAuthRequest(const char *str);
	static Message createSetTextRequest(const char *str);
	static Message createGetTextRequest();
	static Message createSetWifiConfigRequest(const char *ssid, const char* password, uint32_t ip, uint32_t mask);
	static Message createGetWifiConfigRequest();
	static Message createSetAnimationParametersRequest(uint8_t blinkRate, uint8_t slideRate);
	static Message createGetAnimationParametersRequest();

	// Responses
	static Message createAuthResponse(StatusCode statusCode);
	static Message createSetTextResponse(StatusCode statusCode);
	static Message createGetTextResponse(StatusCode statusCode, const char *str);
	static Message createSetWifiConfigResponse(StatusCode statusCode);
	static Message createGetWifiConfigResponse(StatusCode statusCode, const char *ssid, const char* password, uint32_t ip, uint32_t mask);
	static Message createSetAnimationParametersResponse(StatusCode statusCode);
	static Message createGetAnimationParametersResponse(StatusCode statusCode, uint8_t blinkRate, uint8_t slideRate);

	StatusCode statusCode() const;
	const char *text() const;
	const char *password() const;
	const char *wifiSSID() const;
	const char *wifiPassword() const;
	uint32_t wifiIP() const;
	uint32_t wifiSubnet() const;
	uint8_t blinkRate() const;
	uint8_t slideRate() const;
	uint8_t size() const;
	Type type() const {return mType;};

	uint16_t addRawData(const void *raw, uint16_t size);

	static const uint8_t MinimumMessageSize;
	static const uint8_t MaximumMessageSize;
	static const uint8_t HeaderSize;
	static const uint8_t SupportedProtocolVersion;
	static constexpr uint16_t BufferSize = 256;
private:
	Message(Type type);

	void setStatusCode(enum StatusCode statusCode);
	void setPassword(const char *password);
	void setText(const char *text);
	void setWifiIP(uint32_t ip);
	void setWifiPassword(const char *password);
	void setWifiSSID(const char *str);
	void setWifiSubnet(uint32_t mask);
	void setSlideRate(uint8_t srate);
	void setBlinkRate(uint8_t brate);
	void setSize(uint8_t size);
	void setType(Type type);
	void updateSize();
	void repair();

	Type mType;
	uint16_t mBufferDim;
	char mRaw[BufferSize];

	friend class MessageTest;
};
