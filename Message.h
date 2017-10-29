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
	Message(const void *raw);

	// Getters
	const char *text() const;

	const char *wifiSSID() const;
	const char *wifiPassword() const;
	uint32_t wifiIP() const;
	uint32_t wifiSubnet() const;

	uint8_t blinkRate() const;
	uint8_t slideRate() const;

	const char *password() const;

	uint8_t size() const
	{
		return mSize;
	};

	Type type() const {
		return mType;
	};

	// Setters
	void setWifiSSID(const char *str);
	void setWifiPassword(const char *password);
	void setWifiIP(uint32_t ip);
	void setWifiSubnet(uint32_t mask);

	void setBlinkRate(uint8_t brate);
	void setSlideRate(uint8_t srate);

	void setPassword(const char *password);

	void prepare();

private:
	Type mType;
	uint8_t mSize;
	char rawContent[256];
};
