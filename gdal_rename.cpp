//##################################################################################//
//									gdal_rename										//
//**********************************************************************************//
// Desc: Command line executable to rename a GDAL raster dataset with it's SRS 		//
//		 coordinates including it's sibling files (world file...) with some		    //
//		 formating options (Prefix, suffix, separator, left zero padding, numbers  	//
//		 of digits,...). Orginaly it was made to rename tiled datasets downloaded  	//
//		 from manysources to have consistent namming pattern or to update the name 	//
//		 after a SRS change with gdalwarp. 											//
//		 																			//
//		 																			//
// Author: 	Mathieu Lattes (mathieu dot lattes at yahoo dot fr)						//
//																					//
//		 																			//
// Version:	v0.0.8_20170530-01 : Refactored "comp*" variables to "sibling*" and     //
//          compiled on linux (gdal 2.2.0 + g++ 5.4.0 + Ubuntu 16.04 +              //
//          Code::Blocks 16.01. Change renameFileNoOverWrite proc as the posix      //
//		 	rename() function mapped by ::VSILRename overwrite by default on linux  //
//          and return error on windows                                             //
//																					//
//**********************************************************************************//
// History:	v0.0.7_20170320-01 : Added --output-console option to allow the print	//
//			(in the console) of the command line to perform the rename instead of	//
//			doing it. It allow user to create a batch file using redirection	 	//
//			operator of the system	console '>', '>>', '|'...						//
//			Added a "sDirSep" with the OS dependant directory separator. 			//
//		 																			//
//          v0.0.6_20170317-01 : Changed EQUALN to EQUAL on some tests and added	//
//			sibling files for jp2, j2k and img datasets.							//
//			v0.0.5_20170303-01 : Added double coord formating						//
//			v0.0.4_20170228-01 : Added Coord Sign Policy							//
//			v0.0.3_20170224-01 : Small buggs fixs (only first sibling file renamed  //
//								, Missing EQUAL() for -f test...					//
//			v0.0.2_20170106-01 : using std lib instead of char * as str container	//
//			v0.0.1 : initial release without std lib								//
//																					//
//##################################################################################//

#include <iostream>

#include "gdal_priv.h"

#include "cpl_error.h"
#include "cpl_conv.h"
#include "cpl_vsi.h"

#include "json_object.h"
#include "ogr_srs_api.h"

#include "commonutils.h"

#include <string>
#include <math.h>

#ifdef _WIN32
std::string  sDirSep = "\\";
#else //Linux or MacOsX
std::string  sDirSep = "/";
#endif

//!Return number of digits from integer part of double
int nDigits (double dValue);

//!Get integer coordinate from GDALDataset
double getCoord (char refPointPart, double *adfGeoTransform, GDALDataset *poDataset);
//!Get char of coordinate hemisphere from GDALDataset

char getCoordHemi (char refPointPart, double *adfGeoTransform, GDALDataset *poDataset);
//!Call VSIL rename function and rename old file if already existing
bool renameFileNoOverWrite (std::string sourcePath, std::string newPath);

//!Get world file extension of a GDALDataset
std::string getWorldFileExt (std::string inputFile);

//!Get an array of sibling files extensions of a dataset (ie: tfw, prj
//!for a tif file). Need to be populated for other datasets !!!
std::vector<std::string> getSiblingFilesExt (std::string inputFile);

//!Usage
static void Usage(const char* pszErrorMsg = NULL);

//!Check whether or not the given refpoint is good
bool checkRefPoint(const char* CoordRefPoint);

//!format the rename command line for output in the console
std::string formatRenameCmdLine(const char* pszSystemType, const char* pszSourceName, const char* pszNewName);

//***********************************************************************//
//!Return number of digits from integer part of double
//***********************************************************************//
int nDigits (double dValue)
	{
	return int(ceil(log10(dValue)));
	}

//***********************************************************************//
//!Get integer coordinate from GDALDataset
//***********************************************************************//
double getCoord (char refPointPart, double *adfGeoTransform, GDALDataset *poDataset)
	{
	double Coord = 0.;

	switch ( refPointPart )
		{
		case 'W' : Coord = adfGeoTransform[0]; break;
		case 'E' : Coord = (adfGeoTransform[0]+(poDataset->GetRasterXSize()*adfGeoTransform[1])); break;
		case 'S' : Coord = (adfGeoTransform[3]+(poDataset->GetRasterYSize()*adfGeoTransform[5])); break;
		case 'N' : Coord = adfGeoTransform[3]; break;
		default	 : Coord = adfGeoTransform[0];
		}

	return Coord;
	}

//***********************************************************************//
//!Get char of coordinate hemisphere from GDALDataset
//***********************************************************************//
char getCoordHemi (char refPointPart, double *adfGeoTransform, GDALDataset *poDataset)
	{
	double Coord = 0.0;
	char Hemi = 'N';

	switch ( refPointPart )
		{
		case 'W' : Coord = adfGeoTransform[0]; break;
		case 'E' : Coord = (adfGeoTransform[0]+(poDataset->GetRasterXSize()*adfGeoTransform[1])); break;
		case 'S' : Coord = (adfGeoTransform[3]+(poDataset->GetRasterYSize()*adfGeoTransform[5])); break;
		case 'N' : Coord = adfGeoTransform[3]; break;
		default	 : Coord = adfGeoTransform[0];
		}

	if ( Coord >= 0 )
		{
		if (refPointPart == 'W' || refPointPart == 'E')
			{ Hemi = 'E'; }
		else
			{ Hemi = 'N'; }
		}
	else
		{
		if (refPointPart == 'W' || refPointPart == 'E')
			{ Hemi = 'W'; }
		else
			{ Hemi = 'S'; }
		}

	return Hemi;
	}

//***********************************************************************//
//!Call VSIL rename function and rename old file if already existing
//***********************************************************************//
bool renameFileNoOverWrite (std::string sourcePath, std::string newPath)
	{
    ::VSIStatBufL psStatBuf;
    int renameSuccess = -1, renameBackupSuccess = -1;

    if (::VSIStatExL ( newPath.c_str(), &psStatBuf, VSI_STAT_EXISTS_FLAG) == 0 )
        {
		std::string sNewFilePathTmp = newPath.substr(0,newPath.length()-4);
		sNewFilePathTmp += "_OldOne" + newPath.substr(newPath.length()-4, 4);

        if ( ::VSIStatExL ( sNewFilePathTmp.c_str(), &psStatBuf, VSI_STAT_EXISTS_FLAG) == 0 )
			{
			std::cout << "Error while renaming \"" << sourcePath << "\" to \"" << newPath << "\"" << std::endl;
			std::cout << "Both new name and new name backup file already exist, unable to perform the rename...." << std::endl ;
			return false;
            }
        else
            {
            //backuping the file already existing with the new naming convention
            renameBackupSuccess = ::VSIRename( newPath.c_str(), sNewFilePathTmp.c_str() );
            if ( renameBackupSuccess == 0 )
                //do the inital rename if success
                { renameSuccess =  ::VSIRename( sourcePath.c_str(), newPath.c_str() ); }
            else
                {
                std::cout << "Error while backuping \"" << newPath << "\" to \"" << sNewFilePathTmp << "\"" << std::endl;
                std::cout << "unable to perform the rename...." << std::endl ;
                return false;
                }
            }
        }
    else
        { renameSuccess =  ::VSIRename( sourcePath.c_str(), newPath.c_str() ); }

	if ( renameSuccess == 0 )
		{ return true;}
    else
        { std::cout << "Error while renaming \"" << sourcePath << "\" to \"" << newPath << "\"" << std::endl; }

	}

//***********************************************************************//
//!Get world file extension of a GDALDataset
//***********************************************************************//
std::string getWorldFileExt (std::string inputFile)
	{
	std::vector<std::string> siblingFilesExt;

	std::string inputExt = ::CPLGetExtension(inputFile.c_str());

	std::string wfExt = inputExt.substr(0,1) + inputExt.substr(2,1) + 'w';

	return wfExt;
	}


//***********************************************************************//
//!Get an array of sibling files extensions of a dataset (ie: tfw, prj
//!for a tif file). Need to be populated for other datasets !!!
//***********************************************************************//
std::vector<std::string> getSiblingFilesExt (std::string inputFile)
	{
	std::vector<std::string> siblingFilesExt;
	std::string inputExt = ::CPLGetExtension(inputFile.c_str());

	siblingFilesExt.push_back(getWorldFileExt(inputFile));

	if ( EQUAL(inputExt.c_str(), "tif"))
		{ siblingFilesExt.push_back("prj"); }

	if ( EQUAL(inputExt.c_str(), "ecw"))
		{ siblingFilesExt.push_back("ers"); }

	if ( EQUAL(inputExt.c_str(), "img"))
		{
		siblingFilesExt.push_back("led");
		siblingFilesExt.push_back("nul");
		siblingFilesExt.push_back("trl");
		siblingFilesExt.push_back("vol");
		}

	if ( EQUAL(inputExt.c_str(), "jp2") || EQUAL(inputExt.c_str(), "j2k"))
		{ siblingFilesExt.push_back("prj"); }

	return siblingFilesExt;
	}



/************************************************************************/
/*                               Usage()                                */
/************************************************************************/
static void Usage(const char* pszErrorMsg)
	{
    printf( "Usage: gdal_rename [--help|-h] [--input-file-to-rename|-i] [--refpoint|-r]\n"
			"                   { [--coord-zero-padding|-z] [--coord-length|-l]\n"
			"                     [--coord-type|-t] [--coord-sep|-s] [--prefix|-p]\n"
			"                     [--suffix|-s] } | { [--printf-syntax|-f] }\n"
			"					[--output-console|-o]\n"
			"					dataset_to_rename\n\n\n" );

    printf( " --help|-h: print this message\n"
			" --refpoint|-r: Ref point to use in the name. Any pair of 'W', 'E' and 'N',\n"
			"                'S' (ie: WN, NW, EN, NE, WS, SW, ES, SE).\n"
			" --prefix|-p: Prefix to use for the new file name format (ie: \n"
			"              Prefix_XXXXXXXXXXX.EXT)\n"
			" --coord-length|-l: number of digits to use in coordinates value starting with\n"
			"                    high wheight digits (ie: using 4 with easting value of \n"
			"                    621000.0 will print 06210 with zero padding and \n"
			"                    6210 without)\n"
			" --coord-decimal-length|-d: number of digits to use in decimal part for real coordinates\n"
			" --coord-zero-padding|-z: wether or not padding coordinate with 0 if inequal \n"
			"                          number of digits between easting and northing\n \n"
			" --coord-type|-t: 'int' or 'real' used to format coordinates\n"
			" --coord-sign|-g: 'std': sign printed if negative, 'force': allways printed (+/-) or 'geo': hemisphere display E-W or N-S\n"
			" --coord-sep|-s: separator of coordinates components when formating\n"
			" --suffix|-x: suffix to use for the new file name format (ie: \n"
			"              XXXXXXXXXXX_suffix.EXT)\n"
			" --printf-syntax|-f: a string with printf syntax instead of all the previous parameters\n"
			"                     (ie: Tiles_%.4d-%.4d_Lambert93) /!\ no syntaxic check\n"
			"                     and only 2 substitution allowed for easting/northing\n"
			" --output-console|-o: whether or not outpouting the console command to perform\n"
			"					   the rename instead of doing it on the file system. 'win' or 'unix' to\n"
			"					   output the command for windows batch or linux bash format\n");

    if( pszErrorMsg != NULL )
        fprintf(stderr, "\nFAILURE: %s\n", pszErrorMsg);

    exit( 1 );
	}

//***********************************************************************//
//!Check whether or not the given refpoint is good
//***********************************************************************//
bool checkRefPoint(const char* CoordRefPoint)
	{
	bool Success = false;

	if ( strlen(CoordRefPoint)==2)
		{
		if ( EQUAL(CoordRefPoint, "WN" ) || EQUAL(CoordRefPoint, "EN" ) || EQUAL(CoordRefPoint, "NW" ) || EQUAL(CoordRefPoint, "NE" ) ||
			 EQUAL(CoordRefPoint, "WS" ) || EQUAL(CoordRefPoint, "ES" ) || EQUAL(CoordRefPoint, "SW" ) || EQUAL(CoordRefPoint, "ES" ) )
			{ Success = true ; }
		}

	return Success;
	}

//***********************************************************************//
//!format the rename command line for output in the console
//***********************************************************************//
std::string formatRenameCmdLine(const char* pszSystemType, const char* pszSourceName, const char* pszNewName)
	{
	bool Success = false;
	const char* pszCommand = "move";
	std::string sCommandLine;

	if ( EQUAL(pszSystemType, "unix") )
		{ pszCommand = "mv"; }

	sCommandLine += pszCommand;
	sCommandLine += " \"";
	sCommandLine += pszSourceName;
	sCommandLine += "\" \"";
	sCommandLine += pszNewName;
	sCommandLine += "\"\n";

	return sCommandLine.c_str();
	}

//#######################################################################//
//!Main
//#######################################################################//
int main(int argc, char* argv[])
	{
	//for --config options
    ::EarlySetConfigOptions(argc, argv);

    ::GDALAllRegister();

	const char *pszFilePath = "Test.tif";
	const char *pszCoordRefPoint = "WN"; // any Easting/Northing pair combination in the following list N, E, S, W
	bool		pszCoordPadding = true;
	bool		printUsage = false;
	const char *pszCoordLenght = "7";
	const char *pszCoordDecLenght = "3";
	const char *pszCoordType = ""; //"reel"
	const char *pszCoordSignType = "std"; // "std , force or geo"
	const char *pszCoordSep = "_";
	const char *pszPrefix = "";
	const char *pszSuffix = "";
	const char *pszPrintf= ""; // printf syntax for the whole renaming string ex: "Tile_%.4d_%.4d_SRS" /!\ no syntax check
	const char *pszOutputConsole = "";	//"win" or "unix"

    argc = ::GDALGeneralCmdLineProcessor( argc, &argv, 0 );

    if( argc > 1 )
		{
		for( int i = 0; argv != NULL && argv[i] != NULL; i++ )
			{
			if( EQUAL(argv[i], "--help") || EQUAL(argv[i], "-h") )
				{ printUsage = true; }

			if( EQUAL(argv[i], "--refpoint") || EQUAL(argv[i], "-r") ) //WN, WS, EN, ES
				{
				pszCoordRefPoint = argv[i+1];

				if ( !checkRefPoint(pszCoordRefPoint) )
					{ pszCoordRefPoint = "WN"; } // Top Left by default
				}

			if( EQUAL(argv[i], "--coord-zero-padding") || EQUAL(argv[i], "-z") )
				{ pszCoordPadding = true; }

			if( EQUAL(argv[i], "--coord-length") || EQUAL(argv[i], "-l") ) // number of digits for coord to use for renaming
				{ pszCoordLenght = argv[i+1]; }							   // /!\ if integer and less than number removed by right

			if( EQUAL(argv[i], "--coord-decimal-length") || EQUAL(argv[i], "-d") ) // size of number of coord to use for renaming /!\ if less than number removed by right
				{ pszCoordDecLenght = argv[i+1]; }

			if( EQUAL(argv[i], "--coord-type") || EQUAL(argv[i], "-t") ) // int or "real", default "int"
				{ pszCoordType = argv[i+1]; }

			if( EQUAL(argv[i], "--coord-sign") || EQUAL(argv[i], "-g")  ) //"std": sign printed if negative, "force": +/- or "geo": N/S - E/W, default "std"
				{ pszCoordSignType = argv[i+1]; }

			if( EQUAL(argv[i], "--coord-sep") || EQUAL(argv[i], "-s") ) //any string
				{ pszCoordSep = argv[i+1]; }

			if( EQUAL(argv[i], "--prefix") || EQUAL(argv[i], "-p") ) //any string
				{ pszPrefix = argv[i+1]; }

			if( EQUAL(argv[i], "--suffix") || EQUAL(argv[i], "-x") ) //any string
				{ pszSuffix = argv[i+1]; }

			if( EQUAL(argv[i], "--printf-syntax") || EQUAL(argv[i], "-f") ) // use instead of --coord-sign, --coord-padding, --coord-type, --coord-lenght, --prefix, --suffix
				{ pszPrintf = argv[i+1]; }

			if( EQUAL(argv[i], "--output-console")|| EQUAL(argv[i], "-o")  ) //"win" or "unix", default empty, performing the rename on the console instead of the file system
				{ pszOutputConsole = argv[i+1]; }

			}
		pszFilePath = argv [argc - 1];
		}

	if (argc <= 1 || printUsage)
		{ Usage(); }

    ::GDALDataset  *poDataset = NULL;

	std::string inputDataset = pszFilePath;

	std::string sFilename	= ::CPLGetFilename(pszFilePath);
	std::string sDirName	= ::CPLGetDirname(pszFilePath);
	std::string sExt		= ::CPLGetExtension(pszFilePath);
	std::string sBasename	= ::CPLGetBasename(pszFilePath);

	std::string sNewFileName;
	std::string sSiblingFileName = sDirName + sDirSep + sBasename + "." ;
	std::string sMainPrintfStx ;


	poDataset = (GDALDataset *) ::GDALOpen( pszFilePath, GA_ReadOnly );

	if( poDataset != NULL )
		{
		double        adfGeoTransform[6];

		if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
			{
			std::string sCoordPrintfStx ;

			double Coord0 = .0, Coord1 = .0;
			int iCoord0 = 0, iCoord1 = 0;
			char Sign0 = 0x00, Sign1 = 0x00;

			Coord0 = getCoord(pszCoordRefPoint[0], adfGeoTransform, poDataset);
			if ( EQUAL(pszCoordSignType , "geo") )
				{Sign0 = getCoordHemi(pszCoordRefPoint[0], adfGeoTransform, poDataset);}

			Coord1 = getCoord(pszCoordRefPoint[1], adfGeoTransform, poDataset);
			if ( EQUAL(pszCoordSignType , "geo") )
				{Sign1 = getCoordHemi(pszCoordRefPoint[1], adfGeoTransform, poDataset);}

			//
			if ( EQUAL(pszPrintf, "") )
				{
				sCoordPrintfStx += "%";

				//Sign formatting
				if ( EQUAL(pszCoordSignType , "geo") ) // E/W or N/S adding an extra %c to add the char
					{sCoordPrintfStx+="c%";}
				else if ( EQUALN(pszCoordSignType , "force", 4) )
					{ sCoordPrintfStx +=  "+"; }

				if ( EQUAL(pszCoordType , "real") )
					{

					if ( pszCoordPadding && pszCoordLenght != "") //if real padding is made with %0n where n is a digit or %0* with extra param
						{ sCoordPrintfStx = sCoordPrintfStx + "0" + pszCoordLenght + "." + pszCoordDecLenght + "f"; }

					}
				else
					{

					if ( pszCoordPadding && pszCoordLenght != "") //if real padding is made with %0n where n is a digit or %0* with extra param
						{ sCoordPrintfStx = sCoordPrintfStx + "." + pszCoordLenght + "d"; }

					int baseDigits = std::max(nDigits(Coord0), nDigits(Coord1));
					int formatDigits = atoi(pszCoordLenght);

					iCoord0 = (int) Coord0;
					iCoord1 = (int) Coord1;

					if (baseDigits > formatDigits )
						{
						int exp = baseDigits-formatDigits;
						iCoord0 = Coord0 / pow(10.,exp);
						iCoord1 = Coord1 / pow(10.,exp);
						}
					}

				sMainPrintfStx = pszPrefix + sCoordPrintfStx + pszCoordSep + sCoordPrintfStx + pszSuffix;
				}
			else
				{ sMainPrintfStx = pszPrintf;}

			char NewFileNameBuff [256] = "\0";
			char* pszNewFileName = (char*)&NewFileNameBuff;

			try
				{
				if ( EQUAL(pszCoordType , "real") )
					{
					if ( EQUAL(pszCoordSignType , "geo") )
						{::CPLsnprintf(pszNewFileName, 256, sMainPrintfStx.c_str(), Sign0 ,Coord0, Sign1 , Coord1);}
					else
						{::CPLsnprintf(pszNewFileName, 256, sMainPrintfStx.c_str(), Coord0 , Coord1);}
					}
				else
					{
					if ( EQUAL(pszCoordSignType , "geo") )
						{::CPLsnprintf(pszNewFileName, 256, sMainPrintfStx.c_str(), Sign0 ,iCoord0, Sign1 , iCoord1);}
					else
						{::CPLsnprintf(pszNewFileName, 256, sMainPrintfStx.c_str(), iCoord0 , iCoord1);}
					}
				}
			catch (...)
				{
				::CPLprintf( "/!\\ printf issue while building new file name \"%s\"\nIf you used --printf-syntax|-f check it is right\notherwise please submit the bugg at https://github.com/MattLatt/gdal_rename\n", pszFilePath);
				::GDALClose((GDALDatasetH)poDataset);
				return 1;
				}


			::GDALClose((GDALDatasetH)poDataset);

			std::string sNewFilePath = sDirName + sDirSep + pszNewFileName + "." + sExt ;

			if ( EQUAL(pszOutputConsole, "") )
				{
				bool Success =  renameFileNoOverWrite(pszFilePath, sNewFilePath) ;
				if (!Success)
					{ ::CPLprintf( "/!\\ printf issue while renaming file \"%s\"\nCheck the file is not locked or still exist !!!\n", pszFilePath); }
				}
			else
				{ std::cout << formatRenameCmdLine( pszOutputConsole, pszFilePath, sNewFilePath.c_str()).c_str() << std::endl;}

			std::vector<std::string> SiblingFilesExts = getSiblingFilesExt(pszFilePath);
			std::vector<std::string>::iterator it = SiblingFilesExts.begin();

			while ( it != SiblingFilesExts.end() )
				{
				std::string sCurSiblingFileName = sSiblingFileName;

				sCurSiblingFileName += (*it).c_str();
				std::string sNewSiblingFilePath = sDirName + sDirSep + pszNewFileName + "." + (*it).c_str() ;

				if ( EQUAL(pszOutputConsole, "") )
					{ renameFileNoOverWrite(sCurSiblingFileName, sNewSiblingFilePath) ; }
				else
					{ std::cout << formatRenameCmdLine( pszOutputConsole, sCurSiblingFileName.c_str(), sNewSiblingFilePath.c_str()).c_str() << std::endl ;}

				++it;
				}

			}
		else
			{
			::CPLprintf( "/!\\ No GeoRef information found by GDAL in the file \"%s\", Exiting...\n", pszFilePath);
			::GDALClose((GDALDatasetH)poDataset);
			return 1;
			}

		}


	return 0;
	}

