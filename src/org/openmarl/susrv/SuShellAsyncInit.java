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
import android.os.AsyncTask;
import android.util.Log;

/**
 * SU shell session asynchronous initializer.
 *
 * <p>Typical usage within an Activity implementation should be
 * <code>new SuShellAsyncInit(this).execute()</code></p>
 *
 * <p>Once initialization has completed, the current shell session is available
 * through <code>SuShell.getInstance()</code></p>
 *
 * <p>To be notified of SU shell initialization completion, the activity can implement
 * <code>SuShellAsyncObserver</code>.</p>
 */
public class SuShellAsyncInit extends AsyncTask<Void,Void,SuShell> {

    private Context mContext;
    private SuShellAsyncObserver mObserver;

    /**
     * Creates a new initializer.
     *
     * @param ctx Any valid context.
     */
    public SuShellAsyncInit(Context ctx) {
        mContext = ctx;
        if (ctx instanceof SuShellAsyncObserver) {
            mObserver = (SuShellAsyncObserver) ctx;
        }
    }

    @Override
    protected void onPostExecute(SuShell suShell) {
        super.onPostExecute(suShell);

        if (suShell != null) {
            Log.i(TAG, "SU native Shell session started successfully");

            if (mObserver != null) {
                mObserver.onShellInitComplete(suShell);
            }
        }
    }

    @Override
    protected SuShell doInBackground(Void... params) {
        try {
            return SuShell.getInstance(mContext);
        }
        catch(IllegalStateException e) {
            Log.e(TAG, String.format("Failed to initialize SU native shell session: %s", e.toString()));
            return null;
        }
    }

    private static final String TAG = "SuShellAsyncInit";
}
