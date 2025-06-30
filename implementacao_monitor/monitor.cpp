#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

class OneLaneBridge {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int carro_ponte = 0;
    int direcao_atual = 0; // 0 = none, 1 = vem do norte, 2 = vem do sul

public:
    void arrive(int direcao, int id) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() {
            return carro_ponte == 0 || direcao_atual == direcao;
        });

        // Entrando na ponte
        carro_ponte++;
        direcao_atual = direcao;
        std::cout << "Carro " << id << " esta atravessando da direcao "
                  << (direcao == 1 ? "NORTE" : "SUL") << "\n";
    }

    void leave(int id) {
        std::unique_lock<std::mutex> lock(mtx);
        carro_ponte--;
        std::cout << "Carro " << id << " saiu da ponte.\n";
        if (carro_ponte == 0) {
            direcao_atual = 0;
            cv.notify_all(); // Permitir carros do lado oposto
        }
    }
};

OneLaneBridge bridge;

void car_thread(int direcao, int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000)); // chegada aleatória
    bridge.arrive(direcao, id);

    // Simula a travessia
    std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 500));
    
    bridge.leave(id);
}

int main() {
    srand(time(NULL));
    std::vector<std::thread> cars;

    const int num_cars_norte = 5;
    const int num_cars_sul = 5;

    // Cria carros vindo do norte
    for (int i = 0; i < num_cars_norte; ++i)
        cars.emplace_back(car_thread, 1, 100 + i); // 1 = norte

    // Cria carros vindo do sul
    for (int i = 0; i < num_cars_sul; ++i)
        cars.emplace_back(car_thread, 2, 200 + i); // 2 = sul

    for (auto& t : cars)
        t.join();

    return 0;
}
