package com.bizzra.dumper

import android.app.Service
import android.content.Intent
import android.graphics.Color
import android.graphics.PixelFormat
import android.graphics.drawable.GradientDrawable
import android.os.Build
import android.os.IBinder
import android.text.TextUtils
import android.util.TypedValue
import android.view.Gravity
import android.view.MotionEvent
import android.view.View
import android.view.View.OnTouchListener
import android.view.ViewGroup
import android.view.WindowManager
import android.webkit.WebView
import android.widget.Button
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.RelativeLayout
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import kotlin.math.roundToInt

class FloatingService : Service() {
    // --- Window Management ---
    var windowManager: WindowManager? = null
    var windowParams: WindowManager.LayoutParams? = null

    // --- Core Layout Views ---
    var rootFrame: FrameLayout? = null
    var rootContainer: RelativeLayout? = null
    var collapsedView: RelativeLayout? = null // The floating icon
    var expandedMenuView: LinearLayout? = null // The main menu body

    // --- Menu Content Views ---
    var menuScrollView: ScrollView? = null
    var featuresLayout: LinearLayout? = null // Holds the generated toggles/buttons
    var settingsLayout: LinearLayout? = null
    var activeCollapseLayout: LinearLayout? = null // Reference for nested accordion views

    // --- Layout Parameters ---
    var scrollParams: LinearLayout.LayoutParams? = null
    var expandedScrollParams: LinearLayout.LayoutParams? = null

    // --- Layout & Dimensions ---
    var menuWidth: Int = 300
    var menuHeight: Int = 200
    var menuCornerRadius: Float = 30f
    var iconSize: Int = 50
    var iconOpacity: Float = 0.8f

    val featuresBuilder = Features(this)

    external fun Title(textView: TextView?)

    external fun SubTitle(textView: TextView?)

    external fun Icon(): String?
    external fun CloseThreads()

    external fun GetFeatureList(): Array<String>?

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onStartCommand(intent: Intent?, n: Int, n2: Int): Int {
        return START_NOT_STICKY
    }

    override fun onTaskRemoved(intent: Intent?) {
        super.onTaskRemoved(intent)
        CloseThreads()
        stopSelf()
    }

    override fun onDestroy() {
        super.onDestroy()
        if (rootFrame != null) {
            windowManager?.removeView(rootFrame)
        }

        CloseThreads()
    }

    override fun onCreate() {
        super.onCreate()

        setupRootContainers()
        setupExpandedMenuContainer()
        val wView = createMenuIcon()
        val titleText = createTitleContainer()
        val heading = createSubTitleText()
        setupFeatureScrollLayout()
        val bottomButtons = createBottomButtons()
        setupWindowParameters()

        assembleMenuViews(wView, titleText, heading, bottomButtons)
        loadFeatures()
    }

    private fun toggleMenuFocus(isFocusable: Boolean) {
        windowParams?.let {
            it.flags = if (isFocusable) {
                WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
            } else {
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
            }
            windowManager?.updateViewLayout(rootFrame, it)
        }
    }

    private fun setupRootContainers() {
        rootFrame = FrameLayout(this)
        rootContainer = RelativeLayout(this)

        collapsedView = RelativeLayout(this)
        collapsedView!!.visibility = View.VISIBLE
        collapsedView!!.setAlpha(iconOpacity)
    }

    private fun setupExpandedMenuContainer() {
        //********** The box of the mod menu **********
        expandedMenuView = LinearLayout(this)
        expandedMenuView!!.visibility = View.GONE
        expandedMenuView!!.setBackgroundColor(featuresBuilder.colorBgMenu)
        expandedMenuView!!.orientation = LinearLayout.VERTICAL
        expandedMenuView!!.setLayoutParams(
            LinearLayout.LayoutParams(
                convertSizeToDp(menuWidth.toFloat()),
                ViewGroup.LayoutParams.WRAP_CONTENT
            )
        )
        val gdMenuBody = GradientDrawable()
        gdMenuBody.setCornerRadius(menuCornerRadius)
        gdMenuBody.setColor(featuresBuilder.colorBgMenu)
        gdMenuBody.setStroke(3, featuresBuilder.colorAccent) // Match neon accent
        expandedMenuView!!.background = gdMenuBody // Actually apply the background to show borders!
    }

    private fun createMenuIcon(): WebView {
        //********** The icon in Webview to open mod menu **********
        val wView = WebView(this)
        wView.layoutParams = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
        val applyDimension2 = TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            iconSize.toFloat(),
            resources.displayMetrics
        ).toInt() //Icon size

        wView.layoutParams.height = applyDimension2
        wView.layoutParams.width = applyDimension2

        val CircularMenuImg = "<html>" +
                "<head>" +
                "<style>" +
                "   img {" +
                "       border-radius: 50%;" +
                "       height: auto;" +
                "   }" +
                "</style>" +
                "</head>" +
                "<body style=\"margin: 0; padding: 0\">" +
                "<img src=\"" + Icon() + "\" width=\"" + iconSize + "\" height=\"" + iconSize + "\" >" +
                "</body>" +
                "</html>"

        wView.loadData(CircularMenuImg, "text/html", "utf-8")
        wView.setBackgroundColor(0x00000000)
        wView.setAlpha(iconOpacity)
        wView.setOnTouchListener(onTouchListener())

        return wView
    }

    private fun createTitleContainer(): RelativeLayout {
        //********** Title text & Settings Icon **********
        val titleText = RelativeLayout(this)
        titleText.setPadding(10, 5, 10, 5)
        titleText.setVerticalGravity(16)

        // Title TextView
        val title = TextView(this)
        title.setTextColor(featuresBuilder.colorTextMain)
        title.textSize = 19.0f
        title.setGravity(Gravity.CENTER)

        val rlTitle = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
        rlTitle.addRule(RelativeLayout.CENTER_HORIZONTAL)
        title.setLayoutParams(rlTitle)
        Title(title)
        titleText.addView(title)
        return titleText
    }

    private fun createSubTitleText(): TextView {
        val heading = TextView(this)
        heading.ellipsize = TextUtils.TruncateAt.MARQUEE
        heading.marqueeRepeatLimit = -1
        heading.setSingleLine(true)
        heading.setSelected(true)
        heading.setTextColor(featuresBuilder.colorTextMain)
        heading.textSize = 10.0f
        heading.setGravity(Gravity.CENTER)
        heading.setPadding(0, 0, 0, 5)
        SubTitle(heading)

        return heading
    }

    private fun setupFeatureScrollLayout() {
        //********** Mod menu feature list **********
        menuScrollView = ScrollView(this)
        scrollParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, convertSizeToDp(menuHeight.toFloat()))
        expandedScrollParams = LinearLayout.LayoutParams(expandedMenuView!!.layoutParams)
        expandedScrollParams!!.weight = 1.0f

        menuScrollView!!.setLayoutParams(scrollParams)
        menuScrollView!!.setBackgroundColor(Color.TRANSPARENT)

        featuresLayout = LinearLayout(this)
        featuresLayout!!.orientation = LinearLayout.VERTICAL

        settingsLayout = LinearLayout(this)
        settingsLayout!!.orientation = LinearLayout.VERTICAL

        menuScrollView!!.addView(featuresLayout)
    }

    private fun createBottomButtons(): RelativeLayout {
        val relativeLayout = RelativeLayout(this)
        relativeLayout.setPadding(10, 3, 10, 3)
        relativeLayout.setVerticalGravity(Gravity.CENTER)

        //********** Hide/Kill button **********
        val lParamsHideBtn = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
        lParamsHideBtn.addRule(RelativeLayout.ALIGN_PARENT_LEFT)

        val hideBtn: Button = Button(this)
        hideBtn.setLayoutParams(lParamsHideBtn)
        hideBtn.setBackgroundColor(Color.TRANSPARENT)
        hideBtn.text = "HIDE"
        hideBtn.setTextColor(featuresBuilder.colorTextMain)
        hideBtn.setOnClickListener({ view ->
            collapsedView?.visibility = View.VISIBLE
            collapsedView?.setAlpha(0f)
            expandedMenuView?.visibility = View.GONE
            toggleMenuFocus(false)
            Toast.makeText(
                view.context,
                "Menu hidden. Remember the icon position",
                Toast.LENGTH_LONG
            ).show()
        })
        hideBtn.setOnLongClickListener({ view ->
            Toast.makeText(view.context, "Menu Terminated", Toast.LENGTH_LONG).show()
            this@FloatingService.stopSelf()
            false
        })

        //********** Close button **********
        val lParamsCloseBtn = RelativeLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT
        )
        lParamsCloseBtn.addRule(RelativeLayout.ALIGN_PARENT_RIGHT)

        val closeBtn = Button(this)
        closeBtn.setLayoutParams(lParamsCloseBtn)
        closeBtn.setBackgroundColor(Color.TRANSPARENT)
        closeBtn.text = "MINIMIZE"
        closeBtn.setTextColor(featuresBuilder.colorTextMain)
        closeBtn.setOnClickListener({ view ->
            collapsedView?.visibility = View.VISIBLE
            collapsedView?.setAlpha(iconOpacity)
            expandedMenuView?.visibility = View.GONE
            toggleMenuFocus(false)
        })

        relativeLayout.addView(hideBtn)
        relativeLayout.addView(closeBtn)

        return relativeLayout
    }

    private fun setupWindowParameters() {
        //********** Params **********
        windowParams = WindowManager.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT,
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) 2038 else 2002,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
            PixelFormat.TRANSLUCENT
        )
        windowParams!!.gravity = Gravity.LEFT or Gravity.TOP
        windowParams!!.x = 0
        windowParams!!.y = 100
    }

    private fun assembleMenuViews(wView: WebView, titleText: RelativeLayout, heading: TextView, bottomButtons: RelativeLayout) {
        //********** Adding view components **********
        rootFrame!!.addView(rootContainer)
        rootContainer!!.addView(collapsedView)
        rootContainer!!.addView(expandedMenuView)

        collapsedView!!.addView(wView) //Gif Icon
        expandedMenuView!!.addView(titleText) //Title Container
        expandedMenuView!!.addView(heading) //Heading
        expandedMenuView!!.addView(menuScrollView) //Scroll view for hacks
        expandedMenuView!!.addView(bottomButtons) //Hide Button and Close Button container

        windowManager = getSystemService(WINDOW_SERVICE) as WindowManager
        windowManager?.addView(rootFrame, windowParams)

        rootFrame?.setOnTouchListener(onTouchListener())
    }

    private fun loadFeatures() {
        featuresLayout!!.removeAllViews()
        GetFeatureList()?.let {
            FeatureList(it, featuresLayout!!)
        }
    }

    private fun onTouchListener(): OnTouchListener {
        return object : OnTouchListener {
            private var initialTouchX = 0f
            private var initialTouchY = 0f
            private var initialX = 0
            private var initialY = 0

            override fun onTouch(view: View?, motionEvent: MotionEvent): Boolean {
                val collapsed = collapsedView ?: return false
                val expanded = expandedMenuView ?: return false
                val wm = windowManager ?: return false
                val lp = windowParams ?: return false

                when (motionEvent.action) {
                    MotionEvent.ACTION_DOWN -> {
                        initialX = lp.x
                        initialY = lp.y
                        initialTouchX = motionEvent.rawX
                        initialTouchY = motionEvent.rawY
                        return true
                    }

                    MotionEvent.ACTION_UP -> {
                        val rawX = (motionEvent.rawX - initialTouchX).toInt()
                        val rawY = (motionEvent.rawY - initialTouchY).toInt()
                        expanded.setAlpha(1f)
                        collapsed.setAlpha(1f)
                        if (rawX < 10 && rawY < 10 && isViewCollapsed) {
                            collapsed.visibility = View.GONE
                            expanded.visibility = View.VISIBLE
                            toggleMenuFocus(true)
                        }
                        return true
                    }

                    MotionEvent.ACTION_MOVE -> {
                        expanded.setAlpha(0.5f)
                        collapsed.setAlpha(0.5f)
                        lp.x = initialX + ((motionEvent.rawX - initialTouchX).toInt())
                        lp.y = initialY + ((motionEvent.rawY - initialTouchY).toInt())
                        wm.updateViewLayout(rootFrame, lp)
                        return true
                    }

                    else -> return false
                }
            }
        }
    }

    private fun FeatureList(listFT: Array<String>, linearLayout: LinearLayout) {
        var currentLayout = linearLayout
        var featNum: Int
        var subFeat = 0
        val llBak = linearLayout

        for (i in listFT.indices) {
            var switchedOn = false
            var feature = listFT[i]
            if (feature.contains("True_")) {
                switchedOn = true
                feature = feature.replaceFirst("True_".toRegex(), "")
            }

            currentLayout = llBak
            if (feature.contains("CollapseAdd_")) {
                currentLayout = activeCollapseLayout ?: llBak
                feature = feature.replaceFirst("CollapseAdd_".toRegex(), "")
            }
            val str: Array<String?> =
                feature.split("_".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()

            if (TextUtils.isDigitsOnly(str[0]) || str[0]!!.matches("-[0-9]*".toRegex())) {
                featNum = str[0]!!.toInt()
                feature = feature.replaceFirst((str[0] + "_").toRegex(), "")
                subFeat++
            } else {
                featNum = i - subFeat
            }
            val strSplit: Array<String?> =
                feature.split("_".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()
            when (strSplit[0]) {
                "Toggle" -> currentLayout.addView(featuresBuilder.Switch(featNum, strSplit[1], switchedOn))
                "SeekBar" -> currentLayout.addView(
                    featuresBuilder.SeekBar(
                        featNum,
                        strSplit[1],
                        strSplit[2]!!.toInt(),
                        strSplit[3]!!.toInt()
                    )
                )

                "Button" -> currentLayout.addView(featuresBuilder.Button(featNum, strSplit[1]))
                "ButtonOnOff" -> currentLayout.addView(
                    featuresBuilder.ButtonOnOff(
                        featNum,
                        strSplit[1]!!,
                        switchedOn
                    )
                )

                "Spinner" -> {
                    currentLayout.addView(featuresBuilder.Spinner(featNum, strSplit[1], strSplit[2]!!))
                }

                "InputField" -> {
                    currentLayout.addView(featuresBuilder.InputField(featNum, strSplit[1], strSplit[2]!!))
                }

                "InputText" -> {
                    currentLayout.addView(featuresBuilder.InputText(featNum, strSplit[1]))
                }

                "InputValue" -> {
                    if (strSplit.size == 3) {
                        currentLayout.addView(
                            featuresBuilder.InputNum(
                                featNum,
                                strSplit[2],
                                strSplit[1]!!.toIntOrNull() ?: 0
                            )
                        )
                    } else if (strSplit.size == 2) {
                        currentLayout.addView(featuresBuilder.InputNum(featNum, strSplit[1], 0))
                    }
                }

                "InputLValue" -> {
                    if (strSplit.size == 3) {
                        currentLayout.addView(
                            featuresBuilder.InputLNum(
                                featNum,
                                strSplit[2],
                                strSplit[1]!!.toLongOrNull() ?: 0L
                            )
                        )
                    } else if (strSplit.size == 2) {
                        currentLayout.addView(featuresBuilder.InputLNum(featNum, strSplit[1], 0L))
                    }
                }

                "CheckBox" -> currentLayout.addView(featuresBuilder.CheckBox(featNum, strSplit[1], switchedOn))
                "RadioButton" -> currentLayout.addView(
                    featuresBuilder.RadioButton(
                        featNum,
                        strSplit[1],
                        strSplit[2]!!
                    )
                )

                "Collapse" -> {
                    featuresBuilder.Collapse(currentLayout, strSplit[1])
                    subFeat++
                }

                "ButtonLink" -> {
                    subFeat++
                    currentLayout.addView(featuresBuilder.ButtonLink(strSplit[1], strSplit[2]))
                }

                "Category" -> {
                    subFeat++
                    currentLayout.addView(featuresBuilder.Category(strSplit[1]))
                }

                "RichTextView" -> {
                    subFeat++
                    currentLayout.addView(featuresBuilder.RichTextView(strSplit[1]))
                }

                "RichWebView" -> {
                    subFeat++
                    currentLayout.addView(featuresBuilder.RichWebView(strSplit[1]!!))
                }
            }
        }
    }

    fun convertSizeToDp(f: Float): Int {
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, f, resources.displayMetrics).roundToInt()
    }

    private val isViewCollapsed: Boolean
        get() = rootFrame == null || collapsedView?.visibility == View.VISIBLE
}