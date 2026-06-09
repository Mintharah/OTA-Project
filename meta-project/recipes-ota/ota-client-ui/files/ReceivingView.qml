import QtQuick

Column {
    anchors.centerIn: parent
    spacing: 14

    property int    progress: 0
    property string statusText: ""

    AnimatedImage {
        width: 140; height: 140
        anchors.horizontalCenter: parent.horizontalCenter
        source: "file:///usr/share/ota-client-ui/images/loading_melody.gif"
        fillMode: Image.PreserveAspectFit
        playing: true
        cache: false
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        text: statusText
        font.pixelSize: 14; font.weight: Font.Medium
        color: "#C084A0"
    }

    Column {
        width: 300
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 6

        Rectangle {
            width: parent.width; height: 8; radius: 4
            color: "#F8D7E3"

            Rectangle {
                width: parent.width * (progress / 100.0)
                height: parent.height; radius: 4
                color: "#E08AAE"
                Behavior on width {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }

                Rectangle {
                    width: 40; height: parent.height; radius: 4
                    color: Qt.rgba(1, 1, 1, 0.35)
                    x: -40
                    NumberAnimation on x {
                        from: -40; to: parent.parent.width
                        duration: 1200; loops: Animation.Infinite; running: true
                    }
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: progress + "%"
            font.pixelSize: 13; font.weight: Font.Medium
            color: "#C084A0"
        }
    }
}