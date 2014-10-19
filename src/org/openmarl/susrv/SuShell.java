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
 * Simple client to a native SU shell session.
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
     * @param ctx A context.
     *
     * @return An shell session ready to accept commands.
     *
     * @throws SuSrvException When session initialization fails.
     */
    public static SuShell getInstance(Context ctx) throws SuSrvException {
        if (_instance == null) {
            _instance = new SuShell(ctx);
            _instance.initNativeSession();
        }
        return _instance;
    }

    private void initNativeSession() throws SuSrvException {
        int opened = LibSusrv.openShellSession(mBaseDir);

        switch(opened) {
            case LibSusrv.SU_SRV_PFS_ERR:
                throw new SuSrvException("Failed to initialize (native) private filesystem");
            case LibSusrv.SU_SRV_SYS_ERR:
                throw new SuSrvException("Failed to initialize (native) SU shell session");
            case LibSusrv.SU_SRV_SESSION_EXISTS_ERR:
                return; // we keep happy with that session
            case 0:
                return; // created session
            default:
                throw new SuSrvException("Unexpected error during libsusrv initialization");
        }
    }

    /**
     * Executes a shell command.
     *
     * @param command The command string.
     *
     * @return The return value specified by {@link org.openmarl.susrv.LibSusrv#exec(String)}.
     *
     * @throws SuSrvException When no shell session available
     * (try {@link #getInstance(android.content.Context)}.
     */
    public int exec(String command) throws SuSrvException {
        if (LibSusrv.isReady() != 0) {
            return LibSusrv.exec(command);
        }
        else {
            _instance = null;
            throw new SuSrvException("No shell session bound to requiring process");
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
     * <code>PATH</code> (resp.), and <code>LD_LIBRARY_PATH</code>.</p>
     */
    public void updateEnvironment() {
        // update PATH
        String newPATH = String.format("%s/bin:$PATH", mBaseDir);
        if (LibSusrv.exec(String.format("export PATH=%s", newPATH)) == 0) {
            Log.i(TAG, String.format("updated PATH: %s", newPATH));
        }
        else {
            Log.e(TAG, String.format("Failed to update PATH to %s", newPATH));
        }
        // update LD_LIBRARY_PATH
        String newLD_PATH = String.format("%s/lib", mBaseDir);
        if (LibSusrv.exec(String.format("export LD_LIBRARY_PATH=%s", newLD_PATH)) == 0) {
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
        if (LibSusrv.exec(String.format("cd %s", mBaseDir)) == 0) {
            Log.i(TAG, String.format("updated CWD: %s", mBaseDir));
        }
        else {
            Log.e(TAG, String.format("Failed to cd to %s", mBaseDir));
        }
        ;
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