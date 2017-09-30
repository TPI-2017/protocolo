# Protocolo para control de cartel luminoso

## Codificación
Cada mensaje del protocolo es un paquete de tamaño variable, con un límite máximo de 255 octetos. (Incluyendo todos los headers)
En notación C, cada mensaje tiene el formato:
struct Message {
	uint8_t type;
	uint8_t size;
	char content[254];
}

Message.type es un enumerativo que codifica el tipo de mensaje. Message.size indica el tamaño del mensaje, tiene rango [2, 255].
Un mensaje inválido se descarta y se termina la conexión. (bajo TLS, no puede tratarse de corrupción)

## Procedimientos
Cada procedimiento es un mensaje de pedido del cliente al servidor. Cada pedido se reconoce con una respuesta indicando éxito o error.

### Authenticate(Key):
Cuando se inicia la conexión SSL, el cliente manda la password para entrar al sistema. El servidor responde con una respuesta de OK y la conexión permanece establecida hasta que el cliente decida cerrarla. (Cerrar la conexión SSL implica primero señalizar su fin, solo hacer FIN o RST se considera como una intrusión a la conexión y es detectable por ambas partes)

### Disconnect():
El cliente señaliza que va a desconectar y luego desconecta. Esto se hace en la capa TLS asi que no existe un mensaje que se transfiera a nivel aplicación.

### SetMessage(string message):
El cliente manda un pedido de cambio de mensaje, con el nuevo mensaje y todas las configuraciones (animación, parpadeo, etc).
El servidor responde con un OK o con un código de error.
