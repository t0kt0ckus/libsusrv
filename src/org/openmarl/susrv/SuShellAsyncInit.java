/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

import android.content.Context;
import android.os.AsyncTask;

import java.util.ArrayList;
import java.util.List;

/**
 * SU shell session asynchronous initializer.
 *
 * @author t0kt0ckus
 */
public class SuShellAsyncInit extends AsyncTask<Void,Void,SuShell> {

    private List<SuShellAsyncObserver> mObservers = new ArrayList<SuShellAsyncObserver>();
    private Context mContext;

    /**
     *
     * @param ctx
     */
    public SuShellAsyncInit(Context ctx) {
        mContext = ctx;
    }

    /**
     *
     * @param observer
     * @return
     */
    public SuShellAsyncInit addObserver(SuShellAsyncObserver observer) {
        mObservers.add(observer);
        return this;
    }

    /**
     *
     * @param observer
     */
    public void removeObserver(SuShellAsyncObserver observer) {
        mObservers.remove(observer);
    }

    @Override
    protected void onPostExecute(SuShell suShell) {
        super.onPostExecute(suShell);
        for (SuShellAsyncObserver observer : mObservers)
            observer.onShellInitComplete(suShell);
    }

    @Override
    protected SuShell doInBackground(Void... params) {
        return SuShell.getInstance(mContext, true);
    }
}
