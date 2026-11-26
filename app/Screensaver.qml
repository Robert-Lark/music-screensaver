// One fullscreen slideshow window. Context properties from main.cpp:
//   imageUrls, imageCaptions (already shuffled), intervalMs, logSlides.
import QtQuick
import QtQuick.Window
import QtQuick.Effects

Window {
    id: win
    visible: true
    visibility: Window.FullScreen
    color: "black"
    title: "Album art screensaver"

    readonly property int fadeMs: 3000
    readonly property int count: imageUrls.length
    property var order: []
    property int pos: 0
    property bool startupGrace: true
    property bool hasMotionBaseline: false
    property point motionBaseline

    function shuffled(avoidFirst) {
        let a = [...Array(count).keys()]
        for (let i = a.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [a[i], a[j]] = [a[j], a[i]]
        }
        if (a.length > 1 && a[0] === avoidFirst) [a[0], a[1]] = [a[1], a[0]]
        return a
    }

    function nextIndex() {
        pos += 1
        if (pos >= order.length) { order = shuffled(order[order.length - 1]); pos = 0 }
        return order[pos]
    }

    function captionFor(i) { return imageCaptions[i].replace(" - ", "  —  ") }

    property Item front: slideA
    property Item back: slideB

    function advance() {
        if (count < 2) return
        back.show(nextIndex())   // starts loading; it fades in from onReady
    }

    function crossfade() {
        back.z = 1; front.z = 0
        back.fadeIn()
        front.fadeOut()
        const t = front; front = back; back = t
        logSlide(front.index)
    }

    function logSlide(i) { if (logSlides) console.info("slide: " + imageCaptions[i]) }

    Component.onCompleted: {
        order = [...Array(count).keys()]   // main.cpp already shuffled the list
        slideA.show(order[0])
        logSlide(order[0])
    }

    Timer { interval: intervalMs; running: true; repeat: true; onTriggered: win.advance() }
    Timer {
        interval: 2500; running: true
        onTriggered: {
            win.startupGrace = false
            if (!win.hasMotionBaseline && pointer.containsMouse) {
                win.motionBaseline = Qt.point(pointer.mouseX, pointer.mouseY); win.hasMotionBaseline = true
            }
        }
    }

    component Slide: Item {
        id: slide
        anchors.fill: parent
        opacity: 0
        property int index: -1
        property bool pending: false
        readonly property bool ready: cover.status === Image.Ready && backdropSrc.status === Image.Ready

        function show(i) { pending = true; index = i; Qt.callLater(() => { if (ready) loaded() }) }
        function loaded() { if (!pending) return; pending = false; if (slide === win.front) fadeIn(); else win.crossfade() }
        onReadyChanged: if (ready) loaded()

        // The incoming slide fades in on top; the outgoing one stays opaque
        // underneath until that finishes, so the screen never dips to black.
        function fadeIn() { outDelay.stop(); fade.stop(); fade.to = 1; fade.start(); drift.restart(); captionFade.restart() }
        function fadeOut() { captionFade.stop(); captionOut.start(); outDelay.restart() }
        Timer { id: outDelay; interval: win.fadeMs + 100; onTriggered: { slide.opacity = 0; drift.stop() } }
        NumberAnimation on opacity { id: fade; duration: win.fadeMs; easing.type: Easing.InOutQuad; running: false }

        // Backdrop: the same art, tiny, blurred and darkened to fill the screen.
        // It is blurred at 1/8 size and scaled up, which is cheap and very soft.
        Item {
            id: backdrop
            width: Math.ceil(win.width / 8); height: Math.ceil(win.height / 8)
            transformOrigin: Item.TopLeft
            scale: 8
            Image {
                id: backdropSrc
                anchors.fill: parent
                source: slide.index >= 0 ? imageUrls[slide.index] : ""
                sourceSize: Qt.size(256, 256)
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                smooth: true
                layer.enabled: true
                layer.smooth: true
                layer.effect: MultiEffect {
                    blurEnabled: true; blur: 1.0; blurMax: 32
                    brightness: -0.35; saturation: 0.1
                }
            }
        }
        Rectangle {   // vignette so the edges sink into black
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#aa000000" }
                GradientStop { position: 0.35; color: "#22000000" }
                GradientStop { position: 0.65; color: "#22000000" }
                GradientStop { position: 1.0; color: "#cc000000" }
            }
        }

        // The cover, centred, with a soft shadow and a slow drift-zoom.
        Item {
            id: coverBox
            readonly property real side: Math.min(win.height * 0.8, win.width * 0.8)
            width: side; height: side
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -win.height * 0.015
            Image {
                id: cover
                anchors.fill: parent
                source: slide.index >= 0 ? imageUrls[slide.index] : ""
                sourceSize: Qt.size(coverBox.side * Screen.devicePixelRatio, coverBox.side * Screen.devicePixelRatio)
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                smooth: true; mipmap: true
                visible: false
            }
            MultiEffect {
                anchors.fill: cover
                source: cover
                autoPaddingEnabled: true
                shadowEnabled: true; shadowBlur: 1.0; shadowOpacity: 0.85
                shadowVerticalOffset: win.height * 0.012; shadowColor: "black"
            }
            NumberAnimation on scale {
                id: drift; from: 1.0; to: 1.04
                duration: intervalMs + win.fadeMs * 2; easing.type: Easing.Linear; running: false
            }
        }

        Text {
            id: caption
            text: slide.index >= 0 ? win.captionFor(slide.index) : ""
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: win.height * 0.03
            width: parent.width * 0.8
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: "white"
            font.pixelSize: Math.round(win.height * 0.017)
            font.letterSpacing: 1.5
            opacity: 0
            SequentialAnimation on opacity {
                id: captionFade; running: false
                PauseAnimation { duration: win.fadeMs }
                NumberAnimation { to: 0.55; duration: 2500; easing.type: Easing.InOutQuad }
                PauseAnimation { duration: 12000 }
                NumberAnimation { to: 0.0; duration: 4000; easing.type: Easing.InOutQuad }
            }
            NumberAnimation on opacity { id: captionOut; to: 0; duration: 800; running: false }
        }
    }

    Slide { id: slideA }
    Slide { id: slideB }

    // Input: any key, click or scroll quits; so does real mouse movement once
    // the launch has settled (the launcher moves focus between monitors).
    Item {
        anchors.fill: parent
        focus: true
        Keys.onPressed: Qt.quit()
    }
    MouseArea {
        id: pointer
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.BlankCursor
        acceptedButtons: Qt.AllButtons
        onPressed: Qt.quit()
        onWheel: Qt.quit()
        onPositionChanged: (m) => {
            // During the grace period just track where the pointer rests.
            if (win.startupGrace) { win.motionBaseline = Qt.point(m.x, m.y); win.hasMotionBaseline = true; return }
            // First event seen: remember it; quit once the pointer travels > 30px.
            if (!win.hasMotionBaseline) { win.motionBaseline = Qt.point(m.x, m.y); win.hasMotionBaseline = true }
            else if (Math.hypot(m.x - win.motionBaseline.x, m.y - win.motionBaseline.y) > 30) Qt.quit()
        }
    }
}
