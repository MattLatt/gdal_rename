# gdal_rename
==============================================================================
This repository contain the cpp file of the gdal_rename application

Overview:
------------------------------------------------------------------------------

The goal of this application is to allow the user to rename a dataset regarding 
the geographic metadatas (i.e.: coordinates) with some formating options 
(Prefix, suffix, separator, zero left padding, numbers of digits, ...).
It was originaly designed to be used on tiled datasets (as aerial orthophotos splited
in 1 sq km tiles for example) to add a prefix or the zero left padding for sorting purpose.

It is renaming one file at a time (the last parameter),so you'll have to use command like
"for file" on windows batch or "find" on linux shell to rename all the files
of a folder.

This repository contain only the cpp file, you can find a full Visual Studio 2010
Workspace linked with GDAL dll in my GDAL_2.1.x_VC repository
here : https://github.com/MattLatt/GDAL_2.1.x_VC   

Authors:
------------------------------------------------------------------------------

* [Mathieu Lattes] (mathieu.lattes@yahoo.fr)

Version:
------------------------------------------------------------------------------

* v0.0.5_20170303-01: Added double coord formating

Usage:
------------------------------------------------------------------------------

gdal_rename [--help|-h] [--refpoint|-r]
                {
					[--coord-zero-padding|-z] [--coord-length|-l] [--coord-decimal-length|-d]
					[--coord-type|-t] [--coord-sep|-s] [--prefix|-p]
					[--suffix|-s] 
				} 
			| 
				{ [--printf-syntax|-f] }
			dataset_to_rename

Parameters full descirption :

* **--help|-h**: print usage message

* **--refpoint|-r**: ref point to use in the name. Any pair of 'W', 'E' 
					 and 'N','S' (ie: WN, NW, EN, NE, WS, SW, ES, SE).
					
* **--prefix|-p**: prefix to use for the new file name format (i.e.:
				   Prefix_XXXXXXXXXXX.EXT)
				  
* **--coord-length|-l**: number of digits to use in coordinates value 
						 for file renaming starting with high wheight 
						 digits (i.e.: using 4 with a easting value of
						 621000.0 will print 06210 with zero padding and
						 6210 without)

* **--coord-decimal-length|-d**: number of digits to use in decimal part for real coordinates\n"
						
* **--coord-zero-padding|-z**: wether or not left padding coordinate with 0 
							   if there is an inequal number of digits between
							   easting and northing value
							  
* **--coord-type|-t**: '''int''' or '''real''' used to format coordinates in name

* **--coord-sep|-s**: separator of coordinates components when formating

* **--suffix|-x**: suffix to use for the new file name formatting (i.e.:
				   XXXXXXXXXXX_suffix.EXT)
			
* **--printf-syntax|-f**: a string with printf syntax to rename the file. 
						  It is exclusive with all previous parameters 
						  except --refpoint.
						  /!\ be carefull there is no printf syntax check and 
						  only 2 substitution allowed for easting/northing.
						  Don't forget to use %% instead of % for substitutions
						  on windows batch.
						  Use this parameters if you know what you are doing...

History:
------------------------------------------------------------------------------
* v0.0.4_20170228-01: Added Coord Sign Policy

* v0.0.3_20170224-01: small buggs correction (only first companion file
					renamed, Missing EQUAL() for -f parameter test...

* v0.0.2_20170106-01: using std:string instead of char * as str container, keeping
					CPL function
					
* v0.0.1: initial release with only char * and CPL function
