package org.yorgl.ui

data class YorRect(val x: Float, val y: Float, val width: Float, val height: Float) {
    fun contains(px: Float, py: Float): Boolean = px >= x && py >= y && px <= x + width && py <= y + height
}

data class YorColor(val r: Float, val g: Float, val b: Float, val a: Float = 1f) {
    companion object {
        val Transparent = YorColor(0f, 0f, 0f, 0f)
        val White = YorColor(1f, 1f, 1f, 1f)
    }
}

data class YorSize(val width: Float, val height: Float)

interface YorCanvas {
    fun rect(bounds: YorRect, color: YorColor)
    fun text(value: String, bounds: YorRect, color: YorColor)
}

interface YorNode {
    var bounds: YorRect
    fun measure(maxWidth: Float, maxHeight: Float): YorSize
    fun layout(bounds: YorRect) {
        this.bounds = bounds
    }
    fun render(canvas: YorCanvas)
    fun pointerDown(x: Float, y: Float): Boolean = false
}

class YorBox(
    private val background: YorColor = YorColor.Transparent,
    private val padding: Float = 0f,
    private val children: MutableList<YorNode> = mutableListOf(),
) : YorNode {
    override var bounds = YorRect(0f, 0f, 0f, 0f)

    fun add(node: YorNode): YorBox {
        children += node
        return this
    }

    override fun measure(maxWidth: Float, maxHeight: Float): YorSize {
        var height = padding
        var width = 0f
        for (child in children) {
            val size = child.measure(maxWidth - padding * 2f, maxHeight)
            width = maxOf(width, size.width)
            height += size.height
        }
        return YorSize(width + padding * 2f, height + padding)
    }

    override fun layout(bounds: YorRect) {
        this.bounds = bounds
        var y = bounds.y + padding
        val childWidth = maxOf(0f, bounds.width - padding * 2f)
        for (child in children) {
            val size = child.measure(childWidth, bounds.height)
            child.layout(YorRect(bounds.x + padding, y, childWidth, size.height))
            y += size.height
        }
    }

    override fun render(canvas: YorCanvas) {
        if (background.a > 0f) canvas.rect(bounds, background)
        children.forEach { it.render(canvas) }
    }

    override fun pointerDown(x: Float, y: Float): Boolean {
        for (child in children.asReversed()) {
            if (child.bounds.contains(x, y) && child.pointerDown(x, y)) return true
        }
        return false
    }
}

class YorText(
    private val value: String,
    private val color: YorColor = YorColor.White,
    private val height: Float = 20f,
) : YorNode {
    override var bounds = YorRect(0f, 0f, 0f, 0f)

    override fun measure(maxWidth: Float, maxHeight: Float): YorSize =
        YorSize(maxWidth, minOf(height, maxHeight))

    override fun render(canvas: YorCanvas) = canvas.text(value, bounds, color)
}

class YorButton(
    private val label: String,
    private val onClick: () -> Unit,
    private val background: YorColor = YorColor(0.18f, 0.18f, 0.18f, 0.92f),
    private val foreground: YorColor = YorColor.White,
    private val height: Float = 40f,
) : YorNode {
    override var bounds = YorRect(0f, 0f, 0f, 0f)

    override fun measure(maxWidth: Float, maxHeight: Float): YorSize =
        YorSize(maxWidth, minOf(height, maxHeight))

    override fun render(canvas: YorCanvas) {
        canvas.rect(bounds, background)
        canvas.text(label, bounds, foreground)
    }

    override fun pointerDown(x: Float, y: Float): Boolean {
        if (!bounds.contains(x, y)) return false
        onClick()
        return true
    }
}
