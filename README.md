# VascoDaGammaRay

Minimal tool to help debugging Qt applications when GammaRay can't be used


## Usage:

- socat

  `echo -n "print_windows" | socat - UNIX-CONNECT:/tmp/testvasco-IpcPipe`

- nc

  `echo -n "print_windows" | nc -U /tmp/testvasco-IpcPipe`
