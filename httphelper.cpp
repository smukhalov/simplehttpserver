#include "unp.h"

extern vector<string> split(const string& str, const char& ch);
extern void readfile(const char *, char *);
extern void getfile(const char*, size_t *);
extern std::string str_tolower(std::string) ;

static const char* template200 = "HTTP/1.0 200 OK\r\n"
		           "Content-length: %ld\r\n"
		       	   "Connection: close\r\n"
		       	   "Content-Type: text/html\r\n"
		       	   "\r\n"
		       	   "%s";

static const char* template200_image_jpeg = "HTTP/1.0 200 OK\r\n"
		           "Content-length: %ld\r\n"
		       	   "Connection: close\r\n"
		       	   "Content-Type: image/jpeg\r\n"
		       	   "\r\n"
		       	   "%s";

static const char template404[] = "HTTP/1.0 404 NOT FOUND\r\n"
                    "Content-length: 0\r\n"
		       	    "Connection: close\r\n"
                    "Content-Type: text/html\r\n"
                    "\r\n";

const char filenotfound[] = "File not found";

void process_request(int fd, const char *root_dir){
    const char question_mark = '?';
    char bufrequest[4096];
    char *bufresponse;

//printf("before process_request - pid - %d\n", getpid());
    ssize_t nrequest = recv(fd, bufrequest, sizeof(bufrequest), MSG_NOSIGNAL | MSG_DONTWAIT);
    
    //printf("after process_request - %ld, pid - %d\n", nrequest, getpid());
    if(nrequest == 0){ //EOF
        return;
    }
    
    if(nrequest < 0){
        //perror("process_request read");
        return;
    }

    bufrequest[nrequest] = 0;
    //printf("buf - %s", bufrequest);

    char *protocol;
    char *url;

    vector<string> request_parts = split(bufrequest, '\n');
    for (vector<string>::const_iterator it = request_parts.begin(); it != request_parts.end(); it++) {
    //for(auto s: request_parts){
        int i = 0;
        vector<string> protocol_parts = request_parts = split(*it, ' ');
        for (vector<string>::const_iterator it1 = protocol_parts.begin(); it != protocol_parts.end(); it1++) {
        //for(auto p: protocol_parts){
            string p = *it1;
            unsigned long size = strlen(p.c_str())+1;
            if(i == 0){
                protocol = (char*)malloc(size);

                memset(protocol, 0, size);
                strncpy(protocol, p.c_str(), size);
            }
            else if(i == 1){
                if(p == "/"){
                    //printf("1p - %s\n", p.c_str());
                    p.append("index.html");
                    size = strlen(p.c_str())+1;
                    //printf("2p - %s\n", p.c_str());
                }

                url = (char*)malloc(size);

                memset(url, 0, size);
                strncpy(url, p.c_str(), size);
                for(int i = 0; i < size; i++){
                    if(question_mark == *(url + i)){
                        *(url + i) = 0;
                        break;
                    } 
                }
            }
            else{
                break;
            }
            i++;
        }
        break;
    }

    if(strcmp(protocol, "GET") != 0){
        //printf("Протокол %s не поддерживается", protocol);
        return;
    }

    size_t url_size = strlen(root_dir) + strlen(url) + 1;
    char *full_path = (char*)malloc(url_size);
    memset(full_path, 0, url_size);

    strcat(full_path, root_dir);
    strcat(full_path, url);

    //printf("full_path - %s\n", full_path);

    string std_filename(full_path);
    //string ext = str_tolower(std_filename.substr(std_filename.find_last_of(".") + 1));
    string ext = std_filename.substr(std_filename.find_last_of(".") + 1);
    //printf("ext - %s\n", ext.c_str());

    size_t size = 0;
    getfile(full_path, &size);

    if(size == -1 || ext != "html"){
        //printf("1Not found - %s\n", full_path);

        //bufresponse = (char*)malloc(strlen(template404) + sizeof(filenotfound) + 1);
        bufresponse = (char*)malloc(strlen(template404) + 1);

        ssize_t nresponse = sprintf(bufresponse, template404);
        //ssize_t nresponse = sprintf(bufresponse, template404, sizeof(filenotfound)-1, filenotfound);
        bufresponse[nresponse] = 0;

        send(fd, bufresponse, nresponse, MSG_NOSIGNAL);
    }
    else {
        char *content = (char*)malloc(size+1);
        content[size] = 0;

        readfile(full_path, content);

        bufresponse = (char*)malloc(sizeof(template200) + size + 1);

        ssize_t nresponse = sprintf(bufresponse, template200, size, content);
        bufresponse[nresponse] = 0;

        send(fd, bufresponse, nresponse, MSG_NOSIGNAL);
    }

    free(full_path);
    free(protocol);
    free(url);
    free(bufresponse);
}
