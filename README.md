libsusrv
=

A simple native Android SU client library.

This native library provides priviledged shell sessions on any Android system where a suitable SU application is installed. It consists of the shared library (`libsusrv.so`) for any considered architecture, and the `org.openmarl.susrv.LibSusrv` class that publish the JNI-exported API.

This library:
- forks a single SU Shell child process per Android application, with which it then communicates through a local UNIX socket - if the shell process terminates abnormaly, the session is properly destroyed, and a new one can be initialized
- permits to execute any Shell *command string*, whether it is a *Simple Command*, a *Pipeline*, a *List* or a *Compound Command*, as specified by the relevant man page
- permits access to the exit code returned by the *command string*
- permits access to what would be the output produced on a controlling terminal
- executes the *command strings* in the order they are submitted
- is accessible from both native C and Java code

**Disclaimer:** Great documentation and examples, targeted to developers that need to integrate priviledged commands execution from an Android application, are maintained by Chainfire at [libsuperuser](https://github.com/Chainfire/libsuperuser). The available `libsuperuser` library may offer more features and be more flexible than this one. I've written this simple trick because I feel it better suits my present requirements, and I'm now just sharing some code. I also admit that the choice of a native library may not provide any sensible performance benefit in most situations, this wasn't a motivation here.

**License**

```
t0kt0ckus@gmail.com
(C) 2014

This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
```

Requirements
===

Requirements are obvious:
- A rooted Android device, with an installed SU application, that is an application that provides a `su` binary that can spawn priviledged shell sessions. The library will search into the following directories: `/sbin`, `/system/sbin`, `/system/bin`, and `/system/xbin`.
- The native implementation relies upon stable bionic C APIs, and should apply to most of Android OS versions.

The library seems to work fine with Chainfire's [SuperSU](http://www.chainfire.eu/projects/52/SuperSU/) on Android KitKat 4.x.  

Overview
===

According to Chainfire's [How-To SU](http://su.chainfire.eu/), in order to support most of the wildely adopted SU applications, and to avoid spawning an ephemeral `su` process upon each command string execution, we may choose to fork a single shell, to which we'll write command strings, and from which we'll read back what would be the output produced on a controlling terminal.

This is implemented through native POSIX primitives, and as such, a Shell session comprises:
- a running `su` child process
- a local UNIX socket to communicate with
- a thread that consumes and interpret the socket output
- mutexes to synchronize things
 
And the API is very simple:
- and `init()` call, that initializes a session as described above
- an `exec()` call, that permits to execute command strings, blocking untill the command exit code is available
- an `exit()` call, that terminates the shell process and releases the session resources

Though the library's implementation allows client code to use threads to *queue* commands to be executed, preserving order of submit, keep in mind that there's a unique shell child process per Android application to execute these commands.

(Technicaly, one child SU shell may exist per process that loads the `libsusrv.so` shared library.)

A SU shell session opens two files:
- `<app dir>/var/log/su_session-<pid>.log`: contains some debug information and what would be the output produced on a controlling terminal
- `<app dir>/var/run/su_session-<pid>`: which is the AF UNIX address `sun_path` of the rendez-vous socket, that will be unlinked as soon as the client peer connection to the shell process is established or closed, so it should be an ephemeral file

where `<app dir>` is the root of the embedding application private filesystem as answered by `android.content.Context.getFilesDir().getPath()`, usually `/data/data/<application package>/files`.


Build
===



Developer's guide (native C)
===

Developer's guide (Java)
===

