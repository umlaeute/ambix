/*
**  Declarations for Pd-Externals for ambiX (c) iem.at
**  

**  
**  2010-12-12	kr	created
*/

//#define DLOG(...) post(__VA_ARGS__)
#define DLOG(...)

#define AMBIX_PD_VERSION "0.01"
#define AMBIX_XML_UUID "IEM.AT/AMBIX/XML"


#define	PDLOG(message) post("error@%s::%s::%d::%s\n", __FILE__, __FUNCTION__, __LINE__, message)
							
#include 	"/Applications/Pd-extended.app/Contents/Resources/include/pdextended/m_pd.h"
//#include 	"/usr/include/pdextended/m_pd.h"
#include 	"../ambix.h"

#include	<sndfile.h>


void ax_help_setup(void);
void ax_write_tilde_setup(void);
void ax_read_tilde_setup(void);



typedef struct _ax_write_tilde
{
	t_object x_obj;
	t_canvas *x_canvas;
	t_float f; // notwendige Dummy-Variable, falls kein Sig. am Eingang
	
	long blockcount;
	
	int mode;
	int blocksize;
	int channels;
	int samplerate;
	
	SNDFILE *file;
	SF_INFO	sfinfo;
	
	float *buffer;
	t_float **sig;
	

} t_ax_write_tilde;


typedef struct _ax_read_tilde
{
	t_object x_obj;
	t_canvas *x_canvas;
	t_float f; // notwendige Dummy-Variable, falls kein Sig. am Eingang
	
	long blockcount;
	
	int mode;
	int blocksize;
	int channels;
	int samplerate;
	
	SNDFILE *file;
	SF_INFO	sfinfo;
	
	float *buffer;
	t_float **sig;
	
	int extended; // 0...basic, 1...extended

	t_outlet	*out_position;
	
	// adaptor matrix (extended format)
	int			rows;
	int			cols;
	t_atom		*atombuffer;
	t_outlet	*out_matrix;
	
	t_outlet	*out_bang;
	
} t_ax_read_tilde;
