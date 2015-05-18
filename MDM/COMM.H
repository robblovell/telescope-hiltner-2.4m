#ifndef SerialIOhh
#define SerialIOhh
#include <sys/z8530.h>

#include <iostream.h>
#include <sys/stropts.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctype.h>
#include <fcntl.h>
#include <termio.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include "device.h"
#include <stdio.h>


/*
extern "C" {
  extern void setraw ( int fd, struct termio *rsold, struct termio *rsnew);
  extern void serial_cleanup ();
  extern void init_serial ();
};
*/
class SerialIO {
 public:
  struct termio old_fd, new_fd;
  int thebaud;
  int rs422on;
  int fd;
  char buf[256];
  int len;

  SerialIO () {
    thebaud = 19200;
    rs422on = 1;
/* REL
    if ((fd = open("/dev/ttyf1", O_NOCTTY | O_NDELAY | O_RDWR, 0666)) == EOF) {
        perror("open");
    }
    setraw (fd,&old_fd,&new_fd);
*/
  }
  void setup () {
    if ((fd = open("/dev/ttyf1", O_NOCTTY | O_NDELAY | O_RDWR, 0666)) == EOF) {
        perror("Comm::setup:open");
        exit (0);
    }
    setraw (fd,&old_fd,&new_fd);
  }
  ~SerialIO () {
/* REL
    serial_cleanup ();
*/
  }

  void setraw ( int fd, struct termio *rsold, struct termio *rsnew);
  void serial_cleanup ()
  {
    if (ioctl(fd, TCSETAF, &old_fd) == -1) {
        perror("ioctl");
        exit(6);
    }
  }

  void pack3 (char byte0,char byte1,char byte3) {
    buf[0] = byte0; buf[1] = byte1; buf[2] = byte3; buf[3] = '\0';
    len = 3;
  }
  int receive () {
    if ((len = read(fd, buf, 256)) != NULL) {
      return 1;
    }
    else {
      strcpy (buf,"");
      len = 0; 
      return 0;
    }
  }
  int receive (char *(&buff)) {
    if ((len = read(fd, buf, 256)) != NULL) {
      buff = buf;
      return 1;
    }
    else {
      strcpy (buf,"");
      buff = buf;
      len = 0; 
      return 0;
    }
  }
  int send () {
    int err = write (fd,buf,len);
    cerr <<"send:"<<len<<"  |"<<buf<<"|"<<"  "<<buf[0]<<"  "<<buf[1]<<"  "<<buf[2]<<endl;
    if (err <0) {
      perror("SerialIO::send():");
      exit (0);
    }
    return err;
  }
  int send (char *buff) {
    int err = write (fd,buff,strlen (buff));
    cerr <<"send:"<<buff<<endl;
    if (err <0) {
      perror("SerialIO::send(char *buff):");
      exit (0);
    }
    return err;
  }
  friend ostream &operator << (ostream& os, SerialIO& p) {
    os <<"Serial IO::";
    for (int i=0;i<p.len;i++) {
      os <<(int)p.buf[i]<<" ";
    }
    return os;
  }
};
#endif
