#include "unp.h"

extern void daemon_init();
extern void parent(int, int);
extern void child(int, const char*);
extern bool check_dir(const char*);

/*void echo(int sockfd){
	ssize_t n;
	char buf[MAXLINE];

	for(;;){
		n = read(sockfd, buf, sizeof(buf));
		if(n == 0){
			printf("EOF\n");
			return;
		}
		else if(n < 0){
			printf("Ошибка\n");
			return;
		}

		write(sockfd, buf, n);
	}
}*/

int main(int argc, char **argv){
    static const char *optString = "h:d:p:";

    int					listenfd, connfd;
	struct sockaddr_in	servaddr;
	char				buff[MAXLINE];
	//time_t				ticks;

    //const char *ipaddress = "127.0.0.1";
    int port; // = 12345;

    //const char *test_url = "/home/sergey/tmp/";

    int opt = 0;
    string str_ipaddress, str_dir;
     
    opt = getopt( argc, argv, optString );
    while( opt != -1 ) {
        switch( opt ) {
            case 'h':
                str_ipaddress.append(optarg);
                //printf("h - %s\n", optarg);
                break;
                 
            case 'p':
                port = atoi(optarg);
                //printf("p - %s\n", optarg);
                break;
                 
            case 'd':
                str_dir.append(optarg);
                if(str_dir[str_dir.size()-1] == '/'){
                    str_dir = str_dir.substr(0, str_dir.size()-2);
                    //str_dir.pop_back();
                }

                //printf("d - %s\n", optarg);
                break;
                 
            default:
                /* сюда попасть невозможно. */
                break;
        }
         
        opt = getopt( argc, argv, optString );
    }

    /*char *root_dir = (char*)malloc(17);
    bzero(root_dir, 17);

    strcpy(root_dir, test_url);
    if( *(root_dir + strlen(root_dir) - 1) == '/' ){
        *(root_dir + strlen(root_dir) - 1) = 0;
    }

    if(!check_dir(root_dir)){
        printf("Папка %s не существует\n", root_dir);
        return EXIT_FAILURE;
    }*/

    if(inet_pton(AF_INET, str_ipaddress.c_str(), &servaddr.sin_addr) <= 0){
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

	daemon_init();

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd <= 0){
        perror("socket create failure");
        exit(EXIT_FAILURE);
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);	

    int enabled = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) < 0){
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

	if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }

	if(listen(listenfd, SOMAXCONN) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    signal(SIGHUP, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(CLONE_CHILD_CLEARTID, SIG_IGN);
    signal(CLONE_CHILD_SETTID, SIG_IGN);
    //printf("Before for\n");
	for ( ; ; ) {
		connfd = accept(listenfd, NULL, NULL);
        if(connfd < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        //printf("connfd - %d\n", connfd);

        int sv[2];
        int pid;

        if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0) {
            perror("socketpair");
            exit(1);
        }

        switch ((pid = fork())) {
        case 0:
            //printf("Fork создан - %d\n", getpid());
            close(listenfd);
            close(sv[0]);
            close(connfd);

            child(sv[1], str_dir.c_str());
            
            close(sv[1]);
            //printf("Fork завершен - %d\n", getpid());
            exit(0);
            //break;
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        default:
            close(sv[1]);

            parent(sv[0], connfd);
            close(connfd);
            close(sv[0]);
            //break;
        }
	}
	return 0;
}