


#include 	"ax.h"

void ax_setup(){
	post("");
	post("ambiXchange for Pd v%s", AMBIX_PD_VERSION);
	post("\tcompiled %s : %s", __DATE__, __TIME__);
	post("");

	ax_help_setup();
	ax_write_tilde_setup();
	ax_read_tilde_setup();
	
}
