/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
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
 * Simple client to a native SU Shell session.
 *
 * @author t0kt0ckus
 */
public class SuShell {

    private final String mBaseDir;
    private final Context mContext;

    private SuShell(Context ctx) {
        mContext = ctx;
        mBaseDir = mContext.getFilesDir().getPath();
        initPrivateRootFilesystem();
    }

    /**
     * Accessor to the SU shell session currently bound to the requiring process.
     *
     * <p>A new session is created if needed.</p>
     *
     * @param ctx A context.
     *
     * @return A shell session ready to accept commands, or <code>null</code> when a suitable
     * session does not exist and couldn't be created.
     */
    public static SuShell getInstance(Context ctx) {
        if (_instance == null) {
            _instance = new SuShell(ctx);
            if (! _instance.init())
                _instance = null;
        }
        return _instance;
    }
    private boolean init() {
        int rval = LibSusrv.openShellSession(mBaseDir);
        return (rval == 0) || (rval == LibSusrv.SESSION_EXISTS_ERR);
    }

    /**
     * Answers whether a valid SU shell session is currently bound to the requesting process.
     *
     * @return <code>true</code> when a session exists, accepting commands.
     */
    public boolean isReady() {
        return (LibSusrv.stat() == 0);
    }

    /**
     * Executes a shell command within the SU shell session currently bound to the requesting process.

     * @param command The command string to execute.
     *
     * @return The command exit code.
     *
     * @throws NoShellSessionError When no valid shell session is bound to the requesting process.
     * This may happen consequently to a call to {@link #exit()} or because the associated native
     * shell process has died abnormally.
     */
    public int exec(String command) throws NoShellSessionError {
        int rval = LibSusrv.exec(command);

        if (rval != LibSusrv.NO_SESSION_ERR)
            return rval;
        else {
            _instance = null;
            throw new NoShellSessionError();
        }
    }

    /**
     * Exits the current shell session.
     */
    public void exit()
    {
        LibSusrv.exitShellSession();
        _instance = null;
    }

    private void initPrivateRootFilesystem(){
        initPrivateDir("var/log");
        initPrivateDir("var/run");
        initPrivateDir("bin");
        initPrivateDir("lib");
    }

    private void initPrivateDir(String privatePath) {
        String dirPath = String.format("%s/%s", this.mBaseDir, privatePath);
        File dirFile = new File(dirPath);
        dirFile.mkdirs();
        dirFile.exists();
        dirFile.setReadable(true, true);
        dirFile.setWritable(true, true);
    }


    private static SuShell _instance;
    private static final String TAG = SuShell.class.getSimpleName();
}