#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

char *substr(char *src, int first, int num) //функция выделения подстроки: исходник, индекс начала и количество
{
	if (num == -1) //значит, что надо вырезать до конца строки
		num = strlen(src) - first; //ищем количество символом до конца
	if (first + num >= strlen(src)) //если количество превышает оставшуюся длину строки
		num += strlen(src) - first - num; //убираем излишки
	char *res = (char *) malloc(sizeof(char) * (num + 1)); //выделим память на подстроку
	
	memcpy(res, src + first, num); //скопируем её
	res[num] = '\0'; //конец строки
	return res; //вернём подстроку
}

int main(int argc, char *argv[])
{
	if (argc == 1) { //хотя бы один аргумент должен быть
		printf("No agruments\n");
		return 1;
	}
	int fi = argc, fo = argc, fn = argc, o = argc; //индексы соответствущих ключей командной строки
	int one = EOF - 1; //байт для записи размера папок
	char **files, **folders, *filename = "archive.arc", chr; //массивы для чтения и записи файлов и папок и служебный символ
	int right, *fl_num, fin, fon; //служебная переменная, массив размеров файлов и количество файлов и папок в архиве
	FILE *file, *archive; //указатели на файлы для архива и сам архив
	
	for (int i = 1 ; i < argc ; ++i) { //поиск ключей командной строки
		if (!strcmp(argv[i], "-fi"))
			fi = i; //нашёл - отметил индекс
		if (!strcmp(argv[i], "-fo"))
			fo = i;
		if (!strcmp(argv[i], "-fn"))
			fn = i;
		if (!strcmp(argv[i], "-o"))
			o = i;
	}
	if (o < argc && (fi < argc || fo < argc || fn < argc)) { //проверка, что режимы (запаковка/распаковка) не перепутаны
		printf("Invalid agruments\n");
		return 0;
	} else
	if (o == argc && fi != 1 && fo != 1 || fi > fo || fi > fn || fo > fn) { //проверка, что ключи для запаковки не перепутаны
		printf("Invalid agruments\n");
		return 0;
	}
	if (o == argc) { //запаковка архива
		//определяем, где заканчивается список файлов
		if (fi < argc) { //ключ файлов не пропущен (список файлов для запаковки существует)
			if (fo < argc) //ключ папок тоже существует
				right = fo;
			else if (fn < argc) //папок нет, но есть имя файла
				right = fn;
			else
				right = argc; //вообще ничего нет

			fin = right - fi - 1; //количество файлов
			files = (char **)malloc(sizeof(char *) * fin); //выделение памяти массиву имён файлов и массиву размеров
			fl_num = (int *)malloc(sizeof(int) * fin);
			for (int i = fi + 1 ; i < right ; ++i) { //время проверять файлы на существование и записывать размер
				file = fopen(argv[i], "r"); //попытка открыть очередной файл
				if (file == NULL) { //не открылся
					printf("No file: %s\n", argv[i]); //сообщаем; продолжаем
					continue;
				}
				fseek(file, 0, SEEK_END); //указатель в конец файла
				fl_num[i - fi - 1] = ftell(file); //так можно посчитать размер файла
				fclose(file); //закрыть файл
				files[i - fi - 1] = argv[i]; //надо записать имя файла
			}
		}

		if (fo < argc) { //ключ папок не пропущен (есть папки для запаковки)
		//определим, где заканчивается список папок
			if (fn < argc) //у архива есть имя
				right = fn; //граница найдена
			else	//имени нет
				right = argc; //граница - конец списка аргументов
			fon = right - fo - 1; //количество папок
			folders = (char **)malloc(sizeof(char *) * fon); //выделение памяти под массив
			for (int i = fo + 1 ; i < right ; ++i) { //проверяем папки на существование
				if (opendir(argv[i]) == NULL) { //не удалось открыть?
					printf("No folder: %s\n", argv[i]); //сообщаем; продолжаем
					continue;
				}
				folders[i - fo - 1] = argv[i]; //запоминаем имя папки
			}
		}
		if (fn < argc) { //если имя архива указано
			filename = argv[fn + 1]; //запоминаем имя папки
			if (strcmp(substr(filename, strlen(filename) - 4, -1), ".arc")) //если нету расширения - приписываем
				strcat(filename, ".arc");
		}
		archive = fopen(filename, "wb"); //создаём файл архива для записи
		if (archive == NULL) { //если не удалось создать
			printf("Error creating archive\n");
			return 1; //ошибка
		}
		fwrite(&fin, sizeof(int), 1, archive); //записываем количество файлов и папок
		fwrite(&fon, sizeof(int), 1, archive);
		for (int i = 0 ; i < fin ; ++i) { //записываем имена и размеры файлов
			fwrite(files[i], sizeof(char), strlen(files[i]) + 1, archive); //имя
			fwrite((void *)&fl_num[i], sizeof(int), 1, archive); //размер
		}
		for (int i = 0 ; i < fon ; ++i) { //записываем имена папок
			fwrite(folders[i], sizeof(char), strlen(folders[i]) + 1, archive); //имя
			fwrite((void *)&one, sizeof(int), 1, archive); //служебный символ вместо размера
		}
		int num = 256; //переменный размер буфера
		char *buffer = (char *)malloc(sizeof(char) * num); //выделяем память на буфер, которым будем читать содержимое файлов
		for (int i = 0 ; i < fin ; ++i) { //цикл чтения содержимого
			file = fopen(files[i], "rb"); //попытка открыть очередной файл
			if (file == NULL) { //не удалось?
				printf("Error cannot open file \"%s\"\n", files[i]);
				return 1; //ошибка
			}
			while (fread(buffer, sizeof(char), num, file)) { //цикл считывания и записи
				fwrite(buffer, sizeof(char), num, archive); //записываем
			}
			fclose(file); //закрываем файл, когда нечего читать
		}
		fclose(archive); //когда всё записано, закрываем архив
	} else { //режим распаковки архива
		archive = fopen(argv[2], "r"); //пытаемся открыть архив
		if (archive == NULL) { //не удалось?
			printf("Error opening archive.\n");
			return 1; //ошибка
		}
		fread(&fin, sizeof(int), 1, archive); //читаем количество файлов и папок
		fread(&fon, sizeof(int), 1, archive);
		files = (char **)malloc(sizeof(char *) * fin); //выделяем память для массивов имён и размеров файлов (папки будем сразу создавать)
		fl_num = (int *)malloc(sizeof(int) * fin);
		char buff[255]; //буфер для чтения архива
		for (int i = 0 ; i < fin ; ++i) { //основной цикл чтения архива
			for (int j = 0 ; j < 255 ; ++j) { //цикл побайтового чтения имени файла
				fread(buff + j, sizeof(char), 1, archive);
				if (buff[j] == '\0') //конец строки
					break;
			}
			files[i] = (char *)malloc(sizeof(char) * (strlen(buff) + 1)); //выделение памяти для имени файла
			strcpy(files[i], substr(buff, 0, strlen(buff) + 1)); //копирование имени
			fread(&fl_num[i], sizeof(int), 1, archive); //чтение размера файла
		}
		struct stat st = {0}; //структура для проверки существования папки
		for (int i = 0 ; i < fon ; ++i) { //цикл чтения списка папок
			for (int j = 0 ; j < 255 ; ++j) { //цикл чтения имени папки
				fread(buff + j, sizeof(char), 1, archive); //побайтовое чтение
				if (buff[j] == '\0') //конец строки
					break;
			}
			fwrite(&one, sizeof(int), 1, archive); //чтение служебного символа ("размера")

			if (stat(buff, &st) == -1) //папка не существует?
			    mkdir(buff, 0700); //создадим
		}
		for (int i = 0 ; i < fin ; ++i) { //цикл чтения содержимого файлов
			file = fopen(files[i], "w+"); //попытка создать файл
			if (file == NULL) { //не удалось?
				printf("Error opening file.\n");
				return 1; //ошибка
			}
			for (int j = 0 ; j < fl_num[i] ; ++j) { //чтение и запись файла из архива побайтово
				fread(&chr, 1, sizeof(char), archive);
				fwrite(&chr, 1, sizeof(char), file);
			}
			fclose(file); //закрываем файл
		}
		fclose(archive); //после считывания закрываем архив
	}
	return 0;
}
