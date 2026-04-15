package com.bizzra.dumper

import android.content.Context

class MenuChanges(var ctx: Context?) {

    external fun Changes(context: Context?, fInt: Int, fName: String?, value: Int, bool: Boolean, string: String?)

    fun changeFeatureInt(featureName: String?, featureNum: Int, value: Int) {
        Changes(ctx, featureNum, featureName, value, false, null)
    }

    fun changeFeatureString(featureName: String?, featureNum: Int, str: String?) {
        Changes(ctx, featureNum, featureName, 0, false, str)
    }

    fun changeFeatureBool(featureName: String?, featureNum: Int, bool: Boolean) {
        Changes(ctx, featureNum, featureName, 0, bool, null)
    }
}