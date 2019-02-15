#ifndef CHAR_UTF8_UTF16
#define CHAR_UTF8_UTF16

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
int Utf16ToUtf8(char* des, int des_size, const char* src, int src_size);



/**
 * 文字コードをUTF-8よりUTF16へと変換。
 *
 * @param[out] dest 出力文字列UTF-16
 * @param[in]  dest_size destのサイズをバイト単位で指定
 * @param[in]  src 入力文字列UTF-8
 * @param[in]  src_size 入力文字列のバイト数
 *
 * @return 成功時には出力文字列の文字数を戻します。
 *         dest_size に0を指定し、こちらの関数を呼び出すと、変換された
 *         文字列を格納するのに必要なdestのサイズのバイト数を戻します。
 *         関数が失敗した場合には、FALSEを戻します。
 */
int Utf8ToUtf16(char* des, int des_size, const char* src, int src_size);


int SJIStoUTF8(char* dse, int des_size, const const char* src, int src_size);

int UTF8toSJIS(char* des, int des_size, const char* src, int src_size);

int UTF16toSJIS(char* des, int des_size, const char* src, int src_size);

int SJIStoUTF16(char* des, int des_size, const char* src, int src_size);

#endif