#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <list>
#include <forward_list>
#include <memory>
#include <algorithm>
#include <execution>
#include <iterator>
#include <functional>
#include <chrono>
#include <thread>
#include <future>
#include "Timer.h"
#include "PrimeFactors.h"

using namespace std::literals;
using std::cout, std::flush, std::endl;

namespace Concurrency {
    int fib(int n) {
        if (n < 2)
            return n;

        return fib(n - 2) + fib(n - 1);
    }

    void run1() {
        cout << "Einfache fib-Berechnung in separatem Thread\n" << endl;

        std::thread thr{ []() { std::cout << fib(30) << std::endl; } };
        // … Main Thread arbeitet weiter…
        // … Hilfsthread gibt 832040 aus …
        thr.join();
    }

    void run2() {
        cout << "Paralleles Inkrementieren einer Shared Variable ohne Synchronisation\n" << endl;

        int shared_var = 0;

        auto fct = [&shared_var]() {
            for (int i = 0; i < 10'000'000; ++i) {
                ++shared_var;
            }
        };

        Timer t1;

        for (int i = 0; i < 5; ++i) {
            shared_var = 0;

            std::thread thr1{ fct };
            std::thread thr2{ fct };
            std::thread thr3{ fct };

            thr1.join();
            thr2.join();
            thr3.join();

            cout << "shared_var = " << shared_var << endl;
        }
        t1.stop_and_print_runtime();
    }


    void run3() {
        cout << "Paralleles Inkrementieren einer Shared Variable mit Synchronisation via std::mutex\n" << endl;

        int shared_var = 0;
        std::mutex mtx;

        auto fct = [&shared_var, &mtx]() {
            for (int i = 0; i < 10'000'000; ++i) {
                mtx.lock();
                ++shared_var;
                mtx.unlock();
            }
        };

        Timer t1;

        for (int i = 0; i < 5; ++i) {
            shared_var = 0;

            std::thread thr1{ fct };
            std::thread thr2{ fct };
            std::thread thr3{ fct };

            thr1.join();
            thr2.join();
            thr3.join();

            cout << "shared_var = " << shared_var << endl;
        }
        t1.stop_and_print_runtime();
    }


    void run3b() {
        cout << "Paralleles Inkrementieren einer Shared Variable mit Synchronisation via std::lock_guard\n" << endl;

        int shared_var = 0;
        std::mutex mtx;

        auto fct = [&shared_var, &mtx]() {
            for (int i = 0; i < 10'000'000; ++i) {
                std::lock_guard<std::mutex> lck{ mtx };
                ++shared_var;
            }
        };

        Timer t1;

        for (int i = 0; i < 5; ++i) {
            shared_var = 0;

            std::thread thr1{ fct };
            std::thread thr2{ fct };
            std::thread thr3{ fct };

            thr1.join();
            thr2.join();
            thr3.join();

            cout << "shared_var = " << shared_var << endl;
        }
        t1.stop_and_print_runtime();
    }


    void run3c() {
        cout << "Paralleles Inkrementieren einer Shared Variable vom Typ std::atomic<int>\n" << endl;

        std::atomic<int> shared_var = 0;

        auto fct = [&shared_var]() {
            for (int i = 0; i < 10'000'000; ++i) {
                ++shared_var;
            }
        };

        Timer t1;

        for (int i = 0; i < 5; ++i) {
            shared_var = 0;

            std::thread thr1{ fct };
            std::thread thr2{ fct };
            std::thread thr3{ fct };

            thr1.join();
            thr2.join();
            thr3.join();

            cout << "shared_var = " << shared_var << endl;
        }
        t1.stop_and_print_runtime();
    }


    void run4() {
        cout << "Producer / Consumer mit std::condition_variable\n" << endl;

        std::queue<int> fibs_to_do;
        std::mutex mtx;
        std::condition_variable cond;

        auto fct_put_work = [&]() {
            for (int n = 1; n <= 40; ++n) {
                std::lock_guard<std::mutex> lck{ mtx };
                fibs_to_do.push(n);
                if (n == 40)
                    fibs_to_do.push(0);  // Ende signalisieren
                cond.notify_one();
            }
        };

        auto fct_do_work = [&]() {
            while (true) {
                int fib_to_do;
                {
                    std::unique_lock<std::mutex> lck{ mtx };
                    cond.wait(lck, [&]() { return !fibs_to_do.empty(); });
                    fib_to_do = fibs_to_do.front();
                    fibs_to_do.pop();
                    if (fib_to_do == 0)
                        return;
                }
                std::cout << "fib(" << fib_to_do << ") = "
                    << fib(fib_to_do) << std::endl;
            } // Ausgabe: Fibonacci-Zahlen von 1..40
        };

        std::thread thr1{ fct_put_work };
        std::thread thr2{ fct_do_work };
        thr1.join();
        thr2.join();
    }


    void run5() {
        cout << "Berechnen von fib(42) in separatem Thread mit std::future\n" << endl;

        std::promise<int> pr;                                 // Der Bewahrer des „Shared State“
        std::future<int> fut{ pr.get_future() };              // Zugang verschaffen zum „Lese-Ende“ des Mini-Kanals

        auto fct_do_work = [](std::promise<int> pr, int n) {
            pr.set_value(fib(n));                               // fib(n) berechnen und in der Promise hinterlegen
        };

        std::thread thr{ fct_do_work, std::move(pr), 42 };    // Arbeit in neuem Thread anstoßen
        cout << fut.get() << endl;                            // std::future::get() wartet auf das Ergebnis → 267914296
        thr.join();
    }


    void run5b() {
        cout << "Berechnen von fib(42) in separatem Thread mit std::packaged_task\n" << endl;

        //    std::packaged_task<int(int)> task{ [](int n) { return fib(n); } };
        std::packaged_task<int(int)> task{ fib };
        std::future<int> fut{ task.get_future() };

        std::thread thr{ std::move(task), 42 };
        cout << fut.get() << endl;
        thr.join();
    }


    void run5c() {
        cout << "Berechnen von fib(42) mit std::async\n" << endl;

        //    auto do_work = [](int n) { return fib(n); };
        //    std::future<int> fut{ std::async(do_work, 42) };
        std::future<int> fut{ std::async(fib, 42) };

        cout << fut.get() << endl;
    }


    void run6() {
        cout << "Berechnen der Fibonacci-Zahlen von 1..42:\n" << endl;

        std::vector<int> fib_in(42), fib_out(42);
        std::iota(fib_in.begin(), fib_in.end(), 1);  // Befüllen von fib_in mit 1..42

        Timer t1;

        cout << "  Sequenzielles std::transform" << endl;
        t1.start();
        //    auto do_work = [](int n) { return fib(n); };
        //    std::transform(fib_in.begin(), fib_in.end(), fib_out.begin(), do_work);
        std::transform(fib_in.begin(), fib_in.end(), fib_out.begin(), fib);
        t1.stop();
        long long runtime_seq_in_ms = t1.runtime_in_ms();

        cout << "  Paralleles std::transform" << endl;
        t1.start();
        //    std::transform(std::execution::par_unseq, fib_in.begin(), fib_in.end(), fib_out.begin(), do_work);
        std::transform(std::execution::par_unseq, fib_in.begin(), fib_in.end(), fib_out.begin(), fib);
        t1.stop();
        long long runtime_par_in_ms = t1.runtime_in_ms();

        cout << "Ergebnisse:\n" << endl;
        for (size_t n = 0; n < fib_out.size(); ++n) {
            cout << "fib(" << n + 1 << ") = " << fib_out[n] << endl;
        }

        cout << "\nLaufzeit sequenziell: " << std::setw(4) << runtime_seq_in_ms << "ms" << endl;
        cout << "Laufzeit parallel:    " << std::setw(4) << runtime_par_in_ms << "ms" << endl;
    }


    void run7() {
        const int lower_limit = 1'000'000'001;
        const int upper_limit = lower_limit + 50;
        cout << "Berechnung von Primfaktoren der Zahlen " << lower_limit
            << " bis " << upper_limit << "\n" << endl;

        std::vector<PrimeFactors> primeFactors;
        Timer t1;

        bool print_results = false;

        for (int i = lower_limit; i < upper_limit; i += 2)
            primeFactors.push_back(i);

        const int hw_concurrency = std::thread::hardware_concurrency();
        cout << "Hardware Concurrency: " << std::thread::hardware_concurrency() << endl;

        {
            cout << "\nSequenzielle Berechnung:" << endl;

            t1.start();
            for (auto& pf : primeFactors)
                pf.calc_factors();
            t1.stop();

            //      if (print_results)
            for (auto& pf : primeFactors)
                cout << pf.get_factors_as_string() << endl;

            cout << "\nSequenzielle Laufzeit: " << t1.runtime_in_ms() << "ms" << endl;
        }

        auto calc_factors =
            [&primeFactors](size_t low, size_t high) -> void {
            for (size_t i = low; i < high; ++i)
                primeFactors[i].calc_factors();
        };

        for (auto& pf : primeFactors)
            pf.clear_factors();

        cout << "\nfor_each std::execution::par_unseq: " << flush;
        t1.start();
        std::for_each(std::execution::par_unseq, primeFactors.begin(), primeFactors.end(), [](PrimeFactors& pf) { pf.calc_factors(); });
        t1.stop();
        cout << t1.runtime_in_ms() << "ms" << endl;
        if (print_results)
            for (auto& pf : primeFactors)
                cout << pf.get_factors_as_string() << endl;

        {
            for (auto& pf : primeFactors)
                pf.clear_factors();

            cout << "\nThreads naiv für Einzelwerte (" << primeFactors.size() << " Threads): " << flush;

            std::vector<std::thread> threads;
            t1.start();

            for (size_t i = 0; i < primeFactors.size(); ++i) {
                std::thread thr{ calc_factors, i, i + 1 };
                threads.push_back(std::move(thr));
            }

            for (auto& thr : threads)
                thr.join();

            t1.stop();

            if (print_results)
                for (auto& pf : primeFactors)
                    cout << pf.get_factors_as_string() << endl;

            cout << t1.runtime_in_ms() << "ms" << endl;
        }

        for (auto& pf : primeFactors)
            pf.clear_factors();

        {
            cout << "\nasync naiv für Einzelwerte (" << primeFactors.size() << " Aufrufe)" << endl;
            std::vector<std::future<void>> results;
            t1.start();

            for (size_t i = 0; i < primeFactors.size(); ++i) {
                results.push_back(std::async(calc_factors, i, i + 1));
            }

            cout << "Warten auf Ergebnisse (std::future) ... " << std::flush;
            for (size_t i = 0; i < results.size(); ++i) {
                results[i].wait();
            }
            cout << "fertig" << std::endl;

            if (print_results)
                for (auto& pf : primeFactors)
                    cout << pf.get_factors_as_string() << endl;

            t1.stop();
            cout << "async naiv für Einzelwerte: Laufzeit: " << t1.runtime_in_ms() << "ms" << endl;
        }


        for (auto& pf : primeFactors)
            pf.clear_factors();

        size_t elems_per_group = primeFactors.size() / hw_concurrency;
        size_t elems_per_group_rest = primeFactors.size() % hw_concurrency;

        {
            cout << "\nThreads für Gruppen:" << endl;

            std::vector<std::thread> threads;
            t1.start();

            for (size_t low = 0, high, rest = elems_per_group_rest; low < primeFactors.size(); low = high) {
                high = low + elems_per_group;
                if (rest > 0) { ++high; --rest; }
                high = std::min(high, primeFactors.size());
                std::thread thr{ calc_factors, low, high };
                cout << "Start Thread " << std::setw(5) << thr.get_id() << " für Bereich " << low << " bis " << high << endl;
                threads.push_back(std::move(thr));
            }
            for (auto& thr : threads) {
                //        cout << "Joining Thread " << thr.get_id() << endl;
                thr.join();
            }
            t1.stop();

            if (print_results)
                for (auto& pf : primeFactors)
                    cout << pf.get_factors_as_string() << endl;

            cout << "\nThreads für Gruppen: Laufzeit: " << t1.runtime_in_ms() << "ms" << endl;
        }

        for (auto& pf : primeFactors)
            pf.clear_factors();

        {
            cout << "\nasync für Gruppen:" << endl;
            std::vector<std::future<void>> results;
            t1.start();

            for (size_t low = 0, high, rest = elems_per_group_rest; low < primeFactors.size(); low = high) {
                high = low + elems_per_group;
                if (rest > 0) { ++high; --rest; }
                high = std::min(high, primeFactors.size());
                cout << "async für Bereich " << low << " bis " << high << endl;
                results.push_back(std::async(calc_factors, low, high));
            }

            for (size_t i = 0; i < results.size(); ++i) {
                //        cout << "Warten auf Future(" << i << ")" << endl;
                results[i].wait();
            }
            t1.stop();

            if (print_results)
                for (auto& pf : primeFactors)
                    cout << pf.get_factors_as_string() << endl;

            cout << "\nasync für Gruppen: Laufzeit: " << t1.runtime_in_ms() << "ms" << endl;
        }

        cout << endl;
        for (int number_of_threads = hw_concurrency; number_of_threads <= hw_concurrency + 16; number_of_threads += 2) {
            cout << "Threads holen sich Arbeit (" << number_of_threads << " Threads): " << flush;

            for (auto& pf : primeFactors)
                pf.clear_factors();

            std::vector<std::thread> threads;
            std::mutex mtx;
            size_t current_idx = 0;
            auto do_work = [&]() {
                while (true) {
                    mtx.lock();
                    size_t my_idx = current_idx++;
                    mtx.unlock();
                    if (my_idx >= primeFactors.size())  // keine Arbeit mehr zu erledigen
                        return;
                    primeFactors[my_idx].calc_factors();
                }
            };

            t1.start();

            for (int i = 0; i < number_of_threads; ++i) {
                std::thread thr{ do_work };
                threads.push_back(std::move(thr));
            }
            for (auto& thr : threads) {
                thr.join();
            }
            t1.stop();

            if (print_results)
                for (auto& pf : primeFactors)
                    cout << pf.get_factors_as_string() << endl;

            cout << t1.runtime_in_ms() << "ms" << endl;
        }
    }


    void run() {
        run1(); // einfaches fib in separatem Thread
        run2();  // Race Condition
        run3();  // Mutex
        run3b(); // Mutex mit lock_guard
        run3c(); // Atomic
        run4();  // condition_variable
        run5();  // future
        run5b(); // packaged_task
        run5c(); // async
        run6();  // sequenzielles / paralleles fib mit std::tranform
        run7();  // Primfaktoren
    }
}
