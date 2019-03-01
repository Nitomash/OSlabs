#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>

#define false 0
#define true 1
#define bool unsigned char

char *concat(char *a, char *b) //функция адекватно выделяет память на новую строку
{
	char *res;
	res = (char *)malloc((strlen(a) + strlen(b)) * sizeof(char)); //выделяем память для новой строки
	strcpy(res, a); //копируем первую часть
	strcat(res, b); //присоединяем вторую
	return res; //возвращаем
}

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

char *getWelcomeString() //функция, формирующая приветственную строку
{
	FILE *cmd; //понадобится для команд в систему
	char *welcomeString, res[255]; //собственно строка и буфер для чтения результата работы команд
	welcomeString = (char *) malloc(sizeof(char) * 255); //максимальный размер приветственной строки 255
	cmd = popen("whoami", "r"); //собираем приветственную строку
	fgets(res, 255, cmd); //прочитали имя юзера
	pclose(cmd); //закрыли за собой пайп
	res[strlen(res) - 1] = '\0'; //убрали перевод строки
	printf("\n");
	strcpy(welcomeString, res); //прибавили имя к приветственной строке
	strcat(welcomeString, "@"); //разделитель
	cmd = popen("uname -n", "r"); //узнали имя компа
	fgets(res, 255, cmd);
	pclose(cmd);
	res[strlen(res) - 1] = '\0';
	strcat(welcomeString, res);
	strcat(welcomeString, ":");
	cmd = popen("pwd", "r"); //узнали рабочую папку
	fgets(res, 255, cmd);
	pclose(cmd);
	res[strlen(res) - 1] = '\0';
	strcat(welcomeString, res);
	strcat(welcomeString, " ");
	return welcomeString;
}

bool isDelimiter(char a) //вычисляет, разделитель ли этот символ
{
	return a == ' ' || a == '\n' || a == '\r' || a == '\t';
}


char *getWord(char *str, int index) //ищет слово по индексу (индекс с 1)
{
	int a = -index + 2, b = 0; //начало и конец слова
	if (index == 1 && !isDelimiter(str[0])) //если ищется первое слово и оно стоит в начале
		a = 1; //чтоб не вычислять а
	for (int i = 0 ; i < strlen(str) ; ++i) //цикл поиска начала и конца
		if (isDelimiter(str[i])) //нашли разделитель
			if (a < 0) { //слишком рано, это не то слово
				++a; //прибавляем счётчик слов
			} else if (a == 0) { //это нужное слово
				a = i + 1; //запоминаем индекс
			} else { //нашли второй индекс
				b = i; //записали
				break;
			}
	if (a > 0) { //если слово вообще нашли
		if (index == 1 && !isDelimiter(str[0])) //если искали первое слово и оно было в самом начале
			a = 0; //ставим а как надо
		if (b > 0) //если слово было не последним
			return substr(str, a, b - a);
		else //если было последним
			return substr(str, a, -1);
	} else //ничего не нашли
		return "";
}

bool findWord(char *str, char *word) //функция ищет, есть ли такое слово в строке
{
	for (int i = 0 ; i < strlen(str) ; ++i) { //цикл поиска слова
	 //если первая буква нашлась; если перед ней разделитель или ничего; если само слово совпадает; если после него разделитель или пустота
		if (str[i] == word[0] && (i > 0 || isDelimiter(str[i - 1])) && !strcmp(substr(str, i, strlen(word)), substr(word, 0, strlen(word))) && (i + 1 == strlen(str) || (str[i + strlen(word)])))
			return true; //слово найдено
	}
	return false; //слово не найдено
}

bool flag; //глобальная переменная для прерывания

void exit_p(int sig) //функция-приём сигнала
{
	flag = true; //знак, что прерывание было
} //из программы НЕ выходим

int main(int argc, char **argv)
{
	char *welcomeString; //приветственная строка
	welcomeString = NULL;
	char str[255], res[255]; //строки для считывания из пайпов
	FILE *cmd; //указатель на процесс команды
	signal(SIGINT, exit_p); //привязка сигнала к функции
	welcomeString = getWelcomeString(); //формируем приветственную строку
	for (;;) { //основной цикл программы
		printf("%s", welcomeString); //приветственная строка
		fgets(str, 255, stdin); //считать всю строку команды
		if (flag == true || !strcmp(getWord(str, 1), "exit")) //если было прерывание или команда exit
			break; //выходим из цикла
		if (!strcmp(getWord(str, 1), "cd") && strcmp(getWord(str, 2), "")) { //отдельная обработка команды cd
			chdir(getWord(str, 2)); //меняем рабочую папку
			welcomeString = getWelcomeString(); //и приветственную строку
		} else
		if (fopen(getWord(str, 1), "r") == NULL && fopen(concat("/bin/", getWord(str, 1)), "r") == NULL && fopen(concat("/usr/bin/", getWord(str, 1)), "r") == NULL) { //проверка команды на существование (если удалось открыть такой файл в одном из основных каталогов)
			printf("Nope.\n"); //нет такой команды
			continue;
		}
		cmd = popen(str, "r"); //открытие пайпа в процесс с соответствующей командой
		if (!cmd) { //если не удалось выполнить
			printf("R.I.P.\n");
		} else { //если всё успешно
			if (!findWord(str, "&")) //если был фоновый режим, то не выводим
				while (fgets(res, 255, cmd) != NULL) { //иначе выводим результат работы
					if (flag) { //обработка прерывания
						flag = false;
						break;
					}
					printf("%s", res);
				}
		}
		pclose(cmd); //закрываем за собой пайп
	}
	return 0;
}
