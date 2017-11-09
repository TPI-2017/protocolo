#pragma once
#include <stdint.h>

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

	enum ResponseCode {
		Request = 1,
		OK = 0,
		Failure = -1,
		MalformedPackage = -2,
		IncompatiblePackage = -3,
		IllegalWiFiConfig = -4,
		BadPassword = -5
	};

	Message(Type type);
	Message(const void *raw, uint16_t dim);

	ResponseCode responseCode() const;
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
	void setResponseCode(enum ResponseCode responseCode);
	void setPassword(const char *password);
	void setText(const char *text);
	void setWifiIP(uint32_t ip);
	void setWifiPassword(const char *password);
	void setWifiSSID(const char *str);
	void setWifiSubnet(uint32_t mask);
	void setSlideRate(uint8_t srate);
	void setBlinkRate(uint8_t brate);
	void setSize(uint8_t size);
	void prepare();

	Type mType;
	uint16_t mBufferDim;
	char mRaw[BufferSize];
};
