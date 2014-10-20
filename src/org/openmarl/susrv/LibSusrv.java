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
 * @author t0kt0ckus
 */
public class LibSusrv {

    public static final int SYS_ERR = -9999;
    public static final int PFS_ERR = -9998;
    public static final int SESSION_EXISTS_ERR = -9997;
    public static final int NO_SESSION_ERR = -9996;
    public static final int CMD_FAILED_ERR = -9994;

    /**
     * Opens a new SU shell session.
     *
     * On success, the created shell is bound to the requiring process until
     * {@link #exitShellSession()} is called, or the shell process abnormally dies.
     *
     * @param pfsRoot The private filesystem root absolute path.
     *
     * @return <code>0</code> on success, {@link #SESSION_EXISTS_ERR} when a session is already bound
     * to the requiring process, {@link #PFS_ERR} when failed to initialize
     * private filesystem, or {@link #SYS_ERR} when an OS level error occurred during the
     * session initialization.
     */
    public static native int openShellSession(String pfsRoot);

    /**
     * Executes a command within the shell session currently bound to the requiring process.
     *
     * @param cmd The command to execute.
     *
     * @return The command exit code on success, {@link #NO_SESSION_ERR} when no session
     * is currently bound to the requiring process, or {@link #CMD_FAILED_ERR} when the command
     * has been abnormally aborted and the result code is unknown.
     */
    public static native int exec(String cmd);

    /**
     * Answers whether the requesting process is currently bound to a shell session.
     *
     * @return A non-<code>0</code> value when the requesting process is currently not bound to any
     * shell session.
     */
    public static native int stat();

    /**
     * Exits the shell session currently bound to the requiring process.
     *
     * This release all associated native resources (process, thread, socket, ...).
     * Any further attempt to call {@link #exec(String)} will obviously fail.
     *
     * @return <ocde>0</ocde>0 success, {@link #NO_SESSION_ERR} when no session
     * is currently bound to the requesting process, or another non-<code>0</code> value when
     * an OS level error occurred while closing the session.
     */
    public static native int exitShellSession();

    static {
        System.loadLibrary("susrv");
    }
}

