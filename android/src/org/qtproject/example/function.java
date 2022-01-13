package org.qtproject.example;

import org.qtproject.qt5.android.bindings.QtActivity;
import java.lang.System;
import java.lang.Object;
import android.view.OrientationEventListener;
import android.os.Build;
import android.os.Bundle;
import android.view.WindowInsets;
import android.view.DisplayCutout;

import android.content.Context;

import android.view.WindowManager;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

public class function extends QtActivity {

    private static function m_self;

    private Context context;
//    private SensorManager mSensorManager;
//    private Sensor mSensorMagnetic, mAccelerometer;

    public function() {
        m_self = this;
        System.out.println("java -> init");
    }

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
