/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;

/**
 * SU shell session asynchronous initializer.
 *
 * <p>To be notified upon SU shell initialization completion, the activity should implement
 * {@link org.openmarl.susrv.SuShellAsyncObserver}.</p>
 *
 * @author t0kt0ckus
 */
public class SuShellAsyncInit extends AsyncTask<Void,Void,SuShell> {

    private Context mContext;
    private SuShellAsyncObserver mObserver;

    /**
     * Creates a new initializer.
     *
     * @param ctx An activity that should implement {@link org.openmarl.susrv.SuShellAsyncObserver}.
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
        if (mObserver != null) {
            mObserver.onShellInitComplete(suShell);
        }
    }

    @Override
    protected SuShell doInBackground(Void... params) {
        return SuShell.getInstance(mContext);
    }
}
