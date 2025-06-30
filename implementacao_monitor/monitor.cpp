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
    int aN = 0;//contador de chegadas do norte
    int aS = 0;//contador de chegadas do sul
    int eN = 0;// contador de entradas do norte
    int eS = 0;// contador de entradas do sul
    int sN = 0;// contador de saidas do norte
    int sS = 0;// contador de saidas do sul
    //o aN/aS sao estados iniciais dos carros
    //o eN/eS sao estados intermediarios dos carros
    //o sN/sS sao estados finais dos carros
public:
    void arrive(int direcao, int id) {
        //funcao da entrada do carro na ponte
        std::unique_lock<std::mutex> lock(mtx);

        if (direcao == 1) { // norte
            aN++;
            std::cout << "Carro " << id << " chegou do NORTE. Total aN: " << aN << "\n";
        } else { // sul
            aS++;
            std::cout << "Carro " << id << " chegou do SUL. Total aS: " << aS << "\n";
        }
        cv.wait(lock, [&]() {//espera a ponte estar livre ou na direcao correta para passar o carro
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
