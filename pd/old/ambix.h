/*
** Definitions for ambiX-file-format
**  

** created 2010-12-05 by kr for ambiX (c) iem.at
*/


//#define AMBIX_XML_UUID				"IEM.AT/AMBIX/XML"
#define AMBIX_XML_VER				"0.001"
#define AMBIX_TAG_RECONSTRUCTION	"reconstruction"
#define AMBIX_TAG_MATRIXTYPE		"type"


#define AMBIX_TEST_XML				"<?xml version=\"0.001\" encoding=\"utf-8\"?><data><reconstruction>2 3 1.111 2.222 3.333 4.444 5.555 6.678123123123123</reconstruction><created>2010-12-12</created><info><title>amb im X</title><genre>ambisoniX</genre></info></data>"

/*
	<?xml version="0.001" encoding="utf-8"?>
	<data>
		<reconstruction>2 3 1.111 2.222 3.333 4.444 5.555 6.678123123123123</reconstruction>
		<created>2010-12-12</created>
		
		<info>
			<title>amb im X</title>
			<genre>ambisoniX</genre>
		</info>
	</data>
*/


// seek forward in data-chunk to add free space for header information
// seek value is per frame: channels * sizeof(format)
// 		for 2 channels & float: AMBIX_ADDSPACE_PER_FRAME * 2 * 4 bytes
#define AMBIX_ADDSPACE_PER_FRAME	10
// 36 bytes is needed by Header:
//									12 for CAF-Chunk (4: marker, 8 size)
//									24 for UUID-Chunk (16: uuid, 8 size)
// 	1 byte for NULL-termination



#define AMBIX_CAF_DATA_MARKER		"data"
#define AMBIX_CAF_UUID_MARKER		"uuid"



#define AMBIX_METADATA_SIZE			44 // 1 byte for NULL-termination

#define AMBIX_METADATA				"<ambix>this is the first test of wr</ambix>"

#include <stdint.h>



/*
**  this combines this Header original specified by Apple
**  
	struct CAF_UUID_ChunkHeader {
		CAFChunkHeader mHeader;
		UInt8	mUUID[16];
	}
	CAF_UUID_ChunkHeader;  
*/

typedef struct
{
	 char mUUID[16];		// UUID
	 int64_t mChunkSize;		// custom Size of UUID-Content, must be smaller
								// than "real"-UUID-Chunk
}
ambiXuuidChunk;		// 24 byte
#define ambiXuuidChunkFormat "88D"


#define UNUSED(var) var = var

