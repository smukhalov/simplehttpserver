#include "unp.h"

extern void process_request(int, const char*);

ssize_t sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd)
{
    ssize_t     size;

    if (fd) {
        //printf("sock_fd_read - fd - %d\n", *fd);

        struct msghdr   msg;
        struct iovec    iov;
        union {
            struct cmsghdr  cmsghdr;
            char        control[CMSG_SPACE(sizeof (int))];
        } cmsgu;
        struct cmsghdr  *cmsg;

        iov.iov_base = buf;
        iov.iov_len = bufsize;

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);
        size = recvmsg (sock, &msg, 0);
        if (size < 0) {
            perror ("recvmsg");
            return 1;
        }
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf (stderr, "invalid cmsg_level %d\n", cmsg->cmsg_level);
                return 1;
            }
            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf (stderr, "invalid cmsg_type %d\n", cmsg->cmsg_type);
                return 1;
            }

            *fd = *((int *) CMSG_DATA(cmsg));
            //printf ("received fd %d\n", *fd);
        } else
            *fd = -1;
    } else {
        printf("sock_fd_read - if (fd) == false\n");
        size = recv (sock, buf, bufsize, MSG_NOSIGNAL);
        if (size < 0) {
            perror("read");
            return 1;
        }
    }

    //printf("sock_fd_read - size - %ld\n", size);
    return size;
}

ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd)
{
    ssize_t     size;
    struct msghdr   msg;
    struct iovec    iov;
    union {
        struct cmsghdr  cmsghdr;
        char        control[CMSG_SPACE(sizeof (int))];
    } cmsgu;
    struct cmsghdr  *cmsg;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    if (fd != -1) {
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_len = CMSG_LEN(sizeof (int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        //printf ("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        //printf ("not passing fd\n");
    }

    size = sendmsg(sock, &msg, MSG_NOSIGNAL);

    if (size < 0){
        perror ("sendmsg");
    }

    return size;
}

void child(int sock, const char *root_dir)
{
    int fd;
    char    buf[16];
    ssize_t size;

    //printf("child sock - %d, pid - %d \n", sock, getpid());

    //sleep(1);
    //for (;;) {
        size = sock_fd_read(sock, buf, sizeof(buf), &fd);
        if (size <= 0){
            //printf("child break size - %ld\n", size);
            return;
        }

        if (fd != -1) {
            //buf[size] = 0;
            //printf("buf - %s\n", buf);

            process_request(fd, root_dir);
            close(fd);
        }
    //}

    //printf("child end\n");
}

void parent(int sock, int connfd)
{
    char *c = new char;
    *c = 'q';

    /*char filename[] = "/home/sergey/fdpassing.txt";
    int fd = open(filename, O_WRONLY | O_CREAT, 0666);
    if(fd == -1){
        std::cerr << "Не удалось создать файл /home/sergey/fdpassing.txt" << std::endl;
        exit(1);
    }*/

    sock_fd_write(sock, c, 1, connfd);
    //printf ("wrote %ld\n", size);

    delete c;
}