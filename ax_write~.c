/*
	TODO Speicherbereich wieder löschen
*/

#include	"ax.h"
#include	<sndfile.h>


#include 	<stdint.h>	// int32_t, nur zur Sicherheit
#include	<stdlib.h> 	// malloc
#include	<string.h>	// memset

#define AWM_NONE 0
#define AWM_INIT 1
#define	AWM_WRIT 2
#define AWM_DONE 3






// TODO Änderung der Blocksize wird nicht berücksichtigt

static t_class *ax_write_tilde_class;

///////////// into separate file!!!!! ////////////////////////////////////////////////

int littleEndian() {
	int foo = 1;
	if(*(char *)&foo == 1) return 1;
	else return 0; }
	
t_float swap_4f( const float inFloat ) {
   t_float retVal;

   char *floatToConvert = ( char* ) & inFloat;
   char *returnFloat = ( char* ) & retVal;
   returnFloat[0] = floatToConvert[3]; returnFloat[1] = floatToConvert[2];
   returnFloat[2] = floatToConvert[1]; returnFloat[3] = floatToConvert[0];
   return retVal; }

uint32_t swap_4i( const uint32_t inInt ) {
   uint32_t retVal;
   char *intToConvert = ( char* ) & inInt;
   char *returnInt = ( char* ) & retVal;
   returnInt[0] = intToConvert[3]; returnInt[1] = intToConvert[2];
   returnInt[2] = intToConvert[1]; returnInt[3] = intToConvert[0];
   return retVal; }

///////////// into separate file!!!!! ////////////////////////////////////////////////



t_int *ax_write_tilde_perform(t_int *w)
{
	t_ax_write_tilde *x = (t_ax_write_tilde *)(w[1]);
	
	
	
	
	if(x->mode == AWM_WRIT)
	{
		int i = 0;
		
		int frame;
		for(frame = 0; frame < x->blocksize; frame++)
		{
			int channel;
			for(channel = 0; channel < x->channels; channel++)
			{
				x->buffer[i++] = (float) x->sig[channel][frame]; 
				// TODO Umwandlung von t_float auf float
			}	
		}
	
		if(sf_writef_float(x->file, x->buffer, x->blocksize) != x->blocksize)
		{
			post("[ax_write~] Error writing Frames");
		}
		x->blockcount++;
		
		int seconds = (int)((float)x->blockcount * 
							(float)x->blocksize / (float)x->samplerate);
		
		
		outlet_float(x->x_obj.ob_outlet, seconds);
	}
	
	return (w+2);
}	
	
	
void ax_write_tilde_dsp(t_ax_write_tilde *x, t_signal **sp)
{
	
	int i;
	for(i = 0; i < x->channels; i++) x->sig[i] = sp[i]->s_vec;

	x->blocksize = sp[0]->s_n;
	
	dsp_add(ax_write_tilde_perform, 1, x);
}
	
	
void *ax_write_tilde_new(t_floatarg f)
{
	t_ax_write_tilde *x = (t_ax_write_tilde *)pd_new(ax_write_tilde_class);
	x->x_canvas = canvas_getcurrent();
	
	x->channels = (int) f;
	if((int) f < 1) x->channels = 1;
	
	int i = x->channels;
	while(--i) inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	
	
	x->sig = (t_float**)getbytes(sizeof(t_float*)*x->channels);
	
	outlet_new(&x->x_obj, &s_float);
	
	x->mode = AWM_NONE;

	return (void *)x;
}


void ax_write_tilde_free(t_ax_write_tilde *x)
{
	if(x->mode == AWM_WRIT)
	{
		sf_close(x->file);
	}
}

void ax_write_tilde_open(t_ax_write_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	
	s = s; // UNUSED
	
	// get filename and create filepath
	t_symbol *filesym = atom_getsymbolarg(0, argc, argv);
	char *filename = filesym->s_name;
	char filepath[FILENAME_MAX];
	canvas_makefilename(x->x_canvas, filename, filepath, FILENAME_MAX);
	sys_bashfilename(filepath, filepath);


	if((x->mode == AWM_NONE) || (x->mode == AWM_INIT) || (x->mode == AWM_DONE))
	{
		x->samplerate = (int) sys_getsr();

		// TODO Abfrage ob DSP ein

		if(!(x->buffer = malloc(x->channels * x->blocksize * sizeof(float))))
		{
			post("[ax_write~] Cannot allocate Memory.\n");
		}

		memset(&x->sfinfo, 0, sizeof(x->sfinfo));

		x->sfinfo.samplerate	= x->samplerate;
		x->sfinfo.channels		= x->channels;
		x->sfinfo.format		= (SF_FORMAT_CAF | SF_FORMAT_FLOAT);


		if(!(x->file = sf_open (filepath, SFM_WRITE, &x->sfinfo)))
		{
			post("[ax_write~] Not able to open output file.\n");
		}
		
		x->blockcount = 0;
		x->mode = AWM_INIT;
		
		DLOG("opening new file...");
		
	}
	else
	{
		post("[ax_write~] MODE_ERROR");
	}
	
}


static void ax_write_tilde_matrix(t_ax_write_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	
	UNUSED(s);

	if(x->mode == AWM_INIT)
	{
		
		uint32_t rows = (uint32_t) atom_getint(argv++);
		uint32_t cols = (uint32_t) atom_getint(argv++);
		uint32_t elements = rows*cols;


		// =======================
		// = load adaptor-matrix =
		// =======================

		DLOG("load adaptor-matrix for extended format...");


		// iemmatrix....
		if (argc<2){
			post("[ax_write~] crippled matrix");
			return;
		}
	  if ((cols<1)||(rows<1)) {
			post("[ax_write~] invalid dimensions");
			return;
		}
	  if (cols*rows> (uint32_t)(argc-2) ){
			post("[ax_write~] sparse matrix not yet supported : use \"mtx_check\" ([iemmatrix])");    
			return;
		}

		DLOG("maxtrix dimensions: %dx%d", rows, cols);

		// ======================
		// = write custom chunk =
		// ======================


		char *uuid_data;

		int uuid_data_size = 2*sizeof(uint32_t) + elements*sizeof(t_float); // L,C, values



		if(!(uuid_data = malloc(uuid_data_size)))
		{
			post("[ax_write~] Cannot allocate Memory.\n") ;
		}

		int index = 0;

		// check endianess
		if (littleEndian()) {
			rows = swap_4i(rows);
			cols = swap_4i(cols);
		}


		// matrix L, C
		memcpy(uuid_data+index, &rows, sizeof(uint32_t));	
		index += sizeof(uint32_t);

		memcpy(uuid_data+index, &cols, sizeof(uint32_t));	
		index += sizeof(uint32_t);


		// matrix values
	  while(elements--){
	    t_float f = atom_getfloat(argv++);

			// check endianess
			if (littleEndian())
				f = swap_4f(f);

			memcpy(uuid_data+index, &f, sizeof(t_float));		
			index += sizeof(t_float);

	  }

		SF_UUID_INFO uuid;
		memset(&uuid, 0, sizeof(uuid));

		strncpy(uuid.id, AMBIX_XML_UUID, 16);

		uuid.data=uuid_data;
		uuid.data_size=uuid_data_size;

		sf_command(x->file, SFC_SET_UUID, &uuid, sizeof(uuid));
		
	}
	else
	{
		post("[ax_write~] MODE_ERROR");
	}


}


void ax_write_tilde_start(t_ax_write_tilde *x)
{
	if(x->mode == AWM_INIT)
	{
		x->mode = AWM_WRIT;
	}
	else
	{
		post("[ax_write~] MODE_ERROR");
	}
}

void ax_write_tilde_stop(t_ax_write_tilde *x)
{
	
	if(x->mode == AWM_WRIT)
	{
		sf_close(x->file);
		
		int seconds = (int)((float)x->blockcount * 
							(float)x->blocksize / (float)x->samplerate);
		
		post("[ax_write~] Successfully written %d blocks (%d sec)",
			x->blockcount, seconds);
		
		x->mode = AWM_DONE;
	}
	else
	{
		post("[ax_write~] MODE_ERROR");
	}
}


void ax_write_tilde_setup(void)
{
	ax_write_tilde_class = class_new(gensym("ax_write~"),
		(t_newmethod)ax_write_tilde_new,
		(t_method)ax_write_tilde_free,
		sizeof(t_ax_write_tilde),
		CLASS_DEFAULT,
		A_DEFFLOAT,
		0);

	class_addmethod(ax_write_tilde_class,
		(t_method)ax_write_tilde_dsp,
		gensym("dsp"),
		0);

	class_addmethod(ax_write_tilde_class,
		(t_method)ax_write_tilde_open,
		gensym("open"),
		A_GIMME,
		0);
	
	class_addmethod(ax_write_tilde_class,
		(t_method)ax_write_tilde_start,
		gensym("start"),
		0);
		
	class_addmethod(ax_write_tilde_class,
		(t_method)ax_write_tilde_stop,
		gensym("stop"),
		0);
		
	class_addmethod(ax_write_tilde_class,
									(t_method)ax_write_tilde_matrix,
									gensym("matrix"),
									A_GIMME,
									0);
		
	CLASS_MAINSIGNALIN(ax_write_tilde_class, t_ax_write_tilde, f);
}