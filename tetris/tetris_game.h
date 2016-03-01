#pragma once

#include <LedControl.h>

#define MAPA_ROZMER (8)

#define DATA_IN 12
#define CLK 11
#define CS 10

extern LedControl led;

#define MAPA_PRAZDNE (0)
#define MAPA_BLOK (1)

enum class TypBloku
{
	Kostka = 0,
	Cara = 1,
	El = 2
};
#define POCET_TYPU_BLOKU (3)

extern byte tetris_mapa[MAPA_ROZMER][MAPA_ROZMER];
void led_vysvit();

struct Casovac
{
public:
	Casovac(unsigned long cas_oznameni = 1000);

	void aktualizuj(unsigned long cas_delta);
	bool je_pripraven();
	bool reset_pokud_pripraven();
	void reset();

private:
	unsigned long cas_oznameni;
	unsigned long cas_akumulator;
};

struct Pozice
{
	Pozice(char x = 0, char y = 0);

	Pozice operator+(const Pozice& jina);

	bool je_validni();

	char x;
	char y;
};

class Blok
{
public:
	Blok();
	Blok(TypBloku typBloku, Pozice pozice = Pozice());

	bool je_validni(Pozice pozice);
	void zarotuj_90(bool doprava);

	Pozice telo[4];
	Pozice pozice;
	bool usazen;
	TypBloku typBloku;
};

struct Tetris
{
public:
	Tetris();

	bool aktualizuj();
	void aktualizuj_blok(Blok blok);
	void nastav_blok(Blok blok);

	Blok aktivni_blok;

private:
	bool je_blok_validni(Blok blok);
	void konec_hry();
	void odstran_radky();
	bool posun_blok();
	void posun_radky(byte zacatek, byte pocet);
	void vykresli_blok();
};

extern Tetris tetris;
