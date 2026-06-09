import QtQuick

Column {
    anchors.centerIn: parent
    spacing: 14

    property bool   success: true
    property string message: ""

    AnimatedImage {
        width: 150; height: 150
        anchors.horizontalCenter: parent.horizontalCenter
        source: success ? "file:///usr/share/ota-client-ui/images/success_melody.gif"
                        : "file:///usr/share/ota-client-ui/images/error_melody.gif"
        fillMode: Image.PreserveAspectFit
        playing: true
        cache: false
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        text: success ? "Firmware update complete" : "Update failed"
        font.pixelSize: 17; font.weight: Font.Medium
        color: success ? "#A0C4A0" : "#E08AAE"
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        text: message
        font.pixelSize: 12
        color: success ? "#80B080" : "#C084A0"
    }
}