/*
    SuSrv: A simple native Android SU client library.

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
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

/**
 * Represents a SU Shell session oriented API, implemented above the native
 * {@link org.openmarl.susrv.LibSusrv} library.
 *
 * <p>A shell session is bound to an application process after a successful initialization
 * ({@link #getInstance(android.content.Context, boolean) getInstance()}),
 * and it remains associated to this process until invalidated through a call to
 * {@link #exit()}, or the associated native shell process dies abnormally.
 * </p>
 *
 * <p>API calls that require a valid native shell session to be bound throw
 * {@link org.openmarl.susrv.SuShellInvalidatedException SuShellInvalidatedException} when no such
 * session exists for the requesting process.
 * </p>
 *
 * <p>The shell session initializes a <i>private file system</i> (PFS), creating the
 * following directories, all with permissions <code>0700</code>: <code>pfs_root/var</code>,
 * <code>pfs_root/var/run</code>, <code>pfs_root/var/log</code>, <code>pfs_root/bin</code>,
 * <code>pfs_root/lib</code>, and <code>pfs_root/tmp</code>, where <code>pfs_root</code> is the
 * application private storage path, as returned by <code>Context.getFilesDir().getPath()</code>.
 * This is usually something like <code>/data/data/com.android.phone/files</code>.
 * </p>
 *
 * <p>When a session is initialized with the <code>smart</code> flag, it sets the environment
 * to include <code>pfs_root/bin</code> in <code>PATH</code>, and <code>pfs_root/lib</code>
 * in <code>LD_LIBRARY_PATH</code>, and immediately changes its working directory to
 * <code>pfs_root</code>.
 * </p>
 *
 * <p>The shell log file, containing commands output and debug info, is available in the directory
 * <code>pfs_root/var/log</code>.
 * </p>
 *
 * @author t0kt0ckus
 */
public class SuShell {


    private static SuShell _instance;

    private final String mPfsRoot;
    private final Context mContext;

    private final List<SuShellLifecycleObserver> mObservers;

    private SuShell(Context ctx) {
        mContext = ctx;
        mPfsRoot = mContext.getFilesDir().getPath();
        mObservers = new ArrayList<SuShellLifecycleObserver>();
    }

    /**
     * Opens or initializes the SU shell session bound to the requesting process.
     *
     * @param ctx A valid context.
     * @param smartFlag If true, the shell environment is updated to include this session's PFS.
     *
     * @return An initialized SU shell session ready to accept commands, or <code>null</code> when
     * a suitable session didn't exist, and a new one could not be created.
     */
    public static SuShell getInstance(Context ctx, boolean smartFlag) {
        if (_instance == null) {
            _instance = new SuShell(ctx);

            int rval = LibSusrv.openShellSession(_instance.mPfsRoot);
            if (rval == 0) {
                Log.d(TAG, "Created new SU Shell native session");

                if (smartFlag)
                    _instance.updatePrivateEnvironment();
            }
            else if (rval == LibSusrv.SESSION_EXISTS_ERR) {
                Log.d(TAG, "Re-using existing SU Shell native session");
            }
            else {
                _instance = null;
                Log.e(TAG, "Failed to bind to any native SU Shell session");
            }
        }

        return _instance;
    }

    /**
     * Answers the current SU shell session, if any.
     *
     * @return A valid shell session, or <code>null</code> if the requesting process isn't currently
     * bound to any shell session.
     */
    public static SuShell getInstance() {
        return _instance;
    }

    /**
     * Adds a life-cycle observer.
     *
     * The registered observer will be notified when this shell session becomes invalid. Note that
     * observers are notified only once upon session invalidation.
     *
     * @param observer The observer.
     *
     * @return The current count of observers.
     */
    public int addObserver(SuShellLifecycleObserver observer) {
        mObservers.add(observer);
        return mObservers.size();
    }

    /**
     * Removes a life-cycle observer.
     *
     * @param observer The no more interested observer.
     *
     * @return The current count of observers.
     */
    public int removeObserver(SuShellLifecycleObserver observer) {
        mObservers.remove(observer);
        return mObservers.size();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                              State API
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Answers the path to the application PFS's root.
     *
     * @return Something like <code>/data/data/com.example.app/files</code>.
     */
    public String getPfsRoot() {
        return mPfsRoot;
    }

    /**
     * Answers the PFS's temp directory.
     *
     * <p>This directory is cleared upon application restart.
     *
     * @return Its absolute path.
     */
    public String getTmp() {
        return String.format("%s/tmp", mPfsRoot);
    }

    /**
     * Tells whether a valid shell session is currently bound to the requesting process.
     *
     * @return <code>true</code> if this is the case.
     */
    public boolean ping() {
        boolean alive = (LibSusrv.ping() > 0);
        if (! alive)
        {
            try {
                invalidatedSessionError();
            }
            catch(SuShellInvalidatedException e) {}
        }
        return alive;
    }

    /**
     * Answers the path to the shell's session log file.
     *
     * @return Its absolute path, or <code>null</code> when not available.
     */
    public String getTtyPath() { return LibSusrv.getTtyPath(); }

    /**
     * Enables/disables echo-ing "terminal" output to session log file.
     *
     * @param enabled <code>true</code> to echo commands output to the session's log file,
     *                <code>false</code> for a silent session.
     */
    public void setTtyEcho(boolean enabled) {
        LibSusrv.setTtyEcho(enabled ? 1 : 0);
    }

    /**
     * Tells whether "terminal" echo is enabled on the current shell session.
     *
     * @return <code>true</code> when such a session exists and echo is enabled.
     */
    public boolean getTtyEcho() {
        return (LibSusrv.getTtyEcho() > 0);
    }

    /**
     * Answers the last line red on the session terminal.
     *
     * @return The last available line, or <code>null</code> if not available.
     */
    public String getLastTtyLine() {
        String lastLine = LibSusrv.getLastTtyRead();
        if ( (lastLine != null) && lastLine.endsWith("\n"))
            return lastLine.substring(0, lastLine.length() -1);
        else
            return lastLine;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                              Java unpriviledged API
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Imports an asset into the application private storage.
     *
     * <p>This neither requires any privilege, nor is implemented as a native call.
     * </p>
     *
     * @param rawAsset The asset name, that is its file name within the application <code>raw</code>
     *                 resources folder.
     * @param dirPath The destination directory path relative to PFS's root.
     * @param filename The destination filename.
     * @param isExec When <code>true</code>, the destination file will be executable.
     *
     * @return The imported asset size in bytes on success,
     * <code>0</code> when the asset name is undefined, or a negative value on error.
     */
    public int importAsset(String rawAsset, String dirPath, String filename, boolean isExec)
    {
        int assetId = getRawAssetId(rawAsset);
        if (assetId < 0)
            return 0; // unknown asset

        byte buf[] = new byte[64];
        int iBytes = -1;

        try {
            File privateFile = new File(String.format("%s/%s/%s", mPfsRoot, dirPath, filename));
            InputStream is = mContext.getResources().openRawResource(assetId);
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

            Log.i(TAG, String.format("PFS-import asset <%s> (%d bytes): %s/%s/%s",
                    rawAsset,
                    iBytes + 1,
                    mPfsRoot, dirPath, filename));
        }
        catch(IOException e ) {
            iBytes = -1;
            Log.e(TAG, String.format("PFS-import asset <%s> failed: %s",
                    rawAsset, e.toString()));
        }
        catch (Resources.NotFoundException e) {
            iBytes = -1;
            Log.w(TAG, String.format("PFS-import asset <%s> failed: %s",
                    rawAsset, e.toString())); // should not happen
        }

        return ( iBytes > 0) ? iBytes + 1 : -1;
    }

    /**
     * Imports an asset as an executable into <code>pfs_root/bin</code>.
     *
     * <p>This neither requires any privilege, nor is implemented as a native call.
     * </p>
     *
     * @param rawAsset The asset name, that is its file name within the application <code>raw</code>
     *                 resources folder.
     * @param filename The destination file name.
     *
     * @return The imported asset size in bytes on success,
     * <code>0</code> when the asset name is undefined, or a negative value on error.
     */
    public int importExecutable(String rawAsset, String filename) {
        return importAsset(rawAsset, "bin", filename, true);
    }

    /**
     * Imports an asset as a shared library into <code>pfs_root/lib</code>.
     *
     * <p>This neither requires any privilege, nor is implemented as a native call.
     * </p>
     *
     * @param rawAsset The asset name, that is its file name within the application <code>raw</code>
     *                 resources folder.
     * @param filename The destination file name.
     *
     * @return The imported asset size in bytes on success,
     * <code>0</code> when the asset name is undefined, or a negative value on error.
     */
    public int importLibrary(String rawAsset, String filename) {
        return importAsset(rawAsset, "lib", filename, false);
    }

    /**
     * Imports an asset as a temporary file in <code>pfs_root/tmp</code>.
     *
     * <p>This does not require any privilege, and is not accomplished through the native
     * SU shell session.</p>
     *
     * @param rawAsset The asset name, that is its file name within the application <code>raw</code>
     *                 resources folder.
     * @param filename The destination file name.
     *
     * @return The imported asset size in bytes on success,
     * <code>0</code> when the asset name is undefined, or a negative value on error.
     */
    public int importTmp(String rawAsset, String filename) {
        return importAsset(rawAsset, "tmp", filename, false);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                              Native unpriviledged API
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Answers the PID of the first process which name matches a given application name.
     *
     * <p>This is an unprivileged native call, reading Linux <code>proc</code> filesystem.
     * </p>
     *
     * @param procname The process entry to search for, for example <code>com.example.app</code>.
     *
     * @return A non-zero PID, or a negative number on error.
     */
    public int getpid(String procname) {
        return LibSusrv.getpid(procname);
    }

    /**
     * Answers the list of current process names.
     *
     * <p>This is an unprivileged native call, reading Linux <code>/proc</code> filesystem.
     * </p>
     *
     * @return The list of process names, as defined by <code>/proc/pid/cmdline</code>.
     */
    public String[] getproclist() {
        return  LibSusrv.getproclist();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                              Native unpriviledged API
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Invalidates the current shell session, releasing native resources.
     *
     * @return <code>0</code> when the current session has been properly closed,
     * {@link org.openmarl.susrv.LibSusrv#NO_SESSION_ERR}, or an error code when
     * during native shell shutdown failed.
     */
    public int exit() {
        _instance = null;
        return LibSusrv.exitShellSession();
    }

    /**
     * Executes a shell command within the current SU shell session.
     *
     * @param command The command string to execute.
     *
     * @return The command exit code, usually zero (<code>0</code>) on success, or
     * {@link org.openmarl.susrv.LibSusrv#INVALID_RESULT_ERR INVALID_RESULT_ERR}.
     *
     * @throws SuShellInvalidatedException When this session has been invalidated.
     */
    public int exec(String command) throws SuShellInvalidatedException
    {
        int rval = LibSusrv.exec(command);

        if (rval == LibSusrv.NO_SESSION_ERR)
            invalidatedSessionError();

        return rval;
    }

    /**
     * Within the current SU shell session, changes a file or directory permissions.
     *
     * @param path The file or directory path.
     * @param mode The UNIX permission mask (usually as in octal).
     *
     * @return The command exit code, typically <code>0</code> on success.
     *
     * @throws SuShellInvalidatedException When this session has been invalidated.
     */
    public int chmod(String path, int mode) throws SuShellInvalidatedException {
        String cmd = String.format("chmod %o %s", mode, path);

        int rval = LibSusrv.exec(cmd);

        if (rval == LibSusrv.NO_SESSION_ERR)
            invalidatedSessionError();

        return rval;
    }

    /**
     * Within the current SU shell session, creates a new directory and set its permissions.
     *
     * @param path The directory path.
     * @param mode The UNIX permission mask (usually as in octal).
     *
     * @return The command exit code, typically <code>0</code> on success.
     *
     * @throws SuShellInvalidatedException When this session has been invalidated.
     */
    public int mkdir(String path, int mode) throws SuShellInvalidatedException
    {
        String cmd = String.format("mkdir -p %s", path);
        int rval = exec(cmd) ;

        if (rval == LibSusrv.NO_SESSION_ERR)
            invalidatedSessionError();

        if ( ((rval == 0) || (rval == ERRNO_EEXIST))
                && (mode > 0))
            rval = chmod(path, mode);

        return rval;
    }

    /**
     * Within the current SU shell session, copy file(s) from and to the device filesystem.
     *
     * @param sourcePath The source path, may contain wildcards.
     * @param destinationPath The destination path.
     * @param mode The UNIX permission mask (usually as in octal).
     *
     * @return The command exit code, typically <code>0</code> on success.
     *
     * @throws SuShellInvalidatedException When this session has been invalidated.
     */
    public int cp(String sourcePath, String destinationPath, int mode)
            throws SuShellInvalidatedException
    {
        String cmd = String.format("cp %s %s", sourcePath, destinationPath);
        int rval = exec(cmd) ;

        if (rval == LibSusrv.NO_SESSION_ERR)
            invalidatedSessionError();

        if ((rval == 0) && (mode > 0))
            rval = chmod(destinationPath, mode);

        return rval;
    }

    /**
     * Within the current SU shell session, changes the current working directory.
     *
     * @param cwdPath The new working directory.
     *
     * @return The command exit code, typically <code>0</code> on success.
     *
     * @throws SuShellInvalidatedException When this session has been invalidated.
     */
    public int cd(String cwdPath)
            throws SuShellInvalidatedException
    {
        String cmd = String.format("cd %s", cwdPath);
        int rval = exec(cmd) ;

        if (rval == LibSusrv.NO_SESSION_ERR)
            invalidatedSessionError();

        return rval;
    }

    /**
     * Within the current SU shell session, copy an application <i>raw asset</i> to the device
     * filesystem.
     *
     * <p>To copy assets to the application private storage,
     * prefer {@link #importAsset(String, String, String, boolean)}.</p>
     *
     * @param rawAsset The asset name, that is its file name within the application <code>raw</code>
     *                 resources folder.
     * @param destinationPath The destination file path.
     * @param mode The UNIX permission mask (usually as in octal).
     *
     * @return The command exit code. (see {@link #cp(String, String, int)}).
     *
     * @throws SuShellInvalidatedException When this session has been invalidated.
     */
    public int cpa(String rawAsset, String destinationPath, int mode)
            throws SuShellInvalidatedException
    {

        String tmpFileName = String.format("%s.tmp", rawAsset);
        int retval = importTmp(rawAsset, tmpFileName);

        if (retval > 0)
        {
            String tmpFilePath = String.format("%s/%s", getTmp(), tmpFileName);
            retval = cp(tmpFilePath, destinationPath, mode);
        }

        return retval;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                              impl.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    private static final int ERRNO_EEXIST = 17;

    private void invalidatedSessionError() throws SuShellInvalidatedException {
        for (SuShellLifecycleObserver observer : mObservers) {
            observer.onSuShellInvalidated();
        }
        _instance = null;
        mObservers.clear();
        throw new SuShellInvalidatedException();
    }

    private void updatePrivateEnvironment() {
        // PATH
        String newPATH = String.format("%s/bin:$PATH", mPfsRoot);
        if (LibSusrv.exec(String.format("export PATH=%s", newPATH)) == 0) {
            Log.i(TAG, String.format("PATH: %s", newPATH));
        }
        else {
            Log.w(TAG, String.format("Failed to export PATH (%s) !", newPATH));
        }
        // LD_LIBRARY_PATH
        String newLD_PATH = String.format("%s/lib", mPfsRoot);
        if (LibSusrv.exec(String.format("export LD_LIBRARY_PATH=%s", newLD_PATH)) == 0) {
            Log.i(TAG, String.format("LD_LIBRARY_PATH: %s", newLD_PATH));
        }
        else {
            Log.w(TAG, String.format("Failed export LD_LIBRARY_PATH (%s)", newLD_PATH));
        }
        // cd
        try {
            cd(mPfsRoot);
        }
        catch(SuShellInvalidatedException e) {
            Log.e(TAG, "Session's already terminated !?");
        }
    }

    private int getRawAssetId(String rawAsset) {
        int assetId = -1;

        try {
            Class R_class = Class.forName(
                    String.format("%s.R", mContext.getApplicationContext().getPackageName())
            );

            Class R_raw_class = null;
            for (Class clazz : R_class.getDeclaredClasses()) {
                if ("raw".equals(clazz.getSimpleName()))
                    R_raw_class = clazz;
            }

            if (R_raw_class != null) {
                Field R_raw_asset = R_raw_class.getField(rawAsset);
                assetId = R_raw_asset.getInt(null);
            }
        }
        catch (ClassNotFoundException e) {
            Log.w(TAG, e.toString()); // should not happen
        }
        catch (IllegalAccessException e) {
            Log.w(TAG, e.toString()); // should not happen
        }
        catch (NoSuchFieldException e) {
            Log.e(TAG, String.format("Unknown asset: %s", rawAsset));
        }

        return assetId;
    }

    private static final String TAG = SuShell.class.getSimpleName();
}
