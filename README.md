# VascoDaGammaRay

Minimal tool to help debug Qt applications when GammaRay can't be used.

[Vasco da Gama](https://en.wikipedia.org/wiki/Vasco_da_Gama) was a Portuguese explorer who didn't have much technology but still managed to find his way.

## Build

`cmake --preset=rel && cmake --build build-rel`

## Usage:

You can either link to `libvascodagammaray.so` or preload it:

`LD_PRELOAD=<path/to>/libvascodagammaray.so <myapp>`

Then communicate via a socket:

- socat

  `echo -n "print_windows" | socat - UNIX-CONNECT:/tmp/testvasco-IpcPipe`

- nc

  `echo -n "print_windows" | nc -U /tmp/testvasco-IpcPipe`

## Available Commands
  - `quit`
  - `print_windows`
  - `hide_windows`
  - `set_persistent_windows_false`
  - `print_info`
  - `track_window_events`
  