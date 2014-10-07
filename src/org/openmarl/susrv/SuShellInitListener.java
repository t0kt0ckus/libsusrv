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

/**
 * An activity should implement this interface, and then use an
 * <code>SuShellInitializer</code> async task.
 */
public interface SuShellInitListener {

    /**
     * Notifies that the SU shell session should be ready.
     *
     * @param shell An initialized SU shell session, or <code>null</code>
     *              when initialization failed.
     */
    public void onShellInitComplete(SuShell shell);

}
