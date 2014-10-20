libsusrv
=

A simple native Android SU client library.

This library provides priviledged shell sessions on any Android system where a suitable SU application is installed.

This library:
- forks a single SU Shell child process per Android application, with which it then communicates through a local UNIX socket 
- permits to execute Shell *command strings*, where a *command string* may be a *Simple Command*, a *Pipeline*, a *List* or a *Compound Command*, as specified by the relevant man page
- permits access to the exit code returned by the command string 
- permits access to what would be the output produced on a controlling terminal
- executes the command strings in the order they are submitted
- is accessible from both native C and Java code

**Note:** This primarily comprises a native shared library (`libsusrv.so`) for any considered architecture, and the definition of a Java object (`org.openmarl.susrv.LibSusrv`) that publishes the JNI-exported API. These components are standard POSIX/JNI stuff and may be used in a non Android context.

**Disclaimer:** Great documentation and examples, targeted to developers that need to integrate priviledged commands execution from an Android application, are maintained by Chainfire at [libsuperuser](https://github.com/Chainfire/libsuperuser). The `libsuperuser` library available there may offer more features and be more flexible than this one. I've written this simple trick because I feel it better suits my present requirements, and I'm now just sharing some code. I also admit that the choice of a native library may not provide any sensible performance benefit in most situations, this wasn't a motivation here.

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
- A rooted Android device, with an installed SU application, that is an application that provides a `su` binary that can spawn priviledged shell sessions. The library will search into the following directories: `/sbin`, `/system/sbin`, `/system/bin`, and `/system/xbin`, in that order.
- The native implementation relies upon POSIX C APIs, implemented in bionic, and should apply to most of Android OS versions.

The library seems to work fine with Chainfire's [SuperSU](http://www.chainfire.eu/projects/52/SuperSU/) on Android KitKat 4.x.  


Overview
===

According to Chainfire's [How-To SU](http://su.chainfire.eu/), in order to support most of the wildely adopted SU applications, and to avoid spawning an ephemeral `su` process upon each command string execution, we may choose to fork a single shell, to which we'll write command strings, and from which we'll read back what would be the output produced on a controlling terminal.

This is implemented through native POSIX primitives, and as such, a Shell session comprises:
- a running `su` child process
- a local UNIX socket to communicate with
- a thread that consumes and interpret the socket I/O
- mutexes to synchronize things
 
And the API is very simple:
- and `init()` call, that initializes a session as described above
- an `exec()` call, that permits to execute command strings, blocking untill the command exit code is available
- an `exit()` call, that terminates the shell process and releases the session resources

Though the library's implementation allows client code to use threads to *queue* command strings, preserving order of submit, keep in mind that there's a unique shell child process per Android application to execute these commands.

(Technicaly, one child SU shell may exist per process that loads the `libsusrv.so` shared library.)

A SU shell session opens two files:
- `<app-dir>/var/log/su_session-<pid>.log`: contains some debug information and what would be the output produced on a controlling terminal
- `<app-dir>/var/run/su_session-<pid>`: which is the UNIX address `sun_path` of the rendez-vous socket, that will be unlinked as soon as the client peer connection to the shell process is established or denied, so it should be ephemeral

where `<app-dir>` is the root of the embedding application private filesystem as answered by `android.content.Context.getFilesDir().getPath()`, usually `/data/data/<application-package>/files`.


Build
===

Obvisouly, both Android SDK and NDK must be installed, and the `ANDROID_SDK` and `ANDROID_NDK` environment variables propertly set.

To build a library archive for all architectures:
```
$ git clone https://github.com/t0kt0ckus/libsusrv.git
$ cd libsusrv
$ ./make.sh 
[armeabi-v7a] Compile thumb  : susrv <= su_srv.c
[armeabi-v7a] Compile thumb  : susrv <= su_srv_jni.c
[armeabi-v7a] Compile thumb  : susrv <= su_srv_log.c
[armeabi-v7a] Compile thumb  : susrv <= su_srv_pfs.c
[armeabi-v7a] Compile thumb  : susrv <= su_shell_session.c
[armeabi-v7a] SharedLibrary  : libsusrv.so
[armeabi-v7a] Install        : libsusrv.so => libs/armeabi-v7a/libsusrv.so
[armeabi] Compile thumb  : susrv <= su_srv.c
[armeabi] Compile thumb  : susrv <= su_srv_jni.c
[armeabi] Compile thumb  : susrv <= su_srv_log.c
[armeabi] Compile thumb  : susrv <= su_srv_pfs.c
[armeabi] Compile thumb  : susrv <= su_shell_session.c
[armeabi] SharedLibrary  : libsusrv.so
[armeabi] Install        : libsusrv.so => libs/armeabi/libsusrv.so
[x86] Compile        : susrv <= su_srv.c
[x86] Compile        : susrv <= su_srv_jni.c
[x86] Compile        : susrv <= su_srv_log.c
[x86] Compile        : susrv <= su_srv_pfs.c
[x86] Compile        : susrv <= su_shell_session.c
[x86] SharedLibrary  : libsusrv.so
[x86] Install        : libsusrv.so => libs/x86/libsusrv.so
[mips] Compile        : susrv <= su_srv.c
[mips] Compile        : susrv <= su_srv_jni.c
[mips] Compile        : susrv <= su_srv_log.c
[mips] Compile        : susrv <= su_srv_pfs.c
[mips] Compile        : susrv <= su_shell_session.c
[mips] SharedLibrary  : libsusrv.so
[mips] Install        : libsusrv.so => libs/mips/libsusrv.so
  adding: org/openmarl/susrv/SuShell.class (deflated 47%)
  adding: org/openmarl/susrv/LibSusrv.class (deflated 37%)
  adding: org/openmarl/susrv/SuShellAsyncInit.class (deflated 49%)
  adding: org/openmarl/susrv/SuShellAsyncObserver.class (deflated 25%)
  adding: org/openmarl/susrv/SuSrvException.class (deflated 43%)
  adding: lib/armeabi/libsusrv.so (deflated 58%)
  adding: lib/armeabi-v7a/libsusrv.so (deflated 58%)
  adding: lib/mips/libsusrv.so (deflated 91%)
  adding: lib/x86/libsusrv.so (deflated 63%)
Multi-arch library archive: /marl/git/t0kt0ckus/libsusrv/dist/libsusrv.jar  
```

The produced  `dist/libsusrv.jar` contains:
```
// The native shared libraries
//
+ lib/armeabi/libsusrv.so
+ lib/armeabi-v7a/libsusrv.so
+ lib/mips/libsusrv.so
+ lib/x86/libsusrv.so

// The JNI object that publishes the native API
//
+ org/openmarl/susrv/LibSusrv.class

// Some Android specific more friendly API
//
+ org/openmarl/susrv/SuShell.class
+ org/openmarl/susrv/SuShellAsyncInit.class
+ org/openmarl/susrv/SuShellAsyncObserver.class
+ org/openmarl/susrv/NoShellSessionError.class
```

This script also generates the Java API documentation to the `dist/api` directory. And `dist/include` will show the relevant C headers. 

As the actual build configuration is defined through `Android.mk` and `Applciation.mk`, one may also invoke directly the NDK tools as she uses to.
A standard `Makefile` is also given, that help building the shared library to target a non Android context.

Developer's guide
===

We'll detail bellow the Android application developer's point of view. 

**Installation**

To make both the native shared library and the Java API available, one have to:
- build a suitable JAR archive, for example using the provided `make.sh` script
- copy `libsusrv.jar` to the `libs` directory of the Android application (this actual location depends upon the development tools chain)

The Java API package is `org.openmarl.susrv`.

**Session initialization**

Initialization is initiated by a call to the `SuShell.getInstance(context)` static method, which return a `SuShell` instance on success. The native SU shell session represented by this object is then bound to the requesting process.

Keeping a reference to this instance may be a good idea, though any further call to `SuShell.getInstance(null)` will immediately returns the currently bound SU shell session, without any overhead.

Initialization should not occur on the main thread, as the SU shell child process creation may take some time. Instead, have an activity that implement `SuShellAsyncObserver`, and starts an `SuShellAsyncInit` async task from some appropriate point of its life-cycle.

**Executing commands**

Any valid *command string* may be executed using the blocking API call:
```java
int rval =  SuShell.getInstance(null).exec(cmd);
```
where `rval` is the integer exit code of the command execution sub-process, as returned by `echo $?`. 

If the shell session has not been initialized or has been invalidated, a `NoShellSessionError` exception is thrown.

Command strings execution may be queued by wrapping `exec()` calls within distinct threads: commands will then execute in the order they have been submitted, and each thread will have the appropriate return value delivered.

**Session termination**

A session is invalidated either normaly when `SuShell.exit()` is called, or when the child shell process terminates abnormally. In both situations, one can safely initiate a new session by calling `SuShell.getInstance(context)` again.


Example
===

This example is available in the `org.openmarl.susrv.example` package.


We consider an activity like bellow:

```java
public class SomeActivity extends Activity implements SuShellAsyncObserver {

    private SuShell mSuShell;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);

        new SuShellAsyncInit(this).execute();
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mSuShell != null) {
            mSuShell.exit();
            mSuShell = null;
        }
    }
    
    @Override
    public void onShellInitComplete(SuShell suShell) {
        mSuShell = suShell;

        if (mSuShell != null) {
            Log.d(TAG, "SU shell session successfully initialized");
            try {
                int rval = mSuShell.exec("id"); // rval will have the value 0
            }
            catch(NoShellSessionError e) {
                mSuShell = null;
                Log.w(TAG, "SU shell session has just been invalidated, no luck");
            }
        }
        else
            Log.e(TAG, "Failed to initialize SU shell session");
    }
}
```

This will produce the log file bellow:
```
root@falcon_umts:/ # cat /data/data/org.openmarl.susrvexample/files/var/log/su_session-10729.log
[su_srv] Initializing SU shell session:
[su_srv] owner PID: 10729
[su_srv] AF UNIX path: /data/data/org.openmarl.susrvexample/files/var/run/su_session-10729
[su_srv] AF UNIX rendez-vous complete
[su_srv] Found system SU binary: /system/xbin/su
[su_srv] Created SU shell child process (PID: 10753)
[su_srv] SU shell session initialization complete
# id
uid=0(root) gid=0(root) context=u:r:init:s0
```
