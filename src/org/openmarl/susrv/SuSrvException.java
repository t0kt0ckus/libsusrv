package org.openmarl.susrv;

/**
 * Created by chris on 10/18/14.
 */
public class SuSrvException extends Exception {
    public SuSrvException() {
        super();
    }

    public SuSrvException(String detailMessage) {
        super(detailMessage);
    }

    public SuSrvException(String detailMessage, Throwable throwable) {
        super(detailMessage, throwable);
    }

    public SuSrvException(Throwable throwable) {
        super(throwable);
    }
}
