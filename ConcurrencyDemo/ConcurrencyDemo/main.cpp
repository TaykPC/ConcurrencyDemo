// MSVC-Hack: Korrekte Codepage-Erkennung erzwingen: дцья

#include <windows.h>
#include <iostream>
#include <iomanip>


namespace Concurrency {
	void run();
}


int main() {
	SetConsoleOutputCP(CP_UTF8);

	try {
		Concurrency::run();
	}
	catch (std::exception& e) {
		std::cerr << "EXCEPTION: " << e.what() << std::endl;
	}

	std::cin.ignore();
}
