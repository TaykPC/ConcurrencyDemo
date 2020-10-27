#ifndef PRIMEFACTORS_H
#define PRIMEFACTORS_H

#include <vector>
#include <stdexcept>

class PrimeFactors {
	const int n_;
	std::vector<std::pair<int, int>> pfactors_;
	long long calctime_in_ms_ = 0;
	int divide_as_often_as_possible(int& n, int divisor);
public:
	PrimeFactors(int n) : n_{ n } {  // pfactors wird erst durch get_factors berechnet
		if (n < 2) throw std::invalid_argument{ "PrimeFactors: n < 2" };
	}

	void calc_factors();

	void clear_factors() {
		pfactors_.clear();
	}

	const std::vector<std::pair<int, int>>& get_factors() {
		calc_factors(); // ist no-op, falls Berechnung bereits erfolgte
		return pfactors_;
	}

	std::string get_factors_as_string();
};


#endif // PRIMEFACTORS_H
