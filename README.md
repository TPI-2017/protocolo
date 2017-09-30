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
	uint8_t type;
	uint8_t size;
	char content[254];
}
~~~

Message.type es un enumerativo que codifica el tipo de mensaje. Message.size indica el tamaño del mensaje, tiene rango [2, 255].

Un mensaje inválido se descarta y termina la conexión. (bajo TLS, no puede tratarse de corrupción, asi que es un cliente mal implementado o malicioso)

Message.content tiene una estructura que depende de Message.type y se define para cada procedimiento más adelante.

### Strings
Los strings tienen un límite de longitud dado por el lugar remanente en el mensaje y están codificados según el estándar ISO/IEC 8859-1:1998. Antes del contenido del string debe haber un byte que indique la longitud del string.

## Procedimientos
Cada procedimiento es un mensaje de pedido del cliente al servidor. Cada pedido se reconoce con una respuesta indicando éxito o error.

### Authenticate
Cuando se inicia la conexión SSL, el cliente manda la password para entrar al sistema. El servidor responde con una respuesta de OK y la conexión permanece establecida hasta que el cliente decida cerrarla. (Cerrar la conexión SSL implica primero señalizar su fin, solo hacer FIN o RST se considera como una intrusión a la conexión y es detectable por ambas partes)

### Disconnect
El cliente señaliza que va a desconectar y luego desconecta. Esto se hace en la capa TLS asi que no existe un mensaje que se transfiera a nivel aplicación.

### SetText
El cliente manda un pedido de cambio de mensaje, con el nuevo mensaje.

El servidor responde con un OK o con un código de error.

Descripción del contenido del mensaje:
~~~
struct SetTextRequest {
	uint8_t strLength;
	char msg[strLength];
}
~~~

### SetBitmap

Descripción del contenido del mensaje:
~~~
struct SetBitmapRequest {
	...
}
~~~

### SetAnimationParameters
El cliente manda un pedido para setear los parámetros de animación.

Descripción del contenido del mensaje:
~~~
struct SetAnimParamsRequest {
	ufp855 brate;
	sfp855 srate;
}
~~~

MsgChangeBody.brate y MsgChange.srate son la frequencia de parpadeo en Hz y la velocidad de deslizamiento en píxeles por segundo.

Se asume que si MsgChangeBody.brate es cero, no se debe parpadear el contenido. De la misma forma, si MsgChangeBody.srate es cero, no se debe deslizar el contenido.
