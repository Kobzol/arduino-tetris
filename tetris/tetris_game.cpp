#include "tetris_game.h"

LedControl led = LedControl(DATA_IN, CLK, CS, 1);

byte tetris_mapa[MAPA_ROZMER][MAPA_ROZMER] = { 0 };
Tetris tetris;

Casovac::Casovac(unsigned long cas_oznameni) : cas_oznameni(cas_oznameni), cas_akumulator(0)
{

}

void Casovac::aktualizuj(unsigned long cas_delta)
{
	this->cas_akumulator += cas_delta;
}
bool Casovac::je_pripraven()
{
	return this->cas_akumulator >= this->cas_oznameni;
}
bool Casovac::reset_pokud_pripraven()
{
	if (this->je_pripraven())
	{
		this->reset();
		return true;
	}
	else return false;
}
void Casovac::reset()
{
	this->cas_akumulator = 0;
}

Pozice::Pozice(char x, char y) : x(x), y(y)
{

}

Pozice Pozice::operator+(const Pozice& jina)
{
	return Pozice(this->x + jina.x, this->y + jina.y);
}

bool Pozice::je_validni()
{
	return	/*this->x >= 0 && */this->x < MAPA_ROZMER &&
		this->y >= 0 && y < MAPA_ROZMER;
}

Blok::Blok() : usazen(false)
{

}
Blok::Blok(TypBloku typBloku, Pozice pozice) : pozice(pozice), usazen(false), typBloku(typBloku)
{
	switch (typBloku)
	{
	case TypBloku::Kostka:
		{
			this->telo[0] = Pozice(0, 0);
			this->telo[1] = Pozice(0, 1);
			this->telo[2] = Pozice(1, 0);
			this->telo[3] = Pozice(1, 1);
		}
		break;
	case TypBloku::Cara:
		{
			this->telo[0] = Pozice(-1, 0);
			this->telo[1] = Pozice(0, 0);
			this->telo[2] = Pozice(1, 0);
			this->telo[3] = Pozice(2, 0);
		}
		break;
	case TypBloku::El:
		{
			this->telo[0] = Pozice(-2, 0);
			this->telo[1] = Pozice(-1, 0);
			this->telo[2] = Pozice(0, 0);
			this->telo[3] = Pozice(0, 1);
		}
		break;
  case TypBloku::Klin:
    {
      this->telo[0] = Pozice(0, -1);
      this->telo[1] = Pozice(0, 0);
      this->telo[2] = Pozice(0, 1);
      this->telo[3] = Pozice(1, 0);
    }
    break;
  case TypBloku::Es:
    {
      this->telo[0] = Pozice(0, -1);
      this->telo[1] = Pozice(0, 0);
      this->telo[2] = Pozice(1, 0);
      this->telo[3] = Pozice(1, 1);
    }
    break;
	default:
		break;
	}
}

bool Blok::je_validni(Pozice pozice)
{
	for (byte i = 0; i < 4; i++)
	{
		if (!(this->telo[i] + pozice).je_validni())
		{
			return false;
		}
	}

	return true;
}
void Blok::zarotuj_90(bool doprava)
{
	if (this->typBloku == TypBloku::Kostka) return;

  static int c = 0;
  Serial.print("Rotuj ");
  Serial.println(c++);
	for (byte i = 0; i < 4; i++)
	{
		char x = this->telo[i].x;
		char y = this->telo[i].y;

		if (doprava) x *= -1;
		else y *= -1;

		this->telo[i] = Pozice(y, x);
	}
}

Tetris::Tetris()
{
	this->aktivni_blok.usazen = true;
}

void Tetris::nastav_blok(Blok blok)
{
	this->aktivni_blok = blok;
	this->aktivni_blok.usazen = false;
}

bool Tetris::aktualizuj()
{
	// posunuti bloku
	bool usazen_nove = this->posun_blok();

	// odstraneni rad
	this->odstran_radky();

	// vykresleni usazenych bloku
	led.clearDisplay(0);
	led_vysvit();

	// vykresleni aktivniho bloku
	this->vykresli_blok();

	return usazen_nove;
}
void Tetris::aktualizuj_blok(Blok blok)
{
	if (!this->aktivni_blok.usazen)
	{
		// kontrola, jestli se lze posunout
		if (!this->je_blok_validni(blok))
		{
			return;
		}

		// vypnout zobrazeni stare pozice
		for (byte i = 0; i < 4; i++)
		{
			Pozice stara_pozice = this->aktivni_blok.telo[i] + this->aktivni_blok.pozice;
			led.setLed(0, stara_pozice.x, stara_pozice.y, false);
		}

		this->aktivni_blok = blok;
		this->vykresli_blok();
	}
}

bool Tetris::je_blok_validni(Blok blok)
{
	for (byte i = 0; i < 4; i++)
	{
		Pozice pozice = blok.telo[i] + blok.pozice;
		if (!pozice.je_validni())
		{
			return false;
		}
    if (pozice.x >= 0 && tetris_mapa[pozice.x][pozice.y] != MAPA_PRAZDNE)
    {
      return false;
    }
	}

	return true;
}
void Tetris::konec_hry()
{
	this->aktivni_blok.usazen = true;

	memset(tetris_mapa, 0, sizeof(tetris_mapa));

	for (byte x = 0; x < MAPA_ROZMER; x++)
	{
		for (byte y = 0; y < MAPA_ROZMER; y++)
		{
			if (x == y)
			{
				tetris_mapa[x][y] = 1;
				tetris_mapa[x][MAPA_ROZMER - y - 1] = 1;
			}
		}
	}

	led_vysvit();
	delay(3000);

	memset(tetris_mapa, 0, sizeof(tetris_mapa));
}
void Tetris::odstran_radky()
{
	char posunuti_zacatek = -1;
	for (char i = MAPA_ROZMER - 1; i >= 0; i--)
	{
		bool odstranit = true;
		for (byte j = 0; j < MAPA_ROZMER; j++)
		{
			if (tetris_mapa[i][j] != MAPA_BLOK)
			{
				odstranit = false;
				break;
			}
		}

		if (odstranit)
		{
			posunuti_zacatek = i;
      break;
		}
	}

	if (posunuti_zacatek != -1)
	{
		this->posun_radky(posunuti_zacatek);
    this->odstran_radky();
	}
}
bool Tetris::posun_blok()
{
	if (!this->aktivni_blok.usazen)	// pokud mame nejaky blok
	{
		bool usazen_nove = false;

		Pozice snizena = this->aktivni_blok.pozice;
		snizena.x += 1;

		bool zcela_na_scene = true;

		for (byte i = 0; i < 4; i++)
		{
			Pozice pozice = this->aktivni_blok.telo[i] + snizena;

			// telo bloku jeste neni na scene
			if (pozice.x < 0)
			{
				zcela_na_scene = false;
				break;
			}
		}

		for (byte i = 0; i < 4; i++)
		{
			Pozice pozice = this->aktivni_blok.telo[i] + snizena;

			// telo bloku jeste neni na scene
			if (pozice.x < 0)
			{
				continue;
			}
			else if (!zcela_na_scene && tetris_mapa[pozice.x][pozice.y] != MAPA_PRAZDNE)
			{
				this->konec_hry();
				return true;
			}
			else if (pozice.x >= MAPA_ROZMER || tetris_mapa[pozice.x][pozice.y] != MAPA_PRAZDNE)
			{
				usazen_nove = true;
				break;
			}
		}

		if (!usazen_nove)
		{
			this->aktivni_blok.pozice = snizena;
		}
		else
		{
			this->aktivni_blok.usazen = true;
			for (byte j = 0; j < 4; j++)
			{
				Pozice pozice = this->aktivni_blok.telo[j] + this->aktivni_blok.pozice;

				if (pozice.je_validni())
				{
					tetris_mapa[pozice.x][pozice.y] = MAPA_BLOK;
				}
			}
		}

		return usazen_nove;
	}
	else return false;
}
void Tetris::posun_radky(byte zacatek)
{
	for (char i = zacatek - 1; i >= 0; i--)
	{
		for (byte j = 0; j < MAPA_ROZMER; j++)
		{
			tetris_mapa[i + 1][j] = tetris_mapa[i][j];
		}
	}

  for (byte j = 0; j < MAPA_ROZMER; j++)
  {
    tetris_mapa[0][j] = MAPA_PRAZDNE;
  }
}
void Tetris::vykresli_blok()
{
	if (!this->aktivni_blok.usazen)
	{
		for (int j = 0; j < 4; j++)
		{
			Pozice pozice = this->aktivni_blok.telo[j] + this->aktivni_blok.pozice;
			if (pozice.je_validni())
			{
				led.setLed(0, pozice.x, pozice.y, true);
			}
		}
	}
}

void led_vysvit()
{
	for (byte i = 0; i < MAPA_ROZMER; i++)
	{
		byte value = 0;
		for (byte j = 0; j < MAPA_ROZMER; j++)
		{
			if (tetris_mapa[i][j] != 0)
			{
				value |= (1 << (MAPA_ROZMER - j - 1));
			}
		}

		led.setRow(0, i, value);
	}
}
