import QtQuick
import org.kde.kwin

Item {
    id: root

    QtObject {
        id: dbusInterface
        objectName: "org.kwin.WindowMover"

        function moveByPidOffset(targetpid, offset_x, offset_y) {
            var windows = Workspace.windowList();
            for (var i = 0; i < windows.length; i++) {
                if (windows[i].pid === targetpid) {
                    var geo = windows[i].frameGeometry;
                    geo.x += offset_x;
                    geo.y += offset_y;
                    windows[i].frameGeometry = geo;
                    break;
                }
            }
        }
    }
}
