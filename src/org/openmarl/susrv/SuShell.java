/*
 * libsusrv: Android SU native client library.
 *
 * <t0kt0ckus@gmail.com>
 * (C) 2014
 *
 * License: GPLv3
 *
 */
package org.openmarl.susrv;

import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Simple client to a native SU shell session.
 *
 */
public class SuShell {


    public static final int SU_SRV_CMD_FAILED = -999;

    private final String mBaseDir;
    private final Context mContext;

    private SuShell(Context ctx) {
        mContext = ctx;
        mBaseDir = mContext.getFilesDir().getPath();

        if (! initPrivateRootFilesystem()) {
            throw new IllegalStateException(
                    "Filesystem initialization error, try to see logcat");
        }
        if (initSuSrv(mBaseDir) != 0) {
            throw new IllegalStateException(
                    "SuShell initialization error, try to see var/log/libsusrv.log");
        }
    }

    /**
     * Opens an SU shell session, creating it if needed.
     *
     * <p>This is a blocking call, to be wrapped into an asynchronous initializer.</p>
     *
     * @param ctx Any valid context.
     *
     * @return An initialized SU shell.
     *
     * @throws java.lang.IllegalStateException when an initialization error occurs.
     */
    public static SuShell getInstance(Context ctx) {
        if (_instance == null) {
            _instance = new SuShell(ctx);
        }
        return _instance;
    }

    /**
     * Quick accessor to the current SU shell session.
     *
     * @return The current SU shell.
     *
     * @throws java.lang.IllegalStateException w

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Simple client to a native SU shell session.
 *hen the session is uninitialized.
     */
    public static SuShell getInstance() {
        if (_instance == null) {
            throw new IllegalStateException("The SU shell session is uninitialized");
        }

        return _instance;
    }

    /**
     * Executes a shell command on the current session.
     *
     * @param command The command string.
     *
     * @return The result code value, as given by <code>$?</code>, or <code>SU_SRV_CMD_FAILED</code>
     * when an error occurs.
     */
    public native int exec(String command);

    /**
     * Exits the current shell session.
     *
     * <p>Any further attempt to use <code>exec()</code> will fail with an
     * <code>IllegalStateException</code></p>.
     *
     * <p>Should be called for eg. on <code>Activity.onDestroy()</code>.</p>
     *
     */
    public void exit()
    {
        exitSuSrv();
        _instance = null;
    }

    /**
     * Imports an asset into the session private filesystem.
     *
     * @param rawAssetId The identifier of a binary asset in <code>res/raw</code>.
     * @param dirPath The destination path relative to the private filesystem.
     * @param filename The destination filename.
     * @param isExec Whether the destination file should be executable.
     *
     * @return The size in bytes of the imported asset, or <code>0</code> if an error's occurred.
     */
    public int importAsset(int rawAssetId, String dirPath, String filename, boolean isExec) {
        byte buf[] = new byte[64];
            int iBytes = 0;

        try {
            initPrivateDir(dirPath);
            File privateFile = new File(String.format("%s/%s/%s", mBaseDir, dirPath, filename));
            InputStream is = mContext.getResources().openRawResource(rawAssetId);
            OutputStream os = new FileOutputStream(privateFile);

            int red;
            while((red = is.read(buf)) > 0) {
                os.write(buf, 0, red);
                iBytes += red;
            }
            is.close();
            os.close();

            if (isExec) {
                privateFile.setExecutable(true, true);
            }

            Log.i(TAG,
                    String.format("Imported %d bytes asset as %s/%s (%s)",
                            iBytes,
                            dirPath, filename,
                            privateFile.canExecute() ? "rwx" : "rw-"));
        }
        catch(IOException e ) {
            Log.e(TAG,
                    String.format("Failed to import asset <%d> to %s/%s: %s", rawAssetId, dirPath,
                            filename, e.toString()));
        }
        catch (Resources.NotFoundException e) {
            Log.e(TAG,
                    String.format("Failed to import asset <%d> to %s/%s: %s", rawAssetId, dirPath,
                            filename, e.toString()));
        }

        return iBytes;
    }

    /**
     * Updates the shell session environment to include this private filesystem.
     *
     * <p>The private <code>bin</code> and <code>lib</code> directories are added to
     * <code>PATH</code>, and <code>LD_LIBRARY_PATH</code> (resp.).</p>
     */
    public void updateEnvironment() {
        // update PATH
        String newPATH = String.format("%s/bin:$PATH", mBaseDir);
        if (exec(String.format("export PATH=%s", newPATH)) == 0) {
            Log.i(TAG, String.format("updated PATH: %s", newPATH));
        }
        else {
            Log.e(TAG, String.format("Failed to update PATH to %s", newPATH));
        }
        // update LD_LIBRARY_PATH
        String newLD_PATH = String.format("%s/lib", mBaseDir);
        if (exec(String.format("export LD_LIBRARY_PATH=%s", newLD_PATH)) == 0) {
            Log.i(TAG, String.format("updated LD_LIBRARY_PATH: %s", newLD_PATH));
        }
        else {
            Log.e(TAG, String.format("Failed to update LD_LIBRARY_PATH to %s", newLD_PATH));
        }
    }

    /**
     * Sets the current working directory to the application private filesystem root.
     */
    public void cd() {
        if (exec(String.format("cd %s", mBaseDir)) == 0) {
            Log.i(TAG, String.format("updated CWD: %s", mBaseDir));
        }
        else {
            Log.e(TAG, String.format("Failed to cd to %s", mBaseDir));
        }
        ;
    }

    private native int initSuSrv(String cwd);
    private native void exitSuSrv();

    private boolean initPrivateRootFilesystem(){
        try {
            // create dirs
            initPrivateDir("var/log");
            initPrivateDir("var/run");
            initPrivateDir("bin");
            initPrivateDir("lib");
            Log.i(TAG,
                    String.format("Initialized private filesystem: %s", this.mBaseDir));
        }
        catch(IllegalArgumentException e) {
            Log.e(TAG, String.format("Failed to initialize local fs: %s", e.toString()));
            return false;
        }

        return true;
    }

    private void initPrivateDir(String privatePath) {
        String dirPath = String.format("%s/%s", this.mBaseDir, privatePath);
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

    static {
        System.loadLibrary("susrv");
    }

    private static SuShell _instance;
    private static final String TAG = "SuShell";
}

// Logcat
/*
10-09 11:14:46.309  18008-18021/org.openmarl.susrv D/dalvikvm﹕ Trying to load lib /data/app-lib/org.openmarl.susrv-1/libsusrv.so 0x4290cea8
10-09 11:14:46.310  18008-18021/org.openmarl.susrv D/dalvikvm﹕ Added shared lib /data/app-lib/org.openmarl.susrv-1/libsusrv.so 0x4290cea8
10-09 11:14:46.310  18008-18021/org.openmarl.susrv D/dalvikvm﹕ No JNI_OnLoad found in /data/app-lib/org.openmarl.susrv-1/libsusrv.so 0x4290cea8, skipping init
10-09 11:14:46.317  18008-18021/org.openmarl.susrv I/SuShell﹕ Initialized private filesystem: /data/data/org.openmarl.susrv/files
10-09 11:14:46.394  18008-18008/org.openmarl.susrv I/SuShellAsyncInit﹕ SU Shell session started successfully
10-09 11:14:50.010  18008-18008/org.openmarl.susrv I/SuShell﹕ updated CWD: /data/data/org.openmarl.susrv/files
10-09 11:14:50.012  18008-18008/org.openmarl.susrv I/SuShell﹕ updated PATH: /data/data/org.openmarl.susrv/files/bin:$PATH
10-09 11:14:50.013  18008-18008/org.openmarl.susrv I/SuShell﹕ updated LD_LIBRARY_PATH: /data/data/org.openmarl.susrv/files/lib
10-09 11:14:50.039  18008-18008/org.openmarl.susrv I/SuShell﹕ Imported 21972 bytes asset as bin/hijack (rwx)
*/

// libsusrv.log
/*
[su_srv] Initializing libsusrv ...
[su_srv] Initialized AF_UNIX/SOCK_STREAM socket: /data/data/org.openmarl.susrv/files/var/run/susrv_sock
[su_srv] Using su binary: /system/xbin/su
[su_srv] SU shell PID: 18022
[su_srv] SU shell session initialization's complete
[su_srv] ------------------------------------------
# id
uid=0(root) gid=0(root) context=u:r:init:s0
# pwd
/
# cd /data/data/org.openmarl.susrv/files
# pwd
/data/data/org.openmarl.susrv/files
# export PATH=/data/data/org.openmarl.susrv/files/bin:$PATH
# export LD_LIBRARY_PATH=/data/data/org.openmarl.susrv/files/lib
# ls bin
hijack
# exit
[su_srv] Disconnected SU shell process
[su_srv] SU shell process terminated
*/