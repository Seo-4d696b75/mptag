/**
 * mptag
 * 
 * @author Seo-4d696b75
 * @version 2019-01-09
 */
#include <fcntl.h>
#include <getopt.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "tagcontainer.h"

#define true 1
#define false 0

const char* optstring = "lrd:v:aAptcgyTe:o:";

#define LONG_OPT_FRAME_NAME 0
#define LONG_OPT_FRAME_ALIAS 1
const struct option long_opts[] = {
    //{"name", has_argument, *flag, value},
    {"APIC", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"COMM", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TALB", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TCON", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TDRC", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TIT2", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TPE1", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TRCK", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TBPM", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TCOM", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TCOP", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TDLY", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TDOR", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TDRL", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TENC", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TEXT", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TFLT", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TIT1", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TIT3", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TKEY", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TLAN", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TLEN", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TMED", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TOAL", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TOFN", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TOLY", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TOPE", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TOWN", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TPE2", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TPE3", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TPE4", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TPOS", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TPUB", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TRSN", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TRSO", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TSRC", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"TSSE", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"USER", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"USLT", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WCOM", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WCOP", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WOAP", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WOAR", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WAOS", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WORS", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WPAY", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"WPUB", optional_argument, NULL, LONG_OPT_FRAME_NAME},
    {"picture", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"pic", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"comment", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"comm", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"album", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"genre", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"date", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"year", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"title", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"artist", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"track", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"lyrics", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {"lyric", optional_argument, NULL, LONG_OPT_FRAME_ALIAS},
    {NULL, 0, NULL, 0}};

#define ACTION_UNABLE 0
#define ACTION_SHOW_FRAME 2
#define ACTION_DELETE_FRAME 3
#define ACTION_SET_FRAME 4

static int flag_output_raw = false;
static int flag_extract_data = false;
static int flag_show_all_frame = false;
static char* extract_data_des = '\0';
static int flag_write_file = false;
static char version_output = -1;
static char src_encoding;
static char des_encoding;

static int opt_parse_err = false;

typedef struct argument_t {
    int action;
    char frame_id[5];
    DataType data_type;

    char* arg_data;
    char encoding;

    char* text;
    char* description;
    char* data_format;  // language, or picture MIME type
    char image_type;    // APIC only
    char* data_file;    // text data or file name
    char* url;          // URL text
} Argument;

const char* parse_frame_alias(const char* value);
int parse_long_opt(Argument* des, const struct option* src, const char* optarg,
                   int flag);
int parse_opt(Argument* des, int result, const char* value);
int is_url_text(const char* str);
void release_arg(Argument* arg);
void show_one_frame(TagContaner* container, const char* id, FILE* out,
                    FILE* des);
void show_all_frame(TagContaner* container);
int set_one_frame(TagContaner* container, Argument* arg, FILE* out);
void execute(Argument* list, int size, const char* file);
void remove_one_frame(TagContaner* container, const char* id, FILE* out);
int parse_setting_opt(const char* value, Argument* arg);
char parse_encoding(const char* value);

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "file name not specifiend.\n");
        return 1;
    }
    // printf("file : %s\n", argv[1]);

    src_encoding = ENCODING_DEFAULT;
    des_encoding = ENCODING_DEFAULT;

    Argument* arg_list = (Argument*)malloc(sizeof(Argument) * (argc - 1));
    int arg_size = 0;
    opterr = 0;
    optind = 2;
    int result = -1;
    int long_index = -1;
    char* value = argv[2];
    while ((result = getopt_long(argc, argv, optstring, long_opts,
                                 &long_index)) != -1) {
        Argument* arg = arg_list + arg_size;
        arg->text = NULL;
        arg->description = NULL;
        arg->data_format = NULL;
        arg->data_file = NULL;
        arg->url = NULL;
        arg->arg_data = NULL;
        if (result == LONG_OPT_FRAME_NAME || result == LONG_OPT_FRAME_ALIAS) {
            const struct option* opt = long_opts + long_index;
            if (parse_long_opt(arg, opt, optarg, result)) {
                arg_size++;
            } else {
                release_arg(arg);
            }
        } else {
            if (parse_opt(arg, result, value)) {
                arg_size++;
            } else {
                release_arg(arg);
            }
        }
        if (opt_parse_err) {
            // error has occurred parsing opts.
            break;
        }
        value = argv[optind];
    }

    if (!opt_parse_err) execute(arg_list, arg_size, argv[1]);

    for (int i = 0; i < arg_size; i++) {
        release_arg(arg_list + i);
    }
    free(arg_list);
    return 0;
}

void execute(Argument* list, int size, const char* file) {
    TagContaner* container = (TagContaner*)malloc(sizeof(TagContaner));
    if (container == NULL) return;
    if (initialize_container(container, file)) {
		
        // show all the flags if flag set
        if (flag_show_all_frame) show_all_frame(container);
        // output stream if any
        FILE* des = NULL;
        if (flag_extract_data) {
            if (strcmp(extract_data_des, "-") == 0) {
                // for windows
                setmode(fileno(stdout), O_BINARY);
                des = stdout;
            } else {
                des = fopen(extract_data_des, "wb");
            }
            if (des == NULL) {
                fprintf(stderr, "Error : fail to open file \"%s\"\n",
                        extract_data_des);
                flag_extract_data = false;
                flag_write_file = false;
            }
        }

		FILE* out = des == stdout ? stderr : stdout;
		// check version
		if ( version_output > 0 ){
			if ( version_output != container->version ){
				fprintf(out, "change version ID3v2.%d -> ID3v2.%d\n", container->version, version_output);
				container->version = version_output;
				flag_write_file = true;
			}
		}else{
			version_output = container->version;
		}

        // iterate argument
        for (int i = 0; i < size; i++) {
            Argument* arg = list + i;
            if (arg->action == ACTION_SHOW_FRAME) {
                show_one_frame(container, arg->frame_id, out, des);
            } else if (arg->action == ACTION_SET_FRAME) {
				if ( !set_one_frame(container, arg, out) ){
					flag_write_file = false;
					break;
				}
            } else if (arg->action == ACTION_DELETE_FRAME) {
                remove_one_frame(container, arg->frame_id, out);
            }
        }

        if (flag_write_file) {
            if (des == NULL) {
                des = fopen("temp.mp3temp", "wb");
            }
            if (des != NULL && write_tag(container, des)) {
                fprintf(stderr, "success to write file.\n");
            } else {
                fprintf(stderr, "fail to write file.\n");
                flag_write_file = false;
            }
        }
        if (des != NULL && des != stdout) fclose(des);
        release_container(container);
        if (flag_write_file && !flag_extract_data) {
            remove(file);
            rename("temp.mp3temp", file);
        }
    }
    free(container);
}

int remove_frame(TagContaner* container, const char* id) {
    TagFrame* previous = NULL;
    TagFrame* frame = container->frame;
    int cnt = 0;
    while (frame != NULL) {
        TagFrame* next = frame->next;
        if (strcmp(frame->frame_id, id) == 0) {
            if (previous == NULL) {
                container->frame = next;
            } else {
                previous->next = next;
            }
            release_frame(frame);
            cnt++;
        } else {
            previous = frame;
        }
        frame = next;
    }
    return cnt;
}

int set_one_frame(TagContaner* container, Argument* arg, FILE* out) {
    int cnt = remove_frame(container, arg->frame_id);
    if (cnt > 0) {
        fprintf(out, "  frame override ID=%s\n", arg->frame_id);
    }
    TagFrame* previous = container->frame;
    while (previous->next != NULL) previous = previous->next;
    TagFrame* frame = (TagFrame*)malloc(sizeof(TagFrame));
    if (frame == NULL) return false;
	int result = false;
    FILE* data = NULL;
    switch (arg->data_type) {
        case TYPE_TEXT_EN:
        case TYPE_TEXT_EN_DES:
        case TYPE_TEXT_EN_LAN:
		case TYPE_TEXT_EN_LAN_DES:
			// stdin
			if ( strcmp(arg->text, "-") == 0 ){
				setmode(fileno(stdin), O_BINARY);
                data = stdin;
			}else{
				data = fopen(arg->text, "rb");
			}
			if ( data != NULL ){
				struct stat s;
				if ( fstat(fileno(data), &s) != 0) break;
				long size = s.st_size;
				arg->text = (char*)malloc(sizeof(char)*(size+3));
				if ( arg->text == NULL ) break;
				fread(arg->text, 1, size, data);
				memset(arg->text+size, 0, 3);
			}
            result = set_text(frame, arg->frame_id, arg->data_type, src_encoding, arg->encoding,
							  arg->data_format, arg->description, arg->text);
            break;
        case TYPE_PICTURE:
            if (strcmp(arg->data_file, "-") == 0) {
                setmode(fileno(stdin), O_BINARY);
                data = stdin;
            } else {
                data = fopen(arg->data_file, "rb");
            }
            if (data == NULL) {
                fprintf(stderr, "Error : fail to open picture file\n");
                break;
            }
            result = set_pricture(frame, src_encoding, arg->encoding,
                                  arg->data_format, arg->image_type,
                                  arg->description, data);
            break;
        case TYPE_BINARY:
            if (strcmp(arg->data_file, "-") == 0) {
                setmode(fileno(stdin), O_BINARY);
                data = stdin;
            } else {
                data = fopen(arg->data_file, "rb");
            }
            if (data == NULL) {
                fprintf(stderr, "Error : fail to open data file\n");
                break;
			}
			result = set_binary(frame, arg->frame_id, data);
			break;
		case TYPE_URL:
			result = set_URL(frame, arg->frame_id, arg->text);
			break;
	}
	if ( data != NULL && data != stdin ) fclose(data);
	if ( result ){
		fprintf(out, "success to set frame.\n");
		previous->next = frame;
		frame->next = NULL;
		show_frame(frame, out);
		return true;
	}else{
		fprintf(out, "Error : fail to set frame.\n");
		free(frame);
		return false;
	}
}

void remove_one_frame(TagContaner* container, const char* id, FILE* out) {
    int cnt = remove_frame(container, id);
    if (cnt == 0) {
		fprintf(out, "Error : frame not found > ID=%s\n", id);
		flag_write_file = false;
    } else if (cnt == 1) {
        fprintf(out, "  remove frame > ID=%s\n", id);
    } else if (cnt > 1) {
        fprintf(out, "Warning : remove multiple frames > ID=%s\n", id);
    }
}

void show_one_frame(TagContaner* container, const char* id, FILE* out,
                    FILE* extract) {
    TagFrame* frame = container->frame;
    while (frame != NULL) {
        if (strcmp(id, frame->frame_id) == 0) {
            show_frame(frame, out);
            // TODO toggle encoding dynamically
            if (extract != NULL &&
                !extract_data(frame, extract, des_encoding)) {
                fprintf(stderr, "Error : fail to write frame ID=%s\n",
                        frame->frame_id);
            }
            return;
        }
        frame = frame->next;
    }
    fprintf(stderr, "Error : frame not found ID=%s\n", id);
}

void show_all_frame(TagContaner* container) {
    TagFrame* frame = container->frame;
    while (frame != NULL) {
        show_frame(frame, stdout);
        frame = frame->next;
    }
}

const char* parse_frame_alias(const char* value) {
    if (strcmp(value, "picture") == 0 || strcmp(value, "pic") == 0) {
        return "APIC";
    } else if (strcmp(value, "comment") == 0 || strcmp(value, "comm") == 0) {
        return "COMM";
    } else if (strcmp(value, "album") == 0) {
        return "TALB";
    } else if (strcmp(value, "genre") == 0) {
        return "TCON";
    } else if (strcmp(value, "date") == 0 || strcmp(value, "year") == 0) {
        return "TDRC";
    } else if (strcmp(value, "title") == 0) {
        return "TIT2";
    } else if (strcmp(value, "artist") == 0) {
        return "TPE1";
    } else if (strcmp(value, "track") == 0) {
        return "TRCK";
    } else if (strcmp(value, "lyric") == 0 || strcmp(value, "lyrics") == 0) {
        return "USLT";
    } else {
        return NULL;
    }
}

int parse_long_opt(Argument* des, const struct option* src, const char* optarg,
                   int alias) {
    if (alias == LONG_OPT_FRAME_NAME) {
        if (strlen(src->name) != 4) return false;
        memcpy(des->frame_id, src->name, 5);
    } else {
        const char* id = parse_frame_alias(src->name);
        if (id == NULL) return false;
        memcpy(des->frame_id, id, 5);
    }
    if (optarg != NULL) {
        des->action = ACTION_SET_FRAME;
        flag_write_file = true;
        return parse_setting_opt(optarg, des);
    } else {
        des->action = ACTION_SHOW_FRAME;
    }
    return true;
}

int is_url_text(const char* str) {
    return strncmp(str, "http:/", 6) == 0 || strncmp(str, "https:/", 7) == 0;
}

void release_arg(Argument* arg) {
    free(arg->arg_data);
    // free(arg->text);
    // free(arg->description);
    // free(arg->data_format);
    // free(arg->url);
}

int parse_opt(Argument* des, int result, const char* value) {
    switch (result) {
        case 'l':
            flag_show_all_frame = true;
            return false;
        case 'r':
            flag_output_raw = 1;
            return false;
        case 'd':
            des->action = ACTION_DELETE_FRAME;
            const char* arg = optarg;
            if (!is_frame_id(arg)) {
                arg = parse_frame_alias(arg);
                if (arg == NULL) {
                    opt_parse_err = true;
                    fprintf(stderr, "invalid frame name found : '-d %s'\n",
                            arg);
                    return false;
                }
            }
            memcpy(des->frame_id, arg, 5);
            flag_write_file = true;
            break;
        case 'v':;
            const char* version = optarg;
            if (strlen(version) != 1 ||
                (version[0] != '3' && version[0] != '4')) {
                opt_parse_err = true;
                fprintf(
                    stderr,
                    "invalid version found : '-v %s'  only 3 or 4 accepted.\n",
                    version);
                return false;
            }
            version_output = (version[0] - '0');
            return false;
        case 'e':;
            char e = parse_encoding(optarg);
            if (e == ENCODING_NONE) {
                opt_parse_err = true;
            } else {
                src_encoding = e;
                des_encoding = e;
            }
            return false;
        case 'o':
            flag_extract_data = true;
            extract_data_des = optarg;
            if (extract_data_des == NULL) opt_parse_err = true;
            return false;
        case 'a':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "TPE1");
            break;
        case 'A':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "TALB");
            break;
        case 'p':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "APIC");
            break;
        case 'c':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "COMM");
            break;
        case 'g':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "TCON");
            break;
        case 'y':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "TDRC");
            break;
        case 'T':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "TRCK");
            break;
        case 't':
            des->action = ACTION_SHOW_FRAME;
            strcpy(des->frame_id, "TIT2");
            break;
        case '?':
            if (strlen(value) == 6 && strncmp(value, "--", 2) == 0 &&
                is_frame_id(value + 2)) {
                des->action = ACTION_SHOW_FRAME;
                strcpy(des->frame_id, value + 2);
                break;
            }
            opt_parse_err = true;
            fprintf(stderr,
                    "invalid option found : '%s' option char invalid, or "
                    "argument lacked.\n",
                    value);
            return false;
        default:
            return false;
    }
    return true;
}

char* check_image_format(const char* value) {
    static char* jpg[5] = {"image/jpeg", "jpg", "JPG", "jpeg", "JPEG"};
    static char* png[3] = {"image/png", "png", "PNG"};
    for (char** p = jpg; p < jpg + 5; p++) {
        if (strcmp(*p, value) == 0) return jpg[0];
    }
    for (char** p = png; p < png + 3; p++) {
        if (strcmp(*p, value) == 0) return png[0];
    }
    return NULL;
}

char parse_encoding(const char* value) {
    static char* iso[7] = {"0",        "iso",  "ISO", "shiftjis",
                           "ShiftJIS", "sjis", "SJIS"};
    static char* utf16bom[7] = {"1",        "utf16",    "UTF16",    "UTF-16",
                                "utf16bom", "UTF16BOM", "UTF-16BOM"};
    static char* utf16be[4] = {"2", "utf16be", "UTF16E", "UTF-16BE"};
    static char* utf8[4] = {"3", "utf8", "UTF-8", "UTF8"};
    char** p;
    for (p = iso; p < iso + 7; p++) {
        if (strcmp(value, *p) == 0) return ENCODING_ISO_8859_1;
    }
    for (p = utf16bom; p < utf16bom + 7; p++) {
        if (strcmp(value, *p) == 0) return ENCODING_UTF_16_BOM;
    }
    for (p = utf16be; p < utf16be + 4; p++) {
        if (strcmp(value, *p) == 0) return ENCODING_UTF_16_BE;
    }
    for (p = utf8; p < utf8 + 4; p++) {
        if (strcmp(value, *p) == 0) return ENCODING_UTF_8;
    }
    fprintf(stderr, "Error : fail to parse encoding '%s'\n", value);
    return ENCODING_NONE;
}

int parse_opt_picture(char* pt, Argument* arg) {
    // album picture
    arg->image_type = (char)0x00;
    arg->description = NULL;
    arg->encoding = ENCODING_UTF_16_BOM;
    // detect image file  stdin '-' accepted
    if (pt == NULL || strlen(pt) == 0) {
        fprintf(stderr, "Error : image file not specified.\n");
        return false;
    }
    arg->data_file = pt;
    // detect iamge type (MIME type)
    pt = strtok(NULL, ":");
    if (pt == NULL || strlen(pt) == 0 ) {
        if (strcmp(arg->data_file, "-") == 0) {
            fprintf(stderr,
                    "Error : image file set stdin, but MIME type unknown.\n");
            return false;
        }
        int length = strlen(arg->data_file);
        if (length > 4 && strcmp(arg->data_file + length - 4, ".jpg") == 0) {
            arg->data_format = "image/jpeg";
            return true;
        } else if (length > 4 &&
                   strcmp(arg->data_file + length - 4, ".png") == 0) {
            arg->data_format = "image/png";
            return true;
        } else {
            fprintf(stderr, "Error : unsupported image type found '%s'\n",
                    arg->data_file);
            return false;
        }
    } else {
        arg->data_format = check_image_format(pt);
        if (arg->data_format == NULL) {
            fprintf(stderr, "Error : invalid image type found '%s'\n", pt);
            return false;
        }
    }
    // detect image type (defiend by ID3.org)
    pt = strtok(NULL, ":");
    if (pt == NULL) return true;
    arg->image_type = (char)atoi(pt);
    if (arg->image_type < 0 || arg->image_type > 0x14) {
        fprintf(stderr, "Error : image-type '%d' out of bounds [0x00,0x14]\n",
                arg->image_type);
        return false;
    }
    // detect description
    pt = strtok(NULL, ":");
    if (pt == NULL) return true;
    arg->description = pt;
    // detect encoding of text;
    pt = strtok(NULL, ":");
    if (pt == NULL) return true;
    char encoding = parse_encoding(pt);
    if (encoding == ENCODING_NONE) return false;
    arg->encoding = encoding;
    return true;
}

int parse_opt_text(char* pt, Argument* arg, int has_desc, int has_language) {
    // detect text (must)
    if (pt == NULL) {
        fprintf(stderr, "Error : text required ID=%s\n", arg->frame_id);
        return false;
    }
    arg->text = pt;
    arg->description = NULL;
    arg->encoding = ENCODING_UTF_16_BOM;
    arg->data_format = NULL;
    // description
    if (has_desc) {
        pt = strtok(NULL, ":");
        if (pt == NULL) return true;
        arg->description = pt;
    }
    // language
    if (has_language) {
        pt = strtok(NULL, ":");
        if (pt == NULL){
			arg->data_format = "eng";
		}else{
			if (strlen(pt) != 3) {
				fprintf(stderr, "Error : invalid language symbol found '%s'\n", pt);
				return false;
			}
			arg->data_format = pt;
		}
    }
    // encoding
    pt = strtok(NULL, ":");
    if (pt == NULL) return true;
    char e = parse_encoding(pt);
    if (e == ENCODING_NONE) return false;
    arg->encoding = e;
    return true;
}

int parse_setting_opt(const char* value, Argument* arg) {
    arg->arg_data = (char*)malloc(sizeof(char) * (strlen(value) + 10));
    strcpy(arg->arg_data, value);
    // check frame ID, then identify data-type
    const char* id = arg->frame_id;
    char* pt = strtok(arg->arg_data, ":");
    if (strcmp(id, "APIC") == 0) {
        arg->data_type = TYPE_PICTURE;
        return parse_opt_picture(pt, arg);
    } else if (strcmp(id, "COMM") == 0) {
        arg->data_type = TYPE_TEXT_EN_LAN_DES;
        return parse_opt_text(pt, arg, true, true);
    } else if (strcmp(id, "USER") == 0) {
        arg->data_type = TYPE_TEXT_EN_LAN;
        return parse_opt_text(pt, arg, false, true);
    } else if (strcmp(id, "USLT") == 0) {
        arg->data_type = TYPE_TEXT_EN_LAN_DES;
        return parse_opt_text(pt, arg, true, true);
    } else if (is_text_type(id)) {
        arg->data_type = TYPE_TEXT_EN;
        return parse_opt_text(pt, arg, false, false);
    } else if (is_url_type(id)) {
        if (pt == NULL) {
            fprintf(stderr, "Error : no url to be set. ID=%s\n", arg->frame_id);
            return false;
        }
        arg->url = pt;
        arg->data_type = TYPE_URL;
        return true;
    } else if (id[0] == 'T' || id[0] == 'W') {
        arg->data_type = TYPE_TEXT_EN_DES;
        return parse_opt_text(pt, arg, true, false);
    } else {
        if (pt == NULL) {
            fprintf(stderr,
                    "Error : no source from which binay to be set. ID=%s\n",
                    arg->frame_id);
            return false;
        }
        arg->data_type = TYPE_BINARY;
        return true;
    }
    return false;
}