// ConsolePractice.cpp: определяет точку входа для консольного приложения.
//

//--------------------------------Управление положением курсора--------->

//    \033[#A передвинуть курсор вверх на # строк
//    \033[#B передвинуть курсор вниз на # строк
//    \033[#С передвинуть курсор вправо на # столбцов
//    \033[#D передвинуть курсор влево на # столбцов
//    \033[#E передвинуть курсор вниз на # строк и поставить в начало строки
//    \033[#F передвинуть курсор вверх на # строк и поставить в начало строки
//    \033[#G переместить курсор в указанный столбец текущей строки
//    \033[#;#H задает абсолютные координаты курсора (строка, столбец)
//    \033]2;BLA_BLA\007   Заголовок окна xterm...


#include "stdafx.h"
#include <iostream> //Заголовочный файл с классами, функциями и переменными для организации ввода-вывода
#include <conio.h>  //Подключено только ради _getch();
#include <fstream>  //Предоставляет интерфейс для чтения/записи данных из/в файл. Для манипуляции с данными файлов используются объекты, называемые потоками («stream»). 

bool DEBAG = false;
const char menuFile[10] = "menu.list"; // Файл пунктов меню
const char madeFile[8] = "made.db";    // Файл с производителями
const char deviceFile[10] = "device.db";//Файл с устройствами
const char typeFile[8] = "type.db";    // Файл с типами устройств
const char infoFile[8] = "info.db";    // Файл со справочником


/// Хранит id, в массиве, экранов меню 
enum MENUToken {
	MAINmenu = 0,    ///< Главное меню
	FILEmenu = 1,    ///< Меню работы с файлом 
	EDITmenu = 2	 ///< Меню работы с данными
};

/// Хранит коды клавиш в верх,вниз и тд
enum KEYS {
	ENTER = 13,
	ESC = 27,
	UP = 72,
	DOWN = 80
};

/// Структура хранящая метаинформацию связанную с меню
struct MENU {
	char** strings;///< Массив хранящий строки меню (пункты)
	unsigned short int count;///< Кол-во пунктов,  0 / 65 535, 2 байта 
	//unsigned short int *sizes; //Массив хранящий размер строки пункта
	MENU() {
		count = 0;
	}
} *menus;

/// Структура хранящая справку
struct info {
	char *name;///< Имя пункта 
	char *infoData;///< Текст пункта
	info() {
		name = NULL;
		infoData = NULL;
	};
};

/// Структура хранящая наименования типов мобильных телефонов, данные берутся из typeFile
struct deviceType {
	unsigned int ID; ///< ID в базе
	char *nameType; ///< Наименование
	deviceType *next;
	deviceType() {
		ID = -1;
		next = NULL;
	};
} *deviceTypes;

/// Структура хранящая наименования производителей мобильных телефонов
struct deviceMade {
	unsigned int ID; ///< ID в базе
	char *nameManufacturer; ///< Наименование прроизводителя
	deviceMade *next;
	deviceMade() {
		ID = -1;
		next = NULL;
	};
} *deviceManufactures;

/// Структура представляющая из себя образ телефона
struct device {
	unsigned int ID; ///< ID в базе
	struct deviceType *type; ///< Связь с deviceType
	struct deviceMade *made; ///< Связь с deviceMade
	bool availability; ///< Наличие
	unsigned int price; ///< Цена
	char *nameDevice; ///< Наименование
	device *next;
	device() {
		ID = -1;
		next = NULL;
		type = NULL;
		made = NULL;
	};
} *devices;

MENU* loadMenus();
void printmenu(MENU menu);
void menu();
void printInfo();
void printHelp();
void openDB();
void addToDB();
unsigned int addMore(bool type);
void removeFromDB();
void sort();
void saveDB();
void closeDB();
void editDB();
void printTOScreen();
void printToTxt();
void manual();

/*!
	\brief Осуществление первоначальной настройки и запуска

	Функция осуществляет подготовку терминала и данных для последующего взаимодействия с пользователем
*/
int main()
{
	system("Chcp 1251");
	//1200 	utf-16 	Unicode UTF-16, little endian byte order (BMP of ISO 10646); available only to managed applications
	//1251 	windows-1251 	ANSI Cyrillic; Cyrillic (Windows)
	devices = NULL;

	loadMenus();
	openDB();
	system("cls");
	menu();
	return 0;
}

/*!
	\brief Реализация навигации по меню

	Реализация обработки ввода пользователяю, связаного с навигацией между разными меню и функциями работы с данными

*/

void menu() {
	unsigned short int select;
	enum MENUToken MenuType = MENUToken::MAINmenu;
	while (true) {
		// example use color https: //stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
		// https: //en.wikipedia.org/wiki/ANSI_escape_code#graphics
		if (devices == NULL || deviceManufactures == NULL || deviceTypes == NULL) //Предупреждение пользователя
			std::cout << "\033[1;31m База не подключена или пуста\033[0m" << std::endl;

		printmenu(menus[MenuType]);
		std::cout << "В ведите номер выбраного вами пункта: ";
		std::cin >> select;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //std::cin >> buff; Оставляет после себя '\n' в буфере 
		if (select > 8) { // Минимальная защита входных данных
			select = 0;
			std::cout << "Вы выбрали несуществующий пункт!" << std::endl;
			system("cls");
			continue;
		}
		system("cls");
		if (DEBAG) std::cout << "DEBAG: Выбранный пункт: " << select << std::endl;

		switch (MenuType) {
		case MENUToken::MAINmenu:
			if (DEBAG) std::cout << "DEBAG: Main menu" << std::endl;
			switch (select) {
			case 1://Файл
				MenuType = MENUToken::FILEmenu;
				break;
			case 2://Редактировать документ
				MenuType = MENUToken::EDITmenu;
				break;
			case 3://Вывести документ на экран
				printTOScreen();
				break;
			case 4://Записать документ в формате txt 
				printToTxt();
				break;
			case 5://Справочник
				manual();
				break;
			case 6://Помощь
				printHelp();
				break;
			case 7://О программе 
				printInfo();
				break;
			case 8://Выход
				exit(0);
				break;

			default: //В случае ввода неизвестного параметра, очистим экран и заного выведем предыдущее меню
				system("cls");
				std::cout << "Вы выбрали несуществующий пункт!" << std::endl;
				break;
			}
			break;
		case MENUToken::FILEmenu:
			if (DEBAG) std::cout << "DEBAG: File menu" << std::endl;
			switch (select) {
			case 1://Open
				if (devices != NULL || deviceManufactures != NULL || deviceTypes != NULL) { //Предупреждение пользователя
					std::cout << "\033[1;31m База уже подключена, сперва отключите подключённую \033[0m" << std::endl;
				}
				else
					openDB();
				break;
			case 2://Save
				saveDB();
				break;
			case 3://Close
				closeDB();
				break;
			case 4://back 
				MenuType = MENUToken::MAINmenu;
				break;
			default: //В случае ввода неизвестного параметра, очистим экран и заного выведем предыдущее меню
				system("cls");
				std::cout << "Вы выбрали несуществующий пункт!" << std::endl;
				break;
			}
			break;
		case MENUToken::EDITmenu:
			if (DEBAG) std::cout << "DEBAG: Edit menu" << std::endl;
			switch (select) {
			case 1://Добавить запись
				addToDB();
				break;
			case 2://Удалить запись
				removeFromDB();
				break;
			case 3://Редактировать запись
				editDB();
				break;
			case 4://back
				MenuType = MENUToken::MAINmenu;
				break;

			default: //В случае ввода неизвестного параметра, очистим экран и заного выведем предыдущее меню
				system("cls");
				std::cout << "Вы выбрали несуществующий пункт!" << std::endl;
				break;
			}
			break;
		default: //В случае ввода неизвестного параметра, очистим экран и заного выведем предыдущее меню
			system("cls");
			std::cout << "Вы выбрали несуществующий пункт!" << std::endl;
			break;
		}
	}
}

/*!
	\brief Отрисовка меню
	\param[in] menu - выбор меню из menus для отрисовки с помощью значения из списка MENUToken

	Отрисовка выбраного меню
*/

void printmenu(MENU menu) { //selection - нумерация с 0.
	int i;
	if (DEBAG) printf("DEBAG: function printmenu: menu.count=%d\n", menu.count);
	unsigned short int add = 0;
	for (i = 0; menu.count > i; i++) {
		std::cout << i + 1 << ") " << menu.strings[i] << std::endl;
	}
}

/*!
	\brief Обработка menuFile
	\attention menuFile очень чувствителен к синтаксису внутри себя, не соблюдение формата файла приведёт к зависанию/вылету/непридвиденным действиям со стороны программы!

	Загрузка данных из файла menuFile, заполнение структуры menus

*/

MENU* loadMenus() {
	FILE *fileMenu = NULL;
	char buff[128], *buffCopy;
	menus = new MENU[2];
	enum MENUToken MenuType;
	unsigned short int stringsCount = 0;

	for (int i = 0; i <= 2; i++) menus[i].count = 0;//Обнуляем, чтобы использовать как счётчики
	fopen_s(&fileMenu, menuFile, "rt");

	if (fileMenu == NULL)
	{
		std::cout << "Ошибка открытия файла" << menuFile << std::endl;
		system("dir");
		system("pause"); exit(1);
	}

	while (!feof(fileMenu)) // Подсчёт кол-ва строк в файле
	{
		fgets(buff, 126, fileMenu);

		if (buff[0] == '#' || buff[0] == '\n') //Пропускаем комментарии и пустые строки
			continue;

		if (strcmp(buff, "MAIN_menu:\n") == 0) { MenuType = MENUToken::MAINmenu; continue; }
		else if (strcmp(buff, "FILE_menu:\n") == 0) { MenuType = MENUToken::FILEmenu; continue; }
		else if (strcmp(buff, "EDIT_menu:\n") == 0) { MenuType = MENUToken::EDITmenu; continue; }

		std::cout << "loadMenus()" << "Type: " << MenuType << " ; " << buff << std::endl;

		menus[MenuType].count++;
		stringsCount++;
	};

	std::cout << "loadMenus()" << "Count MAIN_menu " << menus[MENUToken::MAINmenu].count << std::endl;
	std::cout << "loadMenus()" << "Count FILE_menu " << menus[MENUToken::FILEmenu].count << std::endl;
	std::cout << "loadMenus()" << "Count EDIT_menu " << menus[MENUToken::EDITmenu].count << std::endl;

	std::cout << "loadMenus()" << "Count str " << stringsCount << std::endl;

	for (int i = 0; i <= 2; i++) menus[i].strings = (char **)new char *[menus[i].count];//Выделяем память под нужное кол-во пунтктов в каждом меню

	for (int i = 0; i <= 2; i++) menus[i].count = 0;//Обнуляем, чтобы повторно использовать как счётчики
	rewind(fileMenu); // В начало файла

	while (!feof(fileMenu)) // Заполнение структур
	{
		fgets(buff, 126, fileMenu);

		if (buff[0] == '#' || buff[0] == '\n') //Пропускаем комментарии и пустые строки
			continue;

		if (strcmp(buff, "MAIN_menu:\n") == 0) { MenuType = MENUToken::MAINmenu; continue; }
		else if (strcmp(buff, "FILE_menu:\n") == 0) { MenuType = MENUToken::FILEmenu; continue; }
		else if (strcmp(buff, "EDIT_menu:\n") == 0) { MenuType = MENUToken::EDITmenu; continue; }

		//buff = 0:File;
		buffCopy = &buff[2];//Копируем указатель, чтобы его двигать
		//buffCopy = File;

		//Велосипед, нудно использовать библиотеку, но так быстрей
		for (; (buffCopy[0] == ';' ? buffCopy[0] = '\0', 1 : 0) || buffCopy[0] != '\0'; buffCopy++);//Поиск ';' и замена на \0
		//Завершающий нулевой символ не входит в длину строки, strlen возвращает кол-во символов без '\0'
		//"buff[0] - '0'", Char to int, '5' -> 5 
		std::cout << "loadMenus() " << "buff[0] - '0' " << buff[0] - '0' << std::endl;
		std::cout << "loadMenus() " << strlen(&buff[2]) << std::endl;
		menus[MenuType].strings[buff[0] - '0'] = new char[strlen(&buff[2]) + 1];

		strcpy_s(menus[MenuType].strings[buff[0] - '0'], strlen(&buff[2]) + 1, &buff[2]);//Копирование считаной строки в память которую только что выделили

		menus[MenuType].count++;
		stringsCount++;
	};
	fclose(fileMenu);
	return NULL;
}

/*!
	\brief О программе

	Статическая информация о программе, контакты разработчика
*/


void printInfo() {
	std::cout << "						О Программе				                                       " << std::endl;
	std::cout << "	Практическое задание: Разработка информационной системы по учёту сотовых телефонов." << std::endl;
	std::cout << "	Автор: Проказа Андрей (ИЭУИС 2-4)                                                  " << std::endl;
	std::cout << "	Контакты:                                                                          " << std::endl;
	std::cout << "		e-mail: a.prokaza007@gmail.com                                                 " << std::endl;
	std::cout << "		vk:     https://vk.com/andrey_985235                                           " << std::endl;
	std::cout << "		github: https://github.com/lolmens                                             " << std::endl;
	system("pause");
	system("cls");
}

/*!
	\brief Помощь

	Минимальная информация для начала пользования программой, контакты разработчика
*/

void printHelp() {
	std::cout << "						Помощь				                                           " << std::endl;
	std::cout << "	Инструкция по использованию программы находится в разделе справка.                 " << std::endl;
	std::cout << "	Все вопросы оставшиеся после ознакомления со справочной информацией вы можете задать" << std::endl;
	std::cout << "	связавшись с разработчиком.                                                        " << std::endl;
	std::cout << "	Контакты:                                                                          " << std::endl;
	std::cout << "		e-mail: a.prokaza007@gmail.com                                                 " << std::endl;
	std::cout << "		vk:     https://vk.com/andrey_985235                                           " << std::endl;
	std::cout << "		github: https://github.com/lolmens                                             " << std::endl;
	system("pause");
	system("cls");
}

/*!
	\brief Обработка файлов madeFile, deviceFile, typeFile
	\attention madeFile, deviceFile, typeFile очень чувствительны к синтаксису внутри себя, не соблюдение формата одного из файлов приведёт к зависанию/вылету/непридвиденным действиям со стороны программы!

	Загрузка данных из файлов madeFile, deviceFile, typeFile, заполнение структур deviceStructure, deviceMadeStructure , deviceTypeStructure осуществление линковки для правильного доступа к данным между структур
*/

void openDB() {
	system("cls");
	if (DEBAG) std::cout << "DEBAG: Открытие базы" << std::endl;

	FILE *fileMade = NULL, *fileDevice = NULL, *fileType = NULL;
	unsigned short int fileMadeStringsCount = 0, fileDeviceStringsCount = 0, fileTypeStringsCount = 0; // Для подсчёта кол-ва записей в файлах
	char buff[128], *buffCopy;

	fopen_s(&fileMade, madeFile, "rt");
	fopen_s(&fileDevice, deviceFile, "rt");
	fopen_s(&fileType, typeFile, "rt");

	if (fileMade == NULL)   std::cout << "Ошибка открытия файла" << fileMade << std::endl;
	if (fileDevice == NULL) std::cout << "Ошибка открытия файла" << fileDevice << std::endl;
	if (fileType == NULL)   std::cout << "Ошибка открытия файла" << fileType << std::endl;
	if (fileMade == NULL || fileDevice == NULL || fileType == NULL)
	{
		system("dir");
		system("pause"); exit(1);
	}

	std::cout << "Чтение файла " << fileMade << std::endl;
	while (!feof(fileMade)) { // Подсчёт кол-ва строк в файле madeFile
		fgets(buff, 126, fileMade);

		if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
			continue;

		fileMadeStringsCount++;
	}
	std::cout << "Чтение файла " << fileDevice << std::endl;
	while (!feof(fileDevice)) { // Подсчёт кол-ва строк в файле fileDevice
		fgets(buff, 126, fileDevice);

		if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
			continue;

		fileDeviceStringsCount++;
	}
	std::cout << "Чтение файла " << fileType << std::endl;
	while (!feof(fileType)) { // Подсчёт кол-ва строк в файле fileType
		fgets(buff, 126, fileType);

		if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
			continue;

		fileTypeStringsCount++;
	}

	rewind(fileMade);	// В начало файлов
	rewind(fileDevice);
	rewind(fileType);

	//devices = new device[fileDeviceStringsCount]; //Память под массивы
	//deviceTypes = new deviceType[fileTypeStringsCount];
	//deviceManufactures = new deviceMade[fileMadeStringsCount];

	device *deviceStructure;
	deviceMade *deviceMadeStructure;
	deviceType *deviceTypeStructure;

	fileMadeStringsCount = 0, fileTypeStringsCount = 0, fileDeviceStringsCount = 0;//Обнуляем счётчики
	std::cout << "Обработка файла " << fileMade << std::endl;
	while (!feof(fileMade)) { // Заполнение deviceManufactures 
		fgets(buff, 126, fileMade);

		if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
			continue;

		deviceMadeStructure = new deviceMade();

		buffCopy = buff;
		//#id Manufacturer;Name Manufacturer;
		//buffCopy = 1;Nokia; buff = 1;Nokia;
		buffCopy = strchr(buff, ';');
		buffCopy[0] = '\0';
		//buffCopy = '\0'Nokia; buff = 1'\0'Nokia;
		deviceMadeStructure->ID = atoi(buff);
		buffCopy++;
		//buffCopy = Nokia; buff = 1'\0'Nokia;
		strchr(buffCopy, ';')[0] = '\0';
		//buffCopy = Nokia'\0' buff = 1'\0'Nokia'\0'
		deviceMadeStructure->nameManufacturer = new char[strlen(buffCopy) + 1]; // Не забываем про '\0' который strlen(str) не считает.
		strcpy_s(deviceMadeStructure->nameManufacturer, strlen(buffCopy) + 1, buffCopy);

		// Ставим новыйэлемент в начало, смещая предыдущий на второе место 
		deviceMadeStructure->next = deviceManufactures;
		deviceManufactures = deviceMadeStructure;

		fileMadeStringsCount++;
	}

	std::cout << "Обработка файла " << fileType << std::endl;
	while (!feof(fileType)) { // Заполнение deviceTypes
		fgets(buff, 126, fileType);

		if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
			continue;

		deviceTypeStructure = new deviceType();

		buffCopy = buff;
		//#id Type;Name Type;
		//buffCopy = 1;Apple; buff = 1;Apple;
		buffCopy = strchr(buff, ';');
		buffCopy[0] = '\0';
		//buffCopy = '\0'Apple; buff = 1'\0'Apple;
		deviceTypeStructure->ID = atoi(buff);
		buffCopy++;
		//buffCopy = Apple; buff = 1'\0'Apple;
		strchr(buffCopy, ';')[0] = '\0';
		//buffCopy = Apple'\0' buff = 1'\0'Apple'\0'
		deviceTypeStructure->nameType = new char[strlen(buffCopy) + 1]; // Не забываем про '\0' который strlen(str) не считает.
		strcpy_s(deviceTypeStructure->nameType, strlen(buffCopy) + 1, buffCopy);

		// Ставим новый элемент в начало, смещая предыдущий на второе место 
		deviceTypeStructure->next = deviceTypes;
		deviceTypes = deviceTypeStructure;

		fileTypeStringsCount++;
	}

	std::cout << "Обработка файла " << fileDevice << std::endl;
	while (!feof(fileDevice)) { // Заполнение devices
		fgets(buff, 126, fileDevice);

		if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
			continue;


		deviceStructure = new device();

		buffCopy = buff;
		//idDevice;id Manufacturer; Name device; Availability;id Type; Price;
		//buffCopy = 1;1;8800 Arte Black;1;1;36380; buff = 1;1;8800 Arte Black;1;1;36380;
		buffCopy = strchr(buff, ';');
		buffCopy[0] = '\0';
		//buffCopy = '\0'1;8800 Arte Black;1;1;36380; buff = 1'\0'1;8800 Arte Black;1;1;36380;
		deviceStructure->ID = atoi(buff);
		buffCopy++;
		//buffCopy = 1;8800 Arte Black;1;1;36380; buff = 1'\0'1;8800 Arte Black;1;1;36380;
		strchr(buffCopy, ';')[0] = '\0';
		//buffCopy = 1'\0'8800 Arte Black;1;1;36380; buff = 1'\0'1'\0'8800 Arte Black;1;1;36380;
		unsigned int manufacturerID = atoi(buffCopy);
		for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) {
			if (temp->ID == manufacturerID)
				deviceStructure->made = temp;
		}
		buffCopy = strchr(buffCopy, '\0');
		buffCopy++;
		//buffCopy = 8800 Arte Black;1;1;36380; buff = 1'\0'1'\0'8800 Arte Black;1;1;36380;
		strchr(buffCopy, ';')[0] = '\0';
		//buffCopy = 8800 Arte Black'\0'1;1;36380; buff = 1'\0'1'\0'8800 Arte Black'\0'1;1;36380;
		deviceStructure->nameDevice = new char[strlen(buffCopy) + 1];
		strcpy_s(deviceStructure->nameDevice, strlen(buffCopy) + 1, buffCopy);
		buffCopy = strchr(buffCopy, '\0');
		buffCopy++;
		//buffCopy = 1;1;36380; buff = 1'\0'1'\0'8800 Arte Black'\0'1;1;36380;
		strchr(buffCopy, ';')[0] = '\0';
		//buffCopy = 1'\0'1;36380; buff = 1'\0'1'\0'8800 Arte Black'\0'1'\0'1;36380;
		deviceStructure->availability = atoi(buffCopy);
		buffCopy = strchr(buffCopy, '\0');
		buffCopy++;
		//buffCopy = 1;36380; buff = 1'\0'1'\0'8800 Arte Black'\0'1'\0'1;36380;
		strchr(buffCopy, ';')[0] = '\0';
		//buffCopy = 1'\0'36380; buff = 1'\0'1'\0'8800 Arte Black'\0'1'\0'1'\0'36380;
		unsigned int typeID = atoi(buffCopy);
		for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {
			if (temp->ID == typeID)
				deviceStructure->type = temp;
		}
		buffCopy = strchr(buffCopy, '\0');
		buffCopy++;
		//buffCopy = 36380; buff = 1'\0'1'\0'8800 Arte Black'\0'1'\0'1'\0'36380;
		strchr(buffCopy, ';')[0] = '\0';
		//buffCopy = 36380'\0' buff = 1'\0'1'\0'8800 Arte Black'\0'1'\0'1'\0'36380'\0'
		deviceStructure->price = atoi(buffCopy);

		deviceStructure->next = devices;
		devices = deviceStructure;

		fileDeviceStringsCount++;
	}
	if (DEBAG) for (device *temp = devices; temp != NULL; temp = temp->next)
		std::cout <<
		"ID device: " << temp->ID <<
		" Name: " << temp->nameDevice <<
		" Manufacturer: " << temp->made->nameManufacturer <<
		" Type: " << temp->type->nameType <<
		" Availability: " << (temp->availability ? "yes" : "no") <<
		" Price: " << temp->price << std::endl;

	fclose(fileMade);
	fclose(fileDevice);
	fclose(fileType);

	sort();//Сортируем

	if (DEBAG) {
		std::cout << "DEBAG: База благополучно загружена" << std::endl;
		//system("pause");
	}
}
//
//while (std::cin.getline(buff, 126)) {
//	std::cout << "read line: " << buff << std::endl;
//	std::cin >> buff;
//	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//	std::cout << "read cin line: " << buff << std::endl;
//}

/*!
	\brief Добавление записи
	\attantion После добавления новой записи, изменения в файлах не сохраняются!

	Добавление записи в структуру devices, в случае необходимости возможно добавление в deviceManufactures и/или deviceTypes данные вводятся пользователем с клавиатуры.
*/


void addToDB() {
	char buff[128];
	device *newDevice = new device();
	unsigned int ID = 0;
	for (device *temp = devices; temp != NULL; temp = temp->next) // Создание уникального id 
	{
		if (temp->ID > ID)
			ID = temp->ID + 1;
	}
	newDevice->ID = ID;
	//TODO std::cout << "Выберите с помощью стрелок поле для заполнения." << std::endl;
	//std::cout << "\033[40;40H nidfjnlijnsdfijbnlijsdbfn" << std::endl;
	system("cls");
	std::cout << "Название: " << std::endl; //\033[1;11H
	std::cout << "Наличие (д/н): " << std::endl; //\033[2;16H
	std::cout << "Цена: " << std::endl; //\033[3;7H
	std::cout << "Производитель: " << std::endl; //\033[4;16H
	std::cout << "Тип: " << std::endl; //\033[5;6H
	std::cout << "\033[1;11H"; //move to 1:11
	std::cin.getline(buff, 126, '\n');
	newDevice->nameDevice = new char[strlen(buff) + 1];
	strcpy_s(newDevice->nameDevice, strlen(buff) + 1, buff);
	std::cout << "\033[2;16H"; // move to 2:16
	//std::cin >> buff;
	std::cin.getline(buff, 126, '\n');
	//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //std::cin >> buff; Оставляет после себя '\n' в буфере 
	newDevice->availability = (buff[0] == 'д');
	std::cout << "\033[3;7H"; // move to 3:7
	std::cin.getline(buff, 126, '\n');
	newDevice->price = atoi(buff);

	unsigned int  manufacturerID = addMore(false); // Manufacturer
	if (!manufacturerID) {
		system("cls");
		deviceMade *newDeviceManufacturer = new deviceMade();
		//TODO создавать id которые пропущены после удаления, например 1 2 4 5 6 ... Пропущен 3, его нужно использовать!
		ID = 0;
		for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) // Создание уникального id 
		{
			if (temp->ID > ID)
				ID = temp->ID + 1;
		}
		newDeviceManufacturer->ID = 0;
		std::cout << "Название нового производителя: " << std::endl; //\033[1;32H
		std::cout << "\033[1;32H"; //move to 1:32
		std::cin.getline(buff, 126);
		newDeviceManufacturer->nameManufacturer = new char[strlen(buff) + 1];
		strcpy_s(newDeviceManufacturer->nameManufacturer, strlen(buff) + 1, buff);
		newDeviceManufacturer->next = deviceManufactures;
		deviceManufactures = newDeviceManufacturer;
		newDevice->made = newDeviceManufacturer;
	}
	else
		for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next)
			if (temp->ID == manufacturerID)
				newDevice->made = temp;
	unsigned int  typeID = addMore(true); // device Type
	if (!typeID) {
		system("cls");
		deviceType *newDeviceType = new deviceType();
		ID = 0;
		for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) // Создание уникального id 
		{
			if (temp->ID > ID)
				ID = temp->ID + 1;
		}
		newDeviceType->ID = ID;
		std::cout << "Название нового типа устройств: " << std::endl; //\033[1;33H
		std::cout << "\033[1;33H"; //move to 1:33
		std::cin.getline(buff, 126);
		newDeviceType->nameType = new char[strlen(buff) + 1];
		strcpy_s(newDeviceType->nameType, strlen(buff) + 1, buff);
		newDeviceType->next = deviceTypes;
		deviceTypes = newDeviceType;
		newDevice->type = newDeviceType;
	}
	else
		for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next)
			if (temp->ID == typeID)
				newDevice->type = temp;

	for (device *temp = newDevice; temp != NULL; temp = temp->next)
		std::cout <<
		"ID device: " << temp->ID << std::endl <<
		" Name: " << temp->nameDevice << std::endl <<
		" Manufacturer: " << temp->made->nameManufacturer << std::endl <<
		" Type: " << temp->type->nameType << std::endl <<
		" Availability: " << (temp->availability ? "yes" : "no") << std::endl <<
		" Price: " << temp->price << std::endl;

	newDevice->next = devices;
	devices = newDevice;

	sort(); //Сортировка поместит элемент куда надо.

	std::cout << "Успешно! " << std::endl;
	system("pause");
	system("cls");
}

/*!
	\brief Добавление записи в структуру devices
	\attantion После добавления новой записи, изменения в файлах не сохраняются!
	\return unsigned int - id выбраного пункта, или 0 если создать новый.
	Добавление записи в структуру devices, данные вводятся пользователем с клавиатуры.
*/

//return: id выбраного пункта, или 0 если создать новый.
unsigned int addMore(bool type) { //type =0 - Manufacturer type=1 - deviceTypes
	unsigned int select = 0;//set to default
	unsigned int count = 0; //Используется для ограничения возможность передвижения ползунка выбора
	unsigned char key = '\0';

	while (true) {
		system("cls");
		if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;
		std::cout << "Выберите с помощью стрелок на клавиатуре из представленного ниже списка или добавте новый элемент." << std::endl << std::endl;

		std::cout << (0 == select ? " [*] " : " [ ] ") << "Добвить новый" << std::endl;
		if (type) // Types
			for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {
				std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameType << std::endl;
				count++;
			}
		else // Manufacturer
			for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) {
				std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameManufacturer << std::endl;
				count++;
			}

		key = _getch();
		switch (key) {
		case ENTER:
			return select;
		case UP:
			if (select) // != 0
				select--;
			break;
		case DOWN:
			if (select < count)
				select++;
			break;
		case ESC:
			select = 0;
			break;
		}
		count = 0;
	}
}

/*!
	\brief Добавление записи в структуру devices
	\attantion После добавления новой записи, изменения в файлах не сохраняются!

	Добавление записи в структуру devices, данные вводятся пользователем с клавиатуры.
*/

void removeFromDB() {
	unsigned int select = 1;
	unsigned int count = 0; //Используется для ограничения возможность передвижения ползунка выбора
	unsigned char key = '\0';
	bool type = false;
	bool flag = true;
	char buff[126];

	system("cls");
	char menu[4][25] = {
		"Производители устройств",// 24 символа
		"Типы устройств",
		"Устройства",
		"Отмена"
	};

	while (true)
	{
		while (flag) {
			system("cls");
			if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;
			std::cout << "Выберите из какой категории вы хотели бы удалить." << std::endl;
			std::cout << "Используйте стрелки на клавиатуре для выбора." << std::endl;
			std::cout << "Перед удалением производителя или типа устройства, следует убедиться что они не используются." << std::endl << std::endl;

			for (unsigned short int i = 0; i < 4; i++) {
				std::cout << (i == select ? " [*] " : " [ ] ") << menu[i] << std::endl;
				count++;
			}

			key = _getch();
			switch (key) {
			case ENTER:
				flag = false;
				break;
			case UP:
				if (select) // != 0
					select--;
				break;
			case DOWN:
				if (select < count - 1)//count-1 тк. нумерация в menu начинается с 0
					select++;
				break;
			case ESC:
				select = 0;
				break;
			}
			count = 0;
		}
		count = 0;
		flag = true;
		switch (select) {
		case 0: //"Производители устройств"
			select = 0;//set to default
			while (flag) {
				system("cls");
				if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

				std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
				for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) {
					std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameManufacturer << std::endl;
					count++;
				}

				key = _getch();
				switch (key) {
				case ENTER:
					flag = false;
					break;
				case UP:
					if (select) // != 0
						select--;
					break;
				case DOWN:
					if (select < count)
						select++;
					break;
				case ESC:
					select = 0;
					break;
				}
				count = 0;
			}
			if (!select) {
				flag = true;
				select = 0;//set to default
				continue;
			}
			else {
				flag = false; //Используется для проверки на использование Производителя в каком-либо устройстве
				for (device *temp = devices; temp != NULL; temp = temp->next) { // Проверка удаляемого на использование
					if (temp->made->ID == select) {
						flag = true;
						strcpy_s(buff, strlen(temp->nameDevice) + 1, temp->nameDevice);
						break;
					}
				}
				if (flag) {
					std::cout << buff << " устройство произведено этим производителем, удалите все устройства данного произвоизводителя прежде чем удалить самого производителя." << std::endl;
					flag = true; // чтобы вернуться в первоначальное меню
					system("pause");
					continue;
				}
				deviceMade *removableDeviceManufacturer;
				if (deviceManufactures->ID == select) {
					removableDeviceManufacturer = deviceManufactures;
					deviceManufactures = deviceManufactures->next;
					removableDeviceManufacturer->ID = NULL;
					removableDeviceManufacturer->next = NULL;
					delete[] removableDeviceManufacturer->nameManufacturer;
					removableDeviceManufacturer->nameManufacturer = NULL;
					delete removableDeviceManufacturer;
					sort();//Наводим порядок после себя
					flag = false;//Выход на меню выше 
				}
				else
					for (deviceMade *temp = deviceManufactures; temp->next != NULL; temp = temp->next) // Поиск удаляемого 
					{
						if (temp->next->ID == select) {
							removableDeviceManufacturer = temp->next;
							temp->next = temp->next->next;
							removableDeviceManufacturer->ID = NULL;
							removableDeviceManufacturer->next = NULL;
							delete[] removableDeviceManufacturer->nameManufacturer;
							removableDeviceManufacturer->nameManufacturer = NULL;
							delete removableDeviceManufacturer;
							sort();//Наводим порядок после себя
							flag = false;//Выход на меню выше 
							break;
						}
					}
			}
			flag = true;//Не выйти слишком высоко 
			select = 0;//set to default
			continue;
		case 1://"Типы устройств",
			select = 0;//set to default
			while (flag) {
				system("cls");
				if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

				std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
				for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {
					std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameType << std::endl;
					count++;
				}

				key = _getch();
				switch (key) {
				case ENTER:
					flag = false;
					break;
				case UP:
					if (select) // != 0
						select--;
					break;
				case DOWN:
					if (select < count)
						select++;
					break;
				case ESC:
					select = 0;
					break;
				}
				count = 0;
			}
			if (!select) {
				flag = true;
				select = 0;//set to default
				continue;
			}
			else {
				flag = false; //Используется для проверки на использование данного типа в каком-либо устройстве
				for (device *temp = devices; temp != NULL; temp = temp->next) { // Проверка удаляемого на использование
					if (temp->type->ID == select) {
						flag = true;
						strcpy_s(buff, strlen(temp->nameDevice) + 1, temp->nameDevice);
						break;
					}
				}
				if (flag) {
					std::cout << buff << " устройство является этим типом, удалите все устройства данного типа прежде чем удалить сам тип устройств." << std::endl;
					flag = true; // чтобы вернуться в первоначальное меню
					system("pause");
					continue;
				}
				deviceType *removableDeviceType;
				if (deviceTypes->ID == select) {
					removableDeviceType = deviceTypes;
					deviceTypes = deviceTypes->next;
					removableDeviceType->ID = NULL;
					removableDeviceType->next = NULL;
					delete[] removableDeviceType->nameType;
					removableDeviceType->nameType = NULL;
					delete removableDeviceType;
					sort();//Наводим порядок после себя
					flag = false;//Выход на меню выше 
				}
				else
					for (deviceType *temp = deviceTypes; temp->next != NULL; temp = temp->next) // Поиск удаляемого 
					{
						if (temp->next->ID == select) {
							removableDeviceType = temp->next;
							temp->next = temp->next->next;
							removableDeviceType->ID = NULL;
							removableDeviceType->next = NULL;
							delete[] removableDeviceType->nameType;
							removableDeviceType->nameType = NULL;
							delete removableDeviceType;
							sort();//Наводим порядок после себя
							flag = false;//Выход на меню выше 
							break;
						}
					}
			}
			flag = true;//Не выйти слишком высоко 
			select = 0;//set to default
			continue;
		case 2://"Устройства"
			select = 0;//set to default
			while (flag) {
				system("cls");
				if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

				std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
				for (device *temp = devices; temp != NULL; temp = temp->next) {
					std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameDevice << std::endl;
					count++;
				}

				key = _getch();
				switch (key) {
				case ENTER:
					flag = false;
					break;
				case UP:
					if (select) // != 0
						select--;
					break;
				case DOWN:
					if (select < count)
						select++;
					break;
				case ESC:
					select = 0;
					break;
				}
				count = 0;
			}
			if (!select) {
				flag = true;
				select = 0;//set to default
				continue;
			}
			else {
				device *removableDevice;
				if (devices->ID == select) {
					removableDevice = devices;
					devices = devices->next;
					removableDevice->ID = NULL;
					removableDevice->next = NULL;
					delete[] removableDevice->nameDevice;
					removableDevice->nameDevice = NULL;
					delete removableDevice;
					sort();//Наводим порядок после себя
					flag = false;//Выход на меню выше 
				}
				else
					for (device *temp = devices; temp->next != NULL; temp = temp->next) // Поиск удаляемого 
					{
						if (temp->next->ID == select) {
							removableDevice = temp->next;
							temp->next = temp->next->next;
							removableDevice->ID = NULL;
							removableDevice->next = NULL;
							delete[] removableDevice->nameDevice;
							removableDevice->nameDevice = NULL;
							delete removableDevice;
							sort();//Наводим порядок после себя
							flag = false;//Выход на меню выше 
							break;
						}
					}
			}
			flag = true;//Не выйти слишком высоко 
			select = 0;//set to default
			continue;
			break;
		case 3://"Отмена"
			return;
		}
		break;
	}
	exit(0);
}

//Пояснять происходящее внутри довольно бесполезно, берите бумажку, ручку и рисуйте что происходит
void sort() {
	//Велосипед, тк нас научили писать сортировку самим а не использовать хорошо написанные вещи из стандартной библиотеки..
	//Элементов немного поэтому сортировать будем пузырьком.
	//Кроме сортировки производится поиск пропущенных по порядку id, id которые пропущены после удаления, например 1 2 4 5 6 ... Пропущен 3, его нужно использовать 
	unsigned int ID = 0;//Для поиска пропущенных id 
	// ===== Manufacturer ======
	// add manufacturer with id =0;
	deviceMade *tempDeviceMade = new deviceMade();
	tempDeviceMade->ID = 0;
	tempDeviceMade->nameManufacturer = new char[strlen("for sort") + 1];
	strcpy_s(tempDeviceMade->nameManufacturer, strlen("for sort") + 1, "for sort");
	tempDeviceMade->next = deviceManufactures;
	deviceManufactures = tempDeviceMade;
	tempDeviceMade = NULL;
	//Непосредственно сортировка
	bool flag = true;
	while (flag) {
		flag = false;
		for (deviceMade *temp = deviceManufactures; temp != NULL && temp->next != NULL && temp->next->next != NULL; temp = temp->next) {
			if (temp->next->ID > temp->next->next->ID) {
				tempDeviceMade = temp->next->next;
				temp->next->next = tempDeviceMade->next;
				tempDeviceMade->next = temp->next;
				temp->next = tempDeviceMade;
				break;
			}
		}
		for (deviceMade *temp = deviceManufactures; temp != NULL && temp->next != NULL; temp = temp->next) { //Проверка что всё отсортировалось
			if (temp->ID > temp->next->ID) {
				flag = true;
				break;
			}
		}
	}

	// remove manufacturer with id = 0;
	tempDeviceMade = deviceManufactures;
	deviceManufactures = tempDeviceMade->next;
	tempDeviceMade->ID = NULL;
	tempDeviceMade->next = NULL;
	delete[] tempDeviceMade->nameManufacturer;
	tempDeviceMade->nameManufacturer = NULL;
	delete tempDeviceMade;
	tempDeviceMade = NULL;

	//Поиск пропущенных id 
	ID = 0;
	for (deviceMade *tempMade = deviceManufactures; tempMade != NULL; tempMade = tempMade->next) {
		if (++ID != tempMade->ID) {
			for (device *tempDevice = devices; tempDevice != NULL; tempDevice = tempDevice->next) {
				if (tempDevice->made->ID == tempMade->ID)
					tempDevice->made->ID = ID;
			}
			tempMade->ID = ID;
		}
	}

	if (DEBAG) {
		std::cout << " ===== Manufacturer ====== " << std::endl;
		for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) {

			std::cout << "ID: ";
			std::cout.width(3);
			std::cout << temp->ID << "; Name: " << temp->nameManufacturer << std::endl;
		}
		std::cout << "=================================" << std::endl;
	}

	//====== Type =====
	//add type with id 0;
	deviceType *tempDeviceType = new deviceType();
	tempDeviceType->ID = 0;
	tempDeviceType->nameType = new char[strlen("for sort") + 1];
	strcpy_s(tempDeviceType->nameType, strlen("for sort") + 1, "for sort");
	tempDeviceType->next = deviceTypes;
	deviceTypes = tempDeviceType;
	tempDeviceType = NULL;
	//Непосредственно сортировка
	flag = true;
	while (flag) {
		flag = false;
		for (deviceType *temp = deviceTypes; temp != NULL && temp->next != NULL && temp->next->next != NULL; temp = temp->next) {
			if (temp->next->ID > temp->next->next->ID) {
				tempDeviceType = temp->next->next;
				temp->next->next = tempDeviceType->next;
				tempDeviceType->next = temp->next;
				temp->next = tempDeviceType;
				break;
			}
		}
		for (deviceType *temp = deviceTypes; temp != NULL && temp->next != NULL; temp = temp->next) {//Проверка что всё отсортировалось
			if (temp->ID > temp->next->ID) {
				flag = true;
				break;
			}
		}
	}

	if (DEBAG) {
		std::cout << "====== Type =====" << std::endl;
		for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {

			std::cout << "ID: ";
			std::cout.width(3);
			std::cout << temp->ID << "; Name: " << temp->nameType << std::endl;
		}
		std::cout << "=================================" << std::endl;
	}
	// remove type with id = 0;
	tempDeviceType = deviceTypes;
	deviceTypes = tempDeviceType->next;
	tempDeviceType->ID = NULL;
	tempDeviceType->next = NULL;
	delete[] tempDeviceType->nameType;
	tempDeviceType->nameType = NULL;
	delete tempDeviceType;
	tempDeviceType = NULL;

	//Поиск пропущенных id 
	ID = 0;
	for (deviceType *tempType = deviceTypes; tempType != NULL; tempType = tempType->next) {
		if (++ID != tempType->ID) {
			for (device *tempDevice = devices; tempDevice != NULL; tempDevice = tempDevice->next) {
				if (tempDevice->type->ID == tempType->ID)
					tempDevice->type->ID = ID;
			}
			tempType->ID = ID;
		}
	}

	//====== Device ======
	//TODO test them
	//add device with id 0;
	device *tempDevice = new device();
	tempDevice->ID = 0;
	tempDevice->nameDevice = new char[strlen("for sort") + 1];
	strcpy_s(tempDevice->nameDevice, strlen("for sort") + 1, "for sort");
	tempDevice->next = devices;
	devices = tempDevice;
	tempDevice = NULL;
	//Непосредственно сортировка
	flag = true;
	while (flag) {
		flag = false;
		for (device *temp = devices; temp != NULL && temp->next != NULL && temp->next->next != NULL; temp = temp->next) {
			if (temp->next->ID > temp->next->next->ID) {
				tempDevice = temp->next->next;
				temp->next->next = tempDevice->next;
				tempDevice->next = temp->next;
				temp->next = tempDevice;
				break;
			}
		}
		for (device *temp = devices; temp != NULL && temp->next != NULL; temp = temp->next) {//Проверка что всё отсортировалось
			if (temp->ID > temp->next->ID) {
				flag = true;
				break;
			}
		}
	}
	if (DEBAG) {
		std::cout << "====== Device ======" << std::endl;
		for (device *temp = devices; temp != NULL; temp = temp->next) {

			std::cout << "ID: ";
			std::cout.width(3);
			std::cout << temp->ID << "; Name: " << temp->nameDevice << std::endl;
		}
		std::cout << "=================================" << std::endl;
	}
	// remove device with id = 0;
	tempDevice = devices;
	devices = tempDevice->next;
	tempDevice->ID = NULL;
	tempDevice->availability = NULL;
	tempDevice->made = NULL;
	tempDevice->next = NULL;
	tempDevice->price = NULL;
	tempDevice->type = NULL;
	delete[] tempDevice->nameDevice;
	tempDevice->nameDevice = NULL;
	delete tempDevice;
	tempDevice = NULL;
	//Поиск пропущенных id 
	ID = 0;
	for (device *temp = devices; temp != NULL; temp = temp->next) {
		if (++ID != temp->ID) {
			temp->ID = ID;
		}
	}
}

/*!
	\brief Сохранение изменений в файлах
	\attantion Файлы перезаписываются целиком, требуются права на запись

	Сохранение изменений в файлах madeFile, deviceFile, typeFile, файлы генериются с нуля на основе изменений сделаных пользователем
*/

void saveDB() {
	system("cls");
	std::cout << "Сохранение..." << std::endl;
	/*
	const char madeFile[8] = "made.db";    // Файл с производителями
	const char deviceFile[10] = "device.db";//Файл с устройствами
	const char typeFile[8] = "type.db";    // Файл с типами устройств
	*/
	std::ofstream ofstreamMadeFile, ofstreamDeviceFile, ofstreamTypeFile; // поток для записи
	ofstreamMadeFile.open(madeFile); // окрываем файл для записи
	ofstreamTypeFile.open(typeFile);
	ofstreamDeviceFile.open(deviceFile);

	// Если надо дозаписать текст в конец файла, то для открытия файла нужно использовать режим ios::app:
	//std::ofstream out("D:\\hello.txt", std::ios::app);
	if (ofstreamMadeFile.is_open() && ofstreamTypeFile.is_open() && ofstreamDeviceFile.is_open())
	{
		//Manufacturer
		ofstreamMadeFile << "#id Manufacturer;Name Manufacturer;" << std::endl;
		for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) { //Проверка что всё отсортировалось
			ofstreamMadeFile << temp->ID << ";" << temp->nameManufacturer << ";" << std::endl;
		}
		//Type
		ofstreamTypeFile << "#id Type;Name Type;" << std::endl;
		for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {
			ofstreamTypeFile << temp->ID << ";" << temp->nameType << ";" << std::endl;
		}
		//Device
		ofstreamDeviceFile << "#idDevice;id Manufacturer; Name device; Availability;id Type; Price;" << std::endl;
		for (device *temp = devices; temp != NULL; temp = temp->next) {
			ofstreamDeviceFile << temp->ID << ";" << temp->made->ID << ";" << temp->nameDevice << ";" << temp->availability << ";" << temp->type->ID << ";" << temp->price << ";" << std::endl;
		}

		std::cout << "Сохранение прошло успешно!" << std::endl;
		system("pause");
	}
	else
	{
		system("cls");
		std::cout << "\033[1;31m Ошибка записи в базу, повторите попытку \033[0m" << std::endl;
		std::cout << (ofstreamMadeFile.is_open() ? " [*] " : " [ ] ") << madeFile << std::endl;
		std::cout << (ofstreamTypeFile.is_open() ? " [*] " : " [ ] ") << typeFile << std::endl;
		std::cout << (ofstreamDeviceFile.is_open() ? " [*] " : " [ ] ") << deviceFile << std::endl;
	}
	ofstreamMadeFile.close();
	ofstreamTypeFile.close();
	ofstreamDeviceFile.close();
}

/*!
	\brief Отключение базы
	\attantion Данные на диск не сохраняются!

	Фактически производится очистка структур deviceStructure, deviceMadeStructure и deviceTypeStructure, удобно для последующего открытия другого набора файлов madeFile, deviceFile, typeFile
*/

void closeDB() {
	char buff[128];
	system("cls");
	std::cout << "Все не сохранёные измения потеряются..." << std::endl;
	std::cout << "Продолжить ? (д/н):" << std::endl;
	std::cin >> buff;
	if (buff[0] == 'д')
	{
		std::cout << "Отключение.." << std::endl;
		//Manufacturer
		deviceMade *removableDeviceManufacturer;
		for (deviceMade *temp = deviceManufactures; temp != NULL;) { //Проверка что всё отсортировалось
			removableDeviceManufacturer = temp;
			temp = temp->next;
			removableDeviceManufacturer->ID = NULL;
			delete[] removableDeviceManufacturer->nameManufacturer;
			removableDeviceManufacturer->nameManufacturer = NULL;
			removableDeviceManufacturer->next = NULL;
			delete removableDeviceManufacturer;
			removableDeviceManufacturer = NULL;
		}
		deviceManufactures = NULL;
		//Type
		deviceType *removableDeviceType;
		for (deviceType *temp = deviceTypes; temp != NULL;) {
			removableDeviceType = temp;
			temp = temp->next;
			removableDeviceType->ID = NULL;
			delete[] removableDeviceType->nameType;
			removableDeviceType->nameType = NULL;
			removableDeviceType->next = NULL;
			delete removableDeviceType;
			removableDeviceType = NULL;
		}
		//Device
		device *removableDevice;
		for (device *temp = devices; temp != NULL;) {
			removableDevice = temp;
			temp = temp->next;
			removableDevice->ID = NULL;
			delete[] removableDevice->nameDevice;
			removableDevice->nameDevice = NULL;
			removableDevice->next = NULL;
			removableDevice->availability = NULL;
			removableDevice->price = NULL;
			removableDevice->type = NULL;
			removableDevice->made = NULL;
			delete removableDevice;
			removableDevice = NULL;
		}
		std::cout << "Отключение прошло успешно!" << std::endl;
		system("pause");
	}
	else
	{
		std::cout << "Отмена" << std::endl;
		system("pause");
		return;
	}

}

/*!
	\brief Внесение изменений в данные
	\attantion Данные на диск автоматически не сохраняются!

	Производится изменение в данных структур deviceStructure, deviceMadeStructure и deviceTypeStructure, ввод данных от пользователя через консоль.
*/

void editDB() {
	unsigned int select = 1;
	unsigned int count = 0; //Используется для ограничения возможность передвижения ползунка выбора
	unsigned char key = '\0';
	bool type = false;
	bool flag = true;
	char buff[128];

	system("cls");
	char menu[4][25] = {
		"Производители устройств",// 24 символа
		"Типы устройств",
		"Устройства",
		"Отмена"
	};

	while (true)
	{
		while (flag) {
			system("cls");
			if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;
			std::cout << "Выберите записи из какой категории вы хотели бы изменить." << std::endl;
			std::cout << "Используйте стрелки на клавиатуре для выбора." << std::endl;

			for (unsigned short int i = 0; i < 4; i++) {
				std::cout << (i == select ? " [*] " : " [ ] ") << menu[i] << std::endl;
				count++;
			}

			key = _getch();
			switch (key) {
			case ENTER:
				flag = false;
				break;
			case UP:
				if (select) // != 0
					select--;
				break;
			case DOWN:
				if (select < count - 1)//count-1 тк. нумерация в menu начинается с 0
					select++;
				break;
			case ESC:
				select = 0;
				break;
			}
			count = 0;
		}
		count = 0;
		flag = true;
		switch (select)
		{
		case 0: //"Производители устройств"
			select = 0;//set to default
			while (flag) {
				system("cls");
				if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

				std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
				for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) {
					std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameManufacturer << std::endl;
					count++;
				}

				key = _getch();
				switch (key) {
				case ENTER:
					flag = false;
					break;
				case UP:
					if (select) // != 0
						select--;
					break;
				case DOWN:
					if (select < count)
						select++;
					break;
				case ESC:
					select = 0;
					break;
				}
				count = 0;
			}
			if (!select) {
				flag = true;
				select = 0;//set to default
				continue;
			}
			else {
				for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) {
					if (temp->ID == select) {
						if (DEBAG) std::cout << "ID " << temp->ID << std::endl;
						std::cout << "Старое название производителя: " << temp->nameManufacturer << std::endl;
						std::cout << "Новое: ";
						std::cin.getline(buff, 126, '\n');
						char *newName = new char[strlen(buff) + 1];
						strcpy_s(newName, strlen(buff) + 1, buff);
						std::cout << temp->nameManufacturer << " -> " << newName << std::endl;
						std::cout << "Вы уверены? (д/н) ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] == 'д') {
							delete[] temp->nameManufacturer;
							temp->nameManufacturer = newName;
							std::cout << "Успешно! " << std::endl;
							system("pause");
							system("cls");
						}
						else {
							delete[] newName;
							newName = NULL;
							std::cout << "Отменено!" << std::endl;
							system("pause");
						}
						break;
					}
				}
			}
			flag = true;//Не выйти слишком высоко 
			select = 0;//set to default
			continue;
		case 1://"Типы устройств",
			select = 0;//set to default
			while (flag) {
				system("cls");
				if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

				std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
				for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {
					std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameType << std::endl;
					count++;
				}

				key = _getch();
				switch (key) {
				case ENTER:
					flag = false;
					break;
				case UP:
					if (select) // != 0
						select--;
					break;
				case DOWN:
					if (select < count)
						select++;
					break;
				case ESC:
					select = 0;
					break;
				}
				count = 0;
			}
			if (!select) {
				flag = true;
				select = 0;//set to default
				continue;
			}
			else {
				for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {
					if (temp->ID == select) {
						if (DEBAG) std::cout << "ID " << temp->ID << std::endl;
						std::cout << "Старое наименование типа: " << temp->nameType << std::endl;
						std::cout << "Новое: ";
						std::cin.getline(buff, 126, '\n');
						char *newName = new char[strlen(buff) + 1];
						strcpy_s(newName, strlen(buff) + 1, buff);
						std::cout << temp->nameType << " -> " << newName << std::endl;
						std::cout << "Вы уверены? (д/н) ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] == 'д') {
							delete[] temp->nameType;
							temp->nameType = newName;
							std::cout << "Успешно! " << std::endl;
							system("pause");
							system("cls");
						}
						else {
							delete[] newName;
							newName = NULL;
							std::cout << "Отменено!" << std::endl;
							system("pause");
						}
						break;
					}
				}
			}
			flag = true;//Не выйти слишком высоко 
			select = 0;//set to default
			continue;
		case 2://"Устройства"
			select = 0;//set to default
			while (flag) {
				system("cls");
				if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

				std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
				for (device *temp = devices; temp != NULL; temp = temp->next) {
					std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameDevice << std::endl;
					count++;
				}

				key = _getch();
				switch (key) {
				case ENTER:
					flag = false;
					break;
				case UP:
					if (select) // != 0
						select--;
					break;
				case DOWN:
					if (select < count)
						select++;
					break;
				case ESC:
					select = 0;
					break;
				}
				count = 0;
			}
			if (!select) {
				flag = true;
				select = 0;//set to default
				continue;
			}
			else {
				for (device *tempDevice = devices; tempDevice != NULL; tempDevice = tempDevice->next) {
					if (tempDevice->ID == select) {
						device *newDevice = new device();
						if (DEBAG) std::cout << "ID " << tempDevice->ID << std::endl;
						std::cout << "Введите новое значение или нажмите Enter если хотите оставить значение прежним." << std::endl;

						std::cout << "Наличие (д/н): " << (tempDevice->availability ? "yes" : "no") << " -> ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] != '\0')
							newDevice->availability = (buff[0] == 'д');
						else
							newDevice->availability = tempDevice->availability;

						std::cout << "Цена: " << tempDevice->price << " -> ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] != '\0')
							newDevice->price = atoi(buff);
						else
							newDevice->price = tempDevice->price;

						std::cout << "Название: " << tempDevice->nameDevice << " -> ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] != '\0') {
							newDevice->nameDevice = new char[strlen(buff) + 1];
							strcpy_s(newDevice->nameDevice, strlen(buff) + 1, buff);
						}
						else
							newDevice->nameDevice = tempDevice->nameDevice; //Косяков с паятью нет, тут требуется скопировать ссылку, а не строку в памяти

						std::cout << "Тип: " << tempDevice->type->nameType << " -> ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] != '\0') {
							unsigned int typeID = addMore(true);
							if (typeID) {
								for (deviceType *temp = deviceTypes; temp != NULL; temp = temp->next) {
									if (temp->ID == typeID)
										newDevice->type = temp;
								}
							}
							else {
								std::cout << "В данный момент добавление новых типов ограниченно, воспользуйтесь меню создания для этого." << std::endl;
								newDevice->type = tempDevice->type;
							}
						}
						else
							newDevice->type = tempDevice->type;

						std::cout << "Производитель: " << tempDevice->made->nameManufacturer << " -> ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] != '\0') {
							unsigned int manufacturerID = addMore(false);
							if (manufacturerID) {
								for (deviceMade *temp = deviceManufactures; temp != NULL; temp = temp->next) {
									if (temp->ID == manufacturerID)
										newDevice->made = temp;
								}
							}
							else {
								std::cout << "В данный момент добавление новых производителей ограниченно, воспользуйтесь меню создания для этого." << std::endl;
								newDevice->made = tempDevice->made;
							}
						}
						else
							newDevice->made = tempDevice->made;
						system("cls");
						std::cout << "Проверьте правильность предпологаемых изменений." << std::endl;
						std::cout << "Наличие (д/н): " << (tempDevice->availability ? "yes" : "no") << " -> " << (newDevice->availability ? "yes" : "no") << std::endl;
						std::cout << "Цена: " << tempDevice->price << " -> " << newDevice->price << std::endl;
						std::cout << "Название: " << tempDevice->nameDevice << " -> " << newDevice->nameDevice << std::endl;
						std::cout << "Тип: " << tempDevice->type->nameType << " -> " << newDevice->type->nameType << std::endl;
						std::cout << "Производитель: " << tempDevice->made->nameManufacturer << " -> " << newDevice->made->nameManufacturer << std::endl;

						std::cout << "Подтвердить изменения? (д/н) ";
						std::cin.getline(buff, 126, '\n');
						if (buff[0] == 'д') {

							tempDevice->availability = newDevice->availability;
							tempDevice->price = newDevice->price;
							tempDevice->type = newDevice->type;
							tempDevice->made = newDevice->made;

							if (tempDevice->nameDevice != newDevice->nameDevice) {
								delete[] tempDevice->nameDevice;
								tempDevice->nameDevice = newDevice->nameDevice;
							}
							std::cout << "Успешно! " << std::endl;
							system("pause");
							system("cls");
						}
						else {

							if (tempDevice->nameDevice != newDevice->nameDevice) {
								delete[] newDevice->nameDevice;
								newDevice->nameDevice = NULL;
							}

							std::cout << "Отменено!" << std::endl;
							system("pause");
						}
						break;
					}
				}
			}
			flag = true;//Не выйти слишком высоко 
			select = 0;//set to default
			continue;
			break;
		case 3://"Отмена"
			return;
		}
		break;
	}
}

/*!
	\brief Предостовление полных данных

	Меню выбора устройства по имени предназначенное для отображения полной информации о выбраном объекте
*/


void printTOScreen() {
	unsigned int select = 0, count = 0;
	bool flag = true;
	unsigned char key;
	while (flag) {
		system("cls");
		if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

		std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
		for (device *temp = devices; temp != NULL; temp = temp->next) {
			std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameDevice << std::endl;
			count++;
		}

		key = _getch();
		switch (key) {
		case ENTER:
			flag = false;
			break;
		case UP:
			if (select) // != 0
				select--;
			break;
		case DOWN:
			if (select < count)
				select++;
			break;
		case ESC:
			select = 0;
			break;
		}
		count = 0;
	}
	if (!select) {
		flag = true;
		select = 0;//set to default
	}
	else {
		for (device *tempDevice = devices; tempDevice != NULL; tempDevice = tempDevice->next) {
			if (tempDevice->ID == select) {
				if (DEBAG) std::cout << "ID " << tempDevice->ID << std::endl;
				system("cls");
				std::cout << "Устройство с ID " << tempDevice->ID << std::endl << std::endl;

				std::cout << "Наличие: " << (tempDevice->availability ? "Имеется" : "Отсутствует") << std::endl;
				std::cout << "Цена: " << tempDevice->price << std::endl;
				std::cout << "Название: " << tempDevice->nameDevice << std::endl;
				std::cout << "Тип: " << tempDevice->type->nameType << std::endl;
				std::cout << "Производитель: " << tempDevice->made->nameManufacturer << std::endl;

				system("pause");
				system("cls");
				break;
			}
		}
	}

}

/*!
	\brief Предостовление полных данных, вывод в текстовый файл

	Меню выбора устройства по имени предназначенное для сохранения полной информации о выбраном объекте в файле
*/

void printToTxt() {
	unsigned int select = 0, count = 0;
	bool flag = true;
	unsigned char key;
	std::ofstream ofstreamPrintFile; // поток для записи
	while (flag) {
		system("cls");
		if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

		std::cout << (0 == select ? " [*] " : " [ ] ") << "Отмена" << std::endl;
		for (device *temp = devices; temp != NULL; temp = temp->next) {
			std::cout << (temp->ID == select ? " [*] " : " [ ] ") << temp->nameDevice << std::endl;
			count++;
		}

		key = _getch();
		switch (key) {
		case ENTER:
			flag = false;
			break;
		case UP:
			if (select) // != 0
				select--;
			break;
		case DOWN:
			if (select < count)
				select++;
			break;
		case ESC:
			select = 0;
			break;
		}
		count = 0;
	}
	if (!select) {
		flag = true;
		select = 0;//set to default
	}
	else {
		for (device *tempDevice = devices; tempDevice != NULL; tempDevice = tempDevice->next) {
			if (tempDevice->ID == select) {
				if (DEBAG) std::cout << "ID " << tempDevice->ID << std::endl;
				system("cls");
				std::cout << "Устройство с ID " << tempDevice->ID << std::endl << std::endl;

				std::cout << "Наличие: " << (tempDevice->availability ? "Имеется" : "Отсутствует") << std::endl;
				std::cout << "Цена: " << tempDevice->price << std::endl;
				std::cout << "Название: " << tempDevice->nameDevice << std::endl;
				std::cout << "Тип: " << tempDevice->type->nameType << std::endl;
				std::cout << "Производитель: " << tempDevice->made->nameManufacturer << std::endl;

				char *buff = new char[strlen(tempDevice->nameDevice) + 1 + strlen(".txt")];
				strcpy_s(buff, strlen(tempDevice->nameDevice) + 1, tempDevice->nameDevice);
				strcat_s(buff, strlen(tempDevice->nameDevice) + 1 + strlen(".txt"), ".txt"); // СМ https://msdn.microsoft.com/en-us/library/d45bbxx4.aspx
				ofstreamPrintFile.open(buff);

				if (ofstreamPrintFile.is_open()) {
					std::cout << "Файл " << buff << " успешно создан." << std::endl;

					ofstreamPrintFile << "Устройство с ID " << tempDevice->ID << std::endl << std::endl;

					ofstreamPrintFile << "Наличие: " << (tempDevice->availability ? "Имеется" : "Отсутствует") << std::endl;
					ofstreamPrintFile << "Цена: " << tempDevice->price << std::endl;
					ofstreamPrintFile << "Название: " << tempDevice->nameDevice << std::endl;
					ofstreamPrintFile << "Тип: " << tempDevice->type->nameType << std::endl;
					ofstreamPrintFile << "Производитель: " << tempDevice->made->nameManufacturer << std::endl;
				}
				else {
					std::cout << "Файл " << buff << " неудалось создать возможно в названии есть символы не допускаемые для использования в названиях файлов." << std::endl;
				}
				ofstreamPrintFile.close();
				system("pause");
				system("cls");
				break;
			}
		}
	}

}

/*!
	\brief Справочная система

	Полностью независящая функция предоставляющая возможность получения справочной информации подготовленной в файле infoFile, взаимодействие с пользователем через консоль.
*/

void manual() {
	std::ifstream in(infoFile); // Открываем файл для чтения
	char buff[65536], *buffCopy = NULL;
	unsigned short int count = 0;
	unsigned short int select = 0;
	unsigned char key;
	bool flag = true;
	info *infos;

	system("cls");
	std::cout << "			Справочник" << std::endl;
	if (in.is_open())
	{
		while (!in.eof())
		{
			in.getline(buff, 65535, '\n');
			if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
				continue;
			if (DEBAG) std::cout << buff << std::endl;
			count++;
		}
		in.seekg(0, std::ios_base::beg);//В начало
		infos = new info[count]();
		count = 0;
		while (!in.eof())
		{
			in.getline(buff, 65535, '\n');
			if (buff[0] == '#' || buff[0] == '\n' || strlen(buff) < 2) //Пропускаем комментарии и пустые строки
				continue;
			std::cout << buff << std::endl;
			buffCopy = buff;
			strchr(buffCopy, ';')[0] = '\0';
			infos[count].name = new char[strlen(buffCopy) + 1];
			strcpy_s(infos[count].name, strlen(buffCopy) + 1, buffCopy);
			buffCopy = strchr(buffCopy, '\0');
			buffCopy++;
			infos[count].infoData = new char[strlen(buffCopy) + 1];
			strcpy_s(infos[count].infoData, strlen(buffCopy) + 1, buffCopy);
			count++;
		}
		if (DEBAG) for (unsigned short int i = 0; i < count; i++)
		{
			std::cout << infos[i].name << std::endl;
			std::cout << "	|->" << infos[i].infoData << std::endl;
		}
	}
	else {
		std::cout << "Ошибка открытия файла " << infoFile << std::endl;
		system("pause");
		system("cls");
		return;
	}
	in.close();     // закрываем файл

	while (flag) {
		system("cls");
		std::cout << "			Справочник" << std::endl;
		if (DEBAG) std::cout << "DEBAG: select = " << select << std::endl;

		for (unsigned short int i = 0; i < count; i++) {
			std::cout << (i == select ? " [*] " : " [ ] ") << infos[i].name << std::endl;
		}

		key = _getch();
		switch (key) {
		case ENTER:
			flag = false;
			break;
		case UP:
			if (select) // != 0
				select--;
			break;
		case DOWN:
			if (select < count - 1)
				select++;
			break;
		case ESC:
			select = 0;
			break;
		}
	}
	system("cls");
	std::cout << "			Справочник" << std::endl << std::endl;
	std::cout << infos[select].name << std::endl;
	std::cout << "	-> " << infos[select].infoData << std::endl;
	system("pause");
	system("cls");
	for (unsigned short int i = 0; i < count; i++) {
		delete[] infos[i].name;
		delete[] infos[i].infoData;
		infos[i].name = NULL;
		infos[i].infoData = NULL;
	}
	delete[] infos;
	infos = NULL;
}