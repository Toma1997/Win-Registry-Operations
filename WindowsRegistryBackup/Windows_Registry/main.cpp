
#include "stdafx.h"
#include <Windows.h> // za rad sa Windows sistemskim funkcijama
#include <stdio.h>
#include <string>
#include <malloc.h> // za operacije alociranja memorije u hipu
#include <iostream>
#include <winperf.h> // za definisanje tipa strukture PPERF_DATA_BLOCK za objekte koji opisuju promenljivu

using namespace std;

// konstante
const int MAKS_DUZINA_KLJUCA = 255;
const int MAKS_VREDNOST_NAZIVA = 16383;
const int UKUPNO_BAJTOVA = 16383;
const int INKREMENT_BAJTOVA = 4096;

// funkcija koja pribavlja naziv tipa promenljive iz kljuca registra
LPCWSTR pribaviNazivTipaVrednosti(DWORD tip){

	LPCWSTR naziv_tipa = L"";
	switch (tip){
		case REG_BINARY:
			naziv_tipa = L"REG_BINARY";
			break;
		case REG_DWORD:
			naziv_tipa = L"REG_DWORD";
			break;
		case REG_DWORD_BIG_ENDIAN:
			naziv_tipa = L"REG_DWORD_BIG_ENDIAN";
			break;
		case REG_EXPAND_SZ:
			naziv_tipa = L"REG_EXPAND_SZ";
			break;
		case REG_LINK:
			naziv_tipa = L"REG_LINK";
			break;
		case REG_MULTI_SZ:
			naziv_tipa = L"REG_MULTI_SZ";
			break;
		case REG_NONE:
			naziv_tipa = L"REG_NONE";
			break;
		case REG_QWORD:
			naziv_tipa = L"REG_QWORD";
			break;
		case REG_SZ:
			naziv_tipa = L"REG_SZ";
			break;
	}

	return naziv_tipa;
}

// funkcija koja iscitava vrednosti svih podkljuceva
// HKEY (hive key) - kosnica kljucevi su 5 foldera kao glavni kljucevi registra
void obidjiPodKljuceve(HKEY key, LPCWSTR current_path){

	// current_path vec postoji
	// ispod je definisanje promenljivih o kljucevima i njihovim vrednostima
	DWORD brojPodKljuceva;
	DWORD najduziPodKljuc;
	DWORD brojPromenljivihKljuca;
	DWORD najduzaVrednost;

	TCHAR imeVrednosti[MAKS_VREDNOST_NAZIVA];
	DWORD duzinaImenaVrednosti;

	DWORD vrednost_rezultat;
	DWORD tip_vrednosti;
	DWORD brojBajtovaPodataka;
	DWORD velicinaBafera = UKUPNO_BAJTOVA; // 8 KB ce biti bufer
	HANDLE hv_file;

	BOOL upisni_fleg = FALSE;

	// alocira se memorija velicine bafera za blok podatka o vrednosti
	PPERF_DATA_BLOCK vrednost_podatka = (PPERF_DATA_BLOCK) malloc(velicinaBafera); // vrednost promenljive u kljucu

	DWORD velicinaPodkljuca;
	DWORD podKljuc_rezultat;
	TCHAR podKljucIme[MAKS_VREDNOST_NAZIVA];

	// funkcija ima 12 parametara, neki su opcioni pa je stavljeno NULL
	// funkcija pribavlja informacije za parametre koji nisu inicijalizovani, samo hkey mora biti prethodno definisan
	// generalno pribavlja inormacije za sve podkljuceve i njihove vrednosti
	RegQueryInfoKey(key, NULL, NULL, NULL, &brojPodKljuceva, &najduziPodKljuc, NULL, &brojPromenljivihKljuca, &najduzaVrednost, NULL, NULL, NULL);

	// kreiraj fajlove za svaku vrednost koju ima TRENUTNI kljuc, ako ima promenljivih
	if (brojPromenljivihKljuca){
		for (DWORD i = 0, povratniKod = ERROR_SUCCESS; i < brojPromenljivihKljuca; i++){

			duzinaImenaVrednosti = MAKS_VREDNOST_NAZIVA;
			imeVrednosti[0] = '\0'; // pokazivac na bafer koji ce primiti ime vrednosti sa nulom na kraju

			//funkcija kopira indeksirano ime vrednosti i blok podatka svaki put kad se pozove
			povratniKod = RegEnumValue(key, i, imeVrednosti, &duzinaImenaVrednosti, NULL, NULL, NULL, NULL); 

			if (povratniKod == ERROR_SUCCESS){
				LPCWSTR ext = L".reg"; // ekstenzija za registar
				wstring putanjaFajla = current_path + wstring(imeVrednosti) + ext;

				// kreiraj fajl na datoj putanji u kome ce biti vrednost kljuca
				hv_file = CreateFile(putanjaFajla.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				// pribavi tip i podatak vrednosti promenljive
				vrednost_rezultat = RegQueryValueEx(key, imeVrednosti, NULL, &tip_vrednosti, (LPBYTE)vrednost_podatka, &brojBajtovaPodataka);

				// dokle god ima jos podataka za izvlacenje
				while (vrednost_rezultat == ERROR_MORE_DATA){

					// pribavi bafer koji je dovoljno veliki
					velicinaBafera += INKREMENT_BAJTOVA;
					// realociraj potrebnu memoriju u hipu za povecan bafer da bi smo izvukli ceo podatak o vrednosti
					vrednost_podatka = (PPERF_DATA_BLOCK) realloc(&vrednost_podatka, velicinaBafera);
					brojBajtovaPodataka = velicinaBafera;
					vrednost_rezultat = RegQueryValueEx(key, imeVrednosti, NULL, NULL, (LPBYTE)vrednost_podatka, &brojBajtovaPodataka);
				}

				// kada se izvuce uspesno cela vrednost
				if (vrednost_rezultat == ERROR_SUCCESS){

					// izvuci naziv tipa vrednosti
					LPCWSTR naziv_tipa = pribaviNazivTipaVrednosti(tip_vrednosti);

					DWORD brojUpisanihBajtova; // pokazivac na broj vec upisanih bajtova

					// upisi u fajl podatke o promenljivoj
					upisni_fleg = WriteFile(hv_file, naziv_tipa, (DWORD) wstring(naziv_tipa).size()*2, &brojUpisanihBajtova, NULL);
					brojUpisanihBajtova = (DWORD)(wstring(naziv_tipa).size() * 2);

					// predji u novi red - za Windows je 2 bajta
					upisni_fleg = WriteFile(hv_file, "\n", (DWORD) 2, &lp, NULL);
					brojUpisanihBajtova = (DWORD)(wstring(naziv_tipa).size() * 2 + 2);

					// upisi vrednost promenljive iz kljuca
					upisni_fleg = WriteFile(hv_file, vrednost_podatka, brojBajtovaPodataka, &brojUpisanihBajtova, NULL);
					CloseHandle(hv_file);
				}
			}
		}
	} // zavrsi kreiranje fajlova za svaku vrednost datog kljuca

	// kreiraj foldere za svaki podkljuc (ako ih ima) datog kljuca i pozovi ovu funkciju - obidjiPodKljuceve rekurzivno opet
	if (brojPodKljuceva){
		for (DWORD i = 0; i < brojPodKljuceva; i++){
			velicinaPodkljuca = MAKS_DUZINA_KLJUCA;

			// vrsi enumeraciju podkljuveca otvorenog registarskog kljuca, pribavlja informacije o jednom podkjucu svaki put kad se pozove
			podKljuc_rezultat = RegEnumKeyEx(key, i, podKljucIme, &velicinaPodkljuca, NULL, NULL, NULL, NULL); 

			if (podKljuc_rezultat == ERROR_SUCCESS){
				wstring putanja_podKljuca = wstring(current_path) + L"\\" + wstring(podKljucIme);
				wcout << putanja_podKljuca << endl;

				// ako se uspesno kreira folder ili vec postoji
				if (CreateDirectory(putanja_podKljuca.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()){
					HKEY subKey; // u ovu promenljivu se unosi rezultat nakon RegOpenKeyEx() funkcije
					RegOpenKeyEx(key, podKljucIme, 0, KEY_READ, &subKey); // otvara odredjeni kljuc registra na osnovu imena da ga procita
					obidjiPodKljuceve(subKey, putanja_podKljuca.c_str()); // obidji podkljuceve kljuca rekurzivno

				} else {
					cout << "Ne valja putanja podkljuca !" << endl;
				}
			}
		}
	}
}

// glavna funkcija za izvrsavanje programa
int _tmain(int argc, _TCHAR* argv[]){
	HKEY koreniKljuc;
	LONG statusni_rezultat;
	LPCWSTR pocetniKljuc = L"Software"; // pokazivac na pocetni kljuc u registru koji se kopira
	LPCWSTR DIR_KOREN = L"C:\\"; // pokazivac na lokaciju gde ce biti smesten backup

	// wide character string kao kompletna putanja backup foldera
	wstring putanja = DIR_KOREN + wstring(pocetniKljuc);

	// prikazi putanju - wcout za wide char tip
	wcout << putanja << endl;
	statusni_rezultat = RegOpenKeyEx(HKEY_CURRENT_USER, pocetniKljuc, 0, KEY_READ, &koreniKljuc);

	// ako je uspesno otvoren kljuc
	if (statusni_rezultat == ERROR_SUCCESS){

		// ako se uspesno kreira folder za pocetni klju; ili vec postoji kreni u obilazak podkljuceva i promenljivih kljuca
		if (CreateDirectory(putanja.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()){
			obidjiPodKljuceve(koreniKljuc, putanja.c_str());
		} else {
			cout << "Ta putanja kljuca ne valja !" << endl;
		}
	}

	RegCloseKey(koreniKljuc); // zatvori koreni kljuc
	return 0;
}
