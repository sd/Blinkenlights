package net.notso.blinkenlights;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.android.future.usb.UsbAccessory;
import com.android.future.usb.UsbManager;

public class MainActivity extends Activity implements Runnable {
    private static final String TAG = "Blinkenlights";

    private UsbManager mUsbManager;
    private PendingIntent mPermissionIntent;
    UsbAccessory mAccessory;
    ParcelFileDescriptor mFileDescriptor;
    FileInputStream mInputStream;
    FileOutputStream mOutputStream;

    private static final String ACTION_USB_PERMISSION = "com.google.android.DemoKit.action.USB_PERMISSION";

    protected static final int MODE_PROGRAM = 0;
    protected static final int MODE_MIC = 1;
    protected static int mBlinkenMode = MODE_PROGRAM;
    
    Button ledButtons[] = new Button[8];
    Button resetButton;
    Button fasterButton;
    Button slowerButton;
    Button micButton;
    private boolean mPermissionRequestPending;
    
    public static byte CMD_RESET = 0;
    public static byte CMD_PROGRAM_CHANNEL = 1;
    public static byte CMD_FASTER = 2;
    public static byte CMD_SLOWER = 3;
    public static byte CMD_MIC_DATA = 99;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mUsbManager = UsbManager.getInstance(this);
        mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(
                ACTION_USB_PERMISSION), 0);
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
        registerReceiver(mUsbReceiver, filter);

        if (getLastNonConfigurationInstance() != null) {
            mAccessory = (UsbAccessory) getLastNonConfigurationInstance();
            openAccessory(mAccessory);
        }

        setContentView(R.layout.activity_main);

        fasterButton = (Button) this.findViewById(R.id.button_faster);
        slowerButton = (Button) this.findViewById(R.id.button_slower);
        resetButton = (Button) this.findViewById(R.id.button_reset);

        micButton = (Button) this.findViewById(R.id.button_mic);

        ledButtons[0] = (Button) this.findViewById(R.id.button1);
        ledButtons[1] = (Button) this.findViewById(R.id.button2);
        ledButtons[2] = (Button) this.findViewById(R.id.button3);
        ledButtons[3] = (Button) this.findViewById(R.id.button4);
        ledButtons[4] = (Button) this.findViewById(R.id.button5);
        ledButtons[5] = (Button) this.findViewById(R.id.button6);
        ledButtons[6] = (Button) this.findViewById(R.id.button7);
        ledButtons[7] = (Button) this.findViewById(R.id.button8);
    
        fasterButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) { MainActivity.this.sendCommand(CMD_FASTER, (byte) 0, (byte) 50); }
        });
        slowerButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) { MainActivity.this.sendCommand(CMD_SLOWER, (byte) 0, (byte) 50); }
        });
        resetButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) { MainActivity.this.sendCommand(CMD_RESET, (byte) 0, (byte) 0); }
        });
        
        micButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) { 
                if (MainActivity.mBlinkenMode != MODE_MIC) {
                    MainActivity.mBlinkenMode = MODE_MIC;
                    micButton.setText("MIC!");
                    startAudioCapture();
                }
                else {
                    MainActivity.mBlinkenMode = MODE_PROGRAM;
                    micButton.setText("mic");
                    stopAudioCapture();
                }
            }
        });
        
        OnClickListener ledButtonListener = new OnClickListener() {
            @Override
            public void onClick(View v) {
                Button button = (Button) v;
                int buttonNumber = ((Integer) button.getTag(R.string.tag_button)).intValue();
                MainActivity.this.sendCommand(CMD_PROGRAM_CHANNEL, (byte) buttonNumber, (byte) 1);
            }
        };
        
        for (int i = 0; i < ledButtons.length; i++) {
            ledButtons[i].setTag(R.string.tag_button, new Integer(i));
            
            ledButtons[i].setOnClickListener(ledButtonListener);
        }
    }

    @Override
    public Object onRetainNonConfigurationInstance() {
        if (mAccessory != null) {
            return mAccessory;
        } else {
            return super.onRetainNonConfigurationInstance();
        }
    }
    
    @Override
    public void onResume() {
        super.onResume();

        Intent intent = getIntent();
        if (mInputStream != null && mOutputStream != null) {
            return;
        }

        UsbAccessory[] accessories = mUsbManager.getAccessoryList();
        UsbAccessory accessory = (accessories == null ? null : accessories[0]);
        if (accessory != null) {
            if (mUsbManager.hasPermission(accessory)) {
                openAccessory(accessory);
            } else {
                synchronized (mUsbReceiver) {
                    if (!mPermissionRequestPending) {
                        mUsbManager.requestPermission(accessory,
                                mPermissionIntent);
                        mPermissionRequestPending = true;
                    }
                }
            }
        } else {
            Log.d(TAG, "mAccessory is null");
        }
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }
    
    @Override
    public void onPause() {
        super.onPause();
        closeAccessory();
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mUsbReceiver);
        super.onDestroy();
    }
    
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbAccessory accessory = UsbManager.getAccessory(intent);
                    if (intent.getBooleanExtra(
                            UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        openAccessory(accessory);
                    } else {
                        Log.d(TAG, "permission denied for accessory "
                                + accessory);
                    }
                    mPermissionRequestPending = false;
                }
            } else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
                UsbAccessory accessory = UsbManager.getAccessory(intent);
                if (accessory != null && accessory.equals(mAccessory)) {
                    closeAccessory();
                }
            }
        }
    };


    private void openAccessory(UsbAccessory accessory) {
        mFileDescriptor = mUsbManager.openAccessory(accessory);
        if (mFileDescriptor != null) {
            mAccessory = accessory;
            FileDescriptor fd = mFileDescriptor.getFileDescriptor();
            mInputStream = new FileInputStream(fd);
            mOutputStream = new FileOutputStream(fd);
            Thread thread = new Thread(null, this, "Blinkenlights");
            thread.start();
            Log.d(TAG, "accessory opened");
        } else {
            Log.d(TAG, "accessory open fail");
        }
    }

    private void closeAccessory() {
        try {
            if (mFileDescriptor != null) {
                mFileDescriptor.close();
            }
        } catch (IOException e) {
        } finally {
            mFileDescriptor = null;
            mAccessory = null;
        }
    }

    private int composeInt(byte hi, byte lo) {
        int val = (int) hi & 0xff;
        val *= 256;
        val += (int) lo & 0xff;
        return val;
    }

    public void run() {
        int ret = 0;
        byte[] buffer = new byte[16384];
        int i;

        while (ret >= 0) {
            try {
                ret = mInputStream.read(buffer);
            } catch (IOException e) {
                break;
            }

            i = 0;
            while (i < ret) {
                int len = ret - i;

                switch (buffer[i]) {
                case 0x1:
                    if (len >= 3) {
                        Message m = Message.obtain(mHandler, MSG_EXAMPLE_1);
                        m.obj = new String(buffer, i + 1, 2);
                        mHandler.sendMessage(m);
                    }
                    i += 3;
                    break;

                case 0x4:
                    if (len >= 3) {
                        Message m = Message.obtain(mHandler, MSG_EXAMPLE_1);
                        m.obj = new Integer(composeInt(buffer[i + 1], buffer[i + 2]));
                        mHandler.sendMessage(m);
                    }
                    i += 3;
                    break;

                default:
                    Log.d(TAG, "unknown msg: " + buffer[i]);
                    i = len;
                    break;
                }
            }

        }
    }

    protected static final int MSG_AUDIO_DATA = 1;
    protected static final int MSG_EXAMPLE_1 = 101;
    protected static final int MSG_EXAMPLE_2 = 102;

    protected static final int DOWNSAMPLE_FACTOR = 4;
    protected static final int DATA_SIZE = 128;

    protected static byte data8[] = new byte[DATA_SIZE];
    protected static int data_counter = 0;
    
    Handler mHandler = new Handler() {
        
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_AUDIO_DATA:
                if (MainActivity.mBlinkenMode == MODE_MIC) {
                  short data16[] = (short[]) msg.obj;
                  
//                  byte data8[] = new byte[data16.length];
//                  for(int i = 0; i < data16.length; i++) {
//                      data8[i] = (byte) (data16[i]);
//                  }
//                  sendData(CMD_MIC_DATA, data8);
                    
//                    Log.e(TAG, "Appending " + data16.length + " to " + data_counter);
                    for (int i = 0; i < data16.length && (data_counter < DATA_SIZE); i += DOWNSAMPLE_FACTOR) {
                        data8[data_counter] = (byte) (data16[i] / 256);
                        data_counter++;
                    }
                    if (data_counter >= DATA_SIZE) {
                        sendData(CMD_MIC_DATA, data8);
                        data_counter = 0;
                    }
                }
                break;
                
            case MSG_EXAMPLE_1:
//                SwitchMsg o = (SwitchMsg) msg.obj;
//                handleSwitchMessage(o);
                break;

            case MSG_EXAMPLE_2:
//                TemperatureMsg t = (TemperatureMsg) msg.obj;
//                handleTemperatureMessage(t);
                break;

            }
        }
    };

    public void sendCommand(byte command, byte target, byte value) {
        Log.e(TAG, "Data " + value);
        byte[] buffer = new byte[3];
        if (value > 255)
            value = (byte) 255;

        buffer[0] = command;
        buffer[1] = target;
        buffer[2] = value;
        if (mOutputStream != null && buffer[1] != -1) {
            try {
                mOutputStream.write(buffer);
                mOutputStream.flush();
            } 
            catch (IOException e) {
                Log.e(TAG, "write failed", e);
            }
        }
    }

    public void sendData(byte command, byte[] data) {
        if (data.length > 256) {
            Log.e(TAG, "Too much data!!! " + data.length);
            return;
        }

//        byte[] buffer = new byte[6];
//        buffer[0] = command;
//        
//        if (mOutputStream != null && data.length > 0) {
//          try {
//            int data_sent = 0;
//            while (data_sent < data.length) {
//              buffer[1] = 4;
//              buffer[2] = data[data_sent + 0];
//              buffer[3] = data[data_sent + 1];
//              buffer[4] = data[data_sent + 2];
//              buffer[5] = data[data_sent + 3];
//              data_sent += 4;
//              mOutputStream.write(buffer);
//              mOutputStream.flush();
//            }
//          } 
//          catch (IOException e) {
//            Log.e(TAG, "write failed", e);
//          }
//        }
        
        byte[] buffer = new byte[data.length + 2];

        buffer[0] = command;
        buffer[1] = (byte) (data.length - 1);
        for(int i = 0; i < data.length; i++) {
            buffer[i + 2] = data[i];
        }
        if (mOutputStream != null && data.length > 0) {
            try {
                mOutputStream.write(buffer);
                mOutputStream.flush();
            } 
            catch (IOException e) {
                Log.e(TAG, "write failed", e);
            }
        }
    }

    protected AudioCaptureThread mAudioThread = null;
    protected void startAudioCapture() {
        if (mAudioThread == null || mAudioThread.stopped) {
            mAudioThread = new AudioCaptureThread();
        }
    }
    protected void stopAudioCapture() {
        if (mAudioThread != null) {
            mAudioThread.endCapture();
            mAudioThread = null;
        }
    }
    private class AudioCaptureThread extends Thread {
        protected static final int SAMPLE_RATE = 11025;
        
        private boolean stopped = false;

        private AudioCaptureThread() {
            start();
        }

        @Override
        public void run() {
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
            AudioRecord recorder = null;

            try {
                int n = AudioRecord.getMinBufferSize(SAMPLE_RATE, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);

                recorder = new AudioRecord(AudioSource.CAMCORDER, SAMPLE_RATE, AudioFormat.CHANNEL_IN_MONO,
                    AudioFormat.ENCODING_PCM_16BIT, n * 10);

                recorder.startRecording();

                short[][] buffers = new short[256][256];
                int bufferIndex = 0;
                while (!stopped) {
                    short[] buffer = buffers[bufferIndex++ % buffers.length];

                    n = recorder.read(buffer, 0, buffer.length);

                    Message m = Message.obtain(mHandler, MainActivity.MSG_AUDIO_DATA);
                    m.obj = buffer;
                    mHandler.sendMessage(m);
                }
            }
            catch (Throwable x) {
                Log.e(TAG, "Error reading voice audio", x);
            }
            finally {
                recorder.stop();
                recorder.release();
                stopped = true;
                Log.e(TAG, "Done Recording");
            }
        }

        private void endCapture() {
            stopped = true;
        }

    }

}
