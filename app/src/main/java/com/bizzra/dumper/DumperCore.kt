package com.bizzra.dumper

import android.content.Context

object DumperCore {
    init {
        System.loadLibrary("Dumper")
    }

    @JvmStatic
    external fun CheckOverlayPermission(context: Context)

    @JvmStatic
    fun Start(context: Context) {
        CheckOverlayPermission(context)
    }
}