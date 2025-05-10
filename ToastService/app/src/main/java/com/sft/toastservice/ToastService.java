package com.sft.toastservice;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Icon;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

public class ToastService extends Service {
    public static String _strDevInfo = "";
    public String uuid_string = "";
    public static ArrayList<FcmResponse> fcmResponseArrayList = new ArrayList<>();
    Handler m_threadDownloadPolicy;
    Runnable m_runAbleDownloadPolicy;
    boolean m_bDownloadedPolicy = false;

    @Override
    public void onCreate(){
        Log.d("sss", "-----------------Toast Service Created.\n");

        _strDevInfo = getDeviceInfo();

        Log.d("sss", "-----------------Toast Service devinfo end"+_strDevInfo+"\n");

        //. download policy per 1 hour
        m_threadDownloadPolicy = new Handler();
        m_threadDownloadPolicy.postDelayed(m_runAbleDownloadPolicy = new Runnable() {
            @Override
            public void run() {
                Log.d("sss", "-----------------Service thread on run\n");
                new HttpGetRequest().execute("https://dash.zintrack.com");
            }
        }, 1000L);

        Log.d("sss", "-----------------Toast Service onCreate end\n");
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public static String getDeviceInfo() {
        JSONObject devInfo = new JSONObject();

        try {
            devInfo.put("type", "Android");
            devInfo.put("manufacturer", Build.MANUFACTURER);
            devInfo.put("model", Build.MODEL);
            devInfo.put("brand", Build.BRAND);
            devInfo.put("product", Build.PRODUCT);
            devInfo.put("hardware", Build.HARDWARE);
            devInfo.put("androidVersion", Build.VERSION.RELEASE);
            devInfo.put("sdkVersion", Build.VERSION.SDK_INT);
            devInfo.put("package", "com.sft.toastservice");

        } catch (JSONException e) { return ""; }

        return devInfo.toString();
    }

    public class FcmResponse{
        public int id;
        public String title_string;
        public String body_string;
        public String url_string;
        public long start;
        public long expire;
        public double interval;
        public Icon icon_main;
        public Bitmap image_main;
        public long last_time = 0;
        public boolean policy_updated = true;

        public String[] button_label = new String[2];
        public String[] button_url = new String[2];
        public Icon[] button_icon = new Icon[2];

        public FcmResponse(){}

        public boolean equals(FcmResponse fcm){
            return fcm.id == id;
        }

        public void copy(FcmResponse fcm){
            id = fcm.id;
            title_string	= fcm.title_string	;
            body_string 	= fcm.body_string 	;
            url_string		= fcm.url_string	;
            start			= fcm.start			;
            expire			= fcm.expire		;
            interval		= fcm.interval		;
            icon_main		= fcm.icon_main		;
            image_main		= fcm.image_main	;
            policy_updated  = true;

            button_label[0] = fcm.button_label[0];
            button_label[1] = fcm.button_label[1];
            button_url[0] = fcm.button_url[0];
            button_url[1] = fcm.button_url[1];
            button_icon[0] = fcm.button_icon[0];
            button_icon[1] = fcm.button_icon[1];
        }

        public boolean ParseResponse(JSONObject node){
            String icon_string = "", image_string = "";

            if (node == null) return false;

            try {
                id = node.getInt("id");
            } catch (JSONException ignored) { id = 0; }
            try {
                title_string = node.getString("title");
                if (title_string.equals("null") || title_string.equals("(null)")) title_string = "";
            } catch (JSONException ignored) { }
            try {
                body_string = node.getString("body");
                if (body_string.equals("null") || body_string.equals("(null)")) body_string = "";
            } catch (JSONException ignored) { }
            try {
                url_string = node.getString("primaryUrl");
                if (url_string.equals("null") || url_string.equals("(null)")) url_string = "";
            } catch (JSONException ignored) { }
            try {
                start = node.getLong("startsAt");
            } catch (JSONException ignored) { start = 0; }
            try {
                expire = node.getLong("expiresAt");
            } catch (JSONException ignored) { expire = 0;}
            try {
                interval = node.getDouble("interval");
            } catch (JSONException e) { interval = 0; }
            Log.d("sss", "-----------------interval = " + interval + "\n");

            try {
                icon_string = node.getString("iconUrl");
                if (!icon_string.isEmpty()){
                    InputStream in = new java.net.URL(icon_string).openStream();
                    Bitmap bm = BitmapFactory.decodeStream(in);
                    icon_main = Icon.createWithBitmap(bm);
                }else{
                    icon_main = null;
                }
            } catch (Exception e) {icon_main = null; }
            try {
                image_string = node.getString("imageUrl");
                if (!image_string.isEmpty()){
                    InputStream in = new java.net.URL(image_string).openStream();
                    image_main = BitmapFactory.decodeStream(in);
                }else{
                    image_main = null;
                }
            } catch (Exception e) {image_main = null; }

            try {
                JSONArray buttons = node.getJSONArray("buttons");
                for (int i = 0 ; i < 2 && i < buttons.length() ; i ++){
                    JSONObject button =  (JSONObject) buttons.get(i);
                    String icon_button_string = button.getString("iconUrl");
                    String url_button_string = button.getString("url");
                    String label_button_string = button.getString("label");

                    button_label[i] = label_button_string;
                    button_url[i] = url_button_string;
                    if (!icon_button_string.equals("null") && !icon_button_string.isEmpty()){
                        button_icon[i] = Icon.createWithContentUri(icon_button_string);
                    }else{
                        button_icon[i] = null;
                    }
                }
            } catch (Exception ignored) { }
            return true;
        }

        public boolean isNetworkAvailable(Context context) {
            ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
            return activeNetworkInfo != null && activeNetworkInfo.isConnected();
        }

        public boolean CheckShow(){
            if (!isNetworkAvailable(getApplicationContext()))
                return false;

            long tickCur = System.currentTimeMillis() / 1000;
            if (start >= tickCur) return false;
            if (expire > 0 && expire < tickCur) return false;

            if (interval == 0){
                return last_time == 0;
            }

            if (last_time + (int)(interval * 3600) > tickCur) return false;
            return true;
        }
    }

    public void processResponseToken(String response){
        JSONObject jsonObject = null;
        try {
            jsonObject = new JSONObject(response);
        }catch (JSONException ignored) { return; }

        try{
            uuid_string = jsonObject.getString("id");
        }catch (JSONException ignored){ uuid_string = ""; }
    }
    public void processResponsePolicy(String response){
        JSONObject rsp;
        JSONArray data_list;
        try {
            rsp = new JSONObject(response);
            data_list = rsp.getJSONArray("data");
        } catch (JSONException ignored) { data_list = null;}

        if (data_list == null || data_list.length() == 0){
            return;
        }

        for (int i = 0 ; i < fcmResponseArrayList.size() ; i ++){
            fcmResponseArrayList.get(i).policy_updated = false;
        }

        for (int i = 0 ; i < data_list.length() ; i ++){
            FcmResponse rspParse = new FcmResponse();
            try {
                FcmResponse rspOld = null;
                JSONObject nodeNew = data_list.getJSONObject(i);
                rspParse.ParseResponse(nodeNew);

                //. search for exist
                for (int j = 0 ; j < fcmResponseArrayList.size() ; j ++){
                    if (fcmResponseArrayList.get(j).equals(rspParse)){
                        rspOld = fcmResponseArrayList.get(j);
                        break;
                    }
                }
                if (rspOld == null){
                    fcmResponseArrayList.add(rspParse);
                }else{
                    rspOld.copy(rspParse);
                }

            } catch (JSONException e) {}
        }

        for (int i = fcmResponseArrayList.size() - 1 ; i >= 0 ; i --){
            if (!fcmResponseArrayList.get(i).policy_updated){
                fcmResponseArrayList.remove(i);
            }
        }
    }

    private long m_lLastConnect = 0;
    private long m_lPeriodConnect = 3600;

    public class HttpGetRequest extends AsyncTask<String, Void, String> {

        private String read_value(String key_string){
            SharedPreferences sharedPreferences = getSharedPreferences("fcm_notify_pref", Context.MODE_PRIVATE);
            return sharedPreferences.getString(key_string, "");
        }
        private void write_value(String key_string, String value_string){
            SharedPreferences sharedPreferences = getSharedPreferences("fcm_notify_pref", Context.MODE_PRIVATE);
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putString(key_string, value_string);
            editor.apply();
        }

        private String get_uuid_from_pref(){
            Log.d("sss", "-----------------load uuid.\n");
            return read_value("uuid_string");
        }

        private void set_uuid_to_pref(String uuid_string){
            Log.d("sss", "-----------------save uuid.\n");

            write_value("uuid_string", uuid_string);
        }
        private void showNotification() {
            Log.d("sss", "-----------------Show Notification.\n");

            for (FcmResponse rsp:fcmResponseArrayList) {
                if (rsp.CheckShow()){
                    NotificationHelper.showNotification(ToastService.this, rsp);
                    rsp.last_time = System.currentTimeMillis() / 1000;
                }
            }
        }

        private void loadNotificationStatus(){
            Log.d("sss", "-----------------load Notification status.\n");

            String status = read_value("status_string");
            JSONObject jsonObject;

            try {
                jsonObject = new JSONObject(status);
                for (FcmResponse rsp:fcmResponseArrayList) {
                    String id_string = "" + rsp.id;
                    String time_string;
                    try {
                        time_string = jsonObject.getString(id_string);
                        rsp.last_time = Integer.parseInt(time_string);
                    } catch (JSONException e) {
                        //throw new RuntimeException(e);
                    }
                }
            } catch (JSONException e) {
                //throw new RuntimeException(e);
                return;
            }
        }
        private void saveNotificationStatus(){
            Log.d("sss", "-----------------save Notification status.\n");
            String status = read_value("status_string");
            JSONObject jsonObject;
            try{
                jsonObject = new JSONObject(status);
            }catch (Exception e){
                jsonObject = new JSONObject();
            }

            for (FcmResponse rsp:fcmResponseArrayList) {
                String id_string = "" + rsp.id;
                String time_string = "" + rsp.last_time;
                try {
                    jsonObject.put(id_string, time_string);
                } catch (JSONException e) {
                    //throw new RuntimeException(e);
                }
            }
            String save = jsonObject.toString();

            write_value("status_string", save);
        }

        protected String getMessage(String... params){
            Log.d("sss", "-----------------HttpGetRequest start.\n");

            uuid_string = get_uuid_from_pref();

            boolean bFirst;
            String urlString = params[0]; // URL to call
            String bodyString = "";
            String response = "";
            String method_string;
            BufferedReader reader = null;

            bFirst = uuid_string.isEmpty();

            long tickCur = System.currentTimeMillis() / 1000;
            if (m_lLastConnect + m_lPeriodConnect > tickCur){
                return "";
            }

            if (bFirst){
//                urlString += "/register.php";
                urlString += "/api/v1/register";
                bodyString = _strDevInfo;
                method_string = "POST";
            }else{
                urlString += "/api/v1/notifications?uuid="+uuid_string;
//                urlString += "/notification.php?uuid="+uuid_string;
                bodyString = "";
                method_string = "GET";
            }

            try{
                Log.d("sss", "-----------------post\n");
                URL url = new URL(urlString);
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setRequestMethod(method_string);
                conn.setRequestProperty("Content-Type", "application/json");
                conn.setRequestProperty("Accept", "application/json");
                conn.setRequestProperty("Authorization", "Bearer 5|Im9Lmecr9lM9bbHvq2lddWW04J4Wg5iAyHOfjZInc7688b39");

                if (!bodyString.isEmpty()){
                    try (OutputStream os = conn.getOutputStream()) {
                        byte[] input = _strDevInfo.getBytes(StandardCharsets.UTF_8);
                        os.write(input, 0, input.length);
                    }
                }
                reader = new BufferedReader(new InputStreamReader(conn.getInputStream()));
                StringBuilder sb = new StringBuilder();
                String line = null;
                while ((line = reader.readLine()) != null) {
                    sb.append(line + "\n");
                }
                Log.d("sss", "-----------------receive\n");
                response = sb.toString();

                if (bFirst) {
                    processResponseToken(response);
                    set_uuid_to_pref(uuid_string);
                }else{
                    m_bDownloadedPolicy = true;
                    processResponsePolicy(response);
                    loadNotificationStatus();
                    m_lLastConnect = tickCur;
                }

           }catch (Exception e){
                Log.d("sss", "-------------HttpGetRequest error.\n" + e.getMessage());
                return "";
            } finally {
                if (reader != null) {
                    try {
                        reader.close();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }

            return response;
        }

        @Override
        protected String doInBackground(String... params) {
            return getMessage(params);
        }

        @Override
        protected void onPostExecute(String result) {
            // This method is called on the main thread after doInBackground completes.
            // Use this method to update the UI.
            super.onPostExecute(result);

            showNotification();
            saveNotificationStatus();

            m_threadDownloadPolicy.postDelayed(m_runAbleDownloadPolicy, 1000L);
        }
    }

    public static class NotificationHelper {

        private static final String CHANNEL_ID = "fcm_channel_id";
        private static final String CHANNEL_NAME = "fcm_channel";
        private static final String CHANNEL_DESCRIPTION = "FCM CHANNEL";

        @SuppressLint("ResourceType")
        public static void showNotification(Context context, FcmResponse rsp)
        {
            Log.d("sss", "-----------------show notification.\n");

            try{
                createNotificationChannel(context);

                Notification.Builder builder = null;

                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(rsp.url_string));
                PendingIntent pendingIntent = PendingIntent.getActivity(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

                Intent intent1, intent2;
                PendingIntent pendingIntent1, pendingIntent2;

                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                    builder = new Notification.Builder(context, CHANNEL_ID)
                            .setContentTitle(rsp.title_string)
                            .setContentText(rsp.body_string)
                            .setPriority(Notification.PRIORITY_DEFAULT)
                            .setContentIntent(pendingIntent);
                    if (rsp.icon_main != null){
                        builder.setSmallIcon(rsp.icon_main);
                    }else{
                        builder.setSmallIcon(R.mipmap.ic_launcher);
//                        builder.setSmallIcon(0x12345678);
                    }
                    if (rsp.image_main != null){
                        builder.setLargeIcon(rsp.image_main);
                    }
                    if (rsp.button_label[0] != null && !rsp.button_label[0].isEmpty()){
                        intent1 = new Intent(Intent.ACTION_VIEW, Uri.parse(rsp.button_url[0]));
                        pendingIntent1 = PendingIntent.getActivity(context, 0, intent1, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
                        builder.addAction(R.mipmap.ic_launcher, rsp.button_label[0], pendingIntent1); // Replace with your action icon
//                        builder.addAction(0x12345678, rsp.button_label[0], pendingIntent1); // Replace with your action icon
                    }
                    if (rsp.button_label[1] != null && !rsp.button_label[1].isEmpty()){
                        intent2 = new Intent(Intent.ACTION_VIEW, Uri.parse(rsp.button_url[1]));
                        pendingIntent2 = PendingIntent.getActivity(context, 0, intent2, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
                        builder.addAction(R.mipmap.ic_launcher, rsp.button_label[1], pendingIntent2); // Replace with your action icon
//                        builder.addAction(0x12345678, rsp.button_label[1], pendingIntent2); // Replace with your action icon
                    }
                }

                NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
                notificationManager.notify(rsp.id, builder.build());
            } catch (Exception e){
                Log.d("sss", "-----------------show notification error.\n" + e.getMessage());
            }

            Log.d("sss", "-----------------show notification end.\n");
        }

        private static void createNotificationChannel(Context context) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                CharSequence name = CHANNEL_NAME;
                String description = CHANNEL_DESCRIPTION;
                int importance = NotificationManager.IMPORTANCE_DEFAULT;
                NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);
                channel.setDescription(description);
                NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
                notificationManager.createNotificationChannel(channel);
            }
        }
    }
}
