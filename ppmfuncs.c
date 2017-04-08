/*
 * ppmfuncs.c
 * by Samuel Weaver (sweave05) and Ross Kamen (rkamen02)
 * October 8, 2016
 * HW 3
 *
 * Implementation of functions to transform images using the Pnm_ppm struct.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <a2plain.h>
#include <a2blocked.h>
#include <time.h>
#include "cputiming.h"
#include "ppmfuncs.h"

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        *methods = (METHODS);                                    \
        assert(*methods != NULL);                                \
        *map = (*methods) -> MAP;				 \
        if (*map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (0)

static void 
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

/*
 * validate_args_initialize
 *	purp: determines whether the command line arguments are valid.
		sets the desired mapping function to map.
 *		retrieves the name of the time file if desired.
 *	args: int argc, char *argv[], A2Methods_mapfun **map, 
 *		char **time_file_name, A2Methods_T *methods
 *	rets: 1 if image file is given on the command line. 0 if file is to
 *		be read from stdin.
 */
int validate_args_initialize( int argc, 
			      char *argv[], 
			      A2Methods_mapfun **map,
			      char **time_file_name,
			      A2Methods_T *methods)
{

	int i = 0;
	int rotation = 0;
	char *endptr;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, 
					map_row_major, 
					"row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, 
					map_col_major, 
					"column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
			SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }                        
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
				       "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }

		/* this program implements a flip option */
		} else if (strcmp(argv[i], "-flip") == 0) {
                        if (!(i + 1 < argc)) {      /* no flip type */
                                usage(argv[0]);
                        }
			if ((strcmp(argv[i + 1], "horizontal") != 0) &&
			    (strcmp(argv[i + 1], "vertical") != 0)){
				fprintf(stderr, 
				       "Flip must be vertical \
					 or horizontal\n");
                                usage(argv[0]);
			}
			i++;
                } else if (strcmp(argv[i], "-time") == 0) {
                        *time_file_name = argv[++i];

       		/* this program implements a transpose option */
		} else if (strcmp(argv[i], "-transpose") == 0) {
	      		continue;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, 
				"%s: unknown option '%s'\n", 
				argv[0], 
				argv[i]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }

	if ( i != argc) {
		return 1;
	} else {
		return 0;
	}
}



/**************************************************
 *********** Time Logging Functions ***************
 *************************************************/

/*
 * open_log
 *	purp: opens a file to log the transformation time if desired.
 *	args: char * file name
 *	rets: FILE *
 */
FILE *open_log( char *time_file_name, char *image_file_name)
{
	/* user did not use -time option */
	if (time_file_name == NULL) {
		return NULL;
	}

	FILE *log_file = fopen( time_file_name, "a");
	time_t rawtime = time( NULL); /* to put the real time in header */
	
	if (log_file == NULL) {
		fprintf( stderr, "Could not open or create time log file\n");
		exit( EXIT_FAILURE);
	}
	
	fprintf( log_file, "\n");
	fprintf( log_file, ctime( &rawtime));
	fprintf( log_file, "%s\n", image_file_name);
	
	return log_file;
}


/*
 * log_characteristics
 *	purp: puts the dimensions of the image in the time file
 *	args: Pnm_ppm *, FILE *
 *	rets: none
 */
void log_characteristics( int argc,
			  char *argv[], 
			  Pnm_ppm *image, 
			  FILE *log_file)
{
	int i;

	/* user did not use -time option */
	if ( log_file == NULL) {
		return ;
	}
	
	/* add image width and height to log file */
	int width = (*image) -> width;
	int height = (*image) -> height;
	fprintf( log_file, "%d x %d\n", width, height);

	/* find mapping type in argv and print */
	for (i = argc - 1; i > 0; i--){
		if ( strcmp( argv[i], "-row-major") == 0) {
			fprintf( log_file, "-row-major\n");
			break;
		} else if ( strcmp( argv[i], "-col-major") == 0) {
			fprintf( log_file, "-col-major\n");
			break;
		} else if (strcmp( argv[i], "-block-major") == 0) {
			fprintf( log_file, "-block-major\n");
			break;
		}
	}

	if ( i == 1 ) {
		fprintf( log_file, "default mapping\n");
	}
}

/*
 * log_time
 *	purp: adds the mapping time to the time file
 *	args: char *, double, FILE *
 *	rets: none
 */
void log_time( char *func_name, double time_used, FILE *log_file)
{
	/* user did not use -time option */
	if (log_file == NULL) {
		return;
	}
	
	fprintf( log_file, "%-20s %20.*e\n", func_name, 6, time_used);
}

/*
 * close_log
 *	purp: closes the time file 
 *	args: FILE *
 *	rets: none
 */
void close_log( FILE *log_file)
{
	/* user did not use -time option */
	if (log_file == NULL) {
		return;
	}
	
	time_t rawtime = time( NULL); /* add real time to footer */
	fprintf( log_file, ctime( &rawtime));
	
	fclose( log_file);
}



/**************************************************
 *********** Pnm_ppm Image Manipulations **********
 *************************************************/

/*
 * make_edits
 *	purp: determines desired transformations and calls the appropriate
 *		apply functions for mapping. 
 *	args: int argc, char *argv[], Pnm_ppm, A2Methods_mapfun, FILE *
 *	rets: Pnm_ppm
 */
Pnm_ppm make_edits( int argc, 
		   char *argv[], 
		   Pnm_ppm image, 
		   A2Methods_mapfun *map, 
		   FILE *log_file)
{
	Pnm_ppm temp_image = deep_copy( image);
	
        for (int i = 1; i < argc; i++) {
 
		/* call appropriate pixel transformation function
		   depending on the command line arg */
                if (strcmp(argv[i], "-rotate") == 0) {
			if ( strcmp(argv[++i], "90") == 0) {
				make_edit( &temp_image, 
       			       		   map, 
		       			   log_file, 
	       				   1,
					   rotate_90,
					   "rotate_90");
			} else if ( strcmp(argv[i], "180") == 0) {
				make_edit( &temp_image, 
			       		   map, 
		       			   log_file, 
	       				   0, 
					   rotate_180,
					   "rotate_180");
			} else if ( strcmp(argv[i], "270") == 0) {
				make_edit( &temp_image, 
			       		   map, 
		       			   log_file, 
	       				   1, 
					   rotate_270,
					   "rotate_270");
			}
		} else if (strcmp(argv[i], "-flip") == 0) {
			if ( strcmp(argv[++i], "horizontal") == 0) {
				make_edit( &temp_image, 
					   map, 
				       	   log_file, 
			       		   0, 
					   flip_horiz,
					   "flip_horiz");
			} else {
				make_edit( &temp_image, 
					   map, 
				       	   log_file, 
			       		   0, 
					   flip_vert,
					   "flip_vert");
			}
		} else if (strcmp(argv[i], "-transpose") == 0) {
	      		make_edit( &temp_image, 
				   map, 
			       	   log_file, 
		       		   1,
				   transpose,
				   "transpose");
		}
        }
	
	return temp_image;
}

/*
 * make_edit
 *	purp: calls the map function on the image. times it with CPUTime_T.
 *	args: Pnm_ppm * pre-edited image, A2Methods_mapfun * desired map 
 *		function, FILE * time log file, int rotation (1 to switch 
 *		width/height, 0 to keep the same), A2Methods_applyfun 
 *		edit_function, char *func_name
 *	rets: none
 */
void make_edit( Pnm_ppm *image, 
		A2Methods_mapfun *map, 
		FILE *log_file, 
		int rotation,
		A2Methods_applyfun edit_function,
		char *func_name)
{
	/* after map, blank will be filled in with transformed pixels */
       	Pnm_ppm blank_copy = empty_copy( *image, rotation);
       	A2Methods_UArray2 pixels = (*image) -> pixels;
	double time_used;
	CPUTime_T timer = CPUTime_New();

	CPUTime_Start( timer);
	(*map)( pixels, edit_function, &blank_copy);
	time_used = CPUTime_Stop( timer);
	log_time( func_name, time_used, log_file);	
	CPUTime_Free( &timer);

	Pnm_ppmfree(image);
	*image = blank_copy;
}

/*
 * empty_copy
 *	purp: creates a deep copy duplicate of image. duplicate will have 
 *		uninitialized array of pixels.
 *	args: Pnm_ppm, int rotated (1 to switch width/height, 0 to keep the
 *		 same
 *	rets: Pnm_ppm deep copy duplicate of image arg
 */
Pnm_ppm empty_copy( Pnm_ppm image, int rotated)
{
	A2Methods_T methods = (A2Methods_T)(image -> methods);
	A2Methods_UArray2 blank_pixels;
	Pnm_ppm blank_image = malloc( sizeof( struct Pnm_ppm));
	int size = methods -> size( image -> pixels);
	int denom = image -> denominator;
	int new_width  = image -> width;
	int new_height = image -> height;

	if ( blank_image == NULL) {
		exit( EXIT_FAILURE);
	}

	if (rotated == 1) {
		new_width  = image -> height;
		new_height = image -> width;
	}
	
	blank_pixels = methods -> new( new_width, new_height, size);
	blank_image -> denominator = denom;
	blank_image -> width = new_width;
	blank_image -> height = new_height;
	blank_image -> pixels = blank_pixels;
	blank_image -> methods = methods;
	
	return blank_image;
}

/*
 * deep_copy
 *	purp: creates a complete deep copy duplicate of Pnm_ppm struct.
 *	args: Pnm_ppm image
 *	rets: Pnm_ppm
 */
Pnm_ppm deep_copy( Pnm_ppm image) 
{
	Pnm_ppm temp_image = empty_copy( image, 0);
	A2Methods_UArray2 pixels = image -> pixels;

	((*(image -> methods)).map_default)( pixels, rotate_0, &temp_image);
	
	return temp_image;
}

/*
 * insert_pixel
 *	purp: inserts pixel struct at desired (x,y) coord in image.
 *	args: int x, int y, A2Methods_Object *pixel_in, Pnm_ppm *image
 *	rets: none
 */
void insert_pixel( int x, int y, A2Methods_Object *pixel_in, Pnm_ppm *image)
{
	A2Methods_T methods = (A2Methods_T)((*image) -> methods);
	A2Methods_UArray2 *pixels = &( (*image) -> pixels);
	struct Pnm_rgb *pixel_at = (*methods).at( *pixels, x, y);
	*pixel_at = *((struct Pnm_rgb *)pixel_in);
}



/**************************************************
 *********** Pixel Transformations ****************
 *************************************************/

/*
 * rotate_0
 *	purp: inserts a pixel into image with 1-to-1 mapping
 *	args: int x coord, int y coord, A2Methods_UArray2 array2, 
 *		A2Methods_Object *pixel, void *image
 *	rets: none
 */
void rotate_0( int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image)
{
	(void) array2;
	insert_pixel(x, y, pixel, image);
}


/*
 * rotate_90
 *	purp: inserts a pixel into an image with the pixel rotated 90 
 *		degrees from its original location
 *	args:  int x coord, int y coord, A2Methods_UArray2 array2, 
 *		A2Methods_Object *pixel, void *image
 *	rets:
 */
void rotate_90( int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image)
{
	(void) array2;
	int new_width  = (*((Pnm_ppm *)image)) -> width;
	int new_x = new_width - y- 1;
	int new_y = x;
	insert_pixel( new_x, new_y, pixel, image);
}


/*
 * rotate_180
 *	purp: inserts a pixel into an image with the pixel rotated 180 
 *		degrees from its original location
 *	args: int x coord, int y coord, A2Methods_UArray2 array2, 
 *		A2Methods_Object *pixel, void *image
 *	rets: none
 */
void rotate_180(int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image)
{
	(void) array2;
	int new_width  = (*((Pnm_ppm *)image)) -> width;
	int new_height = (*((Pnm_ppm *)image)) -> height; 
	int new_x = new_width - x- 1;
	int new_y = new_height  - y -1;
	insert_pixel( new_x, new_y, pixel, image);
}


/*
 *
 *	purp:  inserts a pixel into an image with the pixel rotated 270 
 *		degrees from its original location
 *	args: int x coord, int y coord, A2Methods_UArray2 array2, 
 *		A2Methods_Object *pixel, void *image
 *	rets: none
 */
void rotate_270(int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image)
{
	(void) array2;
	int new_height = (*((Pnm_ppm *)image)) -> height; 
	int new_x = y;
	int new_y = new_height -x -1;
	insert_pixel( new_x, new_y, pixel, image);
}


/*
 * flip_horiz
 *	purp: inserts a pixel into an image reflected across the vertical 
 *		axis of symmetry  
 *	args: int x coord, int y coord, A2Methods_UArray2 array2, 
 *		A2Methods_Object *pixel, void *image
 *	rets:
 */
void flip_horiz(int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image)
{
	(void) array2;
	int new_width  = (*((Pnm_ppm *)image)) -> width;
	int new_x = new_width - x - 1;
	int new_y = y;
	insert_pixel( new_x, new_y, pixel, image);
}


/*
 * flip_vert
 *	purp:  inserts a pixel into an image reflected across the 
 *		horizontal axis of symmetry 
 *	args: int x coord, int y coord, A2Methods_UArray2 array2, 
 *		A2Methods_Object *pixel, void *image
 *	rets: none
 */
void flip_vert(	int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image)
{
	(void) array2;
	int new_height = (*((Pnm_ppm *)image)) -> height; 
	int new_x = x;
	int new_y = new_height - y - 1;
	insert_pixel( new_x, new_y, pixel, image);
}


/*
 * transpose
 *	purp: inserts a pixel into an image reflected across the line y=x 
 *	args: int x coord, int y coord, A2Methods_UArray2 array2, 
 *		A2Methods_Object *pixel, void *image
 *	rets: none
 */
void transpose( int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image)
{
	(void) array2;
	int new_x = y;
	int new_y = x;
	insert_pixel( new_x, new_y, pixel, image);
}
