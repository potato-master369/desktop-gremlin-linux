#!/usr/bin/env bash

TMP_FILE="/tmp/kwin_mover_p6.js"

# Write the script using strict Plasma 6 native bindings
cat << 'EOF' > "$TMP_FILE"
registerShortcut("degrli_mover", "degrli mover", "", function() {
    var targetpid = Math.floor(options.configRead("DevMover", "TargetPID", 0));
    var offset_x = Math.floor(options.configRead("DevMover", "OffsetX", 0));
    var offset_y = Math.floor(options.configRead("DevMover", "OffsetY", 0));

    if (targetpid === 0) return;

    var windows = workspace.windowList();
    for (var i = 0; i < windows.length; i++) {
        if (windows[i].pid === targetpid) {
            var geo = windows[i].frameGeometry;
            geo.x += offset_x;
            geo.y += offset_y;
            windows[i].frameGeometry = geo;
            break;
        }
    }
});
EOF

echo "Loading compliant script into KWin 6..."
SCRIPT_ID=$(qdbus6 org.kde.KWin /Scripting org.kde.kwin.Scripting.loadScript "$TMP_FILE" "MoverP6")
#rm -f "$TMP_FILE"

if [ "$SCRIPT_ID" = "-1" ] || [ -z "$SCRIPT_ID" ]; then
    echo "Failed to load script."
    exit 1
fi

REAL_PATH="/Scripting/Script${SCRIPT_ID}"
qdbus6 org.kde.KWin "$REAL_PATH" org.kde.kwin.Script.run

echo "done!"
