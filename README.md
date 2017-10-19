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

## Procedimientos
Cada procedimiento es un mensaje de pedido del cliente al servidor. Cada pedido se reconoce con una respuesta indicando éxito o error.

La respuesta del servidor tiene el siguiente formato:

struct ServerResponse {
	int8_t ResponseCode;
}

ServerResponse puede tomar los siguientes valores:
0: Respuesta genérica indicando éxito.
-1: Error genérico.
-2: Paquete inválido.
-3: Paquete incompatible.
-4: Configuración de WiFi inválida.

### Authenticate
Cuando se inicia la conexión SSL, el cliente manda la password para entrar al sistema. El servidor responde con una respuesta de OK y la conexión permanece establecida hasta que el cliente decida cerrarla. (Cerrar la conexión SSL implica primero señalizar su fin, solo hacer FIN o RST se considera como una intrusión a la conexión y es detectable por ambas partes)

### Disconnect
El cliente señaliza que va a desconectar y luego desconecta. Esto se hace en la capa TLS así que no existe un mensaje que se transfiera a nivel aplicación.

### SetText
El cliente manda un pedido de cambio de mensaje, con el nuevo mensaje.

El servidor responde con un OK o con un código de error.

Descripción del contenido del mensaje:
~~~
struct SetTextRequest {
	char msg[strLength];
}
~~~

El campo msg es una cadena de caracteres terminada en cero.

### SetAnimationParameters
El cliente manda un pedido para setear los parámetros de animación.

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
El cliente manda un pedido para que el cartel se conecte a otra red wifi. Para esto se le debe pasar el nombre de la red (SSID), y la contraseña de la red.

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
