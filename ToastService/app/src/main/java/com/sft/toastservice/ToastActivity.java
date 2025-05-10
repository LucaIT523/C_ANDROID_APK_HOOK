package com.sft.toastservice;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Process;
import android.util.Log;

public class ToastActivity extends Activity {

    public final int PERMISSION_REQUEST_CODE = 0x01;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d("sss", "-----------------Toast Activity onCreate\n");
        super.onCreate(savedInstanceState);
        Log.d("sss", "-----------------Toast Activity super.onCreate\n");
        MyFunc();
        Log.d("sss", "-----------------Toast Activity switch activity\n");
//        finish();
        Log.d("sss", "-----------------Toast Activity finish\n");
    }
    private void MyFunc() {
        Log.d("sss", "-----------------Toast Activity myfunc\n");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            Log.d("sss", "-----------------more than 13\n");
            if (checkPermission(Manifest.permission.POST_NOTIFICATIONS, Process.myPid(), Process.myUid()) == PackageManager.PERMISSION_GRANTED){
                onNextPage();
            }else{
                Log.d("sss", "-----------------reqire\n");
                requestPermissions(new String[]{Manifest.permission.POST_NOTIFICATIONS}, PERMISSION_REQUEST_CODE);
            }
        }else{
            Log.d("sss", "-----------------less than 13\n");
            if (checkSelfPermission(Manifest.permission.VIBRATE) == PackageManager.PERMISSION_GRANTED){
                onNextPage();
            }else{
                requestPermissions(new String[]{Manifest.permission.VIBRATE}, PERMISSION_REQUEST_CODE);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String[] permissions,
                                           int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                onNextPage();
            } else {
                finish();
            }
        }
    }

    public void onNextPage(){
        Log.d("sss", "-----------------on Next\n");
        Intent intentService = new Intent(getApplicationContext(), ToastService.class);;
        startService(intentService);

        Log.d("sss", "-----------------on Next launch main\n");
        Intent intent = new Intent();
        intent.setClassName(this, "com.sft.toastservice.MainActivity");
        startActivity(intent);

        finish();
    }
}