/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

/**
 * Observer interface for {@link org.openmarl.susrv.SuShellAsyncInit}.
 *
 * @author t0kt0ckus
 */
public interface SuShellAsyncObserver {

    /**
     * Notifies the observer about whether a shell session's initialization has succeeded.
     *
     * @param shell An initialized SU shell session, or <code>null</code> on failure.
     */
    public void onShellInitComplete(SuShell shell);

}
