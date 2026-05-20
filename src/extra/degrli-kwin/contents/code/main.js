// script for degrli
// made with love (for umamusume, not you dumass)

function dgl_findbytitle(tcaption) {
	var windows = workspace.windowList();
	for (var i = 0; i < windows.length; ++i) {
		var window = windows[i];

		if (window.caption.indexOf(tcaption) !== -1) {
			return window;
		}
	}
	// YOU DIDNT USE THE CORRECT TITLE!!!
	//   format is: "degrli [IPC ID]"
	return null;
}

function dgl_movebytitle(tcaption, offset_x, offset_y) {
	var target = dgl_findbytitle(tcaption);
	var cg = target.frameGeometry;
	target.frameGeometry = {
		x: cg.x + offset_x,
		y: cg.y + offset_y,
		width: cg.width,
		height: cg.height
	};
}
