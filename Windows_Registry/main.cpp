#include <Windows.h>
#include <iostream>
#include <string>

using namespace std;

HKEY OpenRegistryKey(HKEY hRootKey, wchar_t* strSubKey) {
	HKEY hKey;
	LONG lError = RegOpenKeyEx(hRootKey, strSubKey, NULL, KEY_ALL_ACCESS, &hKey);

	if(ERROR_FILE_NOT_FOUND == lError) {
		//Kreiraj kljuc
		lError = RegCreateKeyEx(hRootKey, strSubKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	}

	if(lError != ERROR_SUCCESS) {
		cout<<"Nesto nije u redu. Ne moze se ni otvoriti ni kreirati kljuc." << endl;
	}

	return hKey;
}

void SetRegistryValues(HKEY hRootKey, LPCTSTR lpVal, DWORD data) {

	LONG nErr = RegSetValueEx(hRootKey, lpVal, NULL, REG_DWORD, (LPBYTE)&data, sizeof(DWORD));

	if(nErr != ERROR_SUCCESS) {
		cout<<"Neuspesno postavljanje vrednosti registra." << endl;
	}
}

void DeleteRegistryKeyAndConfirm(HKEY hRootKey, wchar_t* strSubKey) {

	LONG lRes = RegDeleteKey(hRootKey, strSubKey); // brise rekurzivno sve kljuceve i podkljuceve u registru

	if(ERROR_SUCCESS == lRes) {
		MessageBox(NULL, L"Kljuc uspesno obrisan", L"Delete Key", MB_ICONINFORMATION);

	} else {
		MessageBox(NULL, L" Kljuc nije obrisan", L"Deletion Failed", MB_ICONINFORMATION);
	}
}

DWORD GetValueFromRegistry(HKEY hRootKey, LPCTSTR lpValue) {

	DWORD data = 0;
	DWORD dtype = REG_DWORD;
	DWORD dSize = sizeof(data);
	LONG lErr = RegQueryValueEx(hRootKey, lpValue, NULL, &dtype, (LPBYTE)&data, &dSize);

	if(ERROR_SUCCESS != lErr) {
		cout<<"Ne moze se naci nijedna vrednost." << endl;
	}

	return data;
}

int main() {

	int option;

	wchar_t* nazivKljucaRegistra = L"SOFTWARE\\toma";
	wchar_t* nazivVrednosti = L"My_Value_1";
	DWORD vrednost; // npr 5

	// kreira kljuc u Wow6432Node folderu
	HKEY hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, nazivKljucaRegistra);

	do {

		cout << "Odaberite opciju:" << endl;
		cout << "1. Postavi vrednosti kljuca registra" << endl;
		cout << "2. Pribavi vrednost kljuca registra" << endl;
		cout << "3. Obrisi kljuc registra" << endl;
		cout << "4. Zavrsi program" << endl;

		cin >> option; 

		switch (option) {

			case 1:
				// postavlja vrednosti kljuca
				cout << "Unesite vrednost kljuca:" << endl;
				cin >> vrednost;

				SetRegistryValues(hKey, nazivVrednosti, vrednost);
				break;

			case 2:
				// pribavlja vrednosti kljuca
				vrednost = GetValueFromRegistry(hKey, nazivVrednosti);
				cout << "Vrednost kljuca registra: " << vrednost << endl;
				break;

			case 3:
				DeleteRegistryKeyAndConfirm(HKEY_LOCAL_MACHINE, nazivKljucaRegistra);
				break;

			case 4:
				cout << "Izlaz iz programa !" << endl;
				break;

			default:
				cout << "Ova opcija ne postoji !" << endl;
				break;
		}

	} while (option != 4);

	RegCloseKey(hKey); // zatvaramo pristup kljucu

	system("pause");
	return 0;

}