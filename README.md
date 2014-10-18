libsusrv
=

Android SU native client library.

This native library provides interactive shell sessions on any Android system where a suitable SU application is installed. Its consist of the shared library (`libsusrv.so`) for any considered architecture, and a class (`org.openmarl.susrv.LibSusrv`) that represents the JNI-exported API.

A more friendly *session level* API is provided by `org.openmarl.susrv.SuShell`.

This library should:
- permit to execute any Shell *command strings*, whether *Simple Commands*, *Pipelines*, *Lists* or *Compound Commands*, as specified by the relevant man page
- permit to execute any Shell *command strings*, that would produce an arbitrary length output on a controlling terminal (this is actually limited by the available disk space since this output is written to a log file)
- permit access to the exit code returned by the *command string*
- permit access to what would be the output produced on a controlling terminal
- execute the *command strings* in the order they are submitted
- prevent unneeded process/thread creation and cleanup, allowing the blocking API `LibSusrv.exec()` call latency to depend only upon the *command string* execution time

**Disclaimer:** There's a well known library, full Java, that may offer more features and be more flexible: Chainfire's [libsuperuser](https://github.com/Chainfire/libsuperuser). I've written this simple trick because I feel it may better suit my present and future needs, by focusing on a very few simple aspects. I also admit that the choice of a native library may not provide any sensible performance benefit in most situations, though in some rare ones a few implementation's details could produce a more acceptable behavior. 
 

Requirements
======

The requirements should be obvious:
- A rooted Android device, with an installed SU application, that is a application that provides a `su` binary that spawn priviledged shell sessions. The library will search into the following directories: `/sbin`, `/system/sbin`, `/system/bin`, and `/system/xbin`.
- The native implementation relies upon stable bionic C API, and should apply to most of Android OS versions.  

Overview
======

According to Chainfire's [How-To SU](http://su.chainfire.eu/), in order to support most of wildely adopted SU applications, and to avoid spawning an ephemeral `su` process upon each command string execution, we should fork a single `su` shell, to which we'll write command strings, and from which we'll read what would be the output produced on a controlling terminal.

The actual workflow is give bellow:
1. a process requests an SU shell session's initialization
2. `libsusrv` creates the appropriate `su` process, connected to a local UNIX socket
3. `libsusrv` creates a native handler thread, to read what would be the output produced on a controlling terminal
4. this shell session is now bound to the requesting process
4. when an `SuShell.exec()` call occurs on the requesting process, the *main* thread writes the command string to the socket, and blocks ...
5. ... until the handler thread delivers the command exite code back to the main thread
6. when either the `su` process dies, or `SuShell.exit()` is called, the session is destroyed the best we can to avoid *zombie* sockets, processes, threads, or mutexes ;-)

A SU shell session opens two files:
- `<app dir>/var/log/su_session-<pid>.log`: guess what ...
- `<app dir>/var/run/su_session-<pid>`: which is the AF UNIX address `sun_path` of the rendez-vous socket, that should be unlinked as soon as the client peer connection to the shell process is established
 

License
======
```
This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
```
