/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

/**
 * Thrown during shell session life-cycle, when the associated native shell has died abnormally.
 *
 */
public class SuShellInvalidatedException extends Exception {
    public SuShellInvalidatedException() {
        super();
    }

    public SuShellInvalidatedException(String detailMessage) {
        super(detailMessage);
    }

    public SuShellInvalidatedException(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }

    public SuShellInvalidatedException(Throwable throwable) {
        super(throwable);
    }

    @Override
    public String toString() {
        return "The SU Shell session associated to this process has been invalidated";
    }
}
