package org.openmarl.susrv;

public class SuShell {

  static {
      System.loadLibrary("susrv");
  }
  public native int open(String appfilesDir);
  public native int exec(String command);

  // Android specific
  //
  /*   
  public SuShell(Context ctx) {
      // FIXME/Warning: this will trigger some SU ack dialog, don't miss it !
      open(ctx.getFilesDir().getPath());
  }
  */
  
  public static void main(String args[]) {
    String localRootFS = (args.length > 0) ? args[0] : "/tmp/susrv";
    
    SuShell shell = new SuShell();
    try {
      shell.open(localRootFS);

      shell.exec("id");

      Thread.sleep(1000, 0); 
    }
    catch(Exception e) {
      e.printStackTrace();
    }
    
    System.exit(0);
  }
  
}
