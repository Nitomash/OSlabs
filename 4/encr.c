#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[])
{
	if (argc != 5) { //в командной строке ровно пять аргументов: имя проги, режим, файлы ввода и выводы и ключ шифрования
		printf("Format for encryption: ./a.out -in file_from file_to key\n");
		printf("Format for decryption: ./a.out -out file_from file_to key\nTry again\n");
		return 1;
	}
	FILE * file_in, *file_out; //те самые файлы ввода и вывода
	int chr, counter; //переменная для считывания байта из файла и счётчик для ключа
	file_in = fopen(argv[2], "rb"); //файл ввода
	file_out = fopen(argv[3], "w+b"); //файл вывода
	if (file_in == NULL || file_out == NULL) { //проверка, открылись ли
		printf("Error opening file\n");
		return 1; //ошибка
	}
	if (strcmp(argv[1], "-in") && strcmp(argv[1], "-out")) { //проверка, а был ли верно введён режим
		printf("Invalid command\n");
		return 2; //ошибка
	}
	//считываем первый байт; пока байт не будет концов файла; считываем следующий байт
	for (chr = fgetc(file_in) ; chr != EOF ; chr = fgetc(file_in)) {
		chr = chr^(int)(argv[4][counter]); //xor - сам процесс шифрования
		fputc(chr, file_out); //вывод зашифрованного байта в файл

		counter = (counter + 1) % strlen(argv[4]); //ключ зациклен, так что по достижении конца ключа он начнёт читаться с начала
	}
	fclose(file_in); //закрыть все файлы
	fclose(file_out);
	return 0;
}
