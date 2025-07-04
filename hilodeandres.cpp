#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include "picosha2.h"
#include <cstdio> // Para remove()
#include <ctime>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
using namespace std;

std::mutex cout_mutex;

long long tiempoTotalBase = 0;
long long tiempoTotalOptimizado = 0;

// Funci�n segura para imprimir desde hilos
void printThreadSafe(const string &msg)
{
    std::lock_guard<std::mutex> lock(cout_mutex);
    cout << msg << endl;
}

string encrypt(string contenido)
{
    string encryptedContent = "";
    for (size_t i = 0; i < contenido.size(); ++i)
    {
        char c = contenido[i];
        if (isdigit(c))
        {
            encryptedContent += '9' - (c - '0');
        }
        else if (isupper(c))
        {
            encryptedContent += ((c - 'A' + 3) % 26) + 'A';
        }
        else if (islower(c))
        {
            encryptedContent += ((c - 'a' + 3) % 26) + 'a';
        }
        else
        {
            encryptedContent += c;
        }
    }
    return encryptedContent;
}

string decrypt(string contenido)
{
    string encryptedContent = "";
    for (size_t i = 0; i < contenido.size(); ++i)
    {
        char c = contenido[i];
        if (isdigit(c))
        {
            encryptedContent += '9' - (c - '0');
        }
        else if (isupper(c))
        {
            encryptedContent += ((c - 'A' - 3 + 26) % 26) + 'A';
        }
        else if (islower(c))
        {
            encryptedContent += ((c - 'a' - 3 + 26) % 26) + 'a';
        }
        else
        {
            encryptedContent += c;
        }
    }
    return encryptedContent;
}

void duplicate(int n)
{
    string sourceFile = "original.txt";
    for (int i = 1; i <= n; ++i)
    {
        ifstream src(sourceFile.c_str(), ios::binary);
        stringstream ss;
        ss << i;
        string nombreArchivo = ss.str() + ".txt";
        ofstream dst(nombreArchivo.c_str(), ios::binary);
        if (!src)
        {
            printThreadSafe("No se pudo abrir el archivo fuente: " + sourceFile);
            return;
        }
        if (!dst)
        {
            printThreadSafe("No se pudo crear el archivo destino: " + nombreArchivo);
            return;
        }
        dst << src.rdbuf();
        src.close();
        dst.close();
    }
}

void procesarArchivo(int i)
{
    stringstream ss;
    ss << i;
    string nombreArchivo = ss.str() + ".txt";
    string nombreHash = ss.str() + ".sha";
    string contenido;

    clock_t start = clock();

    // Encriptar
    ifstream inFile(nombreArchivo.c_str());
    if (inFile.is_open())
    {
        getline(inFile, contenido, '\0');
        inFile.close();
        string textoEncriptado = encrypt(contenido);
        ofstream outFile(nombreArchivo.c_str());
        if (outFile.is_open())
        {
            outFile << textoEncriptado;
            outFile.close();
            // Calcular y guardar el hash del archivo encriptado
            string hash = picosha2::hash256_hex_string(textoEncriptado);
            ofstream hashFile(nombreHash.c_str());
            if (hashFile.is_open())
            {
                hashFile << hash;
                hashFile.close();
            }
            else
            {
                printThreadSafe("No se pudo escribir el archivo hash para: " + nombreArchivo);
            }
        }
        else
        {
            printThreadSafe("No se pudo escribir el archivo para encriptar: " + nombreArchivo);
        }

        // Desencriptar
        ifstream inFile2(nombreArchivo.c_str());
        if (inFile2.is_open())
        {
            getline(inFile2, contenido, '\0');
            inFile2.close();
            // Verificar hash
            string hashGuardado, hashActual;
            ifstream hashFile(nombreHash.c_str());
            if (hashFile.is_open())
            {
                getline(hashFile, hashGuardado, '\0');
                hashFile.close();
                hashActual = picosha2::hash256_hex_string(contenido);
                if (hashGuardado != hashActual)
                {
                    printThreadSafe("El hash del archivo " + nombreArchivo + " no coincide con el guardado.");
                    return;
                }
            }
            else
            {
                printThreadSafe("No se pudo abrir el archivo hash para: " + nombreArchivo);
                return;
            }
            string textoDesencriptado = decrypt(contenido);
            ofstream outFile2(nombreArchivo.c_str());
            if (outFile2.is_open())
            {
                outFile2 << textoDesencriptado;
                outFile2.close();
                ifstream inFile3("original.txt");
                getline(inFile3, contenido, '\0');
                inFile3.close();
                if (contenido != textoDesencriptado)
                {
                    printThreadSafe("El archivo " + nombreArchivo + " no coincide con el original.");
                }
            }
            else
            {
                printThreadSafe("No se pudo escribir el archivo para desencriptar: " + nombreArchivo);
            }
        }
    }
    else
    {
        printThreadSafe("No se pudo abrir el archivo " + nombreArchivo + " para encriptar");
    }

    clock_t end = clock();
    double duration = double(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printThreadSafe("Tiempo " + to_string(i) + ": " + to_string(duration) + " ms");
}

void procesarArchivoOptimizado(int i)
{
    stringstream ss;
    ss << i;
    string nombreArchivo = ss.str() + ".txt";
    string nombreHash = ss.str() + ".sha";

    clock_t start = clock();

    // Leer archivo completo
    ifstream inFile(nombreArchivo, ios::binary | ios::ate);
    if (!inFile.is_open())
    {
        printThreadSafe("No se pudo abrir el archivo " + nombreArchivo + " para encriptar");
        return;
    }
    streamsize size = inFile.tellg();
    inFile.seekg(0, ios::beg);
    string contenido(size, '\0');
    inFile.read(&contenido[0], size);
    inFile.close();

    // Encriptar y escribir archivo completo
    string textoEncriptado = encrypt(contenido);
    ofstream outFile(nombreArchivo, ios::binary | ios::trunc);
    outFile.write(textoEncriptado.data(), textoEncriptado.size());
    outFile.close();

    // Calcular y guardar hash
    string hash = picosha2::hash256_hex_string(textoEncriptado);
    ofstream hashFile(nombreHash, ios::trunc);
    hashFile << hash;
    hashFile.close();

    // Leer archivo encriptado completo
    ifstream inFile2(nombreArchivo, ios::binary | ios::ate);
    if (!inFile2.is_open())
    {
        printThreadSafe("No se pudo abrir el archivo " + nombreArchivo + " para desencriptar");
        return;
    }
    streamsize size2 = inFile2.tellg();
    inFile2.seekg(0, ios::beg);
    string contenidoEncriptado(size2, '\0');
    inFile2.read(&contenidoEncriptado[0], size2);
    inFile2.close();

    // Verificar hash
    ifstream hashFileIn(nombreHash);
    string hashGuardado;
    getline(hashFileIn, hashGuardado, '\0');
    hashFileIn.close();
    string hashActual = picosha2::hash256_hex_string(contenidoEncriptado);
    if (hashGuardado != hashActual)
    {
        printThreadSafe("El hash del archivo " + nombreArchivo + " no coincide con el guardado.");
        return;
    }

    // Desencriptar y escribir archivo completo
    string textoDesencriptado = decrypt(contenidoEncriptado);
    ofstream outFile2(nombreArchivo, ios::binary | ios::trunc);
    outFile2.write(textoDesencriptado.data(), textoDesencriptado.size());
    outFile2.close();

    // Leer original completo y comparar
    ifstream inFile3("original.txt", ios::binary | ios::ate);
    if (inFile3.is_open())
    {
        streamsize size3 = inFile3.tellg();
        inFile3.seekg(0, ios::beg);
        string original(size3, '\0');
        inFile3.read(&original[0], size3);
        inFile3.close();
        if (original != textoDesencriptado)
        {
            printThreadSafe("El archivo " + nombreArchivo + " no coincide con el original.");
        }
    }

    clock_t end = clock();
    double duration = double(end - start) * 1000.0 / CLOCKS_PER_SEC;
    printThreadSafe("Tiempo " + to_string(i) + ": " + to_string(duration) + " ms");
}

long long manejarArchivo(int n)
{
    clock_t inicioTotal = clock();
    time_t tiempoInicio = time(0);
    printThreadSafe("TI: " + string(ctime(&tiempoInicio)));
    for (int i = 1; i <= n; i++)
    {
        procesarArchivo(i);
    }
    clock_t finTotal = clock();
    double duracionTotal = double(finTotal - inicioTotal) * 1000.0 / CLOCKS_PER_SEC;
    double duracionPromedio = duracionTotal / n;
    time_t tiempoFinal = time(0);
    printThreadSafe("\nTFIN: " + string(ctime(&tiempoFinal)));
    printThreadSafe("TPPA: " + to_string(duracionPromedio) + " ms");
    printThreadSafe("TT: " + to_string(duracionTotal) + " ms");
    tiempoTotalBase = (long long)duracionTotal;
    return tiempoTotalBase;
}

long long manejarArchivoHilos(int n)
{
    clock_t inicioTotal = clock();
    time_t tiempoInicio = time(0);
    printThreadSafe("TI: " + string(ctime(&tiempoInicio)));
    std::vector<std::thread> hilos;
    for (int i = 1; i <= n; ++i)
    {
        hilos.emplace_back(procesarArchivoOptimizado, i);
    }
    for (auto &t : hilos)
    {
        t.join();
    }
    clock_t finTotal = clock();
    double duracionTotal = double(finTotal - inicioTotal) * 1000.0 / CLOCKS_PER_SEC;
    double duracionPromedio = duracionTotal / n;
    time_t tiempoFinal = time(0);
    printThreadSafe("\nTFIN: " + string(ctime(&tiempoFinal)));
    printThreadSafe("TPPA: " + to_string(duracionPromedio) + " ms");
    printThreadSafe("TT: " + to_string(duracionTotal) + " ms");
    tiempoTotalOptimizado = (long long)duracionTotal;
    return tiempoTotalOptimizado;
}

void eliminarArchivos(int n)
{
    for (int i = 1; i <= n; ++i)
    {
        stringstream ss;
        ss << i;
        string nombreTxt = ss.str() + ".txt";
        string nombreHash = ss.str() + ".sha";
        if (remove(nombreTxt.c_str()) == 0)
        {
            // printThreadSafe("Archivo eliminado: " + nombreTxt);
        }
        else
        {
            printThreadSafe("No se pudo eliminar: " + nombreTxt);
        }
        if (remove(nombreHash.c_str()) == 0)
        {
            // printThreadSafe("Archivo eliminado: " + nombreHash);
        }
        else
        {
            printThreadSafe("No se pudo eliminar: " + nombreHash);
        }
    }
}

int main()
{
    int n;
    cout << "Ingrese la cantidad de archivos que desea procesar" << endl;
    cin >> n;
    if (n <= 50 && n > 0)
    {
        duplicate(n);
        cout << "--------------------------------------------------------------------" << endl;
        cout << "PROCESO BASE \n"
             << endl;
        long long tiempoTotalBase = manejarArchivo(n);
        cout << "--------------------------------------------------------------------" << endl;
        cout << "PROCESO OPTIMIZADO \n"
             << endl;
        long long tiempoTotalOptimizado = manejarArchivoHilos(n);
        cout << "--------------------------------------------------------------------" << endl;
        long long diferencia = tiempoTotalBase - tiempoTotalOptimizado;
        cout << "DF:" << diferencia << "ms" << endl;
        long long porcentaje = (diferencia * 100) / tiempoTotalBase;
        cout << "PM:" << porcentaje << "%" << endl;
        eliminarArchivos(n);
    }
    else
    {
        cout << "El numero de archivos debe ser positivo, distinto de 0 y menor o igual a 50" << endl;
    }
    cout << "Presione ENTER para salir..." << endl;
    cin.ignore();
    cin.get();
    return 0;
}
