# Descripción textual
La idea de este repositorio es que aloje una clase C++ que abstraiga el formato de los mensajes del protocolo, de manera que se use como submódulo en el repositorio del código que corre en el ESP8266 y el cliente de escritorio.

Este README también sirve como documentación del protocolo.
# Protocolo para control de cartel luminoso

## Codificación
Cada mensaje del protocolo es un paquete de tamaño variable, con un límite máximo de 255 octetos. (Incluyendo todos los headers)

Todos los valores numéricos se transmiten con orden de bytes Big-Endian.

En notación C, cada mensaje tiene el formato:
~~~
struct Message {
	uint8_t version;
	uint8_t type;
	uint8_t size;
	char content[253];
}
~~~

Message.version es un número que identifica la versión del protocolo, por si se hicieran cambios y se quiera mantener retrocompatibilidad.

Message.type es un enumerativo que codifica el tipo de mensaje. Message.size indica el tamaño del mensaje, tiene rango [3, 253].

Un mensaje inválido se descarta y termina la conexión. (bajo TLS, no puede tratarse de corrupción, asi que es un cliente mal implementado o malicioso)

Message.content tiene una estructura que depende de Message.type y se define para cada procedimiento más adelante.

### Strings
Los strings tienen un límite de longitud dado por el lugar remanente en el mensaje y están codificados según el estándar ISO/IEC 8859-1:1998. Antes del contenido del string debe haber un byte que indique la longitud del string.

## Establecimiento de conexión
El establecimiento de una conexión está conformado por el establecimiento de una conexión TLS al servidor iniciada por el cliente, seguida de la interacción Authenticate. Si la interacción Authenticate fallase, el servidor debe cerrar la conexión inmediatamente.

El acceso al servidor dura lo que dure la conexión; no debe mantenerse estado de autenticación fuera de una conexión.

## Desconexión
La desconexión es sólo la de TLS. Una desconexión a nivel TCP sin el procedimiento de desconexión de TLS se considera un error.

## Interacciones
Una interacción es una secuencia de dos mensajes: un mensaje de pedido y un mensaje de respuesta. Cada interacción está formada por un mensaje de pedido seguido por un mensaje de respuesta.
Las interacciones posibles son:
* Authenticate
* SetText
* SetAnimationParameters
* SetWifiConfiguration
* GetText
* GetAnimationParameters
* GetWifiConfiguration

La respuesta proviniente del servidor tiene el siguiente formato:

~~~
struct ServerResponse {
	int8_t ResponseCode;
	char response[252];
}
~~~

ResponseCode puede tomar los siguientes valores:
* Respuesta genérica indicando éxito: 0
* Error genérico: -1
* Paquete inválido: -2
* Paquete incompatible: -3
* Configuración de WiFi inválida: -4

El campo response es opcional, puede ser de tamaño nulo y su contenido depende de la interacción de la cual es respuesta. Se dice que una interacción tiene una respuesta vacía cuando el campo response está ausente. Toda interacción tiene respuesta vacía salvo que se indique lo contrario.

### Authenticate
Cuando se inicia la conexión SSL, el cliente manda la password para entrar al sistema. El servidor responde con una respuesta de OK y la conexión permanece establecida hasta que el cliente decida cerrarla. (Cerrar la conexión SSL implica primero señalizar su fin, solo hacer FIN o RST se considera como una intrusión a la conexión y es detectable por ambas partes)

### SetText
Actualiza el mensaje del cartel con el enviado en el pedido.

El servidor responde con un OK o con un código de error.

Descripción del contenido del mensaje:
~~~
struct SetTextRequest {
	char msg[strLength];
}
~~~

El campo msg es una cadena de caracteres terminada en cero.

### SetAnimationParameters
Actualiza la configuración de la animación del cartel.

Descripción del contenido del mensaje:
~~~
struct SetAnimParamsRequest {
	ufp844 brate;
	sfp844 srate;
}
~~~

SetAnimParamsRequest.brate y SetAnimParamsRequest.srate son la frequencia de parpadeo en Hz y la velocidad de deslizamiento en píxeles por segundo.

Se asume que si SetAnimParamsRequest.brate es cero, no se debe parpadear el contenido. De la misma forma, si SetAnimParamsRequest.srate es cero, no se debe deslizar el contenido.

El tipo de dato ufp844 es un número en punto fijo sin signo con 4 bits de parte entera y 4 bits de parte fraccionaria. El tipo de dato sfp844 es lo mismo que ufp844 pero en complemento a dos.

### SetWifiConfiguration
Actualiza la configuración de WiFi del servidor. En caso de éxito, el servidor debe responder con el código de respuesta de éxito, luego debe cerrar la conexión y por último debe conectarse a la red indicada bajo la IP indicada. Si fallara en hacer eso, vuelve a la configuración anterior.

Descripción del contenido del mensaje:
~~~
struct SetWifiConfigurationRequest {
	char SSID[32];
	char password[64];
	uint32_t ip;
	uint32_t subnet;
}
~~~

SSID es el nombre de la red, password es la contraseña de la red, ip es la IP que va a tomar el cartel y subnet es la máscara de la subred.

### GetText
Devuelve la estructura SetTextRequest con el mensaje actual del cartel.
Siempre retorna código de éxito.

### GetAnimationParameters
Devuelve la estructura SetAnimationParameters con la configuración de animación actual del cartel.
Siempre retorna código de éxito.

### GetWifiConfiguration
Devuelve la estructura SetWifiConfigurationRequest con los datos de la red a la que el cartel está conectado y la IP que tiene en ella.
Siempre retorna código de éxito.
