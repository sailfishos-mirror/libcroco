/*
 * csslint.c : a small tester program for CSS2 input
 *
 * Copyright (C) 2002-2003 Gaël Chamoulaud <strider@freespiders.org>
 *                         Dodji Seketeli  <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "libcroco.h"

#include <string.h>

/**
 *Displays the version text.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
showVersion (const char *name) 
{
    fprintf(stderr, "%s: using libcroco version %s\n", name, VERSION);
    fprintf(stderr, "   compiled with: ");
#ifdef HAVE_LIBXML2
	fprintf(stderr, "LIBXML2 ");
#endif
#ifdef WITH_SELENG
	fprintf(stderr, "SELENG ");
#endif
#ifdef WITH_TESTS
	fprintf(stderr, "TESTS ");
#endif
    fprintf(stderr, "\n");
}

/**
 *Displays the usage text.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
usage(const char *name) 
{
    printf("Usage : %s [options] CSS2files ...\n", name);
    printf("\tParse the CSS2 files and output the result of the parsing\n");
    printf("\t--version : display the version of the CSS2 library used\n");
    printf("\nLibcroco project home page: http://www.freespiders.org/projects/libcroco\n");
    printf("To report bugs or get some help check: http://bugzilla.gnome.org/\n");
}

int 
main (int argc, char **argv) 
{
    int i;
	
    if (argc <= 1) 
	{
        usage(argv[0]);
        return(1);
    }
    
	for (i = 1; i < argc ; i++) 
	{
		if (!strcmp(argv[i], "-"))
			break;

		if (argv[i][0] != '-')
			continue;
		
		if ((!strcmp(argv[i], "-version")) || 
			(!strcmp(argv[i], "--version"))) 
		{
			showVersion (argv[0]);
		} 
		else 
		{
			fprintf(stderr, "Unknown option %s\n", argv[i]);
			usage(argv[0]);
			return(1);
		}

	}
	return 1;
}


