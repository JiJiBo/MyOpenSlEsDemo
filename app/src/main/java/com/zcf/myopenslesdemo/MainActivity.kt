package com.zcf.myopenslesdemo

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import com.afollestad.assent.Permission
import com.afollestad.assent.runWithPermissions
import com.rulerbug.bugutils.Utils.BugApp
import com.rulerbug.bugutils.Utils.BugDirUtils
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        BugApp.init(this.applicationContext, Handler(), android.os.Process.myTid())
        runWithPermissions(Permission.WRITE_EXTERNAL_STORAGE, Permission.READ_EXTERNAL_STORAGE) {
            val dirPath = BugDirUtils.getDirPathByDirName("zcf");
            val fileName = "mydream.pcm"
//            val fileName = "a.mp3"
            val file = File(dirPath, fileName)
            if (file.exists()) {
                load(file.absolutePath)
            }
        }
    }


    external fun load(str: String)

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

}
