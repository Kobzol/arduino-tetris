#include "tetris_game.h"

unsigned long cas_predtim = millis();
Casovac casovac_akt = Casovac(1000);
byte counter = 0;

void vygeneruj_blok()
{
	TypBloku typBloku = (TypBloku) random(0, POCET_TYPU_BLOKU);
	Blok blok(typBloku, Pozice(-2, random(0, MAPA_ROZMER - 1)));

	tetris.nastav_blok(blok);
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

  interrupts();
}

void loop()
{ 
	unsigned long cas = millis();
	unsigned long delta = cas - cas_predtim;
	cas_predtim = cas;
	casovac_akt.aktualizuj(delta);

	if (casovac_akt.reset_pokud_pripraven())
	{
		bool usazen_nove = tetris.aktualizuj();

		if (usazen_nove)
		{
			vygeneruj_blok();
		}
	}

	if (Serial.available() > 0 && !tetris.aktivni_blok.usazen)
	{
		char vstup = Serial.read();
		Blok blok = tetris.aktivni_blok;
		bool beze_zmeny = false;
		
		switch (vstup)
		{
		case 'a':
			blok.pozice.y -= 1;
			break;
		case 'd':
			blok.pozice.y += 1;
			break;
		case 's':
			blok.pozice.x += 1;
			break;
		case 'x':
			blok.zarotuj_90(true);
			break;
		case 'y':
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

