#include "unp.h"

void getfile(const char* filename, size_t *size){
    ifstream file( filename, ios::binary );
    if(!file){
        //printf("File not found!!\n");
        *size = -1;
        return;
    }

    streambuf * pbuf = file.rdbuf();
    *size = pbuf->pubseekoff(0, file.end);
    file.close();
}

void readfile(const char* filename, char *content)
{
    ifstream file( filename, ios::binary );
    if(!file){
        //printf("File not found!!\n");
        return;
    }

    streambuf * pbuf = file.rdbuf();
    streamsize size = pbuf->pubseekoff(0, file.end);
    pbuf->pubseekoff(0, file.beg); 

    //content = (char*)malloc(size+1);
    //content[size] = 0;

    pbuf->sgetn(content, size);
    file.close();
    //cout.write (content, size);
    //printf("%s\n", content);
}

/*bool check_dir(const char *path){
    DIR *dir = opendir(path);
    if(dir == NULL){
        return false;
    }

    dirent *entry = readdir(dir);

    if(entry->d_type == DT_DIR){
        return true;
    }

    if(entry->d_type == DT_UNKNOWN){
        return false;
    }
}*/