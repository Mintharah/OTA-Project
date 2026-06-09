import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 480
    height: 380
    visible: true
    title: "OTA Firmware Updater"
    color: "#FFF0F5"

    StackLayout {
        anchors.centerIn: parent
        width: 380
        height: 320
        currentIndex: controller ? controller.state : 0

        Item { WaitingView {} }

        Item {
            ReceivingView {
                progress:   controller ? controller.progress : 0
                statusText: controller ? controller.statusMessage : ""
            }
        }

        Item {
            ResultView {
                success: true
                message: controller ? controller.statusMessage : ""
            }
        }

        Item {
            ResultView {
                success: false
                message: controller ? (controller.errorMessage !== ""
                                       ? controller.errorMessage
                                       : controller.statusMessage) : ""
            }
        }
    }

    Row {
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter }
        anchors.bottomMargin: 14
        spacing: 6

        Rectangle {
            width: 7; height: 7; radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: controller && controller.serviceAvailable ? "#C084A0" : "#E8A0B4"

            SequentialAnimation on opacity {
                running: controller ? !controller.serviceAvailable : true
                loops: Animation.Infinite
                NumberAnimation { to: 0.2; duration: 700 }
                NumberAnimation { to: 1.0; duration: 700 }
            }
        }

        Text {
            text: controller && controller.serviceAvailable
                  ? "Service online" : "Searching for service…"
            font.pixelSize: 11
            color: "#C084A0"
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}