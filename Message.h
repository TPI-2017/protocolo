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
		OK,
		Failure,
		MalformedPackage,
		IncompatiblePackage,
		IllegalWiFiConfig
	};

private:
	Type type;
	uint8_t size;
	char content[256];
};

