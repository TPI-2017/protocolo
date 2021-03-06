#pragma once
#include <stdint.h>

class Message {
public:

	static constexpr uint8_t SUPPORTED_PROTOCOL_VERSION = 1;
	static constexpr uint16_t MESSAGE_SIZE = 256;

	// Estas constantes públicas definen el máximo tamaño de string sin incluir
	// al terminador 0.
	static constexpr uint8_t TEXT_SIZE = 200;
	static constexpr uint8_t PASSWORD_SIZE = 40;
	static constexpr uint8_t WIFI_SSID_SIZE = 32;
	static constexpr uint8_t WIFI_PASSWORD_SIZE = 64;

	// Estas constantes públicas definen el rango de las variables de la
	// velocidad de desplazamiento y frecuencia de parpadeo.
	static const float BLINK_RATE_RESOLUTION;
	static const float SLIDE_RATE_RESOLUTION;
	static const float MAX_SLIDE_RATE;
	static const float MIN_SLIDE_RATE;
	static const float MAX_BLINK_RATE;

	// Type viene determinado por un enumerativo cuyo valor
	// representa el tipo de mensaje.
	enum Type {
		Auth = 1,
		SetPassword,
		GetText,
		SetText,
		GetWiFiConfig,
		SetWiFiConfig,
		GenericResponse,
		GetTextResponse,
		GetWiFiConfigResponse
	};

	// Cuando se pide un código de respuesta, se devuelve un byte. Para
	// determinar cuál valor es, se debe usar máscaras de bits, utilizando las
	// posiciones determinadas en este enumerativo.
	enum ResponseCodePosition {
		OK = 0,
		MalformedPacket = 1,
		BadPassword = 2,
		BadProtocolVersion = 4,
		BadIP = 8,
		BadSubnetMask = 16
	};

	// Constructor vacío que genera un mensaje vacío. Necesario para poder
	// instanciar un mensaje que luego se sobreescriba.
	Message();

	// Los char * que se envían por parámetro deben tener un 0 dentro del rango
	// correspondiente. Si no, se intentará colocar un 0 al inicio del campo.
	// En caso de que no se pueda, el comportamiento es indefinido.
	// Para los request, es necesario introducir la contraseña del servidor.
	// Sin embargo, para los response, no. El campo password, queda sin usar.
	
	// Requests
	static Message createAuthRequest(const char *password, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	static Message createSetPasswordRequest(const char *password, const char *newPassword, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	static Message createGetTextRequest(const char *password, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	static Message createSetTextRequest(const char *password, float blinkRate, float slideRate, const char *text, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	static Message createGetWifiConfigRequest(const char *password, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	static Message createSetWifiConfigRequest(const char *password, const char *ssid, const char *wifiPassword, uint32_t ip, uint32_t mask, uint8_t version = SUPPORTED_PROTOCOL_VERSION);

	// Responses
	static Message createGenericResponse(uint8_t responseCode, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	static Message createGetTextResponse(float blinkRate, float slideRate, const char *text, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	static Message createGetWiFiConfigResponse(const char *ssid, const char *wifiPassword, uint32_t ip, uint32_t mask, uint8_t version = SUPPORTED_PROTOCOL_VERSION);

	// El parámetro rawData debe ser del tamaño del paquete. Si no, el
	// comportamiento es indefinido. Si el tamaño es el correspondiente y
	// la información suministrada no corresponde con ningún tipo de paquete, 
	// especificado en el protocolo, o si no tiene la firma válida, entonces
	// se devuelve un mensaje de tipo vacío. En cualquier otro caso se arma el
	// mensaje correspondiente.
	static Message createMessage(const void *rawData);

	// Los métodos que devuelven char * contienen un 0 dentro del rango en el
	// que se encuentran sus valores. Antes de pedir cualquier valor, es
	// necesario preguntar si el mensaje es vacío. En caso de que el mensaje
	// no sea vacío, se asegura que contendrá un tipo válido. Los resultados
	// que se obtienen dependen del tipo de mensaje. Por lo que en caso de que
	// se solicite un dato que no corresponda con el tipo de mensaje, se
	// devolverá 0 o puntero a null.
	bool          empty()        const;

	uint8_t       version()      const;
	Type          type()         const;
	const char *  password()     const;
	const char *  newPassword()  const;
	float         blinkRate()    const;
	float         slideRate()    const;
	const char *  text()         const;
	const char *  wifiSSID()     const;
	const char *  wifiPassword() const;
	uint32_t      wifiIP()       const;
	uint32_t      wifiSubnet()   const;
	uint8_t       responseCode() const;
	const void *  data()         const;

private:

	// Instancia un constructor con el tipo de mensaje indicado. Se utiliza
	// para luego setearle los demás parámetros.
	Message(Type type, uint8_t version = SUPPORTED_PROTOCOL_VERSION);
	
	// Las funciones que reciban un char * deben contener un 0 en el rango
	// correspondiente. Caso contrario se intentará colocar un 0 al inicio del
	// campo. Si no se puede, el comportamiento es indefinido.
	void setPassword(const char *password);
	void setNewPassword(const char *newPassword);
	void setBlinkRate(float brate);
	void setSlideRate(float srate);
	void setText(const char *text);
	void setWiFiSSID(const char *ssid);
	void setWiFiPassword(const char *password);
	void setWiFiIP(uint32_t ip);
	void setWiFiSubnet(uint32_t mask);
	void setResponseCode(uint8_t responseCode);

	// Esta variable esta contenida dentro del arreglo de bytes, sin embargo
	// se decide conservar su valor detnro de un miembro interno por eficiencia.
	Type mType;

	// Se utiliza para los mensajes vacíos.
	bool mEmpty;
	
	// El arreglo de bytes que contiene toda la información del mensaje.
	char mRaw[Message::MESSAGE_SIZE];

};
