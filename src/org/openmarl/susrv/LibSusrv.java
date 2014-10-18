/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

/**
 * Represents the native <code>libsusrv.so</code> API available via JNI.
 *
 */
public class LibSusrv {

    public static final int SU_SRV_SYS_ERR = -9999;
    public static final int SU_SRV_PFS_ERR = -9998;
    public static final int SU_SRV_SESSION_EXISTS_ERR = -9997;
    public static final int SU_SRV_NO_SESSION_ERR = -9996;

    public static final int SU_SRV_EXEC_ERR = -9994;

    /**
     * Opens a new SU shell session.
     *
     * On success, the created shell is bound to the requiring process until
     * {@link #exitShellSession()} is called, or when the shell process abnormally dies.
     *
     * @param pfsRoot The private filesystem root absolute path.
     *
     * @return <code>0</code> on success, <code>SU_SRV_PFS_ERR</code> when a session is already bound
     * to the requiring process, <code>SU_SRV_PFS_ERR</code> when failed to initialize
     * private filesystem, or <code>SU_SRV_SYS_ERR</code> when a system error occured during
     * native session initialization.
     */
    public static native int openShellSession(String pfsRoot);

    /**
     * Executes a command within the shell session currently bound to the requiring process.
     *
     * @param cmd The command to execute. Can be any valid shell command string.
     *
     * @return The command exit code on success, <code>SU_SRV_NO_SESSION_ERR</code> when no session
     * is currently bound to the requiring process, or <code>SU_SRV_EXEC_ERR</code> when the command
     * has been abnormally aborted.
     */
    public static native int exec(String cmd);

    /**
     * Exits the shell session currently bound to the requiring process.
     *
     * Any further attempt to call {@link #exec(String)} will be rejected.
     *
     * @return 0 on succes.
     */
    public static native int exitShellSession();

    /**
     * Answers whether a shell session is currently bound to the requiring process.
     *
     * @return A non-zero value when this session is ready to accept commands.
     */
    public static native int isReady();

    static {
        System.loadLibrary("susrv");
    }
}

