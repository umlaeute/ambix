ambix-utils: collection of utilities to deal with ambix files


ambix-info:
	get info about an ambix file

ambix-interleave -o <outfile> [-O <order>] [-X <matrixfile>] <infile1> [<infile2> ...]
	merge several (multi-channel) audio files into a single ambix file;
	infile1 becomes W-channel, infile2 becomes X-channel,...
	by default this will write an 'ambix simple' file (only full sets are accepted) 
	eventually files are written as 'ambix extended' file with adaptor matrix set to unity
	if 'order' is specified, all inchannels not needed for the full set are written as 'extrachannels'
	'matrixfile' is a soundfile/octavefile that is interpreted as matrix: each channel is a row, sampleframes are columns
	if 'matrix' is specified it must construct a full-set (it must satisfy rows=(O+1)^2)
	if 'matrix' is specified, all inchannels not needed to reconstruct to a full set are 'extrachannels'
	if both 'order' and 'matrix' are specified they must match

ambix-deinterleave <infile>
	split an ambix file into ambi/non-ambi data
	split an ambi file into separate channels (decoded)
	if the 'infile' is called 'data.caf',
	the output files are called 'data_ambisonics_00.caf' and 'data_extra_00.caf',
	where the number for ambisonics channels is the ACN (ambisonics channel number (starting from 0))
	and the numbers for extra channels start at 0

ambix-jplay <infile>
	playback an ambix file via jack
	jack channels are called "ambisonics_%02d" and "extra_%02d"
	reduced sets are always expanded to full sets
  Disclaimer: this is a port of jack.play from jack-tools
	Copyright (c) 2003-2010, Rohan Drape
	Copyright (c) 2012, IOhannes m zmölnig, IEM
	Licensed under the GPL-2 (or later)

ambix-jrec [-O <order>] [-X <matrixfile>] [-x <#extrachannels>] <outfile>
	record an ambix file via jack (for options see ambix-interleave)
  Disclaimer: this is a port of jack.rec from jack-tools
	Copyright (c) 2003-2010, Rohan Drape
	Copyright (c) 2012, IOhannes m zmölnig, IEM
	Licensed under the GPL-2 (or later)

