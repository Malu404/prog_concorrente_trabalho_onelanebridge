#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <semaphore>
#include <chrono>
#include <string>

using namespace std;

// Inciando variáveis compartilhadas para que possamos controlar o estado da ponte

// Contadores para carros atualmente atravaessando a ponte
int atravessando_norte = 0;
int atravessando_sul = 0;

// Contadores para carros esperando para atravessar
int esperando_norte = 0;
int esperando_sul = 0;

mutex mtx; // Mutex para controlar o acesso dos carros as variáveis declaradas acima
mutex cout_mutex;// Mutex para garantir que a saída no console não seja embaralhada

// Semáforos para controlar o acesso em cada direção
counting_semaphore semaforto_norte{0};
counting_semaphore semaforto_sul{0};

// Vamos criar um funão com o intuito de simular a travessia de um carro pela ponte
void atravessando_ponte(int carro_placa, const string& direcao) {

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "O Carro " << carro_placa << " vindo do " << direcao << "esta atravessando a ponte." << endl;
    }
    
    // Configurando o tempo de travessia
    this_thread::sleep_for(chrono::seconds(3));
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "O Carro " << carro_placa << " vindo do " << direcao << " terminou de atravessar a ponte." << endl;
    }
}

// Lógica para um carro vindo do Norte
void carro_norte(int carro_placa) {
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Carro " << carro_placa << " do Norte chegou na ponte." << endl;
    }

    mtx.lock();

    // Observa se há carros do Sul atravessando a ponte, nesse caso o carro do Norte deve esperar
    if (atravessando_sul > 0) {
        esperando_norte++;
        mtx.unlock();
        // Espera até que o semáforo do Norte seja liberado
        semaforto_norte.acquire();
    } else {
        // Se não há carros do Sul, o carro do Norte pode atravessar
        atravessando_norte++;
        mtx.unlock(); 
    }

    // Chamando a função para atravessar a ponte
    atravessando_ponte(carro_placa, "Norte");
    
    // Aqui estamos após a saída do carro da ponte
    mtx.lock();
    atravessando_norte--;
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Carro " << carro_placa << " do Norte SAIU da ponte. Carros do Norte na ponte: " << atravessando_norte << endl;
    }

    // Garantindo fairness: Caso seja o último carro a sair tenta dar a vez para os carros do Sul
    if (atravessando_norte == 0 && esperando_sul > 0) {

        int carros_a_acordar = esperando_sul;
        esperando_sul = 0; // Reseta o contador de espera
        atravessando_sul = carros_a_acordar;

        {
            lock_guard<mutex> lock(cout_mutex);
            cout << ">>> Ponte liberada para o SUL! Acordando " << carros_a_acordar << " carros." << endl;
        }

        for (int i = 0; i < carros_a_acordar; ++i) {
            semaforto_sul.release();
        }
    }
    mtx.unlock();
}

// Lógica para um carro vindo do Sul
void carro_sul(int carro_placa) {
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Carro " << carro_placa << " do Sul chegou na ponte." << endl;
    }
    
    mtx.lock();

    // Observa se há carros do Norte atravessando a ponte, nesse caso o carro do Sul deve esperar
    if (atravessando_norte > 0) {
        esperando_sul++;
        mtx.unlock();
        // Espera até que o semáforo do Sul seja liberado
        semaforto_sul.acquire();
    } else {
        atravessando_sul++;
        mtx.unlock();
    }

    // Chamando a função para atravessar a ponte
    atravessando_ponte(carro_placa, "Sul");

    // Aqui estamos após a saída do carro da ponte
    mtx.lock();
    atravessando_sul--;
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "Carro " << carro_placa << " do Sul SAIU da ponte. Carros do Sul na ponte: " << atravessando_sul << endl;
    }

    // Garantindo fairness: Caso seja o último carro a sair tenta dar a vez para os carros do Norte
    if (atravessando_sul == 0 && esperando_norte > 0) {
        int carros_a_acordar = esperando_norte;
        esperando_norte = 0; // Reseta o contador de espera
        atravessando_norte = carros_a_acordar;
        
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << ">>> Ponte liberada para o NORTE! Acordando " << carros_a_acordar << " carros." << endl;
        }

        for (int i = 0; i < carros_a_acordar; ++i) {
            semaforto_norte.release();
        }
    }
    mtx.unlock();
}


int main() {
    vector<thread> carros;
    int carro_placa = 1;

    // Criando 3 carros do Norte (eles devem passar primeiro)
    for (int i = 0; i < 3; ++i) {
        carros.emplace_back(carro_norte, carro_placa++);
    }
    this_thread::sleep_for(chrono::milliseconds(100));

    // Criando 5 carros do Sul (eles devem esperar)
    for (int i = 0; i < 5; ++i) {
        carros.emplace_back(carro_sul, carro_placa++);
    }
    this_thread::sleep_for(chrono::milliseconds(100));

    // Criando mais 2 carros do Norte (eles devem passar com o primeiro grupo)
    for (int i = 0; i < 2; ++i) {
        carros.emplace_back(carro_norte, carro_placa++);
    }
    this_thread::sleep_for(chrono::seconds(4));

    // Criando mais 3 carros do Norte, quando eles chegarem é para estar na vez do sul
    // e eles devem esperar
    for (int i = 0; i < 3; ++i) {
        carros.emplace_back(carro_norte, carro_placa++);
    }

    for (auto& carro : carros) {
        carro.join();
    }

    return 0;
}