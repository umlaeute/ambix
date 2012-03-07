/* dummy for ambiX-lib */
/*
**  2011-02-07	kr	created
*/


#include	"ax.h"


    /* the data structure for each copy of "ax_help".  In this case we
    on;y need pd's obligatory header (of type t_object). */
typedef struct ax_help
{
  t_object x_ob;
} t_ax_help;

    /* this is a pointer to the class for "ax_help", which is created in the
    "setup" routine below and used to create new ones in the "new" routine. */
t_class *ax_help_class;

    /* this is called when a new "ax_help" object is created. */
void *ax_help_new(void)
{
    t_ax_help *x = (t_ax_help *)pd_new(ax_help_class);

	/*
	post("");
	post("ambiXchange for Pd v%s", AMBIX_PD_VERSION);
	post("\tcompiled %s : %s", __DATE__, __TIME__);
	post("");
	post("\tdummy help object...");
	post("");
	*/

    return (void *)x;
}

    /* this is called once at setup time, when this code is loaded into Pd. */
void ax_help_setup(void)
{
    ax_help_class = class_new(gensym("ax"), (t_newmethod)ax_help_new, 0,
    	sizeof(t_ax_help), 0, 0);
}

