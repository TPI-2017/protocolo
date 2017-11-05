#include <iostream>
#include <cassert>
#include <cstring>
#include "Message.h"

static const char *testString = "Ut enim ad minima veniam";
static const char *testSSID = "Lorem ipsum dolor sit amet, consectetur ";
static const char *testWifiPassword = "TzBDmm1C7UxE2QyV5hru";

void test1()
{
	/*
	 * Aca solo probamos la funcionalidad básica. Por ejemplo ver que se
	 * mantienen bien los valores seteados
	 */
	{
		Message msg(Message::SetText);
		assert(msg.size() == 0);
		msg.setText(testString);
		msg.prepare();
		assert(msg.size() == 1 + strlen(testString));
		assert(strcmp(testString, msg.text()) == 0);
		assert(strcmp("Avc 123", msg.text()) != 0);
	}

	{
		Message msg(Message::GetText);
		assert(msg.size() == 0);
		msg.setText(testString);
		msg.prepare();
		assert(msg.size() == 1 + strlen(testString));
		assert(strcmp(testString, msg.text()) == 0);
		assert(strcmp("Avc 123", msg.text()) != 0);
	}

	{
		Message msg(Message::Auth);
		assert(msg.size() == 0);
		msg.setPassword(testString);
		msg.prepare();
		assert(msg.size() == 1 + strlen(testString));
		assert(strcmp(testString, msg.password()) == 0);
		assert(strcmp("Avc 123", msg.password()) != 0);
	}

	{
		Message msg(Message::SetAnimationParameters);
		assert(msg.size() == 0);
		msg.prepare();
		assert(msg.size() == 2);
		msg.setBlinkRate(30);
		msg.setSlideRate(60);
		assert(msg.blinkRate() == 30);
		assert(msg.slideRate() == 60);
	}

	{
		Message msg(Message::GetAnimationParameters);
		assert(msg.size() == 0);
		msg.prepare();
		assert(msg.size() == 2);
		msg.setBlinkRate(30);
		msg.setSlideRate(60);
		assert(msg.blinkRate() == 30);
		assert(msg.slideRate() == 60);
	}

	{
		Message msg(Message::SetWifiConfig);
		assert(msg.size() == 0);
		msg.prepare();
		assert(msg.size() == 104);
		msg.setWifiSSID(testSSID);
		msg.setWifiPassword(testWifiPassword);
		msg.setWifiIP(0xC0A80001);
		msg.setWifiSubnet(0xFFFFFF00);
		assert(strcmp(msg.wifiSSID(), testSSID) == 0);
		assert(strcmp(msg.wifiPassword(), testWifiPassword) == 0);
		assert(msg.wifiIP() == 0xC0A80001);
		assert(msg.wifiSubnet() == 0xFFFFFF00);
	}

	std::cout << "Test 1 OK" << std::endl;
}

void test2()
{
	// 251 caracteres (sin terminador)
	const char longString[252] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	const char longestSSID[64] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
	const char longestPassword[32] = "ccccccccccccccccccccccccccccccc";
	const char evenLongerString[253] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	const char evenLongerSSID[65] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
	const char evenLongerPassword[33] = "cccccccccccccccccccccccccccccccc";
	/*
	 * Aca probamos casos límites, strings de longitud máxima.
	 */
	{
		Message msg(Message::SetText);
		msg.setText(longString);
		msg.prepare();
		assert(msg.size() == 252);
		assert(strcmp(longString, msg.text()) == 0);
		assert(strcmp("was anderes", msg.text()) != 0);
	}

	{
		Message msg(Message::GetText);
		msg.setText(longString);
		msg.prepare();
		assert(msg.size() == 252);
		assert(strcmp(longString, msg.text()) == 0);
		assert(strcmp("was anderes", msg.text()) != 0);
	}

	{
		Message msg(Message::SetWifiConfig);
		msg.setWifiSSID(longestSSID);
		msg.setWifiPassword(longestPassword);
		msg.prepare();
		assert(msg.size() == 104);
		assert(strcmp(longestSSID, msg.wifiSSID()) == 0);
		assert(strcmp(longestPassword, msg.wifiPassword()) == 0);
		assert(strcmp("was anderes", msg.wifiSSID()) != 0);
		assert(strcmp("was ganz anderes", msg.wifiPassword()) != 0);
	}

	/*
	 * Aca probamos pasar strings que exceden el tamaño máximo por un caracter.
	 * Se debería simplemente truncar el string.
	 */
	{
		Message msg(Message::SetText);
		msg.setText(evenLongerString);
		msg.prepare();
		assert(msg.size() == 252);
		// Aca comparamos con la truncada
		assert(strcmp(longString, msg.text()) == 0);
		assert(strcmp("was anderes", msg.text()) != 0);
	}

	{
		Message msg(Message::GetText);
		msg.setText(evenLongerString);
		msg.prepare();
		assert(msg.size() == 252);
		// Acá comparamos con la truncada
		assert(strcmp(longString, msg.text()) == 0);
		assert(strcmp("was anderes", msg.text()) != 0);
	}

	{
		Message msg(Message::SetWifiConfig);
		msg.setWifiSSID(evenLongerSSID);
		msg.setWifiPassword(evenLongerPassword);
		msg.prepare();
		assert(msg.size() == 104);
		// De vuelta, usamos las versiones truncadas
		assert(strcmp(longestSSID, msg.wifiSSID()) == 0);
		assert(strcmp(longestPassword, msg.wifiPassword()) == 0);
		assert(strcmp("was anderes", msg.wifiSSID()) != 0);
		assert(strcmp("was ganz anderes", msg.wifiPassword()) != 0);
	}
	std::cout << "Test 2 OK" << std::endl;
}

void test3()
{
	/*
	 * Acá probamos strings vacías.
	 */

	{
		Message msg(Message::SetText);
		msg.setText("");
		msg.prepare();
		// un string vacío ocupa un byte por el terminador
		assert(msg.size() == 1);
		assert(strcmp("", msg.text()) == 0);
		assert(strcmp("not empty", msg.text()) != 0);
	}

	{
		Message msg(Message::GetText);
		msg.setText("");
		msg.prepare();
		assert(msg.size() == 1);
		assert(strcmp("", msg.text()) == 0);
		assert(strcmp("not empty", msg.text()) != 0);
	}

	{
		Message msg(Message::Auth);
		msg.setPassword("");
		msg.prepare();
		assert(msg.size() == 1);
		assert(strcmp("", msg.password()) == 0);
		assert(strcmp("not empty", msg.password()) != 0);
	}

	{
		Message msg(Message::SetWifiConfig);
		msg.setWifiSSID("");
		msg.setWifiPassword("");
		msg.prepare();
		assert(msg.size() == 104);
		assert(strcmp("", msg.wifiSSID()) == 0);
		assert(strcmp("", msg.wifiPassword()) == 0);
		assert(strcmp("not empty", msg.wifiSSID()) != 0);
		assert(strcmp("not empty", msg.wifiPassword()) != 0);
	}

	{
		Message msg(Message::GetWifiConfig);
		msg.setWifiSSID("");
		msg.setWifiPassword("");
		msg.prepare();
		assert(msg.size() == 104);
		assert(strcmp("", msg.wifiSSID()) == 0);
		assert(strcmp("", msg.wifiPassword()) == 0);
		assert(strcmp("not empty", msg.wifiSSID()) != 0);
		assert(strcmp("not empty", msg.wifiPassword()) != 0);
	}
	std::cout << "Test 3 OK" << std::endl;
}

void test4()
{
	/*
	 * Acá probamos strings nulas. Deberían manejarse igual que strings 
	 * vacías
	 */

	{
		Message msg(Message::SetText);
		msg.setText(nullptr);
		msg.prepare();
		// un string vacío ocupa un byte por el terminador
		assert(msg.size() == 1);
		assert(strcmp("", msg.text()) == 0);
		assert(strcmp("not empty", msg.text()) != 0);
	}

	{
		Message msg(Message::GetText);
		msg.setText(nullptr);
		msg.prepare();
		assert(msg.size() == 1);
		assert(strcmp("", msg.text()) == 0);
		assert(strcmp("not empty", msg.text()) != 0);
	}

	{
		Message msg(Message::Auth);
		msg.setPassword(nullptr);
		msg.prepare();
		assert(msg.size() == 1);
		assert(strcmp("", msg.password()) == 0);
		assert(strcmp("not empty", msg.password()) != 0);
	}

	{
		Message msg(Message::SetWifiConfig);
		msg.setWifiSSID(nullptr);
		msg.setWifiPassword(nullptr);
		msg.prepare();
		assert(msg.size() == 104);
		assert(strcmp("", msg.wifiSSID()) == 0);
		assert(strcmp("", msg.wifiPassword()) == 0);
		assert(strcmp("not empty", msg.wifiSSID()) != 0);
		assert(strcmp("not empty", msg.wifiPassword()) != 0);
	}

	{
		Message msg(Message::GetWifiConfig);
		msg.setWifiSSID(nullptr);
		msg.setWifiPassword(nullptr);
		msg.prepare();
		assert(msg.size() == 104);
		assert(strcmp("", msg.wifiSSID()) == 0);
		assert(strcmp("", msg.wifiPassword()) == 0);
		assert(strcmp("not empty", msg.wifiSSID()) != 0);
		assert(strcmp("not empty", msg.wifiPassword()) != 0);
	}

	std::cout << "Test 4 OK" << std::endl;
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	return 0;
}
