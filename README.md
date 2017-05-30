# gdal_rename

This repository contain the cpp file of the gdal_rename application

Overview:
------------------------------------------------------------------------------

The goal of this application is to allow the user to rename a raster dataset 
with it's SRS coordinates including it's sibling files (world file...) with some 
formating options (Prefix, suffix, separator, left zero padding, numbers of digits, ...).
Orginaly it was made to rename tiled datasets downloaded from different sources to have 
a consistent namming pattern or to update the name after a SRS change with gdalwarp.
You can choose a prefix, a corner (W/N by default) of the bounding box of the dataset 
as reference point. You can also specify a separator for coordinate value, the numeric
format of coordinates (integer, double) and the format of the sign.

It rename one file at a time (the last parameter), so you'll have to use command like
"for" or "for files" on windows batch or "find" on linux shell to rename all the files
of a folder.

This repository contain only the cpp file, you can find a full Visual Studio 2010
Workspace linked with GDAL dll in my GDAL_2.2.x_VC repository
here: https://github.com/MattLatt/GDAL_2.2.x_VC   
There is also a repository with a Code::Blocks project to build it with gcc
here: https://github.com/MattLatt/gdal_rename-cb   


Authors:
------------------------------------------------------------------------------

* [Mathieu Lattes] (mathieu.lattes@yahoo.fr)

Version:
------------------------------------------------------------------------------


* v0.0.8_20170530-01 : Refactored "comp*" variables to "sibling*", compiled on linux (gdal 2.2.0 + gcc 5.4.0 + 
        Ubuntu 16.04 + Code::Blocks 16.01). Rewrited proc "renameFileNoOverWrite"  as the posix rename() function
        mapped by VSILRename overwrite already existing by default on linux while failing on windows


Usage:
------------------------------------------------------------------------------

gdal_rename [--help|-h] [--refpoint|-r]  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[--coord-zero-padding|-z] [--coord-length|-l] [--coord-decimal-length|-d]  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[--coord-type|-t] [--coord-sep|-s] [--prefix|-p]  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[--suffix|-s]  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}   
&nbsp;&nbsp;&nbsp;&nbsp;|   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{ [--printf-syntax|-f] }  
&nbsp;&nbsp;&nbsp;&nbsp;[--output-console|-o]  
&nbsp;&nbsp;&nbsp;&nbsp;dataset_to_rename   

Parameters full descirption :

* **--help|-h**: print usage message

* **--refpoint|-r**: ref point to use in the name. Any pair of 'W', 'E' 
					 and 'N','S' (ie: WN, NW, EN, NE, WS, SW, ES, SE).
					
* **--prefix|-p**: prefix to use for the new file name format (i.e.:
				   Prefix_XXXXXXXXXXX.EXT)
				  
* **--coord-length|-l**: number of digits to use in coordinates value 
						 for file renaming, starting with high wheight 
						 digits (i.e.: using 4 with a easting value of
						 621000.0 will print 06210 with zero padding and
						 6210 without)

* **--coord-decimal-length|-d**: number of digits to use in decimal part for real coordinates\n"
						
* **--coord-zero-padding|-z**: wether or not left padding coordinate with 0 
							   in case of an inequal number of digits between
							   easting and northing values.
							  
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

* **--output-console|-o**: accept a string must be "win" or "unix".
						  If specified, print (in the console) the command to
						  perform the rename instead of doing it. It allow the user to  
						  create a batch file using system console redirection operator
						  like '>', '>>', '|'...	

History:
------------------------------------------------------------------------------
* v0.0.7_20170320-01 : Added --output-console option to allow the print(in the console) 
of the command line to perform the rename instead of doing it. It allow user to create 
a batch file using redirection operator of the system	console '>', '>>', '|'...					
					   Added a "sDirSep" with the OS dependant directory separator. 		

* v0.0.6_20170317-01 : Changed EQUALN to EQUAL on some tests and added companion 
					   files for jp2, j2k and img datasets.

* v0.0.5_20170303-01: Added double coord formating

* v0.0.4_20170228-01: Added Coord Sign Policy

* v0.0.3_20170224-01: small buggs correction (only first companion file
					renamed, Missing EQUAL() for -f parameter test...

* v0.0.2_20170106-01: using std:string instead of char * as str container, keeping
					CPL function
					
* v0.0.1: initial release with only char * and CPL function
