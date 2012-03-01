


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
#define	AWM_READ 2
#define	AWM_PAUS 3
#define AWM_DONE 4

// TODO Änderung der Blocksize wird nicht berücksichtigt

static t_class *ax_read_tilde_class;


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


t_int *ax_read_tilde_perform(t_int *w)
{
	t_ax_read_tilde *x = (t_ax_read_tilde *)(w[1]);
	int i, frame, channel;

	// TODO Progress-Bar

	if(x->mode == AWM_READ)
	{
		if(!(x->buffer = malloc(x->channels * x->blocksize * sizeof(float))))
		{
			post("[ax_read~] Cannot allocate Memory.\n") ;
		}

		if(sf_readf_float(x->file, x->buffer, x->blocksize) != x->blocksize)
		{
			//post("[ax_read~] Error reading Frames or EOF reached...");
			
			outlet_bang(x->out_bang);
			
			// TODO mit Nullen auffüllen... Pause Modus?
			x->mode = AWM_DONE;
		}
		
		i = 0;
		for(frame = 0; frame < x->blocksize; frame++)
		{
			for(channel = 0; channel < x->channels; channel++)
			{
				x->sig[channel][frame] = x->buffer[i++];
			}	
		}
		
		x->blockcount++;
		
		int seconds = (int)((float)x->blockcount * (float)x->blocksize / (float)x->samplerate);
		
		outlet_float(x->out_position, (float) seconds);
		
	}
	else
	{
		for(frame = 0; frame < x->blocksize; frame++)
		{
			for(channel = 0; channel < x->channels; channel++)
			{
				x->sig[channel][frame] = 0;
				// TODO gibt immer aus...
			}	
		}
	}
	
	return (w+2);
}	
	
	
void ax_read_tilde_dsp(t_ax_read_tilde *x, t_signal **sp)
{
	
	int i;
	for(i = 0; i < x->channels; i++) x->sig[i] = sp[i]->s_vec;

	x->blocksize = sp[0]->s_n;
	
	dsp_add(ax_read_tilde_perform, 1, x);
}
	
	
void *ax_read_tilde_new(t_floatarg f)
{
	t_ax_read_tilde *x = (t_ax_read_tilde *)pd_new(ax_read_tilde_class);
	x->x_canvas = canvas_getcurrent();
	
	
	// adaptor matrix output
	x->out_matrix = outlet_new(&x->x_obj, 0);
	
	// bang output (end of file)
	x->out_bang = outlet_new(&x->x_obj, 0);
	
	// position output
	x->out_position = outlet_new(&x->x_obj, &s_float);
	
	
	x->channels = (int) f;
	if((int) f < 1) x->channels = 1;
	
	int i = x->channels;
	while(i--) outlet_new(&x->x_obj, &s_signal);
	
	
	x->sig = (t_float**) getbytes(sizeof(t_float*) * x->channels);

	
	x->mode = AWM_NONE;
	
	x->rows = 0; x->cols = 0;
	x->atombuffer = 0;
	
	x->samplerate = (int) sys_getsr();
	
	return (void *)x;
}


void ax_read_tilde_free(t_ax_read_tilde *x)
{
	
	if (x->atombuffer) {
	
		freebytes(x->atombuffer, ( x->cols * x->rows + 2 ) * sizeof(t_atom) );

		x->cols = x->rows = 0;
		x->atombuffer = 0;
		x->out_matrix = 0;
		x->out_position = 0;
		x->out_bang = 0;
	
	}
	
	if(x->mode == AWM_READ)
	{
		sf_close(x->file);
	}
}

void ax_read_tilde_open(t_ax_read_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	s = s; // UNUSED

	x->blockcount = 0;

	// get filename and create filepath
	t_symbol *filesym = atom_getsymbolarg(0, argc, argv);
	char *filename = filesym->s_name;
	char filepath[FILENAME_MAX];
	canvas_makefilename(x->x_canvas, filename, filepath, FILENAME_MAX);
	sys_bashfilename(filepath, filepath);

	
	if((x->mode == AWM_NONE) || (x->mode == AWM_INIT) ||
	   (x->mode == AWM_DONE) || (x->mode == AWM_PAUS))
	{
		x->samplerate = (int) sys_getsr();

		// TODO Abfrage ob DSP ein

		if (!(x->file = sf_open (filepath, SFM_READ, &x->sfinfo)))
		{
			post ("[ax_read~] Not able to open input file.\n") ;
		}


		if (x->sfinfo.samplerate != x->samplerate)
		{
			post ("[ax_read~] Sorry, wrong samplerate!\n") ;
		}

		if (x->sfinfo.format != (SF_FORMAT_CAF | SF_FORMAT_FLOAT))
		{
			post ("[ax_read~] Sorry, wrong format!\n") ;
		}

		if (x->sfinfo.channels != x->channels)
		{
			post ("[ax_read~] Sorry, wrong # of channels!\n");
		}
		
		// TODO Progress Bar: x->sfinfo.frames;


		// ============================
		// = check for adaptor-matrix =
		// ============================

		SF_UUID_INFO uuid;

		memset(&uuid, 0, sizeof(uuid));
		strncpy(uuid.id, AMBIX_XML_UUID, 16);

		if ( !sf_command(x->file, SFC_GET_UUID, &uuid, sizeof(uuid)) )
		{
			//DLOG("UUID[%.16s]: '%s'\n", uuid.id, uuid.data);
			x->extended = 1;
			post("[ax_read~] ambiX extended format");
		}
		else
		{
			// UUID not found: Basic Format
			//post("[ax_read~] Error reading UUID");
			x->extended = 0;
			post("[ax_read~] ambiX basic format");
		}
		
		
		outlet_float(x->out_position, 0);
		
		
		// ======================
		// = get adaptor matrix =
		// ======================
		
		if (x->extended) {
			
			// get L, C
			
			uint32_t rows;
			uint32_t cols;
			
			int index = 0;
			
			memcpy(&rows, uuid.data+index, sizeof(uint32_t));	
			index += sizeof(uint32_t);
			
			memcpy(&cols, uuid.data+index, sizeof(uint32_t));	
			index += sizeof(uint32_t);
			
			// check endianess
			if (littleEndian()) {
				rows = swap_4i(rows);
				cols = swap_4i(cols);
			}
			
			DLOG("matrix: %dx%d", rows, cols);
			
			// get matrix values
			
			uint32_t elements = rows*cols;
			
			// allocate memory for matrix output
			size_t oldelements = x->rows * x->cols;
					
				// IEMMATRIX: adjustsize
				
				if ( oldelements != elements )
				{
				    if (x->atombuffer)
								freebytes(x->atombuffer, (oldelements+2) * sizeof(t_atom) );

					x->atombuffer = (t_atom *) getbytes( (elements+2) * sizeof(t_atom) );
				}
				x->rows = rows;
				x->cols = cols;

			x->atombuffer = (t_atom *) getbytes( (elements+2) * sizeof(t_atom) );
			
			// set L, C
			SETFLOAT(x->atombuffer,   rows);
			SETFLOAT(x->atombuffer+1, cols);
			
			// set values-buffer
			//t_atom *abuffer = x->atombuffer + 2;
			int atom_index = 2;
			
			while(elements--){

				t_float f;
				
				memcpy(&f, uuid.data+index, sizeof(t_float));	
				index += sizeof(t_float);
				
				// check endianess
				if (littleEndian())
					f = swap_4f(f);
				
				// set matrix values
				//SETFLOAT(abuffer++, (t_float) elements);
				SETFLOAT(x->atombuffer+atom_index, f);
				atom_index++;
				
				
				//DLOG("%f", f);
				
		  }
		
			// output matrix
			outlet_anything(x->out_matrix, gensym("matrix"), cols*rows+2, x->atombuffer);
		
		
		}
		
		

		x->mode = AWM_INIT;
	}
	else
	{
		post("[ax_read~] MODE_ERROR");
	}
	
	
}

void ax_read_tilde_start(t_ax_read_tilde *x)
{
	if ( (x->mode == AWM_INIT) || (x->mode == AWM_PAUS) )
	// TODO AWM_PAUS noch nicht implementiert
	{
		x->mode = AWM_READ;
	}
	else
	{
		post("[ax_read~] MODE_ERROR");
	}
}

void ax_read_tilde_stop(t_ax_read_tilde *x)
{
	
	if( (x->mode == AWM_READ) || (x->mode == AWM_PAUS) )
	{
		sf_close(x->file);
		outlet_float(x->out_position, -1);
		
		x->mode = AWM_DONE;
	}
	else
	{
		post("[ax_read~] MODE_ERROR");
	}
}

void ax_read_tilde_pause(t_ax_read_tilde *x)
{
	
	if (x->mode == AWM_READ)
	{
		x->mode = AWM_PAUS;
	}
	else
	{
		post("[ax_read~] MODE_ERROR");
	}
}



void ax_read_tilde_setup(void)
{
	ax_read_tilde_class = class_new(gensym("ax_read~"),
		(t_newmethod)ax_read_tilde_new,
		(t_method)ax_read_tilde_free,
		sizeof(t_ax_read_tilde),
		CLASS_DEFAULT,
		A_DEFFLOAT,
		0);

	class_addmethod(ax_read_tilde_class,
		(t_method)ax_read_tilde_dsp,
		gensym("dsp"),
		0);

	class_addmethod(ax_read_tilde_class,
		(t_method)ax_read_tilde_open,
		gensym("open"),
		A_GIMME,
		0);
	
	class_addmethod(ax_read_tilde_class,
		(t_method)ax_read_tilde_start,
		gensym("start"),
		0);
		
	class_addmethod(ax_read_tilde_class,
		(t_method)ax_read_tilde_pause,
		gensym("pause"),
		0);
		
	class_addmethod(ax_read_tilde_class,
		(t_method)ax_read_tilde_stop,
		gensym("stop"),
		0);
}