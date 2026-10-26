import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtMultimedia
import PipOS

import "../Pages/STAT"
import "../Pages/ITEM"
import "../Pages/DATA"
import "../Pages/MAP"
import "../Pages/RADIO"

Item {
    id: root

    Rectangle {
        id: main
        anchors.fill: root
        color: "black"

        MainNavigation {
            id: mainNav
            anchors {
                top: main.top
                left: main.left
                right: main.right
            }
            activeTabIndex: page.currentIndex
            tabs: Settings.hideMapTab ? ["STAT", "ITEM", "DATA", "RADIO"] : ["STAT", "ITEM", "DATA", "MAP", "RADIO"]
        }

        StackLayout {
            id: page
            anchors {
                top: mainNav.bottom
                left: main.left
                right: main.right
                bottom: main.bottom
            }

            STAT {
                id: statPage
                enabled: StackLayout.isCurrentItem
                subMenuCenter: mainNav.activeTabCenter
            }
            ITEM {
                id: itemPage
                enabled: StackLayout.isCurrentItem
                subMenuCenter: mainNav.activeTabCenter
            }
            DATA {
                id: dataPage
                enabled: StackLayout.isCurrentItem
                subMenuCenter: mainNav.activeTabCenter
            }
            MAP {
                id: mapPage
                enabled: StackLayout.isCurrentItem
            }
            RADIO {
                id: radioPage
                enabled: StackLayout.isCurrentItem
            }
        }

        // Reference image loader for alignment in dev
        // Image {
        //     id: ref
        //     visible: false
        //     width: 800
        //     height: 600
        //     anchors.bottom: main.bottom
        //     anchors.horizontalCenter: main.horizontalCenter
        //     source: "/images/ripped_ref_inv.png"
        //     // anchors.horizontalCenterOffset: -8
        //     z: 1
        //     opacity: 0.2
        //     fillMode: Image.PreserveAspectFit
        // }
    }

    SoundEffect {
        id: sfxRotary
        source: "/sounds/horizontal_tab.wav"
    }

    function nextSubMenu() {
        if (page.currentIndex === 0) {
            statPage.subMenuIndex = (statPage.subMenuIndex + 1) % statPage.pages.length
            sfxRotary.play()
        } else if (page.currentIndex === 1) {
            itemPage.subMenuIndex = (itemPage.subMenuIndex + 1) % itemPage.pages.length
            sfxRotary.play()
        } else if (page.currentIndex === 2) {
            dataPage.subMenuIndex = (dataPage.subMenuIndex + 1) % dataPage.pages.length
            sfxRotary.play()
        }
    }

    function prevSubMenu() {
        if (page.currentIndex === 0) {
            statPage.subMenuIndex = (statPage.subMenuIndex - 1 + statPage.pages.length) % statPage.pages.length
            sfxRotary.play()
        } else if (page.currentIndex === 1) {
            itemPage.subMenuIndex = (itemPage.subMenuIndex - 1 + itemPage.pages.length) % itemPage.pages.length
            sfxRotary.play()
        } else if (page.currentIndex === 2) {
            dataPage.subMenuIndex = (dataPage.subMenuIndex - 1 + dataPage.pages.length) % dataPage.pages.length
            sfxRotary.play()
        }
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.SUB_TAB_NEXT)
        context: Qt.ApplicationShortcut
        autoRepeat: false
        onActivated: root.nextSubMenu()
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.SUB_TAB_PREVIOUS)
        context: Qt.ApplicationShortcut
        autoRepeat: false
        onActivated: root.prevSubMenu()
    }

    Connections {
        target: app
        function onSubTabNext() {
            root.nextSubMenu()
        }
        function onSubTabPrevious() {
            root.prevSubMenu()
        }
        function onButtonSelect() {
            if (page.currentIndex === 0) {
                root.nextSubMenu()
            }
        }
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.TAB_STAT)
        context: Qt.ApplicationShortcut
        autoRepeat: false
        onActivated: {
            if (page.currentIndex !== 0)
                statPage.subMenuIndex = 0
            else if (statPage.pages.length === (statPage.subMenuIndex + 1))
                statPage.subMenuIndex = 0
            else
                statPage.subMenuIndex++
            page.currentIndex = 0
            sfxRotary.play()
        }
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.TAB_ITEM)
        context: Qt.ApplicationShortcut
        autoRepeat: false
        onActivated: {
            if (page.currentIndex !== 1)
                itemPage.subMenuIndex = 0
            else if (itemPage.pages.length === (itemPage.subMenuIndex + 1))
                itemPage.subMenuIndex = 0
            else
                itemPage.subMenuIndex++
            page.currentIndex = 1
            sfxRotary.play()
        }
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.TAB_DATA)
        context: Qt.ApplicationShortcut
        autoRepeat: false
        onActivated: {
            if (page.currentIndex !== 2)
                dataPage.subMenuIndex = 0
            else if (dataPage.pages.length === (dataPage.subMenuIndex + 1))
                dataPage.subMenuIndex = 0
            else
                dataPage.subMenuIndex++
            page.currentIndex = 2
            sfxRotary.play()
        }
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.TAB_MAP)
        context: Qt.ApplicationShortcut
        autoRepeat: false
        onActivated: {
            page.currentIndex = 3
            sfxRotary.play()
        }
    }

    Shortcut {
        sequence: Settings.getKeySequence(Events.TAB_RADIO)
        context: Qt.ApplicationShortcut
        autoRepeat: false
        onActivated: {
            page.currentIndex = 4
            sfxRotary.play()
        }
    }
}
