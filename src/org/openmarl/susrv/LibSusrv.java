/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

/**
 * Represents the direct native <code>libsusrv.so</code> API exported through JNI.
 *
 * <p>This API is specified by <code>jni/su_srv.h</code>.
 * </p>
 *
 * @author t0kt0ckus
 */
public class LibSusrv {

    /**
     * Denotes an undetermined system level error.
     */
    public static final int SYS_ERR = -9999;

    /**
     * Denotes a situation where a SU Shell session is already bound to the requesting process,
     * and no additional session can be opened. This may not be an actual error.
     */
    public static final int SESSION_EXISTS_ERR = -9998;

    /**
     * Denotes a situation when one attempts to execute a command string though no SU Shell session
     * is currently bound to the requesting process.
     */
    public static final int NO_SESSION_ERR = -9997;

    /**
     * Denotes a situation when a command string execution aborted abnormally, before providing
     * its result code.
     */
    public static final int INVALID_RESULT_ERR = -9996;

    /**
     * Opens the SU shell session bound to the requesting process. If no session's currently bound,
     * a new one is initialized.
     *
     * <p>On success, the created shell is bound to the requesting process until
     * {@link #exitShellSession()} is called, or the shell process abnormally dies.
     * </p>
     *
     * <p>This API si specified by the C function <code>su_srv_open_shell_sesssion()</code>.</p>
     *
     * @param pfsRoot The private filesystem root absolute path.
     *
     * @return <code>0</code> on success, {@link #SESSION_EXISTS_ERR}, {@link #SYS_ERR}, or a
     * C/POSIX error code on error.
     */
    public static native int openShellSession(String pfsRoot);

    /**
     * Executes a command within the shell session currently bound to the requesting process.
     *
     * <p>This API si specified by the C function <code>su_srv_exec()</code>.</p>
     *
     * @param cmd The command string to execute.
     *
     * @return {@link #NO_SESSION_ERR}, {@link #INVALID_RESULT_ERR}, or the (should be non-negative,
     * or at least > -1) command exit code.
     */
    public static native int exec(String cmd);

    /**
     * Answers whether the requesting process is currently bound to a shell session.
     *
     * <p>This API si specified by the C function <code>su_srv_ping()</code>.</p>
     *
     * @return A non-<code>0</code> value when the requesting process is currently bound to a
     * valid shell session.
     */
    public static native int ping();

    /**
     * Enables/disables echo-ing the session "terminal" output to the log file.
     *
     * @param echo A non-zero value to enable echo.
     */
    public static native void setTtyEcho(int echo);

    /**
     * Tells whether echo is enabled on current session.
     *
     * @return <code>0</code> if echo is disabled, a positive value if enabled, and a negative
     * value otherwise.
     */
    public static native int getTtyEcho();

    /**
     * Answers the last line read on the current session "terminal" output.
     *
     * @return The las shell output line, or <code>null</code> when not available.
     */
    public static native String getLastTtyRead();

    /**
     * Answers the path to the shell's session log file.
     *
     * @return An absolute path, or <code>null</code> when not available.
     */
    public static native String getTtyPath();

    /**
     * Answers the PID of the first process which name matches a search term.
     *
     * <p>This API si specified by the C function <code>su_srv_getpid()</code>.</p>
     *
     * @param procname The process name to search for, for example <code>com.android.phone</code>.
     *
     * @return A non-negative process PID (should also be > 0), or a negative number when no
     * process found with the given name.
     */
    public static native int getpid(String procname);

    /**
     * Exits the shell session currently bound to the requiring process.
     *
     * <p>This releases all associated native resources (process, thread, socket, ...).
     * Any further attempt to call {@link #exec(String)} will obviously fail.</p>
     *
     * <p>This API si specified by the C function <code>su_srv_exit_shell_sesssion()</code>.</p>
     *
     * @return <ocde>0</ocde>0 success, {@link #NO_SESSION_ERR}, {@link #SYS_ERR}, or a C/POSIX
     * error code when failed to properly close the current session.
     */
    public static native int exitShellSession();

    /**
     *
     * @return
     */
    public static native String[] getproclist();

    static {
        System.loadLibrary("susrv");
    }

    private LibSusrv() {}
}

