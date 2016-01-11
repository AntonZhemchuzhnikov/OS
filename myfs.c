
// More info
// https://www.ibm.com/developerworks/ru/library/l-fuse/
// http://sar.informatik.hu-berlin.de/teaching/2013-w/2013w_osp2/lab/Lab-4-FUSE/lab-FUSE_.pdf
// https://www.cs.hmc.edu/~geoff/classes/hmc.cs135.201001/homework/fuse/fuse_doc.html
// http://www.cs.cmu.edu/~./fp/courses/15213-s07/lectures/15-filesys/index.html

#define FUSE_USE_VERSION  26
#defint NAME_LENGTH 20

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


//static void *_init(struct fuse_conn_info * conn);
//static int _release(const char *path, struct fuse_file_info *fi);
static int _getattr(const char *path, struct stat *stbuf);
static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int _open(const char *path, struct fuse_file_info *fi);
static int _read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info * fi);
static void _destroy(void *a);
static int _truncate(const char *path, off_t size);
static int _create(const char *path, mode_t mode, struct fuse_file_info *fi);
static int _write(const char *path, const char *content, size_t size, off_t offset, struct fuse_file_info *fi);
static int _mkdir(const char *path, mode_t mode, struct fuse_file_info *fi); 

static struct fuse_operations oper = {
    .readdir = _readdir,
    .create = _create,
    .open = _open,
    .read = _read,
    .write = _write,
    .truncate = _truncate,
    .flush = NULL,
    .getattr = _getattr,
    .destroy = _destroy,
    .mkdir 	= _mkdir
     //.init = _init,
    //.release = _release,
};

// структура данных
typedef struct data_header {
	char name[NAME_LENGTH];
    int start; // индекс начального сектора
	int size; // количество секторов
} data_header;

int main(int argc, char *argv[]) {
    
    
    return fuse_main(argc, argv, &oper, NULL);
}

/*static void *_init(struct fuse_conn_info * conn) {
    printf("Filesystem has been initialized!\n");
    return NULL;
}

static int _release(const char *path, struct fuse_file_info * fi) {
    printf("release: %s\n", path);
    return 0;
}*/

static void _destroy(void *a) {
    /*
      Close filesystem 
      ...
    */
    printf("Filesystem has been destroyed!\n");
}

static int _getattr(const char *path, struct stat * stbuf) {
    printf("getattr: %s\n", path);
    /*
      Get file info
      ...
    */

    memset(stbuf, 0, sizeof (struct stat));
    stbuf->st_mode = /*mode*/
    stbuf->st_nlink = /*count of links*/
    stbuf->st_size = /*file size*/

    return 0;
}

static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info * fi) {
    /*
      Get list of files in directory
      ...
    */

    // foreach file
    filler(buf, fileName, NULL, 0); // add filename in directory *path* to buffer

    return 0;
}

static int _open(const char *path, struct fuse_file_info * fi) {
    printf("open: %s\n", path);

    /*
      Get list of files in directory
      ...
    */
    
    fileInfo->fileInode = // set inode;
    fileInfo->fullfileName = // specify file name;
    fileInfo->isLocked = false; // set is file locked

    

    printf("open: Opened successfully\n");

    return 0;
}


static int _read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info * fi) {
    printf("read: %s\n", path);
    /*
      Read file
      ...
      byte *fileContent = readFile(fileInode);
      memcpy(buf, fileContent, size);
    */  
    return size;
}

static int _truncate(const char *path, off_t size) {
    /*
      truncate file
      ...
    */

    printf("truncate: Truncated successfully\n");
    return 0;
}

static int _create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    printf("create: %s\n", path);
    /*
      Create inode
      ...
    */

    return 0;
}

static int _write(const char *path, const char *content, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("write: %s\n", path);
    /*
      Write bytes to fs
      ...
    */
    return 0; // Num of bytes written
}
