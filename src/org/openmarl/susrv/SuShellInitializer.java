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
import android.os.AsyncTask;
import android.util.Log;

/**
 * SU shell session asynchronous initializer.
 *
 */
public class SuShellInitializer extends AsyncTask<Void,Void,SuShell> {

    private Context mContext;
    private static SuShell mSuShell;
    private SuShellInitListener mSessionListener;

    public SuShellInitializer(Context ctx) {
        mContext = ctx;
        if (ctx instanceof SuShellInitListener) {
            mSessionListener = (SuShellInitListener) ctx;
        }
    }

    @Override
    protected void onPostExecute(SuShell suShell) {
        super.onPostExecute(suShell);
        mSuShell = suShell;
        Log.i(TAG, "SU Shell initialization complete");

        if (mSessionListener != null) {
            mSessionListener.onShellInitComplete(mSuShell);
        }
    }

    @Override
    protected SuShell doInBackground(Void... params) {
        try {
            return SuShell.getInstance(mContext);
        }
        catch(IllegalStateException e) {
            Log.e(TAG, String.format("Failed to initialize SU shell session: %s",
                    e.toString()));
            return null;
        }
    }

    public SuShell getShell() {
        return mSuShell;
    }

    private static final String TAG = "SuShell";
}
