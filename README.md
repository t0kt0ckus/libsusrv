libsusrv
=

An Android native SU client library.

This native library provides priviledged shell sessions on any Android system where a suitable SU application is installed. It consists of the shared library (`libsusrv.so`) for any considered architecture, and a class (`org.openmarl.susrv.LibSusrv`) that represents the JNI-exported API.

A more friendly *session level* API is provided by `org.openmarl.susrv.SuShell`.

This library should:
- permit to execute any Shell *command string*, whether it is a *Simple Command*, a *Pipeline*, a *List* or a *Compound Command*, as specified by the relevant man page
- permit to execute any Shell *command string*, that would produce an arbitrary length output on a controlling terminal (this is actually limited by the available disk space since this output is written to a log file)
- permit access to the exit code returned by the *command string*
- permit access to what would be the output produced on a controlling terminal
- execute the *command strings* in the order they are submitted
- prevent unneeded process/thread creation and cleanup, allowing the blocking API `LibSusrv.exec()` call latency to depend only upon the *command string* execution time
- be accessible from native code (see `jni/su_srv.h`)
 
**Disclaimer:** Great documentation and examples, targeted to developers that need to integrate priviledged commands execution from an Android application, are maintained by Chainfire at [libsuperuser](https://github.com/Chainfire/libsuperuser). The available `libsuperuser` library may offer more features and be more flexible that this one. I've written this simple trick because I feel it may better suit my present and future needs, by focusing on a very few simple aspects. I also admit that the choice of a native library may not provide any sensible performance benefit in most situations, though in some rare ones implementation's details may produce more or less acceptable behaviors. 
 
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
- The native implementation relies upon stable bionic C API, and should apply to most of Android OS versions.

The library seems to work fine with Chainfire's [SuperSU](http://www.chainfire.eu/projects/52/SuperSU/) on Android KitKat 4.x.  

Overview
===

According to Chainfire's [How-To SU](http://su.chainfire.eu/), in order to support most of the wildely adopted SU applications, and to avoid spawning an ephemeral `su` process upon each command string execution, we should fork a single shell, to which we'll write command strings, and from which we'll read what would be the output produced on a controlling terminal.

The actual workflow is give bellow:

1. a process requests an SU shell session's initialization
2. `libsusrv` creates the appropriate a `su` process, connected to a local UNIX socket
3. `libsusrv` creates a native handler thread, to read what would be the output produced on a controlling terminal
4. this shell session is now bound to the requesting process
5. whenever an `SuShell.exec()` call occurs on the requesting process, the calling thread writes the command string to the local socket, and blocks ...
6. untill the handler thread delivers the command exit code back to the appropriate thread
7. when either the `su` process dies, or `SuShell.exit()` is called, the session is destroyed the best we can to avoid *zombie* sockets, processes, threads, or mutexes ;-)

A SU shell session opens two files:
- `<app dir>/var/log/su_session-<pid>.log`: guess what ...
- `<app dir>/var/run/su_session-<pid>`: which is the AF UNIX address `sun_path` of the rendez-vous socket, that should be unlinked as soon as the client peer connection to the shell process is established

where `<app dir>` is the root of the embedding application private filesystem as answered by `android.content.Context.getFilesDir().getPath()`, usually `/data/data/<application package>/files`.

