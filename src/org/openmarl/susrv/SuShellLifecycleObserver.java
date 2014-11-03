/*
    SuSrv: A simple native Android SU client library.

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

/**
 * Interface for observers interested in SU Shell session life-cycle.
 *
 * @author t0kt0ckus
 */
public interface SuShellLifecycleObserver {

    /**
     * Notifies the observer about whether a shell session's initialization has succeeded.
     *
     * @param shell An initialized SU Shell session, or <code>null</code> on failure.
     */
    public void onSuShellInitialized(SuShell shell);

    /**
     * Notifies the observer that
     */
    public void onSuShellInvalidated();
}
