package com.bizzra.dumper

import android.app.Activity
import android.content.Context
import android.os.Bundle

class MainActivity : Activity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        DumperCore.CheckOverlayPermission(this)
    }
}