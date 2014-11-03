/*
    SuSrv: A simple native Android SU client library.

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

import android.content.Context;
import android.os.AsyncTask;

/**
 * SU Shell session asynchronous initializer.
 *
 * @author t0kt0ckus
 */
public class SuShellAsyncInit extends AsyncTask<Void,Void,SuShell> {

    private Context mContext;

    /**
     * Creates a new initializer.
     *
     * <p>Actual initialization will be triggered through {@link #execute(Object[])}.
     * </p>
     *
     * @param ctx A valid context, that should implement
     *            {@link org.openmarl.susrv.SuShellLifecycleObserver}.
     */
    public SuShellAsyncInit(Context ctx) {
        mContext = ctx;
    }

    @Override
    protected void onPostExecute(SuShell shell) {
        super.onPostExecute(shell);
        if (mContext instanceof SuShellLifecycleObserver)
            ((SuShellLifecycleObserver) mContext).onSuShellInitialized(shell);
    }

    @Override
    protected SuShell doInBackground(Void... params) {
        return SuShell.getInstance(mContext, true);
    }
}
