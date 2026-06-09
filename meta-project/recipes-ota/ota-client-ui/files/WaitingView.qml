import QtQuick

Column {
    anchors.centerIn: parent
    spacing: 16

    AnimatedImage {
        width: 160; height: 160
        anchors.horizontalCenter: parent.horizontalCenter
        source: "file:///usr/share/ota-client-ui/images/sleep_melody.gif"
        fillMode: Image.PreserveAspectFit
        playing: true
        cache: false
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Waiting for OTA service"
        font.pixelSize: 16; font.weight: Font.Medium
        color: "#C084A0"
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Discovering OTAFlashService on the network…"
        font.pixelSize: 12
        color: "#D4A0B8"
    }
}