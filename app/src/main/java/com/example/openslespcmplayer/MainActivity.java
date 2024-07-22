package com.example.openslespcmplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import com.example.openslespcmplayer.databinding.ActivityMainBinding;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "opensles";
    // Used to load the 'openslespcmplayer' library on application startup.
    static {
        System.loadLibrary("openslespcmplayer");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText("openslespcmplayer");

        String assertDir = "music";
        AssectsUtil.loadAssetsDirfile(getApplicationContext(),assertDir);
        String appFilePath = AssectsUtil.getAppDir(getApplicationContext(),assertDir)+ File.separator;
        Log.d(TAG,"appFilePath="+appFilePath);
        String musicPath = appFilePath+"test_ffmpeg.pcm";
        play(musicPath);
    }

    /**
     * A native method that is implemented by the 'openslespcmplayer' native library,
     * which is packaged with this application.
     */


    public native int play(String filePath);
}