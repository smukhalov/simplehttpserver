#include "unp.h"

extern void daemon_init();
extern void parent(int, int);
extern void child(int, const char*);
extern bool check_dir(const char*);

int set_nonblock(int socketfd)
{
    int flags;
#if defined(O_NONBLOCK)
    flags = fcntl(socketfd, F_GETFL, 0);
    if (flags == -1)
    {
        std::cerr << "fcntl failed (F_GETFL)\n";
        return 0;
    }

    flags |= O_NONBLOCK;
    int s = fcntl(socketfd, F_SETFL, flags);
    if (s == -1)
    {
        std::cerr << "fcntl failed (F_SETFL)\n";
        return 0;
    }

    return 1;

#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

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

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        std::cerr << "setsockopt\n";
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

    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    //printf("Before for\n");
	for ( ; ; ) {
		connfd = accept(listenfd, NULL, NULL);
        if(connfd < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        set_nonblock(connfd);
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

