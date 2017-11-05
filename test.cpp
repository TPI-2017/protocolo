#include <iostream>
#include <cassert>
#include <cstring>
#include "Message.h"

void test1()
{
	/*
	 * Aca solo probamos la funcionalidad básica. Por ejemplo ver que se
	 * mantienen bien los valores seteados
	 */
	{
		Message msg(Message::SetText);
		assert(msg.size() == 0);
		msg.setText("Abc 123");
		msg.prepare();
		assert(msg.size() == 1 + strlen("Abc 123"));
		assert(strcmp("Abc 123", msg.text()) == 0);
		assert(strcmp("Avc 123", msg.text()) != 0);
	}

	{
		Message msg(Message::GetText);
		assert(msg.size() == 0);
		msg.setText("Abc 123");
		msg.prepare();
		assert(msg.size() == 1 + strlen("Abc 123"));
		assert(strcmp("Abc 123", msg.text()) == 0);
		assert(strcmp("Avc 123", msg.text()) != 0);
	}

	{
		Message msg(Message::Auth);
		assert(msg.size() == 0);
		msg.setPassword("Abc 123");
		msg.prepare();
		assert(msg.size() == 1 + strlen("Abc 123"));
		assert(strcmp("Abc 123", msg.password()) == 0);
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
		msg.setWifiSSID("Abc 123");
		msg.setWifiPassword("Xyz 789");
		msg.setWifiIP(0xC0A80001);
		msg.setWifiSubnet(0xFFFFFF00);
		assert(strcmp(msg.wifiSSID(), "Abc 123") == 0);
		assert(strcmp(msg.wifiPassword(), "Xyz 789") == 0);
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

	// std::cout << "Test 3 OK" << std::endl;
}

void test4()
{

}

int main()
{
	test1();
	test2();
	test3();
	test4();
	return 0;
}
