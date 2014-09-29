/* Non-Canonical Input Processing */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int main(int argc, char** argv) {
  int fd, res;
  struct termios oldtio, newtio;
  char buf[255];

  if (argc < 2 || (strcmp("/dev/ttyS0", argv[1]) != 0 && strcmp("/dev/ttyS1", argv[1])!=0 )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0) {
      perror(argv[1]);
      exit(-1);
    }

  /* save current port settings */
    if (tcgetattr(fd, &oldtio) == -1) {
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
  newtio.c_cc[VTIME] = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    int i = 0;
  while (STOP == FALSE) {       /* loop for input */
    read(fd, buf + i, 1);   /* returns after 5 chars have been input */
    if (buf[i] == 0)
      STOP = TRUE;

    buf[++i] = 0;               /* so we can printf... */
    printf("received string so far: %s\n", buf);
  }

  printf("result: %s\n", buf);
  write(fd, buf, strlen(buf)+1);

  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
