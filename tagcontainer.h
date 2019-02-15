/**
 * mptag
 * 
 * @author Seo-4d696b75
 * @version 2019-01-09
 */

#ifndef MP3_TAG_CONTAINER_H
#define MP3_TAG_CONTAINER_H

#include <stdio.h>

extern const char TAG_VERTION_ID3v2_3;
extern const char TAG_VERTION_ID3v2_4;

extern const char ENCODING_NONE;
extern const char ENCODING_ISO_8859_1;
extern const char ENCODING_UTF_16_BOM;
extern const char ENCODING_UTF_16_BE;
extern const char ENCODING_UTF_8;
extern const char ENCODING_DEFAULT;

typedef enum data_type_e{
	TYPE_TEXT_EN,
	TYPE_TEXT_EN_LAN,
	TYPE_TEXT_EN_LAN_DES,
	TYPE_TEXT_EN_DES,
	TYPE_PICTURE,
	TYPE_URL,
	TYPE_BINARY
} DataType;

typedef struct frame_t {
	//header
	char frame_id[5];
	int frame_size;
	char frame_flag[2];

	//content
	DataType data_type;
	char encoding;
	int encoding_byte;
	char* text;
	char* description;
	char* data_format; //language, or picture MIME type
	char image_type; //APIC only
	int binary_size; 
	char* binary;	//binary data, includding picture
	char* url;		// URL text


	//next frame pointer
	struct frame_t* next;
} TagFrame;

typedef struct tag_container_t{
	//file
	const char* filename;
	FILE* file;
	long offset;
	// raw data of header
	char header_version[2];
	char header_flag;
	// data from header
	char version;
	int frames_size;
	//extra header
	int has_extra_header;
	int extra_header_size;
	char* extra_header;
	//frames
	int frames_cnt;
	TagFrame* frame;
} TagContaner;

int is_frame_id(const char* name);

int is_text_type(const char* id);

int is_url_type(const char* id);

int initialize_container(TagContaner* container, const char* filename);

void release_container(TagContaner* container);

TagFrame* initialize_frame(FILE* file, char version);

void release_frame(TagFrame* frame);

void show_frame(TagFrame* frame, FILE* des);

int extract_data(TagFrame* frame, FILE* des, char encoding);

int get_raw_frame(FILE* des, TagFrame* src, char version);

int set_text(TagFrame* frame, const char* id, DataType type, char src_encoding, char des_encoding, const char* language, const char* description, const char* text);

int set_URL(TagFrame *frame, const char* id, const char* url);

int set_binary(TagFrame* frame, const char* id, FILE* data);

int set_pricture(TagFrame *frame, char src_encoding, char des_encoding, const char* format,
                  char type, const char* description, FILE* data);

int write_tag(TagContaner* container, FILE* des);


#endif
