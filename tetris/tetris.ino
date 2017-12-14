#include "tetris_game.h"

#include <Keypad.h>

unsigned long cas_predtim = millis();
Casovac casovac_hra(1000);
Casovac casovac_vstup(150);
char aktivniKlavesa = NO_KEY;

char klavesy[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte radky[4] = { 9, 8, 7, 6 };
byte sloupce[4] = { 5, 4, 3, 2 };

Keypad klavesnice(makeKeymap(klavesy), radky, sloupce, 4, 4);

void vygeneruj_blok()
{
	TypBloku typBloku = (TypBloku) random(0, POCET_TYPU_BLOKU);
	Blok blok(typBloku, Pozice(-2, random(0, MAPA_ROZMER - 1)));

	tetris.nastav_blok(blok);
}
void zpracujKlavesu(KeypadEvent klavesa)
{
	if (klavesnice.getState() == KeyState::PRESSED || 
		klavesnice.getState() == KeyState::HOLD)
	{
		aktivniKlavesa = klavesa;
	}
	else aktivniKlavesa = NO_KEY;
}

void setup()
{
  randomSeed(analogRead(A0));
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  led.shutdown(0, false);
  led.setIntensity(0, 2);
  led.clearDisplay(0);

  Serial.begin(9600);

  vygeneruj_blok();

  klavesnice.addEventListener(zpracujKlavesu);

  interrupts();
}

void loop()
{ 
	unsigned long cas = millis();
	unsigned long delta = cas - cas_predtim;
	cas_predtim = cas;
	casovac_hra.aktualizuj(delta);
	casovac_vstup.aktualizuj(delta);

	if (casovac_hra.reset_pokud_pripraven())
	{
		bool usazen_nove = tetris.aktualizuj();
		if (usazen_nove)
		{
			vygeneruj_blok();
		}
	}

	klavesnice.getKey();
	if (aktivniKlavesa != NO_KEY && !tetris.aktivni_blok.usazen && casovac_vstup.reset_pokud_pripraven())
	{
		Blok blok = tetris.aktivni_blok;
		bool beze_zmeny = false;
		
		switch (aktivniKlavesa)
		{
		case '4':
			blok.pozice.y -= 1;
			break;
		case '6':
			blok.pozice.y += 1;
			break;
		case '5':
			blok.pozice.x += 1;
			break;
		case '1':
			blok.zarotuj_90(true);
			break;
		case '2':
			blok.zarotuj_90(false);
			break;
		default:
			beze_zmeny = true;
			break;
		}

		if (!beze_zmeny)
		{
			tetris.aktualizuj_blok(blok);
		}
	}

	delay(100);
}

