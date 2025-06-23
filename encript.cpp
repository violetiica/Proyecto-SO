#include <iostream>
#include <fstream>
#include <string>
#include <cctype> 
#include <chrono>
#include "picosha2.h"
#include <cstdio> // Para remove()
#include <ctime>
using namespace std;


long long tiempoTotalBase = 0; // Variable global para almacenar el tiempo total
long long tiempoTotalOptimizado = 0; // Variable global para almacenar el tiempo total optimizado

string encrypt(string contenido)
{
    string encryptedContent = "";
    for (char c : contenido) {
        if (isdigit(c)) {
            encryptedContent += '9' - (c - '0');
        }
        else if (isupper(c)) {
            // Mayúsculas: circular de 'A' a 'Z'
            encryptedContent += ((c - 'A' + 3) % 26) + 'A';
        }
        else if (islower(c)) {
            // Minúsculas: circular de 'a' a 'z'
            encryptedContent += ((c - 'a' + 3) % 26) + 'a';
        }
        else {
            encryptedContent += c; // Otros caracteres sin cambio
        }
    }
    return encryptedContent;
}
string decrypt(string contenido)
{
    string encryptedContent = "";
    for (char c : contenido) {
        if (isdigit(c)) {
            encryptedContent += '9' - (c - '0');
        }
        else if (isupper(c)) {
            // Mayúsculas: circular de 'A' a 'Z'
            encryptedContent += ((c - 'A' - 3 + 26) % 26) + 'A';
        }
        else if (islower(c)) {
            // Minúsculas: circular de 'a' a 'z'
            encryptedContent += ((c - 'a' - 3 + 26) % 26) + 'a';
        }
        else {
            encryptedContent += c; // Otros caracteres sin cambio
        }
    }
    return encryptedContent;
}

void duplicate(int n)
{
    string sourceFile = "original.txt";
    for (int i = 1; i <= n; ++i) {
        ifstream src(sourceFile, ios::binary);
        ofstream dst(to_string(i) + ".txt", ios::binary);
        if (!src) {
            cout << "No se pudo abrir el archivo fuente: " << sourceFile << endl;
            return;
        }
        if (!dst) {
            cout << "No se pudo crear el archivo destino: " << i << ".txt" << endl;
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
    auto inicioTotal = chrono::high_resolution_clock::now();
    time_t tiempoInicio = chrono::system_clock::to_time_t(chrono::system_clock::now());
    cout << "TI: " << std::ctime(&tiempoInicio)<<endl;
    for (int i = 1; i <= n; i++) {
        auto start = chrono::high_resolution_clock::now();
        string nombreArchivo = to_string(i) + ".txt";
        string nombreHash = to_string(i) + ".sha";
        //aca empieza el encriptar
        ifstream inFile(nombreArchivo);
        if (inFile.is_open()) {
            getline(inFile, contenido, '\0'); 
            inFile.close();
            string textoEncriptado=encrypt(contenido);
            ofstream outFile(nombreArchivo);
            if (outFile.is_open()) {
                outFile << textoEncriptado;
                outFile.close();
                //cout << "Archivo " << nombreArchivo << ".txt encriptado correctamente.\n" << endl;
                // Calcular y guardar el hash del archivo encriptado
                string hash = picosha2::hash256_hex_string(textoEncriptado);
                ofstream hashFile(nombreHash);
                if (hashFile.is_open()) {
                    hashFile << hash;
                    hashFile.close();
                } else {
                    cout << "No se pudo escribir el archivo hash para: " << nombreArchivo << endl;
                }
                //aca termina la generacion del hash
            } else {
                cout << "No se pudo escribir el archivo para encriptar: " << nombreArchivo << ".txt" << endl;
            }
        //aca termina el encriptar

        //aca empieza el desencriptar
            ifstream inFile(nombreArchivo);
            if (inFile.is_open()) {
                getline(inFile, contenido, '\0'); 
                inFile.close();

                // Verificar si el hash guardado coincide con el actual
                string hashGuardado,hashActual;
                ifstream hashFile(nombreHash);
                if (hashFile.is_open()) {
                    getline(hashFile, hashGuardado,'\0');
                    hashFile.close();
                    hashActual= picosha2::hash256_hex_string(contenido);
                    // Comparar el hash guardado con el actual
                    if (hashGuardado != hashActual) {
                        cout << "El hash del archivo " << nombreArchivo << " no coincide con el guardado.\n" << endl;
                        continue;
                    }
                } else {
                    cout << "No se pudo abrir el archivo hash para: " << nombreArchivo << endl;
                    continue; // Salir del bucle si no se puede abrir el archivo hash
                }
                //aca termina la verificación del hash

                string textoDesencriptado=decrypt(contenido);
                ofstream outFile(nombreArchivo);
                if (outFile.is_open()) {
                    outFile << textoDesencriptado;
                    outFile.close();
                    //cout << "Archivo " << nombreArchivo << ".txt desencriptado correctamente.\n" << endl;
                    
                    ifstream inFile("original.txt");
                    getline(inFile, contenido, '\0');
                    inFile.close();
                    if (contenido != textoDesencriptado) {
                        cout << "El archivo " << nombreArchivo << " no coincide con el original.\n" << endl;
                    }
                } else {
                    cout << "No se pudo escribir el archivo para escribir: " << nombreArchivo << " para desencriptar \n" << endl;
                }
            } //
        //aca termina el desencriptar
        } else {
            cout << "No se pudo abrir el archivo "+nombreArchivo+" para encriptar" << endl;
        }
        auto end = chrono::high_resolution_clock::now();
        auto duration =chrono::duration_cast<chrono::milliseconds>(end - start);
        cout << "Tiempo "<<i<<": " << duration.count() << " ms " << endl;
    }
    auto finTotal = chrono::high_resolution_clock::now();
    auto duracionTotal = chrono::duration_cast<chrono::milliseconds>(finTotal - inicioTotal);
    long long duracionPromedio = duracionTotal.count()/n; // Calcular el tiempo promedio
    cout << "\nTFIN: " << duracionTotal.count() << " ms" << endl;
    cout << "TPPA: " << duracionPromedio << " ms" << endl;
    time_t tiempoFinal = chrono::system_clock::to_time_t(chrono::system_clock::now());
    cout << "TT: " << std::ctime(&tiempoFinal);
    tiempoTotalBase = duracionTotal.count(); // Calcular el tiempo total
}

void eliminarArchivos(int n) {  //solo si es necesario eliminar archivos en pruebas
    for (int i = 1; i <= n; ++i) {
        string nombreTxt = to_string(i) + ".txt";
        string nombreHash = to_string(i) + ".sha";
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
    cout<< "Ingrese la cantidad de archivos que desea procesar" << endl;
    cin>>n;
    if (n<=50 and n>0) {
    duplicate(n); // Ejemplo: crea 1.txt, 2.txt, 3.txt
    cout << "--------------------------------------------------------------------" << endl;
    cout << "PROCESO BASE \n" << endl;
    manejarArchivo(n);
    cout << "--------------------------------------------------------------------" << endl;
    } else {
        cout << "El numero de archivos debe ser positivo, distinto de 0 menor o igual a 50" << endl;
    }
    //eliminarArchivos(50); // Elimina los archivos creados

}