#include <stdio.h> // Standard input/output library
#include <arpa/inet.h> // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h> // For the close function
#include <string.h> // For the memset function
#include <stdlib.h> // For sprintf function
#include <netinet/tcp.h> // for rune or cubic

/*
 * @brief The buffer size to store the received message.
 * @note The default buffer size is 1024.
*/
#define BUFFER_SIZE 1024

/*
 * @brief The buffer size to store the file path.
 * @note The default buffer size path is 1024.
*/
#define FPATH_SIZE 1024

/*
 * @brief send file to server function.
 * @param None
 * @return 0 if the file transferred successfully, 1 otherwise.
*/
int send_file(int sock, char* buffer_fpath) {
	int bytes_sent = 0;
    char buffer[BUFFER_SIZE]; // Create a buffer to store data.
	memset(buffer, 0, BUFFER_SIZE);

	FILE* file = fopen(buffer_fpath, "rb");
	if (file == NULL) {
		perror("Failed to open file");
		exit(1);
	}

	fprintf(stdout, "Sending file to the server...\n");

    // Try to send the file to the server using the socket.
	while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
		if ((bytes_sent = send(sock, buffer, sizeof(buffer), 0)) < 0) {
			perror("Error sending file data\n");
        	close(sock);
			fclose(file);
        	exit(1);
		}
	}
	
	// Sending EOF to indicate the end of file transfer.
	buffer[0] = EOF; // EOF is -1
	if ((bytes_sent = send(sock, buffer, 1, 0)) < 0) {
		perror("Error sending EOF data\n");
        close(sock);
		fclose(file);
        exit(1);
	}

    fprintf(stdout, "The file was transferred succesfully to the server.\n");

	return 0;
}

/*
 * @brief checking arguments from user function.
 * @param argc - the amount of arguments from the user.
 * @param argv - the array that stores the arguments.
 * @return 0 if the arguments are correct, 1 otherwise.
*/
int check_args(int argc, char *argv[]) {
	
	int error_flag = 0;

	if (argc != 7)
        error_flag = 1;
	if (strncmp(argv[1], "-ip", 3) != 0)
        error_flag = 1;
	if (strncmp(argv[3], "-p", 2) != 0)
        error_flag = 1;
	if (strncmp(argv[5], "-algo", 5) != 0)
        error_flag = 1;
	if (strncmp(argv[6], "reno", 4) != 0 && strncmp(argv[6], "cubic", 5) != 0)
        error_flag = 1;
	
	if (error_flag) {
		fprintf(stderr,"Usage: ./TCP_Sender -ip [IP] -p [PORT] -algo [reno/cubic]\n");
		exit(1);
	}

	return 0;
}

/*
 * @brief TCP Client main function.
 * @param argc - the amount of arguments from the user.
 * @param argv - the array that stores the arguments.
 * @return 0 if the client runs successfully, 1 otherwise.
*/
int main(int argc, char *argv[]) {

	check_args(argc, argv);
	char* server_ip = argv[2];
	unsigned int server_port = (unsigned int)atoi(argv[4]);

	/* ----- setp 1: Read the created file. ----- */

	// Getting the file path from the user.
	char buffer_fpath[FPATH_SIZE];
	int len = 0;
	memset(buffer_fpath, 0, FPATH_SIZE);
	printf("Please enter the file path: ");
	memset(buffer_fpath, 0, FPATH_SIZE);
	if (fgets(buffer_fpath, FPATH_SIZE - 1, stdin) == NULL) {
		perror("Error reading filename\n");
		return 1;
	}

	len = strlen(buffer_fpath);
	if (len > 0 && buffer_fpath[len - 1] == '\n')
	    buffer_fpath[len - 1] = '\0';
	
	// Openig the file.
    FILE *file = fopen(buffer_fpath, "rb");

    if (file == NULL) {
        perror("Error opening file\n");
        return 1;
    }

	// Checking if file is at least 2 MB.
	unsigned long filesize = 0;
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
	if (filesize < 2 * 1024 * 1024) {
		perror("Error: file size needs to be at least 2 MB\n");
		return 1;
	}
	rewind(file);
    fclose(file);
	
	/* ----- setp 2: Create a TCP socket between the Sender and the Receiver. ----- */

    // The variable to store the socket file descriptor.
    int sock = -1;

    // The variable to store the server's address.
    struct sockaddr_in server;

    // Reset the server structure to zeros.
    memset(&server, 0, sizeof(server));

    // Try to create a TCP socket (IPv4, stream-based, default protocol).
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // If the socket creation failed, print an error message and return 1.
    if (sock == -1)
    {
        perror("Error in creating TCP socket\n");
        return 1;
    }

	// Set the congestion control algorithm to Reno or Cubic.
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, argv[6], strlen(argv[6])) < 0) {
        perror("Could not set TCP congestion control to reno or cubic\n");
        close(sock);
        return 1;
    }

    // Convert the server's address from text to binary form and store it in the server structure.
    // This should not fail if the address is valid (e.g. "127.0.0.1").
    if (inet_pton(AF_INET, server_ip, &server.sin_addr) <= 0)
    {
        perror("Error in converting the server's address from text to binary\n");
        close(sock);
        return 1;
    }

    // Set the server's address family to AF_INET (IPv4).
    server.sin_family = AF_INET;

    // Set the server's port to the defined port. Note that the port must be in network byte order,
    // so we first convert it to network byte order using the htons function.
    server.sin_port = htons(server_port);

    fprintf(stdout, "Connecting to %s:%i...\n", server_ip, server_port);

    // Try to connect to the server using the socket and the server structure.
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Error when connecting to the server\n");
        close(sock);
        return 1;
    }

    fprintf(stdout, "Successfully connected to the server!\n");

	/* ----- setp 3: Sending file to the server. ----- */

	send_file(sock, buffer_fpath);

	/* ----- Step 4: User decision: Send the file again? ----- */

	char ch = '\0';
	
	while(1) {
		fprintf(stdout, "Send the file again?[Y/N].\n");
		ch = getc(stdin);
		if (ch == 'Y' || ch == 'y') {
			send_file(sock, buffer_fpath);
			continue;
		}
		if (ch == 'N' || ch == 'n') {
			break;
		}
		printf("Please enter 'Y' or 'N'\n");
	}

	/* ----- Step 5: Send an exit message to the receiver. ----- */

	if (send(sock, "EXIT", 4, 0) < 0) {
		perror("Error send exit message.\n");
        close(sock);
		return 1;
	}

	/* ----- Step 6: Close the TCP connection. ----- */

    close(sock);

    fprintf(stdout, "Connection closed!\n");


	/* ----- Step 7: Exit. ----- */	

	fprintf(stdout, "TCP_Sender program ran successfully.\nExiting TCP_Sender program...\n");

    return 0;
}
