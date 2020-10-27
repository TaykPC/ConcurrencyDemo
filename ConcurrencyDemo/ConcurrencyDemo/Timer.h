#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>
#include <iomanip>

// FIXME: Wilder Hack, benötigt ein Cleanup
class Timer {
	using tp_t = std::chrono::steady_clock::time_point;
	std::string timer_name_;
	size_t number_of_operations_ = 0;
	std::string op_name_singular_;
	std::string op_name_plural_;
	int min_output_timer_name_length_;
	tp_t time1_;
	tp_t time2_;
	bool timer_running_ = false;

public:
	Timer(int min_output_timer_name_length = 80) : min_output_timer_name_length_{ min_output_timer_name_length } {
		start();
	}

	void start(const std::string& timer_name = "", size_t number_of_operations = 0,
		const std::pair<std::string, std::string> op_name = { "Op  ", "Ops  " }) {
		timer_name_ = timer_name;
		number_of_operations_ = number_of_operations;
		op_name_singular_ = op_name.first;
		op_name_plural_ = op_name.second;
		time1_ = std::chrono::steady_clock().now();
		timer_running_ = true;
	}

	void stop() {
		if (timer_running_) {
			time2_ = std::chrono::steady_clock().now();
			timer_running_ = false;
		}
	}

	void stop_and_print_runtime() {
		stop();
		print_runtime();
	}

	std::chrono::milliseconds::rep runtime_in_ms() {
		stop();
		return std::chrono::duration_cast<std::chrono::milliseconds>(time2_ - time1_).count();
	}

	std::chrono::microseconds::rep runtime_in_us() {
		stop();
		return std::chrono::duration_cast<std::chrono::microseconds>(time2_ - time1_).count();
	}

	std::ostream& print_runtime() {
		time2_ = std::chrono::steady_clock().now();  // Timer läuft weiter, falls er nicht zuvor gestoppt wurde

		if (timer_name_.empty())
			std::cout << "Laufzeit: ";
		else
			std::cout << std::setw(min_output_timer_name_length_) << timer_name_ << ": ";

		if (number_of_operations_ > 0)
			std::cout << std::setw(6) << std::setprecision(3) << std::fixed
			<< runtime_in_us() / static_cast<double>(number_of_operations_) << "us / " << op_name_singular_
			<< " [" << std::setw(8) << number_of_operations_ << " " << op_name_plural_ << " in " << std::setw(4) << runtime_in_ms() << " ms]";
		else
			std::cout << std::setw(4) << runtime_in_ms() << " ms";

		return std::cout << std::endl;
	}
};


inline std::ostream& operator<<(std::ostream& ostr, Timer& timer) {
	return ostr << "Laufzeit: " << timer.runtime_in_ms() << " ms";
}

#endif // TIMER_H
