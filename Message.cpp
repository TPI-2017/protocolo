#include "Message.h"

struct BaseMessage {
	uint8_t version;
	uint8_t type;
	uint8_t size;
	char payload[253];
}__attribute__((packed));

struct SetTextRequest {
	char text[253];
}__attribute__((packed));

struct ServerResponse {
	uint8_t responseCode;
}__attribute__((packed));

