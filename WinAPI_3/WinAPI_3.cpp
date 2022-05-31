/*
3. Отобразить файл размера 32К в память. Реализовать транспортировку
данных из отображения в файл на диске (один поток). Два других потока
записывают некоторые данные в это отображение. Синхронизация
доступа к данным отображения осуществляется при помощи объекта
синхронизации типа "мьютекс".
*/

#define SOURCE_PATH L"E:/file/source.txt"
#define FINAL_PATH L"E:/file/final.txt"
#include <Windows.h>
#include <iostream>
#include <stdio.h>

using namespace std;

LPSTR buffer;
HANDLE hRead;
LONG file_size;
HANDLE hMutex;
BYTE* dataPtr;

DWORD WINAPI Thread2(LPVOID thread) {
	WaitForSingleObject(hMutex, INFINITE); // ожидаем, пока мьютекс не освободится, после чего сразу лочим
	LONG file_size = GetFileSize(hRead, NULL);

	for (int i = 0; i < 10; i++) {		// записываем некоторые данные в отображение 
		dataPtr[file_size + i] = '6';
	}
	ReleaseMutex(hMutex); // освобождаем ранее занятый мьютекс
	return 1;
}

DWORD WINAPI Thread3(LPVOID thread) {
	WaitForSingleObject(hMutex, INFINITE);	// ожидаем, пока мьютекс не освободится, после чего сразу лочим
	LONG file_size = GetFileSize(hRead, NULL);

	for (int i = 10; i < 20; i++) {
		dataPtr[file_size + i] = '4';	// записываем некоторые данные в отображение 
	}

	ReleaseMutex(hMutex);	// освобождаем ранее занятый мьютекс
	return 1;
}

int main()
{
	setlocale(LC_ALL, "Russian");
	HANDLE SecondThread;
	HANDLE ThirdThread;
	hMutex = CreateMutex(NULL, TRUE, NULL); // Создание мьютекса и одновременный его захват

	if (hMutex == NULL) // Проверка на создание мьютекса
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	//HANDLE hRead;
	hRead = CreateFile(SOURCE_PATH,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	HANDLE hWrite;
	hWrite = CreateFile(FINAL_PATH,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD bytesWrite = 0;
	LONG file_size = GetFileSize(hRead, NULL); // размер файла
	cout << "Начальный размер файла: " << file_size << " байт" << endl;

	HANDLE hMapping = CreateFileMapping(hRead, nullptr, PAGE_READWRITE, 0, 0, nullptr); // создаем отображение файла из диска в память

	dataPtr = (BYTE*)MapViewOfFile(hMapping, // получаем начальный адрес отображения на память
		FILE_MAP_WRITE,
		0,
		0,
		file_size);

	
	//buffer = (LPSTR)calloc(file_size + 100, sizeof(CHAR));
	//CopyMemory(buffer, dataPtr, file_size);

	SecondThread = CreateThread(NULL, 0, &Thread2, 0, NULL, NULL);
	ThirdThread = CreateThread(NULL, 0, &Thread3, 0, NULL, NULL);


	ReleaseMutex(hMutex);

	WaitForSingleObject(SecondThread, INFINITE);
	WaitForSingleObject(ThirdThread, INFINITE);

		
	WriteFile(hWrite, dataPtr, file_size + 20, &bytesWrite, NULL);
	cout << "Размер файла после записи другими потоками: " << bytesWrite << " байт" << endl;


	CloseHandle(hMutex);
	CloseHandle(hRead);
	CloseHandle(hWrite);
}