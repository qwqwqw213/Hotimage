package org.qtproject.example;

import org.qtproject.qt5.android.bindings.QtActivity;
import java.lang.System;
import java.lang.Object;
import android.view.OrientationEventListener;
import android.os.Build;
import android.os.Bundle;
import android.view.WindowInsets;
import android.view.DisplayCutout;
import android.content.Intent;
import android.provider.Settings;
import android.content.Context;
import android.net.Uri;
import android.content.BroadcastReceiver;

import android.view.WindowManager;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

// output
import android.util.Log;

// wifi
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager.LocalOnlyHotspotReservation;
import android.os.Handler;
import java.lang.reflect.Method;
// activity
import android.content.IntentFilter;
import android.app.Instrumentation;
import android.content.ComponentName;

import android.content.BroadcastReceiver;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.util.Log;
import java.util.HashMap;
import android.app.PendingIntent;
import java.util.Iterator;

public class function extends QtActivity {

    private static final String TAG = "MainActivity";
    public static function m_self;

    private Context context;
//    private SensorManager mSensorManager;
//    private Sensor mSensorMagnetic, mAccelerometer;

    public function() {
        m_self = this;
        System.out.println("java -> init");
    }

    public int testGetInt() {
        return 0xff;
    }

    private final int MESSAGE_HOTSPOT_UPDATE = 0;
    public native static void AndroidMessage(int msg, int wParam, int lParam);

    private WifiManager wifiManager;
    WifiConfiguration wifiConfig;
    WifiManager.LocalOnlyHotspotReservation hotspotReservation;

    private void initHotspot() {
        wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);

        System.out.println("java -> ap open state: " + isApOpen());

        wifiConfig = null;
        wifiConfig = apConfiguration();
    }

    public void turnToHotspot() {
        System.out.println("java -> turn to hotspot");
        Intent intent = new Intent();
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.setAction("android.intent.action.MAIN");
        ComponentName cn = new ComponentName("com.android.settings", "com.android.settings.Settings$TetherSettingsActivity");
        intent.setComponent(cn);
        startActivity(intent);
    }

    public String apSSID() {
        return wifiConfig == null ? "" : wifiConfig.SSID;
    }

    public String apPassword() {
        return wifiConfig == null ? "" : wifiConfig.preSharedKey;
    }

    public boolean isApOpen() {
        try
        {
            final Method method = wifiManager.getClass().getDeclaredMethod("isWifiApEnabled");
            method.setAccessible(true); //in the case of visibility change in future APIs
            return (Boolean) method.invoke(wifiManager);
        }
        catch (final Throwable ignored)
        {
        }

        return false;
    }

    private WifiConfiguration apConfiguration() {
        if( android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.O )
        {
            try {
              Method method = wifiManager.getClass().getMethod("getWifiApConfiguration");
              return (WifiConfiguration) method.invoke(wifiManager);
            } catch (Exception e) {
              Log.e(this.getClass().toString(), "", e);
              return null;
            }
        }
        else {
            return null;
        }
    }

    /*
    private void lowSdkTurnOnOffHotspot() {
        try {
            // if WiFi is on, turn it off
            boolean status = isApOpen();
            System.out.println("java -> current hotspot status: " + status);
            if( status ) {
                wifiManager.setWifiEnabled(false);
            }
            Method method = wifiManager.getClass().getMethod("setWifiApEnabled",
                                                             WifiConfiguration.class,
                                                             boolean.class);
            method.setAccessible(true);
            boolean flag = (Boolean) method.invoke(wifiManager, wifiConfig, !status);
            if( flag && !status ) {
                wifiConfig = apConfiguration();
                AndroidMessage(MESSAGE_HOTSPOT_UPDATE, 0, 0);
            }
            else {
                AndroidMessage(MESSAGE_HOTSPOT_UPDATE, 0, 0);
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void turnOnHotspot() {
        // wifi
        if( android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.O ) {
            System.out.println("java -> turn on hotspot(sdk < Build.VERSION_CODES.O)");

            // 版本小于Build.VERSION_CODES.O(sdk api < 26, android 8)
            // 打开热点需要申请ACTION_MANAGE_WRITE_SETTINGS权限
            if( !Settings.System.canWrite(this) ) {
                Intent intent = new Intent(android.provider.Settings.ACTION_MANAGE_WRITE_SETTINGS);
                intent.setData(Uri.parse("package:" + context.getPackageName()));
                startActivityForResult(intent, REQUEST_WHITE_SETTINGS);
            }
            else {
                lowSdkTurnOnOffHotspot();
            }
        }
        else
        {
            System.out.println("java -> turn on hotspot");
            wifiManager.startLocalOnlyHotspot(new WifiManager.LocalOnlyHotspotCallback() {
                @Override
                public void onStarted(WifiManager.LocalOnlyHotspotReservation reservation) {
                    super.onStarted(reservation);
                    hotspotReservation = reservation;
                    if( hotspotReservation != null ) {
                        wifiConfig = hotspotReservation.getWifiConfiguration();

                        Log.v("DANG", "THE PASSWORD IS: "
                            + wifiConfig.preSharedKey
                            + " \n SSID is : "
                            + wifiConfig.SSID);
                    }
                    else {
                        System.out.println("java -> on started, WifiManager.LocalOnlyHotspotReservation is null");
                    }
                }

              @Override
              public void onStopped() {
                super.onStopped();
                Log.v("DANG", "Local Hotspot Stopped");
              }

              @Override
              public void onFailed(int reason) {
                super.onFailed(reason);
                Log.v("DANG", "Local Hotspot failed to start");
              }
            }, new Handler());
        }
    }

    private final int REQUEST_WHITE_SETTINGS = 1;
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);
        System.out.println("java -> onActivityResult\n"
                           + " requestCode: " + requestCode);

        switch(requestCode)
        {
        case REQUEST_WHITE_SETTINGS:
            System.out.println("java -> REQUEST_WHITE_SETTINGS status: " + Settings.System.canWrite(this));
            if( Settings.System.canWrite(this) ) {
                lowSdkTurnOnOffHotspot();
            }
            break;
        default: break;
        }
    }
    */

    // usb
    public native static void UsbConnect(int fd, String devName);
    public native static void UsbDisconnect(int fd);
    private UsbManager mUsbManager;
    private UsbDeviceConnection mUsbConnection;

    private PendingIntent mPermissionIntent;
    private static final String ACTION_USB_PERMISSION =
        "com.android.example.USB_PERMISSION";
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if( ACTION_USB_PERMISSION.equals(action) ) {
                synchronized (this) {
                    UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if(device != null){
                          //call method to set up device communication
                          System.out.println("java -> usb insert");
                       }
                    }
                    else {
                        Log.d(TAG, "permission denied for device " + device);
                    }
                }
            }
            else if( UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action) ) {
                // 设备插入
                final UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                updateUsbDevice(device);
            }
            else if( UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action) ) {
                // 设备解除
                final UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null) {
                    System.out.println("java -> usb remove: " + device.getVendorId() + device.getProductId() + device.getProductName());
                    UsbDisconnect(mUsbConnection.getFileDescriptor());
                    mUsbConnection = null;
                }
                else {
                    System.out.println("java -> ACTION_USB_DEVICE_DETACHED device is null");
                }
            }
        }
    };

    public void uvc() {
        System.out.println("java -> uvc init");

        mUsbManager = (UsbManager)context.getSystemService(Context.USB_SERVICE);
        mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);

        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        registerReceiver(mUsbReceiver, filter);

        HashMap<String, UsbDevice> list = mUsbManager.getDeviceList();
        Iterator<UsbDevice> deviceIterator = list.values().iterator();
        while(deviceIterator.hasNext()){
            UsbDevice device = deviceIterator.next();
            System.out.println("java -> inserted usb: " + device.getVendorId() + device.getProductId() + device.getProductName());
            updateUsbDevice(device);
        }
    }

    public void updateUsbDevice(UsbDevice dev) {
        if( dev != null ) {
            if( !mUsbManager.hasPermission(dev) ) {
                System.out.println("java -> request device permission");
                mUsbManager.requestPermission(dev, mPermissionIntent);
            }
            mUsbConnection = mUsbManager.openDevice(dev);
            int fd = mUsbConnection.getFileDescriptor();
            UsbConnect(fd, dev.getProductName());
        }
    }
    // usb

//    private static OrientationEventListener m_OrientationListener;

//    public native static void orientationChanged(int orientation);

    public native static void safeArea(int top, int left, int bottom, int right);

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        System.out.println("java -> on attached to window");

        if( android.os.Build.VERSION.SDK_INT >= 28 )
        {
//            // 获取屏幕安全边界
            WindowInsets windowInsets = getWindow().getDecorView().getRootWindowInsets();
            if( windowInsets != null )
            {
                DisplayCutout displayCutout = windowInsets.getDisplayCutout();
                if( displayCutout != null ) {
                    int left = displayCutout.getSafeInsetLeft();
                    int top = displayCutout.getSafeInsetTop();
                    int right = displayCutout.getSafeInsetRight();
                    int bottom = displayCutout.getSafeInsetBottom();
                    safeArea(top, left, bottom, right);
                }
                else
                {
                    System.out.println("java -> displayCutout is null");
                }
            }
            else
            {
                System.out.println("java -> windowInsets is null");
            }
        }
   }

    @Override
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        System.out.println("java -> on craete");
        this.context = this;

        // 屏幕常亮
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // 刘海屏处理
        System.out.println("java -> android version: " + android.os.Build.VERSION.SDK_INT);
        if( android.os.Build.VERSION.SDK_INT >= 28 )
        {
            System.out.println("java -> display cutouts, set full screen");
            WindowManager.LayoutParams params = getWindow().getAttributes();
            params.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
            getWindow().setAttributes(params);
        }

        // wifi manager
        initHotspot();

        /*
        mSensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
        //获取Sensor
        mSensorMagnetic = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        m_OrientationListener = new OrientationEventListener(this,
            SensorManager.SENSOR_DELAY_NORMAL) {

            @Override
            public void onOrientationChanged(int orientation) {
                System.out.println("java -> onOrientationChanged: " + orientation);
                if (orientation == OrientationEventListener.ORIENTATION_UNKNOWN) {
                    return;  //手机平放时，检测不到有效的角度
                }
                //可以根据不同角度检测处理，这里只检测四个角度的改变
                if (orientation > 350 || orientation < 10) {
                    //0度
                    orientation = 0;
                }
                else if (orientation > 80 && orientation < 100) {
                    //90度
                    orientation = 90;
                }
                else if (orientation > 170 && orientation < 190) {
                    //180度
                    orientation = 180;
                }
                else if (orientation > 260 && orientation < 280) {
                    //270度
                    orientation = 270;
                }
                else {
                    return;
                }
                orientationChanged(orientation);
            }
       };
       */
    }

    /*
    float[] geomagnetic = new float[3];
    private final SensorEventListener mSensorListener = new SensorEventListener() {

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {

        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                geomagnetic = event.values;
            }
            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                float x = event.values[SensorManager.DATA_X];
                float y = event.values[SensorManager.DATA_Y];
                float z = event.values[SensorManager.DATA_Z];
                relayout(x, y, z);
            }
        }
    };

    int oldRotation = 0;
    protected void relayout(float x, float y, float z) {
        if (x > -2.5 && x <= 2.5 && y > 7.5 && y <= 10 && oldRotation != 270) {
            oldRotation = 270;
        } else if (x > 7.5 && x <= 10 && y > -2.5 && y <= 2.5 && oldRotation != 0) {
            oldRotation = 0;
        } else if (x > -2.5 && x <= 2.5 && y > -10 && y <= -7.5 && oldRotation != 90) {
            oldRotation = 90;
        } else if (x > -10 && x <= -7.5 && y > -2.5 && y < 2.5 && oldRotation != 180) {
            oldRotation = 180;
        } else {
            return;
        }
        System.out.println("java -> rotation changed: " + oldRotation);
    }
    */

    @Override
    protected void onDestroy() {
        System.out.println("java destroy");
        super.onDestroy();
        unregisterReceiver(mUsbReceiver);
//        m_OrientationListener.disable();
    }

    @Override
    protected void onResume() {
//        mSensorManager.registerListener(mSensorListener, mSensorMagnetic, SensorManager.SENSOR_DELAY_NORMAL);
//        mSensorManager.registerListener(mSensorListener, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
        super.onResume();
    }

    @Override
    protected void onStop() {
//        mSensorManager.unregisterListener(mSensorListener, mSensorMagnetic);
//        mSensorManager.unregisterListener(mSensorListener, mAccelerometer);
        super.onStop();
    }
}
