#pragma once

#include "RSA.h"
#include <cmath>
#include <time.h>
#include <boost/multiprecision/random.hpp>
#include "Profiler.h"

namespace ImgOps
{
	namespace RSA
	{
		using namespace boost::multiprecision;
		using namespace boost::random;

		mt19937 _randEngine = mt19937(clock());
		Word Rand()
		{
			boost::random::uniform_int_distribution<Word> randGen;
			return randGen(_randEngine);
		}

		Word Rand(Word rangeMin, Word rangeMax)
		{
			boost::random::uniform_int_distribution<Word> randGen(rangeMin, rangeMax);
			return randGen(_randEngine);
		}

		// Mnozenie modulo : a*b mod k
		Word ModuloMultiplcation(Word a, Word b, Word k)
		{
			Word2 a2 = a;
			Word2 b2 = b;
			Word2 ab = a2 * b2;
			Word2 k2 = k;
			Word2 res2 = ab % k2;
			Word res1 = (Word)res2;
			return (Word)res2;
		}

		// Potegowanie modulo : a^e mod k
		// Poteguje uzywajac 'Right-to-left binary method'
		Word ModuloPower(Word base, Word e, Word k)
		{
			Word p = base;
			Word res = Word(1);
			Word bit = Word(1);
			for(int i = 0; i < WordBitCount; ++i, bit <<= 1) // 'bit' to przesuwajacy sie bit (od 1 do liczby bitow w 'Word')
			{
				if(e & bit) 
					res = ModuloMultiplcation(res, p, k);

				p = ModuloMultiplcation(p, p, k);
			}
			return res;
		}

		// Sprawdza czy liczba 'num' jest pierwsza z prawdopodobienstwem 1 - (1/4)^'passes'
		// Niech p będzie nieparzystą liczbą pierwszą zapisaną jako p = 1 + 2^s * d, gdzie d jest nieparzyste. 
		// Wtedy dla dowolnej liczby naturalnej a ∈ <2, p - 2> ciąg Millera-Rabina:
		// a^d, a^(2d), a^(4d), ..., a^(2^(s-1)*d), a^(2^s*d) (mod p)
		// kończy się liczbą 1. 
		// Co więcej, jeśli a^d nie przystaje modulo p do 1, to wyraz ciągu 
		// Millera-Rabina bezpośrednio poprzedzający 1 jest równy p - 1.
		// Jeśli liczba p przejdzie test, to jest albo pierwsza, albo silnie pseudopierwsza przy podstawie a.
		bool CheckMillerRabin(Word num, int passes)
		{
			/*
			Lista kroków:
			K01:	d ← p - 1	; obliczamy s i d
			K02:	s ← 0	 
			K03:	Dopóki d mod 2 = 0, wykonuj K04...K05; usuwamy z p - 1 dzielniki 2 zliczając je w s
			K04:	    s ← s + 1	 
			K05:	    d ← d div 2	 
			//		W praktyce poprostu trzeba znaleźć pierwszy niezerowy bit
			//		Wtedy p - 1 = d << s  <=> p = 1 + 2^s * d
			K06:	Dla i = 1,2,...,n, wykonuj K07...K15; wykonujemy n testów Millera-Rabina
			K07:	    a ← Losuj(2,p-2)	; losujemy bazę a
			K08:	    x ← ad mod p	; wyliczamy pierwszy wyraz ciągu Millera-Rabina
			K09:	    Jeśli (x = 1) ∨ (x = p - 1), to baza nie nadaje sie do testu; wybieramy inna baze (powroc do K06 ale bez inkrementacji i)
			K10:	    j ← 1	 
			K11:	    Dopóki (j < s) ∧ (x ≠ p - 1), wykonuj K12...K14	; rozpoczynamy generację kolejnych wyrazów ciągu Millera-Rabina
			K12:	        x ← x^2 mod p	; obliczamy kolejny wyraz ciagu
			K13:	        Jeśli x = 1, to idź do K17	; tylko ostatni wyraz ciągu Millera-Rabina może mieć wartość 1!
			K14:	        j ← j + 1	 
			K15:	    Jeśli x ≠ p - 1, to idź do K17	; przedostatni wyraz ciągu Millera-Rabina musi być równy p - 1
			K16:	Pętla wykonała n testów i zakończyła się naturalnie. Zwroc TRUE
			K17:	Liczba p nie przeszła testów Millera-Rabina. Zwroc FALSE
			*/

			Word base, d, x;
			int s = 0;

			for(d = num - 1; (d & 1) == 0; ++s) 
				d >>= 1;

			for(int i = 0; i < passes; ++i)
			{
				base = Rand(Word(2), num - Word(2)); // Losujemy baze ciagu
				x = ModuloPower(base, d, num); // Pierwszy wyraz ciagu

				if((x == 1) || (x == num - 1)) // Zla baza
					continue;

				for(int j = 1; (j < s) && (x != num - 1); j++)
				{
					x = ModuloMultiplcation(x, x, num); // Wyliczamy a^(2^j * d) mod num
					if(x == 1) // Tylko ostatni wyraz ciągu Millera-Rabina może mieć wartość 1!
					{
						return false;
					}
				}
				if(x != num - 1) // przedostatni wyraz ciągu Millera-Rabina musi być równy p - 1
				{
					return false;
				}
			}
			return true;
		}

		// Zwraca pare licz pierwszych (p,q) takich ze n = p*q ma zadana liczbe bitow (N = WordBitCount) -> n > 2^(N-1)
		// Czyli dla danego p, q > 2^(N-1) / p i q < 2^N / p (czyli q = (2^(N-1) / p, 2^N / p))
		// Prawdopodobienstwo ze p i q sa rzeczywiscie pierwsze to 1 - (1/4)^'passes'
		void GeneratePrimaryPair(Word* p, Word* q, int testPasses)
		{
			int n2 = WordBitCount / 2;

			Word rangeMin = Word(1) << (n2 - 1);
			Word rangeMax = rangeMin << 1;
			bool primaryTestPassed = false;
			while(!primaryTestPassed)
			{
				// Wygeneruj losowa liczbe z przedzialu (2^(N2-1), 2^(N))
				*p = Rand(rangeMin, rangeMax);
				if((*p & 1) == 0) // Do parzytych dodaj 1
					*p |= 1;
				// Zrob test fermata : Jeśli p jest liczbą pierwszą, to 2^p mod p = 2.
				Word ferm = ModuloPower(Word(2), *p, *p);
				if(ferm != 2)
					continue;
				// Sprawdz czy jest pierwsza z duzym prawdopodobienstwem
				primaryTestPassed = CheckMillerRabin(*p, testPasses);
			}

			// Mamy p pierwsze z duzym prawdopodobienstwem
			// Teraz trezba znalezc q tak aby n = (2^(N-1), 2^N)
			rangeMin = (Word(1) << (WordBitCount - 1)) / *p;
			rangeMax = rangeMin << 1;
			primaryTestPassed = false;
			while(!primaryTestPassed)
			{
				// Wygeneruj losowa liczbe
				*q = Rand(rangeMin, rangeMax);
				if((*q & 1) == 0) // Do parzytych dodaj 1
					*q |= 1;
				// Zrob test fermata : Jeśli q jest liczbą pierwszą, to 2^q mod q = 2.
				Word ferm = ModuloPower(Word(2), *q, *q);
				if(ferm != 2)
					continue;
				// Sprawdz czy jest pierwsza z duzym prawdopodobienstwem
				primaryTestPassed = CheckMillerRabin(*q, testPasses);
			}
		}

		Word Nwd(Word a, Word b)
		{
			Word t;

			while(b != 0)
			{
				t = b;
				b = a % b;
				a = t;
			};
			return a;
		}

		Word FindModuloInverse(Word a, Word n)
		{
			/*
			u,w,x,z	 – 	współczynniki równań. u,v,w,x,y,z   Z
			q	 –  	całkowity iloraz. q   Z
			Lista kroków:

			K01:	u ← 1;  w ← a;
			x ← 0; z ← b	; ustalamy wartości początkowe współczynników
			K02:	Dopóki w ≠ 0 wykonuj kroki K03...K06	; w pętli modyfikujemy współczynniki równań
			K03:	    Jeśli w < z, to  u ↔ x;   w ↔ z	; aby algorytm wyliczał nowe współczynniki, w nie może być mniejsze od z
			; jeśli jest, zamieniamy ze sobą współczynniki równań
			K04:	    q ← w div z	; obliczamy iloraz całkowity
			K05:	    u ← u - q × x	; od równania (1) odejmujemy równanie (2) wymnożone przez q
			K06:	    w ← w - q × z	 
			K07:	Jeśli z ≠ 1, to idź do K10	; dla z różnego od 1 nie istnieje odwrotność modulo
			K08:	Jeśli x < 0, to x ← x + b	; ujemne x sprowadzamy do wartości dodatnich
			K09:	Pisz x i zakończ	; x jest poszukiwaną odwrotnością modulo
			K10:	Pisz brak rozwiązania i zakończ	 */

			Word a0,n0,p0,p1,q,r,t;

			p0 = 0; p1 = 1; a0 = a; n0 = n;
			q  = n0 / a0;
			r  = n0 % a0;
			while(r > 0)
			{
				t = p0 - q * p1;
				if(t >= 0)
					t = t % n;
				else
					t = n - ((-t) % n);
				p0 = p1; p1 = t;
				n0 = a0; a0 = r;
				q  = n0 / a0;
				r  = n0 % a0;
			}
			return p1;
		}

		void GenerateRSAKeys(RSAKey* pubKey, RSAKey* privKey, double maxPrimeError)
		{
			int testPasses = -((log(maxPrimeError) / log(2)) / 2 - 1); // t = log2(maxError) -> 2^t = maxError, each pass -> emax = 2^2, so passes = t/2 + 1
			Word p, q;
			GeneratePrimaryPair(&p, &q, testPasses);
			Word n = p * q;
			// Mamy wygenerowane 2 liczby pierwsze, teraz znajdz wartosc funkcji eulera (p-1)(q-1)
			Word euler = (p - 1) * (q - 1);
			// Wyznaczamy wykładnik publiczny klucza e : powinien byc wzglednie pierwszy z euler
			// Zaczynając od 3 czy sprawdz kolejne liczby nieparzyste sa wzglednie pierwsze z euler (czyli NWD = 1)
			Word e = 3;
			for(e = 3; Nwd(e, euler) != 1; e += 2);
			// Wyznaczamu wykladnik prywatny d : d × e  mod euler = 1
			Word d = FindModuloInverse(e, euler);

			pubKey->n = n;
			pubKey->e = e;
			privKey->n = n;
			privKey->e = d;
		}

		Word EncryptDataChunk(Word data, RSAKey* key)
		{
			// Zaszyfrowane dane : c = t^e mod n
			return ModuloPower(data, key->e, key->n);
		}

		Word DecryptDataChunk(Word data, RSAKey* key)
		{
			// Rozszyfrowane dane : t = c^d mod n
			return ModuloPower(data, key->e, key->n);
		}

		void ByteArrayToWord(byte* bytes, uint32 len, Word* word)
		{
			*word = 0;
			Word byteWord;
			int actLen = min(len, WordByteCount);
			for(int i = 0; i < actLen; ++i)
			{
				byteWord = bytes[i];
				*word = *word + (byteWord << (i*8));
			}
		}

		void WordToByteArray_FullWord(byte* bytes, Word* word)
		{
			auto limbsCount = (*word).backend().size();
			boost::multiprecision::limb_type* limbs = (boost::multiprecision::limb_type*)(*word).backend().limbs();

			int actLen = min(limbsCount * 4, WordByteCount);
			for(int i = 0; i < actLen; ++i)
			{
				byte b = *(limbs + (i >> 2)) >> 8 * (i & 3);
				bytes[i] = *(limbs + (i >> 2)) >> 8 * (i & 3);
			}

			for(int i = actLen; i < WordByteCount; ++i)
			{
				bytes[i] = 0;
			}
		}

		void WordToByteArray(byte* bytes, uint32 len, Word* word)
		{
			auto limbsCount = (*word).backend().size();
			boost::multiprecision::limb_type* limbs = (boost::multiprecision::limb_type*)(*word).backend().limbs();

			int actLen = min(limbsCount * 4, len);
			for(int i = 0; i < actLen; ++i)
			{
				byte b = *(limbs + (i >> 2)) >> 8 * (i & 3);
				bytes[i] = *(limbs + (i >> 2)) >> 8 * (i & 3);
			}

			for(int i = actLen; i < len; ++i)
			{
				bytes[i] = 0;
			}
		}

		void EncryptMessage(byte* data, uint64 dataLen, byte** encryptedData, uint64* encryptedLen, RSAKey* pubKey)
		{
			// 1) Dane beda podzielone na kawalki o dlugosci bitowej 'b'
			// b spelnia : (1 << b-1) < key->n -> czyli znajdujemy ostatni ustawiony bit n
			uint32 chunkLength = WordBitCount - 1;
			for(Word i = Word(1) << (WordBitCount - 1); i > Word(0); i >>= 1)
			{
				--chunkLength;
				if((pubKey->n & i) > 0)
					break;
			}

			chunkLength /= 8; // Dlugosc w bajtach nie bitach
			// 2) Znajdujemy liczbe kawalkow w wiadomosci (zakodowana wiadomosc ma taka sama dlugosc)
			uint64 chunkCount = dataLen % chunkLength == 0 ? dataLen / chunkLength : dataLen / chunkLength + 1;
			*encryptedLen = chunkCount * WordByteCount; // Zakodowany kawalek zawiera 1 Word
			*encryptedData = (byte*)malloc(*encryptedLen);

			// 3) Kodujemy kazdy kawalek
			Word chunkData;
			Word encryptedWord;
			for(int i = 0; i < chunkCount - 1; ++i)
			{
				byte* chunk = data + i * chunkLength;
				byte* encChunk = *encryptedData + i * WordByteCount;
				ByteArrayToWord(chunk, chunkLength, &chunkData);
				encryptedWord = EncryptDataChunk(chunkData, pubKey);
				WordToByteArray_FullWord(encChunk, &encryptedWord);
			}

			// 4) Ostatni kawalek moze byc niepelny
			int remainingBytes = dataLen - chunkLength * (chunkCount - 1);
			byte* chunk = data + (chunkCount - 1) * chunkLength;
			byte* encChunk = *encryptedData + (chunkCount - 1) * WordByteCount;
			ByteArrayToWord(chunk, remainingBytes, &chunkData);
			encryptedWord = EncryptDataChunk(chunkData, pubKey);
			WordToByteArray_FullWord(encChunk, &encryptedWord);
		}

		void EncryptMessage_FixedChunks(byte* data, uint64 dataLen, 
			uint32 chunkLength, uint32* lastChunkSize, 
			byte** encryptedData, uint64* encryptedLen, RSAKey* pubKey)
		{
			// 2) Znajdujemy liczbe kawalkow w wiadomosci (zakodowana wiadomosc ma taka sama dlugosc)
			*lastChunkSize = dataLen % chunkLength;
			uint64 chunkCount = dataLen / chunkLength;
			*encryptedLen =  chunkCount * WordByteCount; // Zakodowany kawalek zawiera 1 Word
			*encryptedLen = *lastChunkSize != 0 ? (chunkCount + 1) * WordByteCount : chunkCount * WordByteCount;
			*encryptedData = (byte*)malloc(*encryptedLen);

			// 3) Kodujemy kazdy kawalek
			Word chunkData;
			Word encryptedWord;
			for(int i = 0; i < chunkCount; ++i)
			{
				byte* chunk = data + i * chunkLength;
				byte* encChunk = *encryptedData + i * WordByteCount;
				ByteArrayToWord(chunk, chunkLength, &chunkData);
				encryptedWord = EncryptDataChunk(chunkData, pubKey);
				WordToByteArray_FullWord(encChunk, &encryptedWord);
			}

			// 4) Ostatni kawalek moze byc niepelny
			if(*lastChunkSize != 0)
			{
				byte* chunk = data + chunkCount * chunkLength;
				byte* encChunk = *encryptedData + chunkCount * WordByteCount;
				ByteArrayToWord(chunk, *lastChunkSize, &chunkData);
				encryptedWord = EncryptDataChunk(chunkData, pubKey);
				WordToByteArray_FullWord(encChunk, &encryptedWord);
			}
		}

		void DecryptMessage(byte* data, uint64 dataLen, byte** decryptedData, uint64* decryptedLen, RSAKey* privKey)
		{
			// 1) Dane sa podzielone na kawalki o dlugosci bitowej 'b'
			// b spelnia : (1 << b-1) < key->n -> czyli znajdujemy ostatni ustawiony bit n
			uint32 chunkLength = WordBitCount - 1;
			for(Word i = Word(1) << (WordBitCount - 1); i > Word(0); i >>= 1)
			{
				--chunkLength;
				if((privKey->n & i) > 0)
					break;
			}

			chunkLength /= 8; // Dlugosc w bajtach nie bitach
			// 2) Znajdujemy liczbe kawalkow w wiadomosci (zdekodowana wiadomosc ma taka sama dlugosc)
			uint64 chunkCount = dataLen / WordByteCount; // Powinno sie dzielic bez reszty	
			*decryptedLen = chunkCount * chunkLength; 
			*decryptedData = (byte*)malloc(*decryptedLen);

			// 3) Dekodujemy kazdy kawalek
			Word chunkData;
			Word decryptedWord;
			for(int i = 0; i < chunkCount; ++i)
			{
				byte* chunk = data + i * WordByteCount;
				byte* encChunk = *decryptedData + i * chunkLength;
				ByteArrayToWord(chunk, WordByteCount, &chunkData);
				decryptedWord = DecryptDataChunk(chunkData, privKey);
				WordToByteArray(encChunk, chunkLength, &decryptedWord);
			}
		}

		void DecryptMessage_FixedChunks(byte* data, uint64 dataLen, 
			uint32 chunkLength, uint32 lastChunkSize, byte** decryptedData, uint64* decryptedLen, RSAKey* privKey)
		{
			// 2) Znajdujemy liczbe kawalkow w wiadomosci (zdekodowana wiadomosc ma taka sama dlugosc)
			uint64 chunkCount = dataLen / WordByteCount - 1;
			if(lastChunkSize == 0)
				lastChunkSize = chunkLength;
			*decryptedLen = chunkCount * chunkLength + lastChunkSize; 
			*decryptedData = (byte*)malloc(*decryptedLen);

			// 3) Dekodujemy kazdy kawalek
			Word chunkData;
			Word decryptedWord;
			for(int i = 0; i < chunkCount; ++i)
			{
				byte* chunk = data + i * WordByteCount;
				byte* encChunk = *decryptedData + i * chunkLength;
				ByteArrayToWord(chunk, WordByteCount, &chunkData);
				decryptedWord = DecryptDataChunk(chunkData, privKey);
				WordToByteArray(encChunk, chunkLength, &decryptedWord);
			}

			// 4) Ostatni kawalek moze byc niepelny
			byte* chunk = data + chunkCount * WordByteCount;
			byte* encChunk = *decryptedData + chunkCount * chunkLength;
			ByteArrayToWord(chunk, WordByteCount, &chunkData);
			decryptedWord = DecryptDataChunk(chunkData, privKey);
			WordToByteArray(encChunk, lastChunkSize, &decryptedWord);
		}
	}
}