#include "common.h"
#include "fs.h"
// #define DEFAULT_ENTRY ((void *)0x4000000)

#define DEFAULT_ENTRY ((void *)0x4000000)

//注意函数声明
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();

extern int fs_open(const char *pathname, int flags, int mode);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);
extern size_t fs_filesz(int fd);

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

#define RAMDISK_SIZE ((&ramdisk_end)-(&ramdisk_start))

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  // assert(filename);
  Log("The image is %s",filename);
  int fd=fs_open(filename,0,0);
  Log("filename=%s,fd=%d",filename,fd);
  fs_read(fd,(void*)DEFAULT_ENTRY,fs_filesz(fd));
  fs_close(fd);

  //ramdisk_read(DEFAULT_ENTRY,0,RAMDISK_SIZE);
  return (uintptr_t)DEFAULT_ENTRY;
}
