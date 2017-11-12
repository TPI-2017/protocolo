#pragma once
#include <stdint.h>

class Message {
public:

	static constexpr uint8_t SUPPORTED_PROTOCOL_VERSION = 1;
	static constexpr uint8_t MESSAGE_SIZE = 255;
	static const uint8_t TEXT_SIZE;
	static const uint8_t PASSWORD_SIZE;
	static const uint8_t WIFI_SSID_SIZE;
	static const uint8_t WIFI_PASSWORD_SIZE;

	enum Type {
		OK = 1,
		Invalid,
		Auth,
		GetText,
		GetTextResponse,
		SetText,
		GetWiFiConfig,
		GetWiFiConfigResponse,
		SetWiFiConfig,
		SetPassword
	};

	enum ErrorCode {
		NoError = 0,
		NoSupportedProtocol,
		InvalidSignature,
		InvalidType,
		InvalidErrorCode,
		InvalidMessage
	};

	// Constructor vacío que genera un mensaje de tipo Invalid con error
	// NoError. Necesario para poder instanciar un mensaje que luego se
	// sobreescriba.
	Message();

	// Los char * que se envían por parámetro deben tener un 0 dentro del rango
	// correspondiente. Si no, se intentará colocar un 0 al inicio del campo.
	// En caso de que no se pueda, el comportamiento es indefinido.
	
	// Requests
	static Message createAuthRequest(const char *password);
	static Message createSetTextRequest(uint8_t blinkRate, uint8_t slideRate, const char *text);
	static Message createGetTextRequest();
	static Message createSetWifiConfigRequest(const char *ssid, const char *wifiPassword, uint32_t ip, uint32_t mask);
	static Message createGetWifiConfigRequest();
	static Message createSetPasswordRequest(const char *password);

	// Responses
	static Message createOKResponse();
	static Message createInvalidResponse(ErrorCode errorCode);
	static Message createGetTextResponse(uint8_t blinkRate, uint8_t slideRate, const char *text);
	static Message createGetWiFiConfigResponse(const char *ssid, const char *wifiPassword, uint32_t ip, uint32_t mask);

	// El parámetro rawData debe ser del tamaño del paquete. Si no, el
	// comportamiento es indefinido. Si la información suministrada no
	// corresponde con ningún tipo de paquete especificado en el protocolo
	// entonces se devuelve un mensaje del tipo inválido.
	static Message createMessage(const void *rawData);

	// Los métodos que devuelven char * contienen un 0 dentro del rango 
	// en el que se encuentran sus valores.
	uint8_t       version()      const;
	Type          type()         const;
	const char *  text()         const;
	uint8_t       blinkRate()    const;
	uint8_t       slideRate()    const;
	const char *  wifiSSID()     const;
	const char *  wifiPassword() const;
	uint32_t      wifiIP()       const;
	uint32_t      wifiSubnet()   const;
	const char *  password()     const;
	ErrorCode     errorCode()    const;
	const void *  data()         const;

private:

	Message(Type type);
	
	// Las funciones que reciban un char * deben contener un 0 en el rango
	// correspondiente. Caso contrario se intentará colocar un 0 al inicio del
	// campo. Si no se puede, el comportamiento es indefinido.
	void setText(const char *text);
	void setBlinkRate(uint8_t brate);
	void setSlideRate(uint8_t srate);
	void setWiFiSSID(const char *str);
	void setWiFiPassword(const char *password);
	void setWiFiIP(uint32_t ip);
	void setWiFiSubnet(uint32_t mask);
	void setPassword(const char *password);
	void setErrorCode(ErrorCode errorCode);

	Type mType;
	char mRaw[Message::MESSAGE_SIZE];

};
