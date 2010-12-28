#ifndef GARSHNE_BGUI_H
#define GARSHNE_BGUI_H

#define WHITESPACE 3

#define ID_QUIT     1
#define ID_HIDE     2
#define ID_TOGGLE   3
#define ID_INFO     4
#define ID_PREFS    5
#define ID_BLANKERS 6
#define ID_SET      7

extern Object *BlankersLvw;
extern Object *PrefsBtn, *InfoBtn, *ToggleBtn;
extern Object *HideBtn, *SettingsBtn, *QuitBtn;
extern Object *BlankWnd;
extern struct Window *BlankerWnd;
extern ULONG BGUI_Sigs;

VOID InitBGUI( VOID );
LONG openMainWindow( VOID );
LONG HandleBlankerIDCMP( VOID );
VOID CloseBlankerWindow( VOID );

#endif /* GARSHNE_BGUI_H */
