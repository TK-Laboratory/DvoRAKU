/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * DvoRAKU
 * 
 * ����z��u�ǂڊy�v
 *
 * Copyright 2024 TK Lab. All rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <windows.h>
#include <imm.h>
#include <stdio.h>

#pragma comment( lib, "User32.lib" )
#pragma comment( lib, "Imm32.lib" )


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK HookProc(int nCode, WPARAM wp, LPARAM lp);
void HookStart();
void HookEnd();

char SendKey( char keyCode );
char SendKey( char modifierCode, char keyCode );

HINSTANCE hInst;
HHOOK hHook;

HWND hWnd;

int lastKeyCode;
bool isLastKeyConsonant;
bool enableDvoRAKU = TRUE;
bool enableExtendedRAKU = TRUE;

int main( int argc, char *argv[] ){

   const TCHAR CLASS_NAME[] = "DvoRAKU";
   
   WNDCLASS wc = { };

   wc.lpfnWndProc = WindowProc;
   HINSTANCE hInstance;
   hInstance = GetModuleHandle( NULL );
   wc.hInstance = hInstance;
   wc.lpszClassName = CLASS_NAME;

   RegisterClass(&wc);


   hWnd = CreateWindowEx(
      WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
      CLASS_NAME,
      "�ǂڊy",
      WS_POPUP | WS_BORDER,
      0, 0, 0, 0,
      NULL,
      NULL,
      hInstance,
      NULL
      );

   if (hWnd == NULL){
      return -1;
   }

   MSG msg;
   while ( GetMessage(&msg, NULL, 0, 0) ) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   return 0;
}

LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   switch (uMsg) {

      case WM_CREATE:
         HookStart();
         break;

      case WM_DESTROY:
         HookEnd();
         PostQuitMessage(0);
         return 0;
   }

   return DefWindowProc( hWnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK HookProc(int nCode, WPARAM wp, LPARAM lp)
{
   if ( nCode != HC_ACTION ){
      return CallNextHookEx(hHook, nCode, wp, lp);
   }

   KBDLLHOOKSTRUCT *key;
   unsigned int keyCode, scanCode, keyFlags, keyChar;

   key = (KBDLLHOOKSTRUCT*)lp;
   
   keyCode = key->vkCode;
   scanCode = key->scanCode;
   keyFlags = key->flags;
   keyChar = MapVirtualKey( keyCode, MAPVK_VK_TO_CHAR );

   printf( "(%c), %u, %u, %u, ", keyChar, keyCode, scanCode, keyFlags );

   bool isKeyUp;
   isKeyUp = keyFlags & LLKHF_UP;
   if ( isKeyUp ){
      printf( "KeyUp " );
   }
   else{
      printf( "KeyDown " );
   }

   bool isInjected;
   isInjected = keyFlags & LLKHF_INJECTED;
   if( isInjected ){
      printf( "injected\n" );
   }
   else{
      printf( "detected\n" );
   }

  // Ctrl�L�[��� 
  // Ctrl + �� �� Ctrl + ���@�ł͂Ȃ� �� �P�łɓ���ւ�����悤��Ctrl��Ԃ�����Ǘ�����
   static bool isControl = FALSE;
   if ( keyCode == VK_CONTROL || keyCode == VK_LCONTROL || keyCode == VK_RCONTROL ){
      if ( keyFlags & LLKHF_INJECTED ){
         return CallNextHookEx(hHook, nCode, wp, lp);
      }
      else {
         if( ( keyFlags & LLKHF_UP ) == 0  ){
            isControl = TRUE;
         }
         else {
            isControl = FALSE;
         }
         return TRUE;
      }
   }

   // Shift�L�[���
   bool isShift;
   isShift = GetAsyncKeyState( VK_SHIFT) & 0x8000;

   // Alt�L�[���
   bool isAlt;
   isAlt = GetAsyncKeyState( VK_MENU ) & 0x8000;

   // �L�[�A�b�v�C�x���g�͓���ւ��������Ȃ�
   if( ( keyFlags & LLKHF_UP ) != 0  ){
      return CallNextHookEx(hHook, nCode, wp, lp);
   }

   // Ctrl�L�[�Ɠ��������̃L�[�o�C���h�ݒ�
   // �ݒ�̂Ȃ��L�[��QWERTY�ŏ�������
   if( isControl ) {
      if ( ( keyFlags & LLKHF_INJECTED ) == FALSE ){
         if ( keyCode == 'M' ){
            SendKey( VK_RETURN );
         }
         else if ( keyCode == 'H' ){
            SendKey( VK_BACK );
         }
         else {
            SendKey( VK_CONTROL, keyCode );
         }

         return TRUE;
      }
   }

   // Alt �L�[�Ƃ̓��������̃L�[�o�C���h�ݒ�
   if( isAlt  ){
      // Alt + Q �� QWERTY �� �ǂڊy�@�؂�ւ�
      if( keyCode == 'Q' ){
         enableDvoRAKU = !enableDvoRAKU;
         return TRUE;
      }
      // Alt + X �� �g�����C���[�̗L���������g�O��
      else if( keyCode == 'X' ){
         enableExtendedRAKU= !enableExtendedRAKU;
         return TRUE;
      }
   }

   // ������Ԃ̂Ƃ�����ւ��������Ȃ�
   if ( enableDvoRAKU == FALSE ){
      return CallNextHookEx(hHook, nCode, wp, lp);
   }
   

   // IME���I�t�̎���QWERTY
   HWND hForeground;
   hForeground = GetForegroundWindow();

   HWND hIME;
   hIME = ImmGetDefaultIMEWnd( hForeground );

   LRESULT imeStatus;
   imeStatus = SendMessage( hIME, WM_IME_CONTROL, 5, 0 );

   if( imeStatus == 0 ){
      return CallNextHookEx(hHook, nCode, wp, lp);
   }


   // �}�����ꂽ�L�[�C�x���g
   if ( keyFlags & LLKHF_INJECTED ){
      //�q���L�[���}�����ꂽ��g�����C���[�ɓ���
      if( keyCode == 'B' ||
          keyCode == 'C' || 
          keyCode == 'D' || 
          keyCode == 'F' || 
          keyCode == 'G' || 
          keyCode == 'H' || 
          keyCode == 'J' || 
          keyCode == 'K' || 
          keyCode == 'L' || 
          keyCode == 'M' || 
          keyCode == 'N' || 
          keyCode == 'P' || 
          keyCode == 'Q' || 
          keyCode == 'R' || 
          keyCode == 'S' || 
          keyCode == 'T' || 
          keyCode == 'V' || 
          keyCode == 'W' || 
          keyCode == 'X' || 
          keyCode == 'Y' || 
          keyCode == 'Z' ) {


         // �����q��2�A���͊g�����C���[�������Ȃ�
         if ( lastKeyCode == keyCode ){
            // NN �͊g�����C���[���������g�����C���[�𔲂���
            if ( lastKeyCode == 'N' && keyCode == 'N' ){
               isLastKeyConsonant = FALSE;
            }
            // �����q��2�A���͊g�����C���[�������Ȃ����A�g�����C���[���p������
            else {
               isLastKeyConsonant = TRUE;
            }
         }
         // XN �͊g�����C���[���������g�����C���[�𔲂���
         else if ( lastKeyCode == 'X' && keyCode == 'N' ){
            isLastKeyConsonant = FALSE;
         }
         // �g�����C���[����
         else if( isLastKeyConsonant == TRUE ){
            // Q<�q��> �� Q���폜����<�q��>�ɒu��������
            // e.q. QS �� SS
            if( lastKeyCode == 'Q' ){
               SendKey( VK_BACK );
               SendKey( keyCode );
               isLastKeyConsonant = TRUE;
            }
            // <�q��1><�q��2> �� <�q��1>A<�q��2>A 
            // e.q. KS �� KASA
            else{
               SendKey( 'A' );
               SendKey( keyCode );
               SendKey( 'A' );
               return TRUE;
            }
         }
         // �q�����}�����ꂽ��g�����C���[�ɓ���
         else {
            if( enableExtendedRAKU ){
               if ( isShift == FALSE ) { 
                  isLastKeyConsonant = TRUE;
               }
            }
         }
      }
      else {
         // �q���ȊO�̃L�[���}�����ꂽ��g�����C���[���甲����
         isLastKeyConsonant = FALSE;
      }

      //�Ō�ɑ}�����ꂽ�L�[���L�^
      lastKeyCode = keyCode;

      return CallNextHookEx(hHook, nCode, wp, lp);
   }

   // �L�[����ւ�����
   //�ŏ�i
   if ( keyCode == '1' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'A' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( '1' );
      }
   }
   else if ( keyCode == '2' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'O' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( '2' );
      }
   }
   else if ( keyCode == '3' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'E' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( '3' );
      }
   }
   else if ( keyCode == '4' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'U' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( '4' );
      }
   }
   else if ( keyCode == '5' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'H' );
         SendKey( 'I' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( '5' );
      }
   }
   else if ( keyCode == VK_OEM_MINUS ){ // -_ 
      SendKey( VK_OEM_4 ); // [{
   }
   else if ( keyCode == VK_OEM_PLUS ){ // =+
      SendKey( VK_OEM_6 ); // ]}
   }
   //��i��
   else if ( keyCode == 'Q' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'A' );
      }
      else {
         SendKey( VK_OEM_7 ); // '"
      }
   }
   else if ( keyCode == 'W' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'O' );
      }
      else {
         SendKey( VK_OEM_COMMA ); // ,<
      }
   }
   else if ( keyCode == 'E' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'E' );
      }
      else {
         SendKey( VK_OEM_PERIOD ); // .>
      }
   }
   else if ( keyCode == 'R' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'Y' );
         SendKey( 'U' );
      }
      else {
         SendKey( VK_OEM_MINUS ); // -_
      }
   }
   else if ( keyCode == 'T' ){
      if( isLastKeyConsonant ){
         isLastKeyConsonant = FALSE;
         SendKey( 'H' );
         SendKey( 'I' );
      }
      else {
         SendKey( VK_OEM_PLUS ); // =+
      }
   }
   //��i�E
   else if ( keyCode == 'Y' ){
      SendKey( 'Y' );
   }
   else if ( keyCode == 'U' ){
      SendKey( 'D' );
   }
   else if ( keyCode == 'I' ){
      SendKey( 'G' );
   }
   else if ( keyCode == 'O' ){
      SendKey( 'F' );
   }
   else if ( keyCode == 'P' ){
      SendKey( 'P' );
   }
   else if ( keyCode == VK_OEM_4 ){ // [{
      SendKey( 'B' );
   }
   else if ( keyCode == VK_OEM_6 ){ // ]}
      SendKey( 'V' );
   }
   //���i��
   else if ( keyCode == 'A' ){
      SendKey( 'A' );
   }
   else if ( keyCode == 'S' ){
      SendKey( 'O' );
   }
   else if ( keyCode == 'D' ){
      SendKey( 'E' );
   }
   else if ( keyCode == 'F' ){
      SendKey( 'U' );
   }
   else if ( keyCode == 'G' ){
      SendKey( 'I' );
   }
   //���i�E
   else if ( keyCode == 'H' ){
      SendKey( 'H' );
   }
   else if ( keyCode == 'J' ){
      SendKey( 'T' );
   }
   else if ( keyCode == 'K' ){
      SendKey( 'K' );
   }
   else if ( keyCode == 'L' ){
      SendKey( 'R' );
   }
   else if ( keyCode == VK_OEM_1 ){ // ;:
      SendKey( 'S' );
   }
   else if ( keyCode == VK_OEM_7 ){ // '"
      SendKey( 'Q');
   }
   //���i��
   else if ( keyCode == 'Z' ){
      if( isLastKeyConsonant ){
         SendKey( 'A' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( VK_OEM_1 ); // ;:
      }
   }
   else if ( keyCode == 'X' ){
      if( isLastKeyConsonant ){
         SendKey( 'O' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( 'X' );
      }
   }
   else if ( keyCode == 'C' ){
      if( isLastKeyConsonant ){
         SendKey( 'E' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( 'C' );
      }
   }
   else if ( keyCode == 'V' ){
      if( isLastKeyConsonant ){
         SendKey( 'U' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( 'L' );
      }
   }
   else if ( keyCode == 'B' ){
      if( isLastKeyConsonant ){
         SendKey( 'I' );
         SendKey( 'N' );
         SendKey( 'N' );
      }
      else {
         SendKey( VK_OEM_2 ); // /?
      }
   }
   //���i�E
   else if ( keyCode == 'N' ){
      SendKey( 'N' );
   }
   else if ( keyCode == 'M' ){
      SendKey( 'M' );
   }
   else if ( keyCode == VK_OEM_COMMA ){ // ,<
      SendKey( 'W' );
   }
   else if ( keyCode == VK_OEM_PERIOD ){ // .>
      SendKey( 'J' );
   }
   else if ( keyCode == VK_OEM_2 ){ // /?
      SendKey( 'Z' );
   }
   //����ւ��ΏۊO
   else {
      lastKeyCode = keyCode;
      isLastKeyConsonant = FALSE;
      return CallNextHookEx(hHook, nCode, wp, lp);
   }

   return TRUE; 
}

void HookStart()
{
   printf( "keyChar, keyCode, scanCode, keyFlags, eventType\n" );
   hHook = SetWindowsHookEx( WH_KEYBOARD_LL, HookProc, hInst, 0);
}

void HookEnd()
{
   UnhookWindowsHookEx(hHook);
}


char SendKey( char keyCode ){

   keybd_event( keyCode, NULL, NULL, NULL );
   keybd_event( keyCode, NULL, KEYEVENTF_KEYUP, NULL );

   return keyCode;
}

char SendKey( char modifierCode, char keyCode ){

   keybd_event( modifierCode, NULL, NULL, NULL );

   keybd_event( keyCode, NULL, NULL, NULL );
   keybd_event( keyCode, NULL, KEYEVENTF_KEYUP, NULL );

   keybd_event( modifierCode, NULL, KEYEVENTF_KEYUP, NULL );

   return keyCode;
}

