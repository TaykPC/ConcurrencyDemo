#include "PrimeFactors.h"
#include <sstream>
#include "Timer.h"


// Teile _Referenzparameter_ 'n' so oft durch 'divisor'
// wie es ohne Rest möglich ist.
// Rückgabewert: Anzahl der erfolgten Divisionen.
int PrimeFactors::divide_as_often_as_possible(int& n, int divisor)
{
	int multiplicity = 0;

	while (n % divisor == 0) {
		++multiplicity;
		n /= divisor;
	}

	return multiplicity;
}


// Führe eine Primfaktorzerlegung der im Objekt gespeicherten Zahl 'n_' durch.
// Vorbedingung: n > 0 (wird nicht geprüft)
// Ermittelt pfactors_: Vektor von {Primfaktor, Exponent}-Paaren,
//   z.B. {{2,3}, {3,4}} für Zerlegung 2^3 * 3^4;
//   leerer Vektor für n = 1;
//   undefiniert für n < 1.
// Naiver Algorithmus:
//   Aufsteigend Teiler von 'n' suchen und durch ggf. wiederholte Division
//   aus 'n' "entfernen". Jeder Teiler, der dabei gefunden wird, ist automatisch
//   eine Primzahl. Wenn 'n == 1' erreicht ist, ist die Primfaktorzerlegung abgeschlossen.
void PrimeFactors::calc_factors()
{
	if (pfactors_.size() == 0) {  // noch keine Berechnung erfolgt
		Timer t1;
		int n = n_; // Kopie anlegen wegen Referenzparameter in divide_as_often_as_possible

		// Teste gerade Zahlen separat, um in der Schleife unten Doppelschritte machen zu können
		if (n % 2 == 0) {
			pfactors_.push_back({ 2, divide_as_often_as_possible(n, 2) });
		}

		for (int k = 3; n > 1; k += 2)
			if (n % k == 0)
				pfactors_.push_back({ k, divide_as_often_as_possible(n, k) });
		calctime_in_ms_ = t1.runtime_in_ms();
	}
}


std::string PrimeFactors::get_factors_as_string() {
	calc_factors();
	std::ostringstream ostr;
	bool first_factor = true;
	ostr << std::setw(10) << n_ << " = ";
	for (const auto&[p, k] : pfactors_) {     // Ausgabe der Art 2^5 * 3^2 * 11^7 erzeugen
		if (first_factor)                        // nur zwischen Faktoren ein '*', nicht vor dem ersten Faktor
			first_factor = false;
		else
			ostr << " * ";
		ostr << p;
		if (k > 1)                               // statt z.B. '2^1' nur '2' anzeigen
			ostr << '^' << k;
	}

	// Ausgabe mit Leerstellen auffüllen, damit Zeitangabe fixe Spalte bekommt
	size_t ostr_fmt_size = ostr.str().size();
	ostr << std::setw(50 - ostr_fmt_size) << ' '
		<< "(" << (calctime_in_ms_ == 0 ? "<1" : std::to_string(calctime_in_ms_)) << "ms)";  // aus '0ms' wird '<1ms'

	return ostr.str();
}

