import QtQuick
import QtMultimedia
import PipOS

Rectangle {
    id: root
    color: "black"

    property var selectedPerk: list.currentItem ? list.currentItem.perk : null

    Rectangle {
        id: listMain
        color: "black"
        anchors {
            top: root.top
            topMargin: 72
            left: root.left
            bottom: root.bottom
            bottomMargin: 34
        }
        width: root.width * 0.49

        ListView {
            id: list
            anchors {
                fill: parent
                leftMargin: 10
                rightMargin: 24
            }
            spacing: 8
            model: Dweller.perks
            currentIndex: 0
            delegate: Item {
                id: item
                required property var modelData

                readonly property var perk: item.modelData
                readonly property string displayName: item.perk.stars > 1 ? "%1 (%2)".arg(item.perk.name).arg(item.perk.stars) : item.perk.name

                width: ListView.view.width
                height: 34

                Text {
                    anchors {
                        left: parent.left
                        leftMargin: 12
                        right: parent.right
                        rightMargin: 12
                        verticalCenter: parent.verticalCenter
                    }
                    text: item.displayName
                    color: item.ListView.isCurrentItem ? "black" : "white"
                    elide: Text.ElideRight
                    font.family: "Roboto Condensed"
                    font.pixelSize: 28
                    verticalAlignment: Text.AlignVCenter
                }
            }
            highlight: Rectangle {
                color: "white"
            }
            highlightRangeMode: ListView.StrictlyEnforceRange
            highlightFollowsCurrentItem: true
            preferredHighlightBegin: 0
            preferredHighlightEnd: list.bottom
            clip: true
        }
    }

    Item {
        id: perkDetail
        anchors {
            top: root.top
            topMargin: 50
            left: listMain.right
            leftMargin: 24
            right: root.right
            rightMargin: 16
            bottom: root.bottom
            bottomMargin: 28
        }

        Image {
            id: perkImage
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }
            source: root.selectedPerk ? root.selectedPerk.image : ""
            sourceSize.width: 220
            sourceSize.height: 220
            width: Math.min(220, parent.width * 0.82)
            height: 220
            fillMode: Image.PreserveAspectFit
            smooth: false
        }

        Row {
            id: rankStars
            anchors {
                top: perkImage.bottom
                topMargin: 24
                left: parent.left
                right: parent.right
            }
            spacing: 3
            height: 26

            Repeater {
                model: root.selectedPerk ? root.selectedPerk.maxStars : 0

                Canvas {
                    id: star
                    required property int index

                    readonly property bool filled: root.selectedPerk && index < root.selectedPerk.stars

                    width: 26
                    height: 26
                    onFilledChanged: requestPaint()
                    onPaint: {
                        const context = getContext("2d")
                        const centerX = width / 2
                        const centerY = height / 2
                        const outerRadius = 12
                        const innerRadius = 5

                        context.clearRect(0, 0, width, height)
                        context.beginPath()
                        for (let point = 0; point < 10; point++) {
                            const radius = point % 2 === 0 ? outerRadius : innerRadius
                            const angle = -Math.PI / 2 + point * Math.PI / 5
                            const x = centerX + Math.cos(angle) * radius
                            const y = centerY + Math.sin(angle) * radius
                            if (point === 0)
                                context.moveTo(x, y)
                            else
                                context.lineTo(x, y)
                        }
                        context.closePath()
                        context.lineWidth = 2
                        context.strokeStyle = "white"
                        context.fillStyle = star.filled ? "white" : "black"
                        context.fill()
                        context.stroke()
                    }
                }
            }
        }

        Text {
            anchors {
                top: rankStars.bottom
                topMargin: 10
                left: parent.left
                right: parent.right
            }
            text: root.selectedPerk ? root.selectedPerk.description : ""
            color: "white"
            font.family: "Roboto Condensed"
            font.pixelSize: 28
            lineHeight: 0.9
            wrapMode: Text.WordWrap
        }
    }

    SoundEffect {
        id: sfxFocus
        source: "/sounds/item_focus.wav"
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.SCROLL_UP)
        enabled: root.enabled
        autoRepeat: false
        onActivated: {
            list.decrementCurrentIndex()
            sfxFocus.play()
        }
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.SCROLL_DOWN)
        enabled: root.enabled
        autoRepeat: false
        onActivated: {
            list.incrementCurrentIndex()
            sfxFocus.play()
        }
    }

    onEnabledChanged: {
        if (enabled)
            list.currentIndex = 0
    }
}
