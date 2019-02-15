/**
 * mptag
 * 
 * @author Seo-4d696b75
 * @version 2019-01-09
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charset.h"
#include "tagcontainer.h"

#define true 1
#define false 0

// select encoding of text-based data into stdout or from stdin
// 1:UTF-8 0:ISO_8859_1(ShiftJIS also accecpted)
#define DEFAULT_ENCODING_UTF8 0

#define LITTLE_ENDIAN 1

// set const param
const char TAG_VERTION_ID3v2_3 = (char)3;
const char TAG_VERTION_ID3v2_4 = (char)4;

#define NONE -1
#define ISO_8859_1 0
#define UTF_16_BOM 1
#define UTF_16_BE 2
#define UTF_8 3

const char ENCODING_NONE = (char)NONE;
const char ENCODING_ISO_8859_1 = (char)ISO_8859_1;
const char ENCODING_UTF_16_BOM = (char)UTF_16_BOM;
const char ENCODING_UTF_16_BE = (char)UTF_16_BE;
const char ENCODING_UTF_8 = (char)UTF_8;
#if DEFAULT_ENCODING_UTF8
const char ENCODING_DEFAULT = (char)UTF_8;
#else
const char ENCODING_DEFAULT = (char)ISO_8859_1;
#endif

#define BYTE_SIZE(encoding) \
    (encoding == ENCODING_ISO_8859_1 || encoding == ENCODING_UTF_8) ? 1 : 2

#define FLAG_SRC_NO_NULL_SUFFIX 0b1
#define FLAG_DES_NO_NULL_SUFFIX 0b10
int convert_text(char* des, int des_size, const char* src, int src_size,
                 char des_encoding, char src_encoding, int flags);


#define convert_str(des,des_size,src,encoding) convert_text(des,des_size,src,0,ENCODING_DEFAULT,encoding,0)
				 

int is_text_type(const char* id) {
    static char* data_array =
        "TALBTCONTDRCTIT2TPE1TRCKTBPMTCOMTCOPTDLYTDORTDRLTENCTEXTTFLTTIT1TIT3TK"
        "EYTLANTLENTMEDTOALTOFNTOLYTOPETOWNTPE2TPE3TPE4TPOSTPUBTRSNTRSOTSRCTSS"
        "E";
    char* end = data_array + strlen(data_array);
    for (char* pt = data_array; pt < end; pt += 4) {
        if (strncmp(id, pt, 4) == 0) return true;
    }
    return false;
}

int is_url_type(const char* id) {
    static char* data_array = "WCOMWCOPWOAFWOARWAOSWORSWPAYWPUB";
    char* end = data_array + strlen(data_array);
    for (char* pt = data_array; pt < end; pt += 4) {
        if (strncmp(id, pt, 4) == 0) return true;
    }
    return false;
}

int is_frame_id(const char* name) {
    for (int i = 0; i < 4; i++) {
        if (name[i] == '\0') return false;
        if (('0' <= name[i] && name[i] <= '9') ||
            ('A' <= name[i] && name[i] <= 'Z')) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

int parse_syncSafeInt(char* bytes) {
    int value = 0x0;
    value += (bytes[0] & 0x7f);
    value <<= 7;
    value += (bytes[1] & 0x7f);
    value <<= 7;
    value += (bytes[2] & 0x7f);
    value <<= 7;
    value += (bytes[3] & 0x7f);
    return value;
}

int parse_int(char* bytes) {
#if LITTLE_ENDIAN
    char temp = bytes[0];
    bytes[0] = bytes[3];
    bytes[3] = temp;
    temp = bytes[1];
    bytes[1] = bytes[2];
    bytes[2] = temp;
#endif
    int value;
    memcpy(&value, bytes, 4);
    return value;
}

void encode_int(char* des, int src) {
    memcpy(des, &src, 4);
#if LITTLE_ENDIAN
    char temp = des[0];
    des[0] = des[3];
    des[3] = temp;
    temp = des[1];
    des[1] = des[2];
    des[2] = temp;
#endif
}

void encode_syncSafeInt(char* des, int src) {
    unsigned int val = src & 0x0FFFFFFF;
    des[3] = (char)(0x7F & val);
    val >>= 7;
    des[2] = (char)(0x7F & val);
    val >>= 7;
    des[1] = (char)(0x7F & val);
    val >>= 7;
    des[0] = (char)(0x7F & val);
}

int check_encoding(char encoding, TagFrame* frame) {
    if (encoding == 0) {
        frame->encoding = ENCODING_ISO_8859_1;
        frame->encoding_byte = 1;
    } else if (encoding == 1) {
        frame->encoding = ENCODING_UTF_16_BOM;
        frame->encoding_byte = 2;
    } else if (encoding == 2) {
        frame->encoding = ENCODING_UTF_16_BE;
        frame->encoding_byte = 2;
    } else if (encoding == 3) {
        frame->encoding = ENCODING_UTF_8;
        frame->encoding_byte = 1;
    } else {
        fprintf(stderr,
                "unexpected encoding:\"%d\" found in text-data frame(ID=%s).\n",
                encoding, frame->frame_id);
        return false;
    }
    return true;
}

int check_mime_type(char* buf, int max_size, TagFrame* frame) {
    int length = strnlen(buf, max_size);
    if ((length == 9 && strcmp(buf, "image/png") == 0) ||
        (length == 10 && strcmp(buf, "image/jpeg") == 0)) {
        frame->data_format = (char*)malloc(sizeof(char) * (length + 1));
        memcpy(frame->data_format, buf, length + 1);
        return length + 1;
    } else {
        if (length == max_size) buf[max_size - 1] = '\0';
        fprintf(stderr, "unexpected MIME type : %s at frame(ID=APIC)\n", buf);
        return 0;
    }
}

int check_description(TagFrame* frame, char* buf, int offset, int size) {
    int cnt = 0;
    for (; offset + cnt < size;) {
        if (frame->encoding_byte == 1) {
            if (buf[offset + cnt] == '\0') break;
            cnt += 1;
        } else {
            if (buf[offset + cnt] == '\0' && buf[offset + cnt + 1] == '\0')
                break;
            cnt += 2;
        }

        if (offset + cnt >= size) {
            fprintf(stderr, "description not found at frame(ID=%s)\n",
                    frame->frame_id);
            return 0;
        }
    }
    cnt += frame->encoding_byte;
    frame->description = (char*)malloc(sizeof(char) * cnt);
    if (frame->description == NULL) return 0;
    memcpy(frame->description, buf + offset, cnt);
    return cnt;
}

int read_text_frame(TagFrame* frame, char* buf, int size, int has_language,
                    int has_description) {
    if (!check_encoding(buf[0], frame)) return false;
    int offset = 1;
    // check language if any
    if (has_language) {
        frame->data_format = (char*)malloc(sizeof(char) * 4);
        frame->data_format[3] = '\0';
        memcpy(frame->data_format, buf + offset, 3);
        if (strlen(frame->data_format) != 3) {
            fprintf(stderr,
                    "invalid language value found : %s at frame(ID=%s)\n",
                    frame->data_format, frame->frame_id);
            return false;
        }
        offset += 3;
    }
    // check description if any
    if (has_description) {
        int length = check_description(frame, buf, offset, size);
        if (length == 0) return false;
        offset += length;
    }
    // check text data
    for (char* pt = buf + offset; pt < buf + size;) {
        if (frame->encoding_byte == 1) {
            if (pt[0] == '\0') break;
            pt += 1;
        } else {
            if (pt[0] == '\0' && pt[1] == '\0') break;
            pt += 2;
        }
        if (pt >= buf + size) {
            fprintf(stderr, "text not end with 'null' at frame(ID=%s)\n",
                    frame->frame_id);
            return false;
        }
    }
    frame->text = (char*)malloc(sizeof(char) * (size - offset));
    if (frame->text == NULL) return false;
    memcpy(frame->text, buf + offset, size - offset);
    return true;
}

int read_picture_frame(TagFrame* frame, char* buf, int size) {
    if (!check_encoding(buf[0], frame)) return false;
    int offset = 1;
    int length = check_mime_type(buf + offset, size - offset, frame);
    if (length == 0) return false;
    offset += length;
    frame->image_type = buf[offset++];
    length = check_description(frame, buf, offset, size);
    if (length == 0) return false;
    offset += length;
    int data_size = size - offset;
    frame->binary = (char*)malloc(sizeof(char) * data_size);
    if (frame->binary == NULL) return false;
    memcpy(frame->binary, buf + offset, data_size);
    frame->binary_size = data_size;
    return true;
}

int read_frame(TagFrame* frame, FILE* src, char version) {
    // check frame ID, then identify data-type
    char* id = frame->frame_id;
    int has_language = false;
    int has_description = false;
    if (strcmp(id, "APIC") == 0) {
        frame->data_type = TYPE_PICTURE;
    } else if (strcmp(id, "COMM") == 0) {
        frame->data_type = TYPE_TEXT_EN_LAN_DES;
        has_language = true;
        has_description = true;
    } else if (strcmp(id, "USER") == 0) {
        frame->data_type = TYPE_TEXT_EN_LAN;
        has_language = true;
    } else if (strcmp(id, "USLT") == 0) {
        frame->data_type = TYPE_TEXT_EN_LAN_DES;
        has_language = true;
        has_description = true;
    } else if (is_text_type(id)) {
        frame->data_type = TYPE_TEXT_EN;
    } else if (is_url_type(id)) {
        frame->data_type = TYPE_URL;
    } else if (id[0] == 'T' || id[0] == 'W') {
        frame->data_type = TYPE_TEXT_EN_DES;
        has_description = true;
    } else {
        frame->data_type = TYPE_BINARY;
    }

    // check frame size
    char buffer[4];
    if (fread(buffer, 1, 4, src) != 4) return false;
    int size = version == TAG_VERTION_ID3v2_3 ? parse_int(buffer)
                                              : parse_syncSafeInt(buffer);
    frame->frame_size = size;

    // read header flags 2byte
    if (fread(frame->frame_flag, 1, 2, src) != 2) return false;

    switch (frame->data_type) {
        case TYPE_BINARY:
            frame->binary = (char*)malloc(sizeof(char) * size);
            if (frame->binary == NULL) break;
            if (fread(frame->binary, 1, size, src) != size) break;
            frame->binary_size = size;
            return true;
        case TYPE_URL:
            frame->url = (char*)malloc(sizeof(char) * size);
            if (frame->url == NULL) break;
            if (fread(frame->url, 1, size, src) != size) break;
            return true;
        case TYPE_PICTURE:
        default:;
            char* buf = (char*)malloc(sizeof(char) * size);
            if (buf == NULL) break;
            if (fread(buf, 1, size, src) != size) break;
            int result = false;
            if (frame->data_type == TYPE_PICTURE) {
                result = read_picture_frame(frame, buf, size);
            } else {
                result = read_text_frame(frame, buf, size, has_language,
                                         has_description);
            }
            free(buf);
            return result;
    }
    return false;
}

TagFrame* initialize_frame(FILE* file, char version) {
    // check ID valid
    char id[5];
    id[4] = '\0';
    if (fread(id, 1, 4, file) != 4) return NULL;
    fseek(file, -4, SEEK_CUR);
    long fd_pos = ftell(file);
    if (strlen(id) < 4) return NULL;
    for (int i = 0; i < 4; i++) {
        if (('0' <= id[i] && id[i] <= '9') || ('A' <= id[i] && id[i] <= 'Z')) {
            continue;
        } else {
            return NULL;
        }
    }
    fseek(file, 4, SEEK_CUR);

    TagFrame* frame = (TagFrame*)malloc(sizeof(TagFrame));
    memcpy(frame->frame_id, id, 5);
    frame->frame_size = -1;
    frame->encoding = ENCODING_NONE;
    frame->text = NULL;
    frame->description = NULL;
    frame->data_format = NULL;
    frame->binary = NULL;
    frame->url = NULL;
    frame->next = NULL;

    if (read_frame(frame, file, version)) {
        return frame;
    } else {
        release_frame(frame);
        free(frame);
        fseek(file, fd_pos, SEEK_SET);
        return NULL;
    }
}

void release_frame(TagFrame* frame) {
    if (frame == NULL) return;
    free(frame->text);
    free(frame->description);
    free(frame->data_format);
    free(frame->binary);
    free(frame->url);

    free(frame);
}

void release_container(TagContaner* container) {
    if (container->file != NULL) fclose(container->file);
    free(container->extra_header);
    TagFrame* frame = container->frame;
    while (frame != NULL) {
        TagFrame* next = frame->next;
        release_frame(frame);
        frame = next;
    }
    // free(container);
}

int read_header(TagContaner* container) {
    // read header
    FILE* src = container->file;
    char buffer[4];
    if (fread(buffer, 1, 3, src) != 3) return false;
    buffer[3] = '\0';
    if (strcmp(buffer, "ID3") != 0) {
        fprintf(stderr, "invalid header ID: %s\n", buffer);
        return false;
    }
    if (fread(buffer, 1, 3, src) != 3) return false;
    if (buffer[0] != TAG_VERTION_ID3v2_3 && buffer[0] != TAG_VERTION_ID3v2_4) {
        fprintf(stderr, "ID3v2.%d.%d not supported.\n", buffer[0], buffer[1]);
        return false;
    }
    container->version = buffer[0];
    memcpy(container->header_version, buffer, 2);
    container->header_flag = buffer[2];
    container->has_extra_header = (buffer[2] & 0b00100000) > 0;

    if (fread(buffer, 1, 4, src) != 4) return false;
    int length = parse_syncSafeInt(buffer);
    container->frames_size = length;

    if (container->has_extra_header) {
        if (fread(buffer, 1, 4, src) != 4) return false;
        int size = container->version == TAG_VERTION_ID3v2_3
                       ? parse_int(buffer)
                       : parse_syncSafeInt(buffer);
        container->extra_header = (char*)malloc(sizeof(char) * size);
        if (container->extra_header == NULL) return false;
        if (fread(container->extra_header, 1, size, src) != size) return false;
    }

    return true;
}

int read_frames(TagContaner* container) {
    FILE* src = container->file;
    TagFrame* frame = initialize_frame(src, container->version);
    if (frame == NULL) {
        fprintf(stderr, "fail to read frame.\n");
        return false;
    }
    container->frame = frame;
    TagFrame* previous = frame;
    int cnt = 1;
    while (true) {
        frame = initialize_frame(src, container->version);
        if (frame == NULL) break;
        previous->next = frame;
        previous = frame;
        cnt++;
    }
    long pos = ftell(src);
    if (pos > 10 + container->extra_header_size + container->frames_size) {
        fprintf(stderr, "Warning : tag size mismatched.\n");
    }
    container->offset = pos;
    container->frames_cnt = cnt;
    return true;
}

int initialize_container(TagContaner* container, const char* filename) {
    FILE* src = fopen(filename, "rb");
    if (src == NULL) {
        fprintf(stderr, "fail to open file: %s\n", filename);
        return false;
    }
    container->filename = filename;
    container->file = src;
    container->offset = -1;
    container->frames_size = -1;
    container->has_extra_header = false;
    container->extra_header_size = 0;
    container->extra_header = NULL;
    container->frames_cnt = 0;
    container->frame = NULL;

    if (!read_header(container) || !read_frames(container)) {
        release_container(container);
        return false;
    }

    return true;
}

int str_length(const char* str, int byte) {
    int cnt = 0;
    if (byte == 2) {
        while (str[0] != '\0' || str[1] != '\0') {
            str += 2;
            cnt++;
        }
    } else if (byte == 1) {
        while (*str != '\0') {
            str++;
            cnt++;
        }
    } else {
        return 0;
    }
    return cnt;
}

void swap_byte(char* str, int size) {
    char buf;
    for (char* pt = str; pt < str + size; pt += 2) {
        buf = pt[0];
        pt[0] = pt[1];
        pt[1] = buf;
    }
}

/*
int convert_str(char* des, int des_size, char* src, char encoding) {
    if (des_size < 0) return 0;
    const int byte =
        (encoding == ENCODING_UTF_16_BE || encoding == ENCODING_UTF_16_BOM) ? 2
                                                                            : 1;
    int length = str_length(src, byte) + 1;
    if (des == NULL || des_size == 0) {
        des = NULL;
        des_size = 0;
    }
    int result = 0;
    int size = byte * length;
    char* buf = NULL;
    unsigned char bom1, bom2;
#if ENCODING_UTF8
    switch (encoding) {
        case UTF_16_BE:
            buf = (char*)malloc(sizeof(char) * size);
            memcpy(buf, src, size);
            swap_byte(buf, size);
            result = Utf16ToUtf8(des, des_size, buf, size);
            break;
        case UTF_16_BOM:
            bom1 = src[0];
            bom2 = src[1];
            size = byte * (length - 1);
            if (bom1 == 0xff && bom2 == 0xfe) {
                // Lillte Endian
                src += 2;
            } else if (bom1 == 0xfe && bom2 == 0xff) {
                buf = (char*)malloc(sizeof(char) * size);
                memcpy(buf, src + 2, size);
                swap_byte(buf, size);
                src = buf;
            } else {
                break;
            }
            result = Utf16ToUtf8(des, des_size, src, size);
            break;
        case UTF_8:
            if (des == NULL) {
                return size;
            } else {
                if (des_size > size) des_size = size;
                memcpy(des, src, des_size);
                return des_size;
            }
        case ISO_8859_1:
            return SJIStoUTF8(des, des_size, src, size);
        case NONE:
            return 0;
    }
#else
    switch (encoding) {
        case UTF_16_BE:
            buf = (char*)malloc(sizeof(char) * size);
            memcpy(buf, src, size);
            swap_byte(buf, size);
            result = UTF16toSJIS(des, des_size, buf, size);
            break;
        case UTF_16_BOM:
            bom1 = src[0];
            bom2 = src[1];
            size = byte * (length - 1);
            if (bom1 == 0xff && bom2 == 0xfe) {
                // Lillte Endian
                src += 2;
            } else if (bom1 == 0xfe && bom2 == 0xff) {
                buf = (char*)malloc(sizeof(char) * size);
                memcpy(buf, src + 2, size);
                swap_byte(buf, size);
                src = buf;
            } else {
                break;
            }
            result = UTF16toSJIS(des, des_size, src, size);
            break;
        case UTF_8:
            return UTF16toSJIS(des, des_size, src, size);
        case ISO_8859_1:
            if (des == NULL) {
                return size;
            } else {
                if (des_size > size) des_size = size;
                memcpy(des, src, des_size);
                return des_size;
            }
        case NONE:
            return 0;
    }
#endif
    free(buf);
    return result;
}*/

char* get_encoding_str(char encoding) {
    static char* str0 = "ISO_8859_1";
    static char* str1 = "UTF-16BOM";
    static char* str2 = "UTF-16BE";
    static char* str3 = "UTF-8";
    switch (encoding) {
        case ISO_8859_1:
            return str0;
        case UTF_16_BOM:
            return str1;
        case UTF_16_BE:
            return str2;
        case UTF_8:
            return str3;
        default:
            return "??";
    }
}

void show_picture_frame(TagFrame* frame, FILE* des) {
    int length = convert_str(NULL, 0, frame->description, frame->encoding);
    char* buf = (char*)malloc(sizeof(char) * length);
    if (buf == NULL) return;
    convert_str(buf, length, frame->description, frame->encoding);
    fprintf(des, "FrameID : APIC\nMIME format : %s\n", frame->data_format);
    fprintf(des, "Description : \"%s\"(%s)\n", buf,
            get_encoding_str(frame->encoding));
    fprintf(des, "PictureType : %d\n", frame->image_type);
    fprintf(des, "ImageSize : %dByte\n", frame->binary_size);
    free(buf);
}

void show_text(TagFrame* frame, FILE* out, int has_language) {
    int length = convert_str(NULL, 0, frame->text, frame->encoding);
    char* buf = (char*)malloc(sizeof(char) * length);
    if (buf == NULL) return;
    convert_str(buf, length, frame->text, frame->encoding);
    fprintf(out, "FrameID : %s\n", frame->frame_id);
    if (has_language) fprintf(out, "Language : %s\n", frame->data_format);
    fprintf(out, "Text : \"%s\"(%s)\n", buf, get_encoding_str(frame->encoding));
    free(buf);
}

void show_text_desc(TagFrame* frame, FILE* out, int has_language) {
    int length = convert_str(NULL, 0, frame->description, frame->encoding);
    char* des = (char*)malloc(sizeof(char) * length);
    convert_str(des, length, frame->description, frame->encoding);
    length = convert_str(NULL, 0, frame->text, frame->encoding);
    char* text = (char*)malloc(sizeof(char) * length);
    convert_str(text, length, frame->text, frame->encoding);
    fprintf(out, "FrameID : %s\n", frame->frame_id);
    if (has_language) fprintf(out, "Language : %s\n", frame->data_format);
    fprintf(out, "Description : \"%s\"\nText : \"%s\"(%s)\n", des, text, get_encoding_str(frame->encoding));
    free(des);
    free(text);
}

void show_frame(TagFrame* frame, FILE* des) {
    switch (frame->data_type) {
        case TYPE_PICTURE:
            show_picture_frame(frame, des);
            break;
        case TYPE_TEXT_EN_LAN:
            // USER
            show_text(frame, des, true);
            break;
        case TYPE_TEXT_EN_LAN_DES:
            // COMM USLT
            show_text_desc(frame, des, true);
            break;
        case TYPE_TEXT_EN:
            // normal text type
            show_text(frame, des, false);
            break;
        case TYPE_TEXT_EN_DES:
            // custom text type
            show_text_desc(frame, des, false);
            break;
        case TYPE_URL:
            fprintf(des, "FrameID : %s\nURL : %s\n", frame->frame_id,
                    frame->url);
            break;
        case TYPE_BINARY:
            fprintf(des, "FrameID : %s\nBinarySize : %d\n", frame->frame_id,
                    frame->binary_size);
            break;
    }
}

int str2utf8(char* des, int des_size, const char* src, int size,
             char encoding) {
    int result = 0;
    char* buf = NULL;
    unsigned char bom1, bom2;
    switch (encoding) {
        case UTF_16_BE:
            buf = (char*)malloc(sizeof(char) * size);
            memcpy(buf, src, size);
            swap_byte(buf, size);
            result = Utf16ToUtf8(des, des_size, buf, size);
            break;
        case UTF_16_BOM:
            bom1 = src[0];
            bom2 = src[1];
            size -= 2;
            if (bom1 == 0xff && bom2 == 0xfe) {
                // Lillte Endian
                src += 2;
            } else if (bom1 == 0xfe && bom2 == 0xff) {
                buf = (char*)malloc(sizeof(char) * size);
                memcpy(buf, src + 2, size);
                swap_byte(buf, size);
                src = buf;
            } else {
                break;
            }
            result = Utf16ToUtf8(des, des_size, src, size);
            break;
        case UTF_8:
            if (des == NULL) {
                return size;
            } else {
                if (des_size > size) des_size = size;
                memcpy(des, src, des_size);
                return des_size;
            }
        case ISO_8859_1:
            return SJIStoUTF8(des, des_size, src, size);
        case NONE:
            return 0;
    }
    free(buf);
    return result;
}

int str2sjis(char* des, int des_size, const char* src, int size,
             char encoding) {
    int result = 0;
    char* buf = NULL;
    unsigned char bom1, bom2;
    switch (encoding) {
        case UTF_16_BE:
            buf = (char*)malloc(sizeof(char) * size);
            memcpy(buf, src, size);
            swap_byte(buf, size);
            result = UTF16toSJIS(des, des_size, buf, size);
            break;
        case UTF_16_BOM:
            bom1 = src[0];
            bom2 = src[1];
            size -= 2;
            if (bom1 == 0xff && bom2 == 0xfe) {
                // Lillte Endian
                src += 2;
            } else if (bom1 == 0xfe && bom2 == 0xff) {
                buf = (char*)malloc(sizeof(char) * size);
                memcpy(buf, src + 2, size);
                swap_byte(buf, size);
                src = buf;
            } else {
                break;
            }
            result = UTF16toSJIS(des, des_size, src, size);
            break;
        case UTF_8:
            return UTF8toSJIS(des, des_size, src, size);
        case ISO_8859_1:
            if (des == NULL) {
                return size;
            } else {
                if (des_size > size) des_size = size;
                memcpy(des, src, des_size);
                return des_size;
            }
        case NONE:
            return 0;
    }
    free(buf);
    return result;
}

int str2utf16(char* des, int des_size, const char* src, int size, char encoding,
              int bom, int big_endian) {
    int src_big_endian = 0;
    int result = 0;
    if (bom && des != NULL) {
        if (des_size < 2) return 0;
        if (big_endian) {
            des[0] = (char)0xfe;
            des[1] = (char)0xff;
        } else {
            des[0] = (char)0xff;
            des[1] = (char)0xfe;
        }
        des += 2;
        des_size -= 2;
    }
    if (bom) result += 2;
    switch (encoding) {
        case UTF_16_BE:
            src_big_endian = true;
            if (des == NULL) {
                result += size;
            } else {
                if (des_size > size) des_size = size;
                memcpy(des, src, des_size);
                result += des_size;
            }
            break;
        case UTF_16_BOM:;
            unsigned char bom1, bom2;
            bom1 = src[0];
            bom2 = src[1];
            if (bom1 == 0xff && bom2 == 0xfe) {
                // Lillte Endian
                src_big_endian = false;
            } else if (bom1 == 0xfe && bom2 == 0xff) {
                src_big_endian = true;
            } else {
                return 0;
            }
            size -= 2;
            if (des == NULL) {
                result += size;
            } else {
                if (des_size > size) des_size = size;
                memcpy(des, src + 2, des_size);
                result += des_size;
            }
            break;
        case UTF_8:
            src_big_endian = false;
            result += Utf8ToUtf16(des, des_size, src, size);
            break;
        case ISO_8859_1:
            src_big_endian = false;
            result += SJIStoUTF16(des, des_size, src, size);
            break;
        case NONE:
            return 0;
    }
    if (des == NULL) return result;
    if (big_endian != src_big_endian) {
        swap_byte(des, des_size);
    }
    return result;
}

int convert_text(char* des, int des_size, const char* src, int src_size,
                 char des_encoding, char src_encoding, int flags) {
    if (src == NULL || des_size < 0) return 0;
    int has_src_null = (flags & FLAG_SRC_NO_NULL_SUFFIX) == 0;
    int has_des_null = (flags & FLAG_DES_NO_NULL_SUFFIX) == 0;
	const int src_byte = BYTE_SIZE(src_encoding);
	const int des_byte = BYTE_SIZE(des_encoding);
    if (des != NULL && has_des_null && des_size < des_byte) return 0;
    int size;
    if (has_src_null) {
        size = src_byte * str_length(src, src_byte);
    } else {
        size = src_size;
        if (size <= 0 || size%src_byte != 0) return 0;
    }
    int result = 0;
    if (des == NULL || des_size == 0) {
        des = NULL;
        des_size = 0;
    }

    switch (des_encoding) {
        case UTF_8:
            result = str2utf8(des, des_size, src, size, src_encoding);
            break;
        case ISO_8859_1:
            result = str2sjis(des, des_size, src, size, src_encoding);
            break;
        case UTF_16_BE:
            result =
                str2utf16(des, des_size, src, size, src_encoding, false, true);
            break;
        case UTF_16_BOM:
            // TODO select endian of output text
            result =
                str2utf16(des, des_size, src, size, src_encoding, true, false);
            break;
        case NONE:
            return 0;
	}
    if (des == NULL) {
        if (has_des_null) result += des_byte;
        return result;
    }
    if (has_des_null) {
        if (result + des_byte > des_size) result = des_size - 2;
        if (des_byte == 1) {
            des[result] = '\0';
        } else {
            des[result] = '\0';
            des[result + 1] = '\0';
        }
        result += des_byte;
    }
    return result;
}

int extract_data(TagFrame* frame, FILE* des, char encoding) {
    int result = 0;
    int length = 0;
    switch (frame->data_type) {
        case TYPE_PICTURE:
        case TYPE_BINARY:
            result = fwrite(frame->binary, 1, frame->binary_size, des);
            return result == frame->binary_size;
        case TYPE_TEXT_EN_LAN:
        case TYPE_TEXT_EN_LAN_DES:
        case TYPE_TEXT_EN:
        case TYPE_TEXT_EN_DES:;
            int size = convert_text(NULL, 0, frame->text, 0, encoding,
                                    frame->encoding, FLAG_DES_NO_NULL_SUFFIX);
            if (size == 0) return false;
            char* buf = (char*)malloc(sizeof(char) * size);
            int result = convert_text(buf, size, frame->text, 0, encoding,
                                      frame->encoding, FLAG_DES_NO_NULL_SUFFIX);
            if (result == size) {
                result = fwrite(buf, 1, size, des);
            }
            free(buf);
            return result == size;
        case TYPE_URL:
            length = strlen(frame->url);
            result = fwrite(frame->url, 1, length, des);
            return result == length;
    }
    return false;
}

int extract_binary(TagFrame* frame, FILE* des) {
    if (frame->binary == NULL) {
        fprintf(stderr, "Error : binary not set.\n");
        return false;
    }
    return fwrite(frame->binary, 1, frame->binary_size, des) ==
           frame->binary_size;
}

int get_raw_frame(FILE* des, TagFrame* src, char version) {
    int size;
    if (fwrite(src->frame_id, 1, 4, des) != 4) return 0;
    fseek(des, 4, SEEK_CUR);
    if (fwrite(src->frame_flag, 1, 2, des) != 2) return 0;
    int result = 0;
    unsigned char encoding_symbol = src->encoding;
    const int byte = src->encoding_byte;
    switch (src->data_type) {
        case TYPE_PICTURE:
            if (fwrite(&encoding_symbol, 1, 1, des) != 1) return 0;
            result = 1;
            size = strlen(src->data_format) + 1;
            if (fwrite(src->data_format, 1, size, des) != size) return 0;
            result += size;
            if (fwrite(&src->image_type, 1, 1, des) != 1) return 0;
            result += 1;
            size = byte * (str_length(src->description, byte) + 1);
            if (fwrite(src->description, 1, size, des) != size) return 0;
            result += size;
            if (fwrite(src->binary, 1, src->binary_size, des) !=
                src->binary_size)
                return 0;
            result += src->binary_size;
            break;
        case TYPE_BINARY:
            size = src->binary_size;
            if (fwrite(src->binary, 1, size, des) != size) return 0;
            result = size;
            break;
        case TYPE_TEXT_EN_LAN:
        case TYPE_TEXT_EN_LAN_DES:
        case TYPE_TEXT_EN:
        case TYPE_TEXT_EN_DES:
            if (fwrite(&encoding_symbol, 1, 1, des) != 1) return 0;
            result = 1;
            if (src->data_format != NULL) {
                if (fwrite(src->data_format, 1, 3, des) != 3) return 0;
                result += 3;
            }
            if (src->description != NULL) {
                size = byte * (str_length(src->description, byte) + 1);
                if (fwrite(src->description, 1, size, des) != size) return 0;
                result += size;
            }
            size = byte * (str_length(src->text, byte) + 1);
            if (fwrite(src->text, 1, size, des) != size) return 0;
            result += size;
            break;
        case TYPE_URL:
            size = strlen(src->url) + 1;
            if (fwrite(src->url, 1, size, des) != size) return 0;
            result = size;
            break;
    }

    fseek(des, -(6 + result), SEEK_CUR);
    char buf[4];
    if (version == TAG_VERTION_ID3v2_3) {
        encode_int(buf, result);
    } else {
        encode_syncSafeInt(buf, result);
    }
    if (fwrite(buf, 1, 4, des) != 4) return 0;
    fseek(des, 2 + result, SEEK_CUR);
    return result + 10;
}

int write_tag(TagContaner* container, FILE* des) {
    if (des == NULL) return false;
    const int header_size = 10 + container->extra_header_size;
    fseek(des, header_size, SEEK_SET);
    TagFrame* frame = container->frame;
    int frame_size = 0;
    int result;
    while (frame != NULL) {
        result = get_raw_frame(des, frame, container->version);
        if (result == 0) {
            fprintf(stderr, "Error : fail to write frame ID=%s\n",
                    frame->frame_id);
            fclose(des);
            return false;
        }
        frame_size += result;
        frame = frame->next;
    }
    fseek(des, 0, SEEK_SET);
    char buf[512];
    memcpy(buf, "ID3", 3);
    memcpy(buf + 3, container->header_version, 2);
    buf[5] = container->header_flag;
    encode_syncSafeInt(buf + 6, frame_size);
    fwrite(buf, 1, 10, des);
    if (container->has_extra_header) {
        fwrite(container->extra_header, 1, container->extra_header_size, des);
    }
    fseek(des, frame_size, SEEK_CUR);
    fprintf(stderr, "MP3 tag size : %d\n", frame_size);
    FILE* src = container->file;
    while (true) {
        result = fread(buf, 1, 512, src);
        if (result == 0) break;
        if (fwrite(buf, 1, result, des) != result) {
            fclose(des);
            return false;
        }
    }

    return true;
}

int set_text(TagFrame* frame, const char* id, DataType type, char text_encoding,
             char des_encoding, const char* language, const char* description,
             const char* text) {
    memcpy(frame->frame_id, id, 5);
    memset(frame->frame_flag, 0, 2);
    frame->data_type = type;
    frame->encoding = des_encoding;
	frame->encoding_byte = BYTE_SIZE(des_encoding);
	frame->data_format = NULL;
	if ( language != NULL ){
		frame->data_format = (char*)malloc(sizeof(char)*(strlen(language)+1));
		strcpy(frame->data_format, language);
	}
    frame->binary = NULL;
	frame->url = NULL;
	frame->description = NULL;
	frame->text = NULL;
	int size;
    if (description != NULL) {
        size = convert_text(NULL, 0, description, 0, des_encoding,
                                ENCODING_DEFAULT, 0);
        if (size == 0) return false;
        frame->description = (char*)malloc(sizeof(char) * size);
        if ( size != convert_text(frame->description, size, description, 0, des_encoding,
                                  ENCODING_DEFAULT, 0)) return false;
	}
	size = convert_text(NULL, 0, text, 0, des_encoding, text_encoding, 0);
	if ( size == 0 ) return false;
	frame->text = (char*)malloc(sizeof(char)*size);
	if ( size != convert_text(frame->text, size, text, 0, des_encoding, text_encoding, 0)) return false;
	return true;
}

int set_URL(TagFrame* frame, const char* id, const char* url){
	memcpy(frame->frame_id, id, 5);
	memset(frame->frame_flag, 0, 2);
	frame->data_type = TYPE_URL;
	frame->encoding = ENCODING_DEFAULT;
	frame->text = NULL;
	frame->description = NULL;
	frame->data_format = NULL;
	frame->binary = NULL;
	frame->url = (char*)malloc(sizeof(char)*(1+strlen(url)));
	if ( frame->url == NULL ) return false;
	strcpy(frame->url, url);
	return true;
}

int set_binary(TagFrame* frame, const char* id, FILE* data){
	memcpy(frame->frame_id, id, 5);
	memset(frame->frame_flag, 0, 2);
	frame->data_type = TYPE_BINARY;
	frame->text = NULL;
	frame->description = NULL;
	frame->data_format = NULL;
	frame->url = NULL;
	frame->binary = NULL;
	char* buf = (char*)malloc(sizeof(char)*512);
	int size = 0;
	while ( true ){
		int result = fread(buf+size, 1, 512, data);
		size += result;
		if ( result < 512 ) break;
		char* ex = (char*)malloc(sizeof(char) * (size+512));
		memcpy(ex, buf, size);
		free(buf);
		buf = ex;
	}
	frame->binary = buf;
	frame->binary_size = size;
	return true;
}

int set_picture(TagFrame* frame, char des_encoding,
                 const char* format, char type, const char* description,
                 FILE* data){

	strcpy(frame->frame_id, "APIC");
	memset(frame->frame_flag, 0, 2);
	frame->data_type = TYPE_PICTURE;
	frame->text = NULL;
	frame->description = NULL;
	frame->data_format = NULL;
	frame->url = NULL;
	frame->binary = NULL;
	frame->image_type = type;
	frame->data_format = (char*)malloc(sizeof(char)*(strlen(format)+1));
	strcpy(frame->data_format, format);
	frame->encoding = des_encoding;
	frame->encoding_byte = BYTE_SIZE(des_encoding);
	int size = convert_text(NULL, 0, description, 0, des_encoding, ENCODING_DEFAULT, 0);
	if ( size == 0 ) return false;
	frame->description = (char*)malloc(sizeof(char)*size);
	if ( size != convert_text(frame->description, size, description, 0, des_encoding, ENCODING_DEFAULT, 0)) return false;
	char* buf = (char*)malloc(sizeof(char)*512);
	size = 0;
	while ( true ){
		int result = fread(buf+size, 1, 512, data);
		size += result;
		if ( result < 512 ) break;
		char* ex = (char*)malloc(sizeof(char) * (size+512));
		memcpy(ex, buf, size);
		free(buf);
		buf = ex;
	}
	frame->binary = buf;
	frame->binary_size = size;
	return true;
}
