#pragma once

#include "TypeDefs.h"
#include <boost\multiprecision\cpp_int.hpp>

namespace ImgOps
{
	namespace RSA
	{
		static const int WordByteCount = 128;
		static const int WordBitCount = 1024;

		//typedef boost::multiprecision::int1024_t Word;
		typedef boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
			WordBitCount + 2, WordBitCount + 2, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void>> 
			Word;

		typedef boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
			(WordBitCount + 2)*2, (WordBitCount + 2)*2, boost::multiprecision::signed_magnitude, boost::multiprecision::unchecked, void>> 
			Word2; // Word that contains at least 2 * WordBitCount

		struct RSAKey
		{
			Word n;
			Word e;
		};

		// Converts array of bytes of length 'len' to Word and stores in 'word'
		void ByteArrayToWord(byte* bytes, uint32 len, Word* word);

		// Converts Word to array of bytes of max length 'len'
		void WordToByteArray(byte* bytes, uint32 len, Word* word);
		
		// Converts Word to array of bytes of length WordByteCount
		void WordToByteArray_FullWord(byte* bytes, Word* word);

		Word Rand();
		Word Rand(Word rangeMin, Word rangeMax);

		// Mnozenie modulo : a*b mod k
		Word ModuloMultiplcation(Word a, Word b, Word k);

		// Potegowanie modulo : a^e mod k
		// Poteguje uzywajac 'Right-to-left binary method'
		Word ModuloPower(Word base, Word e, Word k);

		// Funkcja obliczająca NWD dla dwóch liczb (algorytm Euklidesa)
		Word Nwd(Word a, Word b);

		// Funkcja obliczania odwrotności modulo n (rozszerzony algorytm Euklidesa)
		Word FindModuloInverse(Word a, Word n);

		// Sprawdza czy liczba 'num' jest pierwsza z prawdopodobienstwem 1 - (1/4)^'passes'
		// Niech p będzie nieparzystą liczbą pierwszą zapisaną jako p = 1 + 2^s * d, gdzie d jest nieparzyste. 
		// Wtedy dla dowolnej liczby naturalnej a ∈ <2, p - 2> ciąg Millera-Rabina:
		// a^d, a^(2d), a^(4d), ..., a^(2^(s-1)*d), a^(2^s*d) (mod p)
		// kończy się liczbą 1. 
		// Co więcej, jeśli a^d nie przystaje modulo p do 1, to wyraz ciągu 
		// Millera-Rabina bezpośrednio poprzedzający 1 jest równy p - 1.
		// Jeśli liczba p przejdzie test, to jest albo pierwsza, albo silnie pseudopierwsza przy podstawie a.
		bool CheckMillerRabin(Word num, int passes);

		// Zwraca pare licz pierwszych (p,q) takich ze n = p*q ma zadana liczbe bitow (N = WordBitCount) -> n > 2^(N-1)
		// Czyli dla danego p, q > 2^(N-1) / p i q < 2^N / p (czyli q = (2^(N-1) / p, 2^N / p))
		// Prawdopodobienstwo ze p i q sa rzeczywiscie pierwsze to 1 - (1/4)^'passes'
		void GeneratePrimaryPair(Word* p, Word* q, int testPasses = 10);

		// Generuje pare kluczy RSA, (n,e) stanowia klucz publiczny, (n,d) klucz prywatny
		// Liczby (p,q) sluzace do wyznaczenia kluczy sa pierwsze z prawdopodobienstwem (1 - 'maxPrimeError')
		void GenerateRSAKeys(RSAKey* pubKey, RSAKey* privKey, double maxPrimeError = 1e-10);

		// Zaszyfrowuje pojedynczy kawalek danych
		Word EncryptDataChunk(Word data, RSAKey* pubKey);
		// Rozszyfrowuje pojedynczy kawalek danych
		Word DecryptDataChunk(Word data, RSAKey* privKey);

		// Koduje wiadomosc : tablice bajtow o podanej dlugosci uzywajac klucza publicznego
		// Wynik kodowania zapisany w 'encryptedData'
		void EncryptMessage(byte* data, uint64 dataLen, byte** encryptedData, uint64* encryptedLen, RSAKey* pubKey);
		
		// Koduje wiadomosc : tablice bajtow o podanej dlugosci uzywajac klucza publicznego
		// Wynik kodowania zapisany w 'encryptedData'
		// Koduje kawalki danych o ustalonej dlugosci 'chunkSize' (w bajtach)
		// (zakladane jest ze max liczba w kawalku < n oraz dataLen jest podzielna przez chunkLength)
		// Ostatni kawalek ma dlugosc 'lastChunkSize' jesli jest != 0
		void EncryptMessage_FixedChunks(byte* data, uint64 dataLen, 
			uint32 chunkLength, uint32* lastChunkSize, 
			byte** encryptedData, uint64* encryptedLen, RSAKey* pubKey);

		// Dekoduje wiadomosc : tablice  bajtow o podanej dlugosci uzywajac klucza prywatnego
		// Wynik dekodowania zapisany w 'decryptedData'
		void DecryptMessage(byte* encryptedData, uint64 encryptedLen, byte** decryptedData, uint64* decryptedLen, RSAKey* privKey);
		
		// Dekoduje wiadomosc : tablice  bajtow o podanej dlugosci uzywajac klucza prywatnego
		// Wynik dekodowania zapisany w 'decryptedData'
		// Dekodowane kawalki danych o dlugosci Word sa zapisywane w kawalkach o ustalonej dlugosci 'chunkSize' (w bajtach)
		// (zakladane jest ze max liczba w kawalku < n)
		// Ostatni kawalek ma dlugosc 'lastChunkSize' jesli jest != 0
		void DecryptMessage_FixedChunks(
			byte* encryptedData, uint64 encryptedLen,
			uint32 chunkLength, uint32 lastChunkSize,
			byte** decryptedData, uint64* decryptedLen, RSAKey* privKey);
	}
}