#include <windows.h>
#include <string>
#include <vector>
using namespace std;

bool running = true;
HBRUSH hDarkBrush = NULL;
HBRUSH hBorderBrush = NULL;
HBRUSH hBtnBrush = NULL;
HFONT hCustomFont = NULL;
HBRUSH hSelectedBrush = NULL;
HFONT hTitleFont= NULL;
HFONT hPreview = NULL;



HWND hSep1= NULL;
HWND hSep2 =NULL;

HWND hSidebar = NULL;
HWND hListBox = NULL;
HWND hSearchBox = NULL;
HWND hContentEdit = NULL;

HWND hDelBtn = NULL;
HWND hNewBtn = NULL;

int currentNoteIndex = 0;
bool isUpdatingUI = false; // Flag to prevent recursive updates

vector<string> noteTitels = {
    "Welcome to Notes",
    "Untitled Note #2.",
    "Quick Thoughts #3."
};
vector<string> noteContents = {
    "Super Notes is an easy way to take notes, create to-do lists,\ntrack notes, create thoughts...\n\nMarkdown support enabled.",
    "Tags work a bit like folders, except there's no limit to the number of tags you can add per note.",
    "Drafting ideas for the new C++ project. Native Win32 API gives maximum performance and low memory footprint."
};

//Build a single-line preview anippet from the note body
static string MakePreviewSnippet(const string& content){
    string snippet;
    snippet.reserve(content.size());
    for(char ch : content) {
        if (ch ==  '\r' || ch == '\n'){
            if (!snippet.empty() && snippet.back() != ' ') {
                snippet.push_back(' ');
            }
        }else{
            snippet.push_back(ch);
        }
    }
    while (!snippet.empty() && snippet.front() == ' '){
        snippet.erase(snippet.begin());
    }
    return snippet.empty() ? "No additional text" : snippet;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SIZE:{
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            //fix width fro left and middle panels
            int sidebarWidth = 220;
            int sepWidth = 1;
            int listWidth = 300;

            //lest panel width is the rest of the window
            int contentWidth = width - sidebarWidth - listWidth - (sepWidth * 2);
            if(contentWidth < 100) contentWidth = 100;


            //rasp koordom c uchet rasp
            int x1 = 0;
            int x2 = x1 +sidebarWidth;
            int x3 = x2 + sepWidth;
            int x4 = x3 + listWidth;
            int x5 = x4 + sepWidth;

            //resize panels for changing window size
            if(hSidebar) MoveWindow(hSidebar, x1, 0, sidebarWidth, height, TRUE);
            if(hSep1)    MoveWindow(hSep1, x2, 0, sepWidth, height, TRUE);
            
            if(hSearchBox) MoveWindow(hSearchBox, x3 + 10, 10, listWidth - 115, 28, TRUE);
            if(hNewBtn)    MoveWindow(hNewBtn, x3 + listWidth - 98, 12, 48, 28, TRUE);
            if(hDelBtn)    MoveWindow(hDelBtn, x3 + listWidth - 45, 12, 35, 28, TRUE);

            if(hListBox)   MoveWindow(hListBox, x3, 48, listWidth, height - 48, TRUE);
            if(hSep2)    MoveWindow(hSep2, x4, 0, sepWidth, height, TRUE);
            if(hContentEdit)   MoveWindow(hContentEdit, x5 + 20, 20, contentWidth - 40, height - 40, TRUE);
            
            return 0;
        }
        case WM_MEASUREITEM: {
            LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
            if (lpmis && lpmis->CtlID == 101) {
                lpmis->itemHeight = 64;
                return TRUE;
            }
            break;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
            if (!lpdis || lpdis->CtlID != 101) break;

            int idx = (int)lpdis->itemID;
            if (idx < 0 || idx >= (int)noteTitels.size()) return TRUE;

            bool isSelected = (lpdis->itemState & ODS_SELECTED) != 0;
            FillRect(lpdis->hDC, &lpdis->rcItem, isSelected ? hSelectedBrush : hDarkBrush);

            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(38, 38, 38));
            HPEN hOldPen = (HPEN)SelectObject(lpdis->hDC, hPen);
            MoveToEx(lpdis->hDC, lpdis->rcItem.left + 12, lpdis->rcItem.bottom - 1, NULL);
            LineTo(lpdis->hDC, lpdis->rcItem.right - 12, lpdis->rcItem.bottom - 1);
            SelectObject(lpdis->hDC, hOldPen);
            DeleteObject(hPen);

            int oldBkMode = SetBkMode(lpdis->hDC, TRANSPARENT);

            RECT titleRect = lpdis->rcItem;
            titleRect.left += 14;
            titleRect.right -= 14;
            titleRect.top += 10;
            titleRect.bottom = titleRect.top + 22;

            HFONT hOldFont = (HFONT)SelectObject(lpdis->hDC, hTitleFont);
            SetTextColor(lpdis->hDC, isSelected ? RGB(255, 255, 255) : RGB(225, 225, 225));
            DrawTextA(lpdis->hDC, noteTitels[idx].c_str(), -1, &titleRect,
                      DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

            RECT previewRect = lpdis->rcItem;
            previewRect.left += 14;
            previewRect.right -= 14;
            previewRect.top = titleRect.bottom + 2;
            previewRect.bottom = lpdis->rcItem.bottom - 8;

            string preview = MakePreviewSnippet(noteContents[idx]);
            SelectObject(lpdis->hDC, hPreview);
            SetTextColor(lpdis->hDC, isSelected ? RGB(185, 185, 190) : RGB(145, 145, 145));
            DrawTextA(lpdis->hDC, preview.c_str(), -1, &previewRect,
                      DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

            SelectObject(lpdis->hDC, hOldFont);
            SetBkMode(lpdis->hDC, oldBkMode);
            return TRUE;
        }
        case WM_COMMAND:{
            int notificationCode = HIWORD(wParam);
            int controlID = LOWORD(wParam);

            //switch between listbox and edit control notifications
            if(controlID == 101 && notificationCode == LBN_SELCHANGE){
                if(isUpdatingUI) break; // Prevent recursive updates

                int selectedIndex = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
                if(selectedIndex != LB_ERR && selectedIndex >= 0 && selectedIndex < (int)noteContents.size()){
                    currentNoteIndex = selectedIndex;

                    isUpdatingUI = true; // Set flag to prevent recursive updates 
                    SetWindowTextA(hContentEdit, noteContents[currentNoteIndex].c_str());
                    EnableWindow(hContentEdit, TRUE);
                    isUpdatingUI = false; // Reset flag after update
                }
            }
            //sync changes from edit control to listbox and note contents
            if(controlID == 102 && notificationCode == EN_CHANGE){
                if (isUpdatingUI || currentNoteIndex < 0 || currentNoteIndex >= (int)noteContents.size()) break; // Prevent recursive updates

                int textLength = GetWindowTextLengthA(hContentEdit);
                if (textLength >= 0) {
                    string fullText(textLength, '\0');
                    if( textLength > 0) {
                        GetWindowTextA(hContentEdit, &fullText[0], textLength +1);
                    }
                    noteContents[currentNoteIndex] = fullText;

                    size_t newLinePos = fullText.find('\n');
                    string newTitle = (newLinePos != string::npos) ? fullText.substr(0, newLinePos) : fullText;
                    if (!newTitle.empty() && newTitle.back() == '\r'){
                        newTitle.pop_back();// Trim CR from Win32 multiline EDIT line breaks
                    }
                    if (newTitle.empty()) {
                        newTitle = "Untitled Note #" + to_string(currentNoteIndex +1);
                    }

                    if (noteTitels[currentNoteIndex] != newTitle) {
                        noteTitels[currentNoteIndex] = newTitle;
                        isUpdatingUI = true;
                        SendMessageA(hListBox, LB_DELETESTRING, currentNoteIndex, 0);
                        SendMessageA(hListBox, LB_INSERTSTRING, currentNoteIndex, (LPARAM)newTitle.c_str());
                        SendMessageA(hListBox, LB_SETCURSEL, currentNoteIndex, 0);
                        isUpdatingUI = false;
                    } else {
                        RECT itemRect = {};
                        if (SendMessageA(hListBox, LB_GETITEMRECT, currentNoteIndex, (LPARAM)&itemRect) != LB_ERR) {
                            InvalidateRect(hListBox, &itemRect, TRUE);
                        }
                    }
                }      

            }
            //button "+ New Note"   
            if(controlID == 104 && notificationCode == BN_CLICKED){
                string newTitle = "New Note";
                string newContent = "New Note\r\n\r\nStart typing here...";

                noteTitels.push_back(newTitle);
                noteContents.push_back(newContent);

                int newIndex = (int)noteTitels.size() - 1;
                currentNoteIndex = newIndex;

                isUpdatingUI = true; // Set flag to prevent recursive updates
                SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)newTitle.c_str());
                SendMessage(hListBox, LB_SETCURSEL, newIndex, 0);
                SetWindowTextA(hContentEdit, newContent.c_str());
                EnableWindow(hContentEdit, TRUE);
                isUpdatingUI = false; // Reset flag after update
                SetFocus(hContentEdit);
            }
            //button "Delete Note"
            if(controlID == 105 && notificationCode == BN_CLICKED){
                if(!noteTitels.empty() && currentNoteIndex >= 0 && currentNoteIndex < (int)noteTitels.size()){
                    noteTitels.erase(noteTitels.begin() + currentNoteIndex);
                    noteContents.erase(noteContents.begin() + currentNoteIndex);

                    isUpdatingUI = true; // Set flag to prevent recursive updates
                    SendMessage(hListBox, LB_DELETESTRING, currentNoteIndex, 0);

                    if(noteTitels.empty()){
                        currentNoteIndex = -1; // Move to the previous note
                        SetWindowTextA(hContentEdit, "");
                        EnableWindow(hContentEdit, FALSE);

                    } else {
                        if (currentNoteIndex >= (int)noteContents.size()) {
                            currentNoteIndex = (int)noteTitels.size() - 1; // No notes left
                        }
                        SendMessage(hListBox, LB_SETCURSEL, currentNoteIndex, 0);
                        SetWindowTextA(hContentEdit, noteContents[currentNoteIndex].c_str());
                        EnableWindow(hContentEdit, TRUE);
                    }
                    isUpdatingUI = false; // Reset flag after update
                }
            }
        
        }
        case WM_CTLCOLORSTATIC:{
            HDC hdc = (HDC)wParam;
            HWND hCtl = (HWND)lParam;

            if(hCtl == hSep1 || hCtl == hSep2){
                return (INT_PTR)hBorderBrush;
            }

            SetTextColor(hdc, RGB(220, 220, 220));
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)hDarkBrush;


        }
        case WM_CTLCOLORLISTBOX:{
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(220, 220, 220));
            SetBkColor(hdc, RGB(24, 24, 24));
            return (LRESULT)hDarkBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(220, 220, 220));
            SetBkColor(hdc, RGB(24, 24, 24));
            return (INT_PTR)hDarkBrush;
        }
        case WM_DESTROY:{
            running = false;
            if(hCustomFont) DeleteObject(hCustomFont);
            if(hTitleFont) DeleteObject(hTitleFont);
            if(hPreview) DeleteObject(hPreview);
            DeleteObject(hDarkBrush);
            DeleteObject(hBorderBrush);
            DeleteObject(hBtnBrush);
            DeleteObject(hSelectedBrush);
            PostQuitMessage(0);
            return 0;
        }
        case WM_CLOSE:{
            running = false;
            DestroyWindow(hwnd);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    //create window class
    hDarkBrush = CreateSolidBrush(RGB(24, 24, 24));
    hBorderBrush = CreateSolidBrush(RGB(55, 55, 55));
    hBtnBrush = CreateSolidBrush(RGB(40, 40, 40));
    hSelectedBrush = CreateSolidBrush(RGB(42, 42, 46));

    hCustomFont = CreateFontA(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    hTitleFont = CreateFontA(
        16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    hPreview = CreateFontA(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    WNDCLASSA windows_class = {};
    windows_class.style = CS_HREDRAW | CS_VREDRAW;
    windows_class.lpszClassName = "Window Class";
    windows_class.lpfnWndProc = WindowProc;
    windows_class.hInstance = hInstance;
    windows_class.hbrBackground = hDarkBrush;


    RegisterClassA(&windows_class); //register class


    //create window
    HWND window = CreateWindowA(windows_class.lpszClassName, 
        "Notes", 
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
        CW_USEDEFAULT, CW_USEDEFAULT, 
        1100, 680, 
        NULL, NULL, 
        hInstance, NULL);

    if(window == NULL) return -1;
    
    //1.lest panel(menu and tags)
    hSidebar = CreateWindowExA(
        0, "STATIC", "  All Notes\n  Notifications\n  Settings\n  Trash\n\n",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        0, 0, 220, 680,
        window, NULL, hInstance, NULL
    );
    hSep1 = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE, 220, 0, 1, 680, window, NULL, hInstance, NULL);




    //2.middle panel(list of notes)
    hSearchBox = CreateWindowExA(
        0, "EDIT", " Search all notes",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        230, 10, 185, 28,
        window, (HMENU)103, hInstance, NULL
    );

    hNewBtn = CreateWindowExA(
        0, "BUTTON", "+ New",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        422, 12, 48, 28,
        window, (HMENU)104, hInstance, NULL
    );

    hDelBtn = CreateWindowExA(
        0, "BUTTON", "Del",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        475, 12, 35, 28,
        window, (HMENU)105, hInstance, NULL
    );



    hListBox = CreateWindowExA(
        0, "LISTBOX", " ",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_OWNERDRAWFIXED,
        221, 50, 300, 630,
        window, (HMENU)101, hInstance, NULL
    );

    for(size_t i = 0; i < noteTitels.size(); i++){
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)noteTitels[i].c_str());
    }
    SendMessage(hListBox, LB_SETCURSEL, 0, 0);

    hSep2 = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE, 521, 0, 1, 680, window, NULL, hInstance, NULL);


    //3.right panel(content of note)
    hContentEdit = CreateWindowExA(
0,      "EDIT", noteContents[0].c_str(),
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_WANTRETURN,
        545, 20, 530, 640,
        window, (HMENU)102, hInstance, NULL
    );

    //use custom font for all panels
    SendMessage(hSidebar, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
    SendMessage(hSearchBox, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
    SendMessage(hNewBtn, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
    SendMessage(hDelBtn, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
    SendMessage(hListBox, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
    SendMessage(hContentEdit, WM_SETFONT, (WPARAM)hCustomFont, TRUE);



    while(running){
        //input
        MSG message;
        while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    return 0;
}