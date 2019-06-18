import QtQuick 1.0

Rectangle
{
    anchors.fill: parent

    Grid {
        id: grid
        x: parent.width / 2 - grid.width / 2
        y: 10
        transformOrigin: Item.Center
        spacing: 4
        rows: params.dimArgs + params.dimParams + 2 + 2 * Math.max(params.dimParams, params.dimArgs)
        columns: 2

        Component.onCompleted:
        {
            var i = 0, newText, h;
            for (i = 0; i < params.dimArgs; ++i) {
                newText = Qt.createQmlObject('import QtQuick 1.0; Text{width: 100; text: "Arg' + (i+1) + '"}', grid, "");
                h = newText.height;
                Qt.createQmlObject('import QtQuick 1.0; Rectangle {border.width: 2; border.color: "black"; width: 200; height: ' + h + '; clip: true; TextInput {objectName: "a' + i + '"; width: parent.width - 4; anchors.horizontalCenter: parent.horizontalCenter; validator: DoubleValidator{}}}', grid, "");
            }
            for (i = 0; i < params.dimParams; ++i) {
                newText = Qt.createQmlObject('import QtQuick 1.0; Text{width: 100; text: "Param' + (i+1) + '"}', grid, "");
                h = newText.height;
                Qt.createQmlObject('import QtQuick 1.0; Rectangle {border.width: 2; border.color: "black"; width: 200; height: ' + h + '; clip: true; TextInput {objectName: "b' + i + '"; width: parent.width - 4; anchors.horizontalCenter: parent.horizontalCenter; validator: DoubleValidator{}}}', grid, "");
            }
            for (i = 0; i < Math.max(params.dimParams, params.dimArgs); ++i) {
                newText = Qt.createQmlObject('import QtQuick 1.0; Text{width: 100; text: "Left border ' + (i+1) + '"}', grid, "");
                h = newText.height;
                Qt.createQmlObject('import QtQuick 1.0; Rectangle {border.width: 2; border.color: "black"; width: 200; height: ' + h + '; clip: true; TextInput {objectName: "lb' + i + '"; width: parent.width - 4; anchors.horizontalCenter: parent.horizontalCenter; validator: DoubleValidator{}}}', grid, "");
                newText = Qt.createQmlObject('import QtQuick 1.0; Text{width: 100; text: "Right border ' + (i+1) + '"}', grid, "");
                h = newText.height;
                Qt.createQmlObject('import QtQuick 1.0; Rectangle {border.width: 2; border.color: "black"; width: 200; height: ' + h + '; clip: true; TextInput {objectName: "rb' + i + '"; width: parent.width - 4; anchors.horizontalCenter: parent.horizontalCenter; validator: DoubleValidator{}}}', grid, "");
            }
        }

        Text
        {
            id: eps
            text: "Epsilon"
            width: 100
        }

        Rectangle
        {
            border.width: 2
            border.color: "black"
            width: 200
            height: eps.height
            clip: true
            TextInput
            {
                objectName: "eps"
                width: parent.width - 4
                anchors.horizontalCenter: parent.horizontalCenter
                validator: DoubleValidator{}
            }
        }

        Text
        {
            id: pararg
            text: "Params or args"
            width: 100
        }

        Rectangle
        {
            border.width: 2
            border.color: "black"
            width: 200
            height: pararg.height
            clip: true
            TextInput
            {
                objectName: "pararg"
                width: parent.width - 4
                anchors.horizontalCenter: parent.horizontalCenter
                validator: RegExpValidator{regExp: /([Pp][Aa][Rr][Aa][Mm][Ss]|[Aa][Rr][Gg][Ss])/}
            }
        }
    }
}
