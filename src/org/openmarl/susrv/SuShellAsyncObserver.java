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

/**
 * Observer interface.
 */
public interface SuShellAsyncObserver {

    /**
     * Notifies that the SU shell session should be ready.
     *
     * @param shell An initialized SU shell session, or <code>null</code>
     *              when initialization failed.
     */
    public void onShellInitComplete(SuShell shell);

}
