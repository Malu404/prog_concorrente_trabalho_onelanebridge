#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

enum direcao {
    NORTE = 1,
    SUL = 2
};

class OneLaneBridge {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int carro_ponte = 0;
    int direcao_atual = 0; // 0 = none, 1 = vem do norte, 2 = vem do sul
    int aN = 0;
    int aS = 0;
    int eN = 0;
    int eS = 0;
    int sN = 0;
    int sS = 0;
public:
    void arrive(int direcao, int id) {
        std::unique_lock<std::mutex> lock(mtx);

        if (direcao == 1) { // Norte
            aN++;
            std::cout << "Carro " << id << " chegou do NORTE. Total aN: " << aN << "\n";
        } else { // Sul
            aS++;
            std::cout << "Carro " << id << " chegou do SUL. Total aS: " << aS << "\n";
        }
        cv.wait(lock, [&]() {
            return carro_ponte == 0 || direcao_atual == direcao;
        });

        // Entrando na ponte
        carro_ponte++;
        direcao_atual = direcao;
        if (direcao == 1) {
            eN++;
        } else {
            eS++;
        }
        std::cout << "Carro " << id << " esta atravessando da direcao "
                  << (direcao == 1 ? "NORTE" : "SUL") << "\n";
    }

    void leave(int direcao, int id) {
        std::unique_lock<std::mutex> lock(mtx);
        if (direcao == 1){
            sN++;
        }else {
            sS++;
        }
        std::cout << "Carro " << id << " saiu da ponte. Total de carros do NORTE: " << sN
                  << ", do SUL: " << sS << "\n";
        carro_ponte--;
        std::cout << "Carro " << id << " saiu da ponte.\n";
        if (carro_ponte == 0) {
            direcao_atual = 0;
            cv.notify_all(); // Permitir carros do lado oposto
        }
    }
    void print_info(){
        std::cout << "\nPrintando informacoes:\n";
        std::cout << "Total de carros que chegaram do NORTE: " << aN << "\n";
        std::cout << "Total de carros que chegaram do SUL: " << aS  << "\n";
        std::cout << "Total de carros que sairam do NORTE: " << sN << "\n";
        std::cout << "Total de carros que sairam do SUL: " << sS << "\n";
        std::cout << "Carros na ponte: " << carro_ponte << "\n";
        std::cout << "Direcao atual da ponte: " << (direcao_atual == 1 ? "NORTE" : (direcao_atual == 2 ? "SUL" : "Nenhuma")) << "\n";
        std::cout << "---------------------------------\n";
    }
};

OneLaneBridge ponte;

void car_thread(int direcao, int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000)); // chegada aleatória
    ponte.arrive(direcao, id);

    // Simula a travessia
    std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 500));
    
    ponte.leave(direcao, id);
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
