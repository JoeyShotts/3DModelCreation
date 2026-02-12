#include <iostream>
#include <thread>

int main(){
    unsigned int n_threads = std::thread::hardware_concurrency();
    if (n_threads > 0) {
        std::cout << "System has an estimated " << n_threads << " concurrent threads available." << std::endl;
    } else {
        std::cout << "hardware_concurrency is not supported or could not be determined." << std::endl;
    }
}
