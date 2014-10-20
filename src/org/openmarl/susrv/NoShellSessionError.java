/*
    SuSrv: Android SU native client library

    <t0kt0ckus@gmail.com>
    (C) 2014

    License GPLv3
 */
package org.openmarl.susrv;

/**
 * As its name tells.
 */
public class NoShellSessionError extends Exception {

    public NoShellSessionError() {
        super();
    }
    public NoShellSessionError(String detailMessage) {
        super(detailMessage);
    }
    public NoShellSessionError(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }
    public NoShellSessionError(Throwable throwable) {
        super(throwable);
    }
}

