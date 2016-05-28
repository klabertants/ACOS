#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#include <sys/socket.h> //socket
#include <sys/select.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

int read_from_and_write_to(int fromfd, int tofd)
{
	char buffer[4];
	int len;
	//Читаем из fromfd через buffer по 4 символа, записывам в tofd.
	len = read(fromfd, buffer, sizeof(buffer));
		int rc = write(tofd, buffer, len);
		if (rc == -1)
			perror("write"), exit(1);
	
	return len;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s host port\n", argv[0]);
		exit(1);
	}

	char *host = argv[1];
	//почему мы просто не считываем готовое число?
	int port = strtol(argv[2], NULL, 10); //возвращает число из строки (1 арг) в десятичной системе (10 - третий аргумент), где null - там обычно end (указатель на символ, стоящий после числа)
	if (errno == ERANGE)//это код ошибки выдаваемой, Если результат не может быть представлен как значение типа long int
		perror("Invalid port"), exit(1);

	int sockfd;
	//socket() создаёт конечную точку соединения и возвращает её дескриптор.
	// int socket(int domain, int type, int protocol);

	/*Параметр domain задает домен соединения: выбирает набор протоколов, которые будут использоваться для создания соединения.
	Такие наборы описаны в <sys / socket.h>.
	В настоящее время распознаются такие форматы :
	AF_UNIX, AF_LOCAL   Local communication              unix(7)
	AF_INET             IPv4 Internet protocols(4 байта) ip(7) -------------- мы его используем
	AF_INET6            IPv6 Internet protocols          ipv6(7)*/


	/* Сокет имеет тип type, задающий семантику коммуникации. В настоящее время определены следующие типы:

	SOCK_STREAM     Обеспечивает создание двусторонних надежных и последовательных потоков байтов , поддерживающих соединения.
	Может также поддерживаться механизм внепоточных данных.
	//Протоколы связи, которые реализуют SOCK_STREAM, следят, чтобы данные не были потеряны или дублированы.
	//Если часть данных, для которых имеется место в буфере протокола, не может быть передана за определённое время, соединение считается разорванным.


	SOCK_DGRAM      Поддерживает датаграммы (ненадежные сообщения с ограниченной длиной и не поддерживающие соединения).
	Датаграмма - блок информации, передаваемый протоколом без предварительного установления соединения и создания виртуального канала.

	SOCK_SEQPACKET  Обеспечивает работу последовательного двустороннего канала для передачи датаграмм с поддержкой соединений;
	датаграммы имеют ограниченную длину; от получателя требуется за один раз прочитать целый пакет.	*/

	/*Параметр protocol задает конкретный протокол, который работает с сокетом.
	Обычно существует только один протокол, задающий конкретный тип сокета в определенном семействе протоколов, в этом случае protocol может быть определено, как 0.
	Однако, возможно существование нескольких таких протоколов(в этом случае и используется данный параметр).*/

	//tcp_socket = socket(AF_INET, SOCK_STREAM, 0);  
	//udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	//raw_socket = socket(AF_INET, SOCK_RAW, protocol);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		perror("socket"), exit(1);


	//Структура sockaddr_in описывает сокет для работы с протоколами IP.
	//Значение поля sin_family всегда равно AF_INET.
	//Поле sin_port содержит номер порта который намерен занять процесс.Если значение этого поля равно нулю, то операционная система сама выделит свободный номер порта для сокета.
	//Поле sin_addr содержит IP адрес к которому будет привязан сокет.

	//struct sockaddr_in {
	//	short            sin_family;   // e.g. AF_INET					//Тип адреса (должно быть AF_INET ).
	//	unsigned short   sin_port;     // e.g. htons(3490)				//Порт IP-адресов.
	//	struct in_addr   sin_addr;										//IP-адрес.
	//};

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//htons - преобразует  порядок расположения байтов (от хоста) положительного короткого целого hostshort в сетевой порядок (TCP/IP) расположения байтов.

	struct hostent *h;
	//struct hostent
	//{
	//	char * h_name;				// имя хоста
	//	char ** h_aliases;			// дополнительные названия
	//	short h_addrtype;			// тип адреса
	//	short h_length;				// длинна каждого адреса в байтах
	//	char **h_addr_list;			// список адресов
	//};

	if ((h = gethostbyname(host)) == NULL)
		herror("gethostbyname"), exit(1);
	//gethostbyname - Получает информацию о хосте по его имени. Результат работы помещается в специальную структуру hostent
	//В эту функцию надо передать имя хоста.Если функция выполнится неудачно или с ошибкой, то вернется NULL.
	//Иначе указатель на структуру.Вы не должные изменять эту структуру.
	//Структура hostent используется функциями, чтобы хранить информацию о хосте : его имя, тип, IP адрес, и т.д.
	//Вы никогда не должны пытаться изменять эту структуру или освобождать любой из компонентов.
	//Кроме того, только одна копия структуры hostent должна быть связана с потоком.

	memcpy(&addr.sin_addr, h->h_addr, h->h_length);

	//int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);

	/*Файловый дескриптор sockfd должен ссылаться на сокет.
	Если сокет имеет тип SOCK_STREAM или SOCK_SEQPACKET, то данный системный вызов попытается установить соединение с другим сокетом.
	Другой сокет задан параметром serv_addr, являющийся адресом длиной addrelen в пространстве коммуникации сокета.
	Каждое пространство коммуникации интерпретирует параметр serv_addr по - своему.*/
	if (connect(sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
		perror("connect"), exit(1);

	while (1)
	{
		//структура для хранения набора сокетов(точнее их ФД)
		//fd_set{
		//	u_int  fd_count;
		//	SOCKET fd_array[FD_SETSIZE];
		//}

		//FD_ZERO() очищает набор;
		fd_set readfds;
		FD_ZERO(&readfds);
		//FD_SET() добавляет заданный файловый дескриптор к набору
		FD_SET(0, &readfds);
		FD_SET(sockfd, &readfds);
		//Вызовы select() и pselect() позволяют программам отслеживать изменения нескольких файловых дескрипторов ожидая,
		//когда один или более файловых дескрипторов станут «готовы» для операции ввода-вывода определённого типа (например, ввода).
		//Файловый дескриптор считается готовым,
		//если к нему возможно применить соответствующую операцию ввода-вывода (например, read(2)) без блокировки или очень маленький write(2)).

		//int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
		//Отслеживаются 3 независимых набора файловых дескрипторов.
		//В тех, что перечислены в readfds, будет отслеживаться появление символов, доступных для чтения
		//(говоря более точно, проверяется доступность чтения без блокировки; в частности, файловый дескриптор готов для чтения, если он указывает на конец файла)
		//Значение nfds на единицу больше самого большого номера файлового дескриптора из всех трёх наборов.
		if (select(sockfd + 1, &readfds, NULL, NULL, NULL) == -1)
			perror("select"), exit(1);

		//FD_ISSET() проверяет, является ли файловый дескриптор частью набора
		if (FD_ISSET(sockfd, &readfds))
		{
			int len = read_from_and_write_to(sockfd, 0);
			if (len == 0)
				break;
		}
		if (FD_ISSET(0, &readfds))
			read_from_and_write_to(0, sockfd);
	}

	return 0;
}
