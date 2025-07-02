#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include "picosha2.h"
#include <cstdio> // Para remove()
#include <ctime>
#include <sstream>
using namespace std;

long long tiempoTotalBase = 0;
long long tiempoTotalOptimizado = 0;

string encrypt(string contenido)
{
    string encryptedContent = "";
    for (size_t i = 0; i < contenido.size(); ++i) {
        char c = contenido[i];
        if (isdigit(c)) {
            encryptedContent += '9' - (c - '0');
        }
        else if (isupper(c)) {
            encryptedContent += ((c - 'A' + 3) % 26) + 'A';
        }
        else if (islower(c)) {
            encryptedContent += ((c - 'a' + 3) % 26) + 'a';
        }
        else {
            encryptedContent += c;
        }
    }
    return encryptedContent;
}

string decrypt(string contenido)
{
    string encryptedContent = "";
    for (size_t i = 0; i < contenido.size(); ++i) {
        char c = contenido[i];
        if (isdigit(c)) {
            encryptedContent += '9' - (c - '0');
        }
        else if (isupper(c)) {
            encryptedContent += ((c - 'A' - 3 + 26) % 26) + 'A';
        }
        else if (islower(c)) {
            encryptedContent += ((c - 'a' - 3 + 26) % 26) + 'a';
        }
        else {
            encryptedContent += c;
        }
    }
    return encryptedContent;
}

void duplicate(int n)
{
    string sourceFile = "original.txt";
    for (int i = 1; i <= n; ++i) {
        ifstream src(sourceFile.c_str(), ios::binary);
        stringstream ss;
        ss << i;
        string nombreArchivo = ss.str() + ".txt";
        ofstream dst(nombreArchivo.c_str(), ios::binary);
        if (!src) {
            cout << "No se pudo abrir el archivo fuente: " << sourceFile << endl;
            return;
        }
        if (!dst) {
            cout << "No se pudo crear el archivo destino: " << nombreArchivo << endl;
            return;
        }
        dst << src.rdbuf();
        src.close();
        dst.close();
    }
}

void manejarArchivo(int n)
{
    string contenido;
    clock_t inicioTotal = clock();
    time_t tiempoInicio = time(0);
    cout << "TI: " << ctime(&tiempoInicio) << endl;
    for (int i = 1; i <= n; i++) {
        clock_t start = clock();
        stringstream ss;
        ss << i;
        string nombreArchivo = ss.str() + ".txt";
        string nombreHash = ss.str() + ".sha";
        // Encriptar
        ifstream inFile(nombreArchivo.c_str());
        if (inFile.is_open()) {
            getline(inFile, contenido, '\0');
            inFile.close();
            string textoEncriptado = encrypt(contenido);
            ofstream outFile(nombreArchivo.c_str());
            if (outFile.is_open()) {
                outFile << textoEncriptado;
                outFile.close();
                // Calcular y guardar el hash del archivo encriptado
                string hash = picosha2::hash256_hex_string(textoEncriptado);
                ofstream hashFile(nombreHash.c_str());
                if (hashFile.is_open()) {
                    hashFile << hash;
                    hashFile.close();
                } else {
                    cout << "No se pudo escribir el archivo hash para: " << nombreArchivo << endl;
                }
            } else {
                cout << "No se pudo escribir el archivo para encriptar: " << nombreArchivo << endl;
            }
            // Desencriptar
            ifstream inFile2(nombreArchivo.c_str());
            if (inFile2.is_open()) {
                getline(inFile2, contenido, '\0');
                inFile2.close();
                // Verificar hash
                string hashGuardado, hashActual;
                ifstream hashFile(nombreHash.c_str());
                if (hashFile.is_open()) {
                    getline(hashFile, hashGuardado, '\0');
                    hashFile.close();
                    hashActual = picosha2::hash256_hex_string(contenido);
                    if (hashGuardado != hashActual) {
                        cout << "El hash del archivo " << nombreArchivo << " no coincide con el guardado.\n" << endl;
                        continue;
                    }
                } else {
                    cout << "No se pudo abrir el archivo hash para: " << nombreArchivo << endl;
                    continue;
                }
                string textoDesencriptado = decrypt(contenido);
                ofstream outFile2(nombreArchivo.c_str());
                if (outFile2.is_open()) {
                    outFile2 << textoDesencriptado;
                    outFile2.close();
                    ifstream inFile3("original.txt");
                    getline(inFile3, contenido, '\0');
                    inFile3.close();
                    if (contenido != textoDesencriptado) {
                        cout << "El archivo " << nombreArchivo << " no coincide con el original.\n" << endl;
                    }
                } else {
                    cout << "No se pudo escribir el archivo para escribir: " << nombreArchivo << " para desencriptar \n" << endl;
                }
            }
        } else {
            cout << "No se pudo abrir el archivo " << nombreArchivo << " para encriptar" << endl;
        }
        clock_t end = clock();
        double duration = double(end - start) * 1000.0 / CLOCKS_PER_SEC;
        cout << "Tiempo " << i << ": " << duration << " ms " << endl;
    }
    clock_t finTotal = clock();
    double duracionTotal = double(finTotal - inicioTotal) * 1000.0 / CLOCKS_PER_SEC;
    double duracionPromedio = duracionTotal / n;
    cout << "\nTFIN: " << duracionTotal << " ms" << endl;
    cout << "TPPA: " << duracionPromedio << " ms" << endl;
    time_t tiempoFinal = time(0);
    cout << "TT: " << ctime(&tiempoFinal);
    tiempoTotalBase = (long long)duracionTotal;
}

void eliminarArchivos(int n) {
    for (int i = 1; i <= n; ++i) {
        stringstream ss;
        ss << i;
        string nombreTxt = ss.str() + ".txt";
        string nombreHash = ss.str() + ".sha";
        if (remove(nombreTxt.c_str()) == 0) {
            cout << "Archivo eliminado: " << nombreTxt << endl;
        } else {
            cout << "No se pudo eliminar: " << nombreTxt << endl;
        }
        if (remove(nombreHash.c_str()) == 0) {
            cout << "Archivo eliminado: " << nombreHash << endl;
        } else {
            cout << "No se pudo eliminar: " << nombreHash << endl;
        }
    }
}

int main()
{
    int n;
    cout << "Ingrese la cantidad de archivos que desea procesar" << endl;
    cin >> n;
    if (n <= 50 && n > 0) {
        duplicate(n);
        cout << "--------------------------------------------------------------------" << endl;
        cout << "PROCESO BASE \n" << endl;
        manejarArchivo(n);
        cout << "--------------------------------------------------------------------" << endl;
    } else {
        cout << "El numero de archivos debe ser positivo, distinto de 0 y menor o igual a 50" << endl;
    }
    //eliminarArchivos(n); // Elimina solo los archivos creados
    cout << "Presione ENTER para salir..." << endl;
    cin.ignore();
    cin.get();
    return 0;
}