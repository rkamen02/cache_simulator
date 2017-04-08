/*
 * ppmtrans.c
 * by Samuel Weaver (sweave05) and Ross Kamen (rkamen02)
 * October 8, 2016
 * HW 3
 *
 * Jpeg transformation program with execution preferences.
 *
 */

#include <stdlib.h>
#include <assert.h>
#include "ppmfuncs.h"

int main(int argc, char *argv[]) 
{
        FILE *image_file = NULL;
        FILE *time_log_file = NULL;
        Pnm_ppm image;
        Pnm_ppm edited_image;
        char *time_file_name = NULL;
        char *image_file_name = NULL;
        int has_command_line_img;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        /* verify proper command line args */
        has_command_line_img = validate_args_initialize( argc, 
                                             argv, 
                                             &map,
                                             &time_file_name, 
                                             &methods);

        if ( has_command_line_img == 1) {
                image_file = fopen( argv[argc-1], "r");
                if ( image_file == NULL) {
                        fprintf( stderr, "Could not open image file\n");
                        exit( EXIT_FAILURE);
                }
                image_file_name = argv[argc-1];
        } else {
                image_file = stdin;
                image_file_name = "read from stdin";
        }
        image = Pnm_ppmread( image_file, methods);

        time_log_file = open_log( time_file_name, image_file_name);
        log_characteristics( argc, argv, &image, time_log_file);
        fclose( image_file);

        edited_image = make_edits( argc, argv, image, map, time_log_file);
        
        Pnm_ppmwrite( stdout, edited_image);

        close_log( time_log_file);
        Pnm_ppmfree( &image);
        Pnm_ppmfree( &edited_image);
        
        return EXIT_SUCCESS;
}
