/**
 * UTF8 UTF16の文字列を相互変換するための関数群
 * 
 * UTF-16 → UTF-8 への変換実装サンプルプログラムその１。(Windows APIを使用)
 * http://yanchde.gozaru.jp/utf16_to_utf8/utf16_to_utf8_1.html
 *
 * UTF-8 → UTF-16 への変換実装サンプルプログラムその２。(Windows
 * APIを使用しない) 
 * http://yanchde.gozaru.jp/utf8_to_utf16/utf8_to_utf16_2.html
 * ビットパターン
 * ┌───────┬───────┬───────────────────────────┬──────────────────┐
 * │フォーマット  │Unicode       │UTF-8ビット列                                         │Unicodeビット列                     │
 * ├───────┼───────┼───────────────────────────┼──────────────────┤
 * │1バイトコード │\u0～\u7f     │0aaabbbb                                              │00000000 0aaabbbb                   │
 * ├───────┼───────┼───────────────────────────┼──────────────────┤
 * │2バイトコード │\u80～\u7ff   │110aaabb 10bbcccc                                     │00000aaa bbbbcccc                   │
 * ├───────┼───────┼───────────────────────────┼──────────────────┤
 * │3バイトコード │\u800～\uffff │1110aaaa 10bbbbcc 10ccdddd                            │aaaabbbb ccccdddd                   │
 * ├───────┼───────┼───────────────────────────┼──────────────────┤
 * │4バイトコード │--------------│11110??? 10?????? 10?????? 10??????                   │未対応                              │
 * ├───────┼───────┼───────────────────────────┼──────────────────┤
 * │5バイトコード │--------------│111110?? 10?????? 10?????? 10?????? 10??????          │未対応                              │
 * ├───────┼───────┼───────────────────────────┼──────────────────┤
 * │6バイトコード │--------------│1111110? 10?????? 10?????? 10?????? 10?????? 10?????? │未対応                              │
 * └───────┴───────┴───────────────────────────┴──────────────────┘
 */
#include "charset.h"
#include <windows.h>
#include <stdlib.h>

/**
 * 文字コードをUTF-16よりUTF-8へと変換。
 *
 * @param[out] dest 出力文字列UTF-8
 * @param[in]  dest_size destのバイト数
 * @param[in]  src 入力文字列UTF-16
 * @param[in]  src_size 入力文字列のバイト数
 *
 * @return 成功時には出力文字列のバイト数を戻します。
 *         dest_size に0を指定し、こちらの関数を呼び出すと、変換された
 *         文字列を格納するのに必要なdestのバイト数を戻します。
 *         関数が失敗した場合には、FALSEを戻します。
 */
int Utf16ToUtf8(char* dest, int dest_size, const char* src, int src_size) {
    UINT uCodePage;        /* コードページ */
    DWORD dwFlags;         /* 処理性能とマッピングのフラグ */
    LPCWSTR lpWideCharStr; /* ワイド文字の文字列のアドレス */
    int cchWideChar;       /* 文字列の文字数 */
    LPSTR lpMultiByteStr; /* 新しい文字列のバッファのアドレス */
    int cchMultiByte;     /* バッファのサイズ */
    LPCSTR
    lpDefaultChar; /* 文字をマップ不可能な場合のデフォルト文字のアドレス */
    LPBOOL
    lpfUsedDefaultChar; /* デフォルト文字が使われたときにセットされるフラグのアドレス
                         */

	// AIP details : https://docs.microsoft.com/en-us/windows/desktop/api/stringapiset/nf-stringapiset-multibytetowidechar
	
    int iResult;

    uCodePage = CP_UTF8;
    dwFlags = 0;
    lpWideCharStr = (wchar_t*)src;
    cchWideChar = src_size / sizeof(wchar_t);
    lpMultiByteStr = dest;
    cchMultiByte = dest_size;
    lpDefaultChar = NULL;
    lpfUsedDefaultChar = NULL;
    iResult = WideCharToMultiByte(uCodePage, dwFlags, lpWideCharStr,
                                  cchWideChar, lpMultiByteStr, cchMultiByte,
                                  lpDefaultChar, lpfUsedDefaultChar);
    return iResult;
}

/**
 * 文字コードをUTF-8よりUTF16へと変換。
 *
 * @param[out] dest 出力文字列UTF-16
 * @param[in]  dest_size destのサイズをバイト単位で指定
 * @param[in]  src 入力文字列UTF-8
 * @param[in]  src_size 入力文字列のバイト数
 *
 * @return 成功時には出力文字列のバイト数を戻します。
 *         dest_size に0を指定し、こちらの関数を呼び出すと、変換された
 *         文字列を格納するのに必要なdestのサイズのバイト数を戻します。
 *         関数が失敗した場合には、FALSEを戻します。
 */
int Utf8ToUtf16(char* des, int des_size, const char* src, int src_size) {
	
	wchar_t* dest = (wchar_t*)des;
	int dest_size = des_size / sizeof(wchar_t);
    size_t countNeedsWords;
    size_t cursor;
    char chBuffer[6];
    size_t nReadDataSize;
    int iCh1;
    int sizeBytes;
    wchar_t wcWork1, wcWork2, wcWork3;

    /*
     * 入力パラメータをチェック
     */
    if (dest_size == 0) {
        /* dest_size == 0 */
    } else {
        /* dest_size != 0 */
        if (dest == NULL) {
            /* Error -- Null Pointer Exception : dest */
            return FALSE;
        }
        if (dest_size < 0) {
            /* Error -- dest_size < 0 */
            return FALSE;
        }
    }
    if (src == NULL) {
        /* Error -- Null Pointer Exception : src */
        return FALSE;
    }
    if (src_size < 1) {
        /* Error -- src_size < 1 */
        return FALSE;
    }

    /*
     * BOMの除去
     */
    if ((*src == '\xef') && (*(src + 1) == '\xbb') && (*(src + 2) == '\xbf')) {
        /*
         * UTF-8のBOMはef bb bf
         * LE も BE も存在しないので、BOM付きの場合には除去。
         * BOMが無い場合はそのまま処理。
         */
        /* BOMがある場合にはBOMを無視する */
        src += 3;
    }

    countNeedsWords = 0;
    for (cursor = 0; cursor < src_size;) {
        /* srcより6バイトのデータを読み出し */
        nReadDataSize = (6 < (src_size - cursor))
                            ? 6
                            : (src_size - cursor);
        memcpy(chBuffer, (src + cursor), nReadDataSize);
        memset(chBuffer + nReadDataSize, 0, sizeof(chBuffer) - nReadDataSize);

        /* data size の調べる */
        iCh1 = ((int)(*chBuffer)) & 0x00ff;
        iCh1 = ~iCh1; /* ビット反転 */
        if (iCh1 & 0x0080) {
            /* 0aaabbbb */
            sizeBytes = 1;
        } else if (iCh1 & 0x0040) {
            /* error(ここに出現してはいけないコード) */
            return FALSE;
        } else if (iCh1 & 0x0020) {
            /* 110aaabb 10bbcccc */
            sizeBytes = 2;
        } else if (iCh1 & 0x0010) {
            /* 1110aaaa 10bbbbcc 10ccdddd */
            sizeBytes = 3;
        } else if (iCh1 & 0x0008) {
            /* 未対応のマッピング(UTF-16に存在しないコード) */
            sizeBytes = 4;
        } else if (iCh1 & 0x0004) {
            /* 未対応のマッピング(UTF-16に存在しないコード) */
            sizeBytes = 5;
        } else if (iCh1 & 0x0002) {
            /* 未対応のマッピング(UTF-16に存在しないコード) */
            sizeBytes = 6;
        } else {
            /* error(ここに出現してはいけないコード) */
            return FALSE;
        }

        /*
         * dest_size をチェック
         */
        if (dest_size && (dest_size < (countNeedsWords + 1))) {
            /* Error : memory is not enough for dest */
            return countNeedsWords;
        }

        /* sizeBytes毎に処理を分岐 */
        if (dest_size) switch (sizeBytes) {
                case 1:
                    /*
                     * ビット列
                     * (0aaabbbb)UTF-8 ... (00000000 0aaabbbb)UTF-16
                     */
                    *dest = ((wchar_t)(chBuffer[0])) &
                            (wchar_t)0x00ff; /* 00000000 0aaabbbb */
                    dest++;
                    break;
                case 2:
                    /*
                     * ビット列
                     * (110aaabb 10bbcccc)UTF-8 ... (00000aaa bbbbcccc)UTF-16
                     */
                    wcWork1 = ((wchar_t)(chBuffer[0])) &
                              (wchar_t)0x00ff; /* 00000000 110aaabb */
                    wcWork2 = ((wchar_t)(chBuffer[1])) &
                              (wchar_t)0x00ff; /* 00000000 10bbcccc */
                    wcWork1 <<= 6;             /* 00110aaa bb?????? */
                    wcWork1 &= 0x07c0;         /* 00000aaa bb000000 */
                    wcWork2 &= 0x003f;         /* 00000000 00bbcccc */
                    *dest = wcWork1 | wcWork2; /* 00000aaa bbbbcccc */
                    dest++;
                    break;
                case 3:
                    /*
                     * ビット列
                     * (1110aaaa 10bbbbcc 10ccdddd)UTF-8 ... (aaaabbbb
                     * ccccdddd)UTF-16
                     */
                    wcWork1 = ((wchar_t)(chBuffer[0])) &
                              (wchar_t)0x00ff; /* 00000000 1110aaaa */
                    wcWork2 = ((wchar_t)(chBuffer[1])) &
                              (wchar_t)0x00ff; /* 00000000 10bbbbcc */
                    wcWork3 = ((wchar_t)(chBuffer[2])) &
                              (wchar_t)0x00ff;           /* 00000000 10ccdddd */
                    wcWork1 <<= 12;                      /* aaaa???? ???????? */
                    wcWork1 &= 0xf000;                   /* aaaa0000 00000000 */
                    wcWork2 <<= 6;                       /* 0010bbbb cc?????? */
                    wcWork2 &= 0x0fc0;                   /* 0000bbbb cc000000 */
                    wcWork3 &= 0x003f;                   /* 00000000 00ccdddd */
                    *dest = wcWork1 | wcWork2 | wcWork3; /* aaaabbbb ccccdddd */
                    dest++;
                    break;
                case 4:
                case 5:
                case 6:
                default:
                    /* ダミーデータ(uff1f)を出力 */
                    *dest = (wchar_t)0xff1f;
                    dest++;
                    break;
            }
        countNeedsWords++;
        cursor += sizeBytes;
    }

    return countNeedsWords * sizeof(wchar_t);
}

int SJIStoUTF16(char* des, int des_size, const char* src, int src_size){
	UINT codePage = CP_ACP;
	DWORD dwFlags = 0;
	LPCCH lpMultiByteStr = src;
	int cbMultiByte = src_size;
	LPWSTR lpWideCharStr = (wchar_t*)des;
	int cchWideChar = des_size / sizeof(wchar_t);
	int result;
	result = MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
	return result * sizeof(wchar_t);
}

int UTF16toSJIS(char* des, int des_size, const char* src, int src_size){
	UINT codePage = CP_ACP;
	DWORD dwFlags = 0;
	LPCWSTR lpWideCharStr = (wchar_t*)src;
	int cchWideChar = src_size / sizeof(wchar_t);
	LPSTR lpMutliByteStr = des;
	int cbMultiByte = des_size;
	LPCSTR lpDefaultChar = NULL;
	LPBOOL lpUsedDefaultChar = NULL;
	return WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, lpMutliByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}

int SJIStoUTF8(char* des, int des_size, const char* src, int src_size){
	int length = SJIStoUTF16(NULL, 0 , src, src_size);
	char* buf = (char*)malloc(sizeof(char)*length);
	if ( buf == NULL ) return 0;
	int result = SJIStoUTF16(buf, length, src, src_size);
	if ( result != length ){
		free(buf);
		return 0;
	}
	result = Utf16ToUtf8(des, des_size, buf, length);
	free(buf);
	return result;
}


int UTF8toSJIS(char* des, int des_size, const char* src, int src_size){
	int length = Utf8ToUtf16(NULL, 0, src, src_size);
	char* buf = (char*)malloc(sizeof(char) * length);
	if ( buf == NULL ) return 0;
	int result = Utf8ToUtf16(buf, length, src, src_size);
	if ( result != length  ){
		free(buf);
		return 0;
	}
	result = UTF16toSJIS(des, des_size, buf, length);
	free(buf);
	return result;
}