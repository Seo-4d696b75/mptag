# What?
program name : mptag  
  
**mptag** is a program, with wchich you can edit meta infomation
embedded in a mp3 file. It supports ID3v2.3 and ID3v2.4 format.

# Syntax

  `mptag {file_name} [options...]`
  
# Option

All the valid options are shown below.  
**NOTE** Each frame name consists of 4 letters of 'A-Z0-9',
and they can also be specified with its alias.

|option|details|
|:-:|:--|
|none|interpreted as "-l"|
|-l|show the all human-readable tags|
|-a |show the artist info|
|-A |show the album info|
|-p |show the album image|
|-t |show the song title|
|-c |show the comment|
|-g |show the genre|
|-y |show year/date info|
|-T |show track info|
|-o {filePath}|extract frame data(text, picture, binary) to specified file.|
|-e {Encoding}|set encoding of text data from a file/stdin or text data into a file/stdout.|
|-d {frame}|delete specified frams(s)|
|-v {version}|set tag version. version value must be either 3 or 4|
|--{frame}|show specified frame.|
|--{frame}={argument}|set tag value with specified frame. format of argument is defiend by each frame type.|

# Frame List
All the supported frames are shown below.  

|frame|alias|data type|content|
|:-:|:-:|:--|:--|
|APIC|picture|Picture|picture of albume jacket|
|COMM|comment|Text(Encoding,Language,Description)|comment to this music|
|TALB|album|Text(Encoding)|name of album, to which this music belongs|
|TCON|genre|Text(Encoding)|genre|
|TDRC|date|Text(Encodin)|date of recoding|
|TIT2|title|Text(Encoding)|title of music|
|TPE1|artist|Text(Encoding)|name of artists|
|TRCK|track|Text(Encoding)|track number formatted in "index/sum"|
|T***||Text(Encoding)|other text-data frame defiend by ID3.org|
|T***||Text(Encoding,Description)|user-defined data frame|
|USER||Text(Encoding,Language)|use limit|
|USLT|lyric|Text(Encoding,Language,Description)|lyrics of music|
|W***||URL|URL-based data frame defined by ID3.org|
|W***||Text(Encoding,Description)|user-defined link frame|
|****||Binary|others, or user-defined data frame|


# Data Type of Frame
Data type of each frame is pre-defined, and when you 
set frame, you have to pass argument in required format 
shown at below table. The argument consists of one or more values, 
and each value is separated by symbol ":". Supported encoding is shown in another table.  

|DataType|argument|details|
|:--|:--|:--|
|Text(Encoding)|"{TextData}:{Encoding}"|normal text data|
|Text(Encoding,Description)|"{TextData}:{Description}:{Encoding}"||
|Text(Encoding,Language)|"{TextData}:{Language}:{Encoding}"|use limit only|
|Text(Encoding,Language,Description)|"{TextData}:{Description}:{Language}:{Encoding}"||
|Picture|"{ImageFile}:{ImageFormat}:{ImageType}:{Description}:{Encoding}"|Album jacket image only|
|URL|"{URL}"||
|Binary|{DataFile}|any binary data accepted|

**NOTE**  
- symbol "-" for a value means that data from stdin is imbedded as that content.
- {TextData} : text string, file path or "-". This value can not be omitted.
- {Description} : text string, file path or "-". Empty string will be set when this value omitted.
- {DataFile},{ImageFile} : file path or "-". This value can not be omitted.
- omitting {Encoding} is accepted, then '1'(UTF-16 BOM) is selected.
- omitting {Language} is accepted, then 'jpn' is selected.
- only JPEG and PNG image are supported.  {ImageFormat} must be either 'image/jpeg' or 'image/png'.
- {ImageType} means what kind of image imbedded. value : 0x00 ~ 0x14 defined by ID3.org
- description of picture is at most 64 letters, including 'null' at the end.
- extra header is not supported. 
- {Language} is 3 letters defined in ISO-639. This value is only cheked abount its length.

## Encoding
  
|Encoding|value||
|:-:|:--|:--|
|ISO-8859-1|0|Shift-JIS also accepted as this value|
|UTF-16(BOM)|1|with BOM(BE/LE)|
|UTF-16BE|2|ID3v2.4 only  with no BOM|
|UTF-8|3|ID3v2.4 only|
  
- Encoding of Shift-JIS is not originally defiend in ID3v2, but 
multi-byte letters is encoded with Shift-JIS in some case.

# Example  
Some useful examples are shown here.  

## Show Frame
show all the frames embedded in file 'hoge.mp3'  
`mptag hoge.mp3 -l`  
show frames about its artist and title  
`mptag hoge.mp3 -a -t` or `mptag hoge.mp3 --artist --title`  
you can also specify frame with its ID directly  
`mptag hoge.mp3 --TPE1 --TIT2`  

## Extract Data
you can extract embedded text, picture, binary data to specified file.  
extract lyrics data to file 'text.txt'  
`mptag hoge.mp3 --lyrics -o text.txt`  
extract album image to file 'image.jpg'  
`mptag hoge.mp3 -p -o image.jpg`  
you can also extract data into stdout.  
`mptag hoge.mp3 --lyrics -o - > text.txt`  
`mptag hoge.mp3 -p -o - > image.jpg`


## Set Frame


