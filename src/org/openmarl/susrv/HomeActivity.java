package org.openmarl.susrv;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;


public class HomeActivity extends Activity implements  SuShellAsyncObserver {

    private SuShell mSuShell;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);

        new SuShellAsyncInit(this).execute();
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.home, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        SuShell.getInstance().exit();
    }

    @Override
    public void onShellInitComplete(SuShell shell) {
        if (shell != null) {
            SuShell.getInstance().exec("id");
            /*
            SuShell.getInstance().exec("pwd");
            SuShell.getInstance().cd();
            SuShell.getInstance().exec("pwd");

            SuShell.getInstance().updateEnvironment();
            SuShell.getInstance().importAsset(R.raw.hijack, "bin", "hijack", true);
            SuShell.getInstance().exec("ls bin");
            */
        }
    }

    private static final String TAG = "HomeActivity";
}
