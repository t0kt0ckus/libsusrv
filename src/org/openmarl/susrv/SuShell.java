/*
 * libsusrv: Android SU native client library.
 *
 * Chris Dufaza
 * <t0kt0ckus@gmail.com>
 *
 * (C) 2014
 *
 * License: GPLv3
 *
 */
package org.openmarl.susrv;

import android.content.Context;
import android.util.Log;

import java.io.File;

/** Client session to the device installed SU shell.
 *
 */
public class SuShell {

    static {
        System.loadLibrary("susrv");
    }

    private static SuShell _instance;

    public static final int CMD_FAILED = -999;

    /**
     * Shortcut to access an already initialized SU shell session.
     *
     * @return
     */
    public static SuShell getInstance() {
        if (_instance == null) {
            throw new IllegalStateException("The SU shell session has not been initialized");
        }

        return _instance;
    }

    /**
     * Answers an initialized SU shell session.
     *
     * <p>This is a blocking call.</p>
     *
     * @param ctx
     *
     * @return
     *
     * @throws java.lang.IllegalStateException when fails to properly initialize the session.
     */
    public static SuShell getInstance(Context ctx) {
        if (_instance == null) {
            _instance = new SuShell(ctx.getFilesDir().getPath());
        }
        return _instance;
    }

    /**
     * Executes a shell command on the current session.
     *
     * @param command The command string.
     * @return The result code value, as given by <code>$?</code>, or <code>CMD_FAILED</code>
     * when an error occurs.
     */
    public native int exec(String command);

    /**
     * Exits the current shell session.
     *
     */
    public void exit() {
        close();
        _instance = null;
    }

    private native int open(String cwd);
    private native void close();

    private final String baseDir;

    private SuShell(String baseDir) {
        this.baseDir = baseDir;
        if (initFs() != 0) {
            throw new IllegalStateException(
                    "Filesystem initialization error, try to see logcat");
        }
        if (open(this.baseDir) != 0) {
            throw new IllegalStateException(
                    "SuShell initialization error, try to see var/log/libsusrv.log");
        }
    }

    private int initFs(){
        // init directories
        try {
            initPrivateDir("var/log");
            initPrivateDir("var/run");
            initPrivateDir("bin");
            initPrivateDir("lib");
            Log.d(TAG,
                    String.format("Initialized private file system: %s", this.baseDir));
        }
        catch(IllegalArgumentException e) {
            Log.d(TAG, String.format("Failed to initialize local fs: %s", e.toString()));
            return -1;
        }

        return 0;
    }

    private void initPrivateDir(String privatePath) {
        String dirPath = String.format("%s/%s", this.baseDir, privatePath);
        File dirFile = new File(dirPath);
        dirFile.mkdirs();

        if ( !(dirFile.exists()
                && dirFile.setReadable(true, true)
                && dirFile.setWritable(true, true)) ) {
            // THIS SHOULD NOT HAPPEN
            throw new IllegalArgumentException(
                    String.format("Failed to initialize private directory: %s",
                            dirFile.getAbsolutePath()));
        }
    }

    private static final String TAG = "SuShell";
}
