/*
 * ppmfuncs.c
 * by Samuel Weaver (sweave05) and Ross Kamen (rkamen02)
 * October 8, 2016
 * HW 3
 *
 * Interface to transform images using the Pnm_ppm struct.
 *
 */

#ifndef PPMFUNCS
#define PPMFUNCS

#include <stdio.h>
#include <pnm.h>
#include <a2methods.h>
#include <a2plain.h>
#include <a2blocked.h>

/* Determines whether the command line arguments are valid.
 *		sets the desired mapping function to map.
 *		retrieves the name of the time file if desired.
 * CREs:
 *	- no rotational value given or value is not 0, 90, 180, or 270
 *	- no argument given for flip or argument not vertical or horizontal
 *	- argument not one of:
 *		-rotate
 *	        -flip
 *		-transpose
 *		-time
 *	- more than one argument associated with any one of these tags
 */
int validate_args_initialize( int argc, 
			      char *argv[], 
			      A2Methods_mapfun **map, 
			      char **time_file_name,
			      A2Methods_T *methods);

/*
 * time logging functions:
 */

/* Opens a file to log the transformation time if desired.
 * CRE: could not open provided time file */
FILE *open_log( char *time_file_name, char *image_file_name);

/* Puts the dimensions of the image in the time file */
void log_characteristics( int argc, 
			  char *argv[],
			  Pnm_ppm *image, 
			  FILE *log_file);

/* Puts the dimensions of the image in the time file */
void log_time( char *func_name, double time_used, FILE *log_file);

/* Closes the time file */
void close_log( FILE *log_file);


/*
 * Pnm_ppm image manipulations:
 */

/* Determines desired transformations and calls the appropriate
 * apply functions for mapping. */
Pnm_ppm make_edits( int argc, 
		    char *argv[], 
		    Pnm_ppm image, 
		    A2Methods_mapfun *map, 
		    FILE *log_file);

/*  Calls the map function on the image. times it with CPUTime_T. */
void make_edit( Pnm_ppm *image, 
		A2Methods_mapfun *map, 
		FILE *log_file, 
		int rotation,
		A2Methods_applyfun edit_function,
		char *func_name);

/* Creates a deep copy duplicate of image. duplicate will have 
 *		uninitialized array of pixels.
 * CRE: unable to malloc space for new Pnm_ppm struct */
Pnm_ppm empty_copy( Pnm_ppm image, int rotated);

/* Creates a complete deep copy duplicate of Pnm_ppm struct. */
Pnm_ppm deep_copy( Pnm_ppm image);

/* Inserts pixel struct at desired (x,y) coord in image. */
void insert_pixel( int x, int y, A2Methods_Object *pixel, Pnm_ppm *image);


/*
 * pixel transformations 
 */

/* Inserts a pixel into image with 1-to-1 mapping */
void rotate_0( int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image);

/* Inserts a pixel into an image with the pixel rotated 90 
 * degrees from its original location. */
void rotate_90( int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image);

/*  Inserts a pixel into an image with the pixel rotated 180 
 *  degrees from its original location. */
void rotate_180(int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image);

/*  Inserts a pixel into an image with the pixel rotated 270 
 *  degrees from its original location. */
void rotate_270(int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image);

/*  Inserts a pixel into an image reflected across the vertical 
 *  axis of symmetry. */ 
void flip_horiz(int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image);

/*  Inserts a pixel into an image reflected across the 
 *  horizontal axis of symmetry. */ 
void flip_vert(	int x, 
		int y, 
		A2Methods_UArray2 array2, 
		A2Methods_Object *pixel, 
		void *image);

/* Inserts a pixel into an image reflected across the line y=x. */
void transpose( int x, 
		int y,
		A2Methods_UArray2 array2,
		A2Methods_Object *pixel, 
		void *image);

#endif
