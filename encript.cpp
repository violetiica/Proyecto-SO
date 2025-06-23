#include <iostream>
#include <fstream>
#include <string>
#include <cctype> 
#include <chrono>
using namespace std;

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
    for (int i = 1; i <= n; i++) {
        auto start = chrono::high_resolution_clock::now();
        string nombreArchivo = to_string(i) + ".txt";
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
                cout << "Archivo " << nombreArchivo << ".txt encriptado correctamente.\n" << endl;
            } else {
                cout << "No se pudo escribir el archivo para encriptar: " << nombreArchivo << ".txt" << endl;
            }
        //aca termina el encriptar

        //aca empieza el desencriptar
            ifstream inFile(nombreArchivo);
            if (inFile.is_open()) {
                getline(inFile, contenido, '\0'); 
                inFile.close();
                string textoDesencriptado=decrypt(contenido);
                ofstream outFile(nombreArchivo);
                if (outFile.is_open()) {
                    outFile << textoDesencriptado;
                    outFile.close();
                    cout << "Archivo " << nombreArchivo << ".txt desencriptado correctamente.\n" << endl;
                    
                    ifstream inFile("original.txt");
                    getline(inFile, contenido, '\0');
                    inFile.close();
                    if (contenido == textoDesencriptado) {
                        cout << "El archivo " << nombreArchivo << " coincide con el original.\n" << endl;
                    } else {
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
        cout << "Tiempo de proceso de "+nombreArchivo+":" << duration.count() << " ms \n" << endl;
    }
    auto finTotal = chrono::high_resolution_clock::now();
    auto duracionTotal = chrono::duration_cast<chrono::milliseconds>(finTotal - inicioTotal);
    long long duracionPromedio =duracionTotal.count()/n; // Calcular el tiempo promedio
    cout << "Tiempo promedio de desencriptación: " << duracionPromedio << " ms" << endl;
    cout << "Tiempo total de desencriptación: " << duracionTotal.count() << " ms" << endl;

}


int main()
{
    duplicate(3); // Ejemplo: crea 1.txt, 2.txt, 3.txt
    manejarArchivo(3);
}