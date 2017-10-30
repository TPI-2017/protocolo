#pragma once
#include <stdint.h>

class Message {
public:
	enum Type {
		Auth,
		SetText,
		SetAnimationParameters,
		SetWifiConfig,
		GetText,
		GetAnimationParameters,
		GetWifiConfig,
		Response
	};

	enum Error {
		OK = 0,
		Failure = -1,
		MalformedPackage = -2,
		IncompatiblePackage = -3,
		IllegalWiFiConfig = -4
	};

	Message(Type type);
	Message(const void *raw, uint16_t dim);

	const char *text() const;
	void setText(const char *text);

	const char *password() const;
	void setPassword(const char *password);

	const char *wifiSSID() const;
	void setWifiSSID(const char *str);

	const char *wifiPassword() const;
	void setWifiPassword(const char *password);

	uint32_t wifiIP() const;
	void setWifiIP(uint32_t ip);

	uint32_t wifiSubnet() const;
	void setWifiSubnet(uint32_t mask);
	
	uint8_t blinkRate() const;
	void setBlinkRate(uint8_t brate);

	uint8_t slideRate() const;
	void setSlideRate(uint8_t srate);

	uint8_t size() const;
	Type type() const {return mType;};

	void prepare();
	uint16_t addRawData(const void *raw, uint16_t size);

	static const uint8_t MinimumMessageSize;
	static const uint8_t MaximumMessageSize;
	static const uint8_t HeaderSize;
	static const uint8_t SupportedProtocolVersion;
	static constexpr uint16_t BufferSize = 256;
private:
	Type mType;
	uint16_t mBufferDim;
	char mRaw[BufferSize];
	bool readyToSend = false;

	void computeSize();
};
