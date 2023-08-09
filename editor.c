#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.H>

//extern HWND window;
HWND editorPanel;

#define ID_SHOW_GRID 2
#define ID_TILE_SELECT 3
#define EDITORID_TILE_BUILDING 4
#define EDITORID_TILE_DOOR (TILE_DOOR+100)
#define EDITORID_TILE_WINDOW (TILE_WINDOW+100)
#define EDITORID_TILE_FENCE (TILE_FENCE+100)

int editorShowGrid = 1;
TILE_TYPE editorCurrentTile = EDITORID_TILE_DOOR;

LRESULT CALLBACK panelProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
		case WM_COMMAND: {
			if(wparam == ID_SHOW_GRID) {
				if(editorShowGrid) {
					CheckDlgButton(editorPanel, ID_SHOW_GRID, BST_UNCHECKED);
				} else {
					CheckDlgButton(editorPanel, ID_SHOW_GRID, BST_CHECKED);
				}
				editorShowGrid = !editorShowGrid;
			}
			
			if(HIWORD(wparam)==BN_CLICKED) {
				printf("tile select\n");
				switch(LOWORD(wparam)) {
					case EDITORID_TILE_BUILDING:
					printf("tile building\n");
					break;
					case EDITORID_TILE_DOOR:
					printf("tile DOOR\n");
					break;
					case EDITORID_TILE_WINDOW:
					printf("tile window\n");
					break;
					case EDITORID_TILE_FENCE:
					printf("tile fence\n");
					break;
				}
				editorCurrentTile = LOWORD(wparam);
			}
		} break;
	}
	
	return DefWindowProcA(hwnd, msg, wparam, lparam);
}

void setupEditor(HWND window) {
	HBRUSH panelColor = CreateSolidBrush(RGB(240,240,240));
	WNDCLASSA wndclass = {0};
	wndclass.lpszClassName = "EditorPanelClass";
	wndclass.hbrBackground = panelColor;
	wndclass.lpfnWndProc = panelProc;
	RegisterClassA(&wndclass);
	
	editorPanel = CreateWindowA("EditorPanelClass", NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, 0, 0, 200, 1080, window, (HMENU)100, NULL, NULL);
	if(editorPanel) {
		int x = 0;
	} else {
		char errorString[64];
		int errorCode = GetLastError();
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, 0, errorString, 64, 0);
		MessageBox(editorPanel, errorString, "Error", MB_OK);
	}
	
	CreateWindowA("button", "Test", WS_VISIBLE | WS_CHILD, 20, 20, 80, 25, editorPanel, (HMENU)1, 0, 0);
	
	CreateWindowA("button", "Show Grid",
				  WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
				  20, 80, 185, 35, editorPanel, (HMENU) ID_SHOW_GRID, 0, 0);
	CheckDlgButton(editorPanel, ID_SHOW_GRID, BST_CHECKED);
	
	HWND hwndEdit = CreateWindowA("Edit", 0, 
							 WS_CHILD | WS_VISIBLE | WS_BORDER,
							 20, 140, 150, 20, editorPanel, (HMENU) 101,
								  0, 0);
	
	CreateWindowA("Button", "Tiles", 
                  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                  20, 200, 120, 130, editorPanel, (HMENU) ID_TILE_SELECT, 0, 0);
	CreateWindowA("Button", "Building",
                  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                  30, 220, 100, 30, editorPanel, (HMENU) EDITORID_TILE_BUILDING , 0, 0);
	CreateWindowA("Button", "Door",
                  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                  30, 245, 100, 30, editorPanel, (HMENU) EDITORID_TILE_DOOR , 0, 0);
	CreateWindowA("Button", "Window",
                  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                  30, 270, 100, 30, editorPanel, (HMENU) EDITORID_TILE_WINDOW , 0, 0);
	CreateWindowA("Button", "Fence",
                  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                  30, 295, 100, 30, editorPanel, (HMENU) EDITORID_TILE_FENCE , 0, 0);
	
	CheckDlgButton(editorPanel, EDITORID_TILE_BUILDING, BST_CHECKED);
	editorCurrentTile = EDITORID_TILE_BUILDING;
}

void editorloop(win_state* win, game_state* game) {
	
}