#include <Magick++.h>
#include <magick/statistics.h>
#include <magick/image.h>
#include <string>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>

using namespace std;
using namespace Magick;

char workingDirectory[1024] = "./";
char fileList[1024];
int inputType = 1;
char outputFile[1024] = "" ;
int debugLevel = 1; // can be between 1 to 4
char cropDimension[24];
bool mCropImage = false ;
int sliceWidth = 0;
int imageWidth = 0;
int imageHeight = 0;
int totalImages = 0;

std::vector<std::string> imageList;


void printUsage(char *binaryName)
{
  fprintf(stdout, "Usage : %s [ -d <path/to/image/directory/>] [ -h|--help]", binaryName);

  fprintf (stdout, "\n");

	fprintf(stdout, "Where : \n");
	fprintf(stdout, "  -d\t\tPath to the directory where images are located. Default is pwd\n");
	fprintf (stdout, "  -h\t\tPrint help information\n");

  exit(1);
}

void read_files()
{
	DIR *dir;
  struct dirent *ent;
  struct dirent **namelist;
  int numEntries;
	string fileName;
	string tmp;
	Image tmp_image;
	Geometry gm;
	FILE *fp = NULL;
	char buffer[1024];

	if ( inputType == 1 ) {
		numEntries = scandir(workingDirectory, &namelist, NULL, alphasort);

		totalImages = 0;
		for(int i=0; i< numEntries; i++) {
			if ( !strcmp(".", namelist[i]->d_name) || !strcmp("..", namelist[i]->d_name))
				continue;

			if ( strlen(namelist[i]->d_name) > 126 )
			{
					fprintf(stdout, "WARNING :  %s file name is longer than 126 characters.. Ignoring\n", namelist[i]->d_name);
			}
		
			fileName = workingDirectory;
			tmp = "/" ;
			fileName = fileName + tmp;
			tmp = namelist[i]->d_name;
			fileName = fileName + tmp;

			imageList.push_back(fileName);

			/* Store the width of the image now */
			if( !imageWidth )
			{
				tmp_image.read(fileName.c_str());		
				gm = tmp_image.size();
				imageWidth = gm.width();
				imageHeight = gm.height();
			}
			totalImages ++;
		}
	}
	else if ( inputType == 2 ) // Read from text file
	{
		fp = fopen(fileList, "r");

		if ( !fp ) {
			cout << "Unable to open the input file : " << fileList << " Exiting... " << endl;
			exit(-1);
		}

		totalImages = 0;
		while( fgets(buffer, 1024, fp) != NULL ) 
		{
			buffer[strlen(buffer)-1] = '\0';
			fileName = buffer;
			imageList.push_back(fileName);

			/* Store the width of the image now */
			if( !imageWidth )
			{
				tmp_image.read(fileName.c_str());		
				gm = tmp_image.size();
				imageWidth = gm.width();
				imageHeight = gm.height();
			}
			totalImages ++;
		}
	}
}


void build_time_slice()
{
	Image tmp;
	int x_cord = 0;
	int mcount = 0;
	int widthCovered = 0 ;

	Image new_image(Geometry(imageWidth, imageHeight), "white");

	std::vector<string>::const_iterator it;	

	/* First calculate the time slice for individual images */
	sliceWidth = imageWidth/totalImages;

	std::cout << "Creating image with " << totalImages << " slices..." << endl;


	for( it = imageList.begin(); it != imageList.end(); ++it)
	{
		//std::cout << *i <<endl;
		tmp.read((*it).c_str());
		cout << "File Name : " << *it << endl;
		/*
		if ( mcount = (totalImages - 1 ) )
		{
			cout << "count...." << endl;
			if ( imageWidth - ( x_cord+sliceWidth)  > 0 ) 
			tmp.crop(Geometry( (imageWidth - ( x_cord+sliceWidth)), imageHeight, x_cord, 0));		
		}
		//else 
		*/
		tmp.crop(Geometry(sliceWidth, imageHeight, x_cord, 0));

		//tmp.display();
		/* Now calculate new x coordinate */

		if ( ! mcount )
			new_image.composite(tmp, 0, 0, InCompositeOp);
		else {
			new_image.composite(tmp, x_cord, 0, OverCompositeOp);
		}

		x_cord = x_cord + sliceWidth;
		mcount++;
	}
	
	new_image.quality(95);
	if ( strlen(outputFile) > 0 ) {
		new_image.write(outputFile);
		std::cout << "Create timeslice with file name : " << outputFile << endl;
	}
	else {
		new_image.write("new.jpg");
		std::cout << "Create timeslice with file name : new.jpg " << endl;
	}
	/*
	tmp_image = orig_image1;

	tmp_image.crop(Geometry(100, 3000, 0,0));

	new_image.composite(tmp_image, 0, 0, InCompositeOp);

	tmp_image = orig_image2;
	tmp_image.crop(Geometry(100, 3000, 0,0));
	new_image.composite(tmp_image, 100, 0,OverCompositeOp);
	*/

}

void processCmdLine(int argc, char **argv)
{
  int param = 0;
  int i = 0;
  int cpuCount = 0;

  if ( argc < 2 )
  {
    printUsage(argv[0]);
  }

  for(i = 1; i < argc; i++ )
  {
    if( '-' == *argv[i] )
    {
      if ( !strcmp("-d", argv[i])){
        strcpy(workingDirectory,  argv[++i]);
      }
			else if ( !strcmp("-f", argv[i])){
				strcpy(fileList	, argv[++i]);
				inputType = 2;
			}
      else if ( !strcmp("-o", argv[i])){
        strcpy(outputFile,  argv[++i]);
      }
      else if ( !strcmp("--debug-level", argv[i]))
        debugLevel = atoi(argv[++i]);
      else if ( !strcmp("--crop", argv[i])){
        strcpy(cropDimension, argv[++i]);
        mCropImage = true;
      }
      else if ( !strcmp("-h", argv[i]))
        printUsage(argv[0]);
      else if ( !strcmp("--help", argv[i]))
        printUsage(argv[0]);
    }
    else
    {
      fprintf(stderr, "Unrecognized parameter : %s\n", argv[i]);
      printUsage(argv[0]);
    }
  }
}

int main(int argc, char *argv[])
{
	InitializeMagick(*argv);
	std::vector<string>::const_iterator i;

	/*
	Image orig_image1;
	Image orig_image2;
	Image new_image(Geometry(4000, 3000), "white");
	Image tmp_image;
	*/

	processCmdLine(argc, argv);
	
	read_files();

	build_time_slice();

/*
	for( i = imageList.begin(); i!= imageList.end(); ++i)
	{
		std::cout << *i <<endl;
	}
	orig_image1.read(argv[1]);
	orig_image2.read(argv[2]);

	tmp_image = orig_image1;

	tmp_image.crop(Geometry(100, 3000, 0,0));

	new_image.composite(tmp_image, 0, 0, InCompositeOp);

	tmp_image = orig_image2;
	tmp_image.crop(Geometry(100, 3000, 0,0));
	new_image.composite(tmp_image, 100, 0,OverCompositeOp);

	new_image.write("new.jpg");
*/
	return 0;
}

