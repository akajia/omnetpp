/***************************************************/
/*            OMNeT++ NEDC (JAR) source            */
/*                                                 */
/*  File: jar.c                                    */
/*                                                 */
/*  Contents:                                      */
/*    some functions called by the parser:         */
/*       include, channel, system                  */
/*    interface to the parser                      */
/*    main()                                       */
/*                                                 */
/*  By: Jan Heijmans                               */
/*      Alex Paalvast                              */
/*      Robert van der Leij                        */
/*  Revised: Andras Varga 1996-2001                */
/*                                                 */
/***************************************************/

/*--------------------------------------------------------------*
  Copyright (C) 1992-2001 Andras Varga
  Technical University of Budapest, Dept. of Telecommunications,
  Stoczek u.2, H-1111 Budapest, Hungary.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>
#include <time.h>

#include "jar_func.h"
#include "jar_lib.h"

#define NEDC_VERSION      "2.0"
#define NEDC_VERSION_HEX  "0x0200"

#ifdef __MSDOS__
        #define SLASH "\\"
#else
        #define SLASH "/"
#endif

char suffix[32] = "_n.cc";

/*--------------------------------------------------------*/
/*--- Declarations of local functions  ---*/

void print_header (FILE *f);
void open_yyin (char *namestr);
int do_firstpass (char *root_fname);
int do_secondpass (char *root_fname);
int compilefile (char *root_fname);

/*------------------------------------------------------*/

int firstpass;
int verbose;
char fname[256];

char current_fname[256];

/*------------------------------------------------------*/
/*--- Functions ---*/

void print_header (FILE *f)
{
        time_t nowt;
        struct tm *now;
        time (&nowt);
        now = localtime (&nowt);

        fprintf (f, "//-----------------------------------------\n");
        fprintf (f, "//\n");
        fprintf (f, "// Generated by NEDC version " NEDC_VERSION "\n");
        fprintf (f, "// date:\t%s", asctime (now));
        fprintf (f, "//\n");
        fprintf (f, "// Input file:\t%s.ned\n", fname);
        fprintf (f, "// Output file:\t%s%s\n", fname, suffix);
        fprintf (f, "//-----------------------------------------\n\n\n");

        fprintf (f, "#include <math.h>\n" );
        fprintf (f, "#include \"omnetpp.h\"\n\n" );

        fprintf (f, "#define check_error() \\\n"
                    "    {if (!simulation.ok()) return;}\n");
        fprintf (f, "#define check_memory() \\\n"
                    "    {if (memoryIsLow()) {opp_error(eNOMEM); return;}}\n");
        fprintf (f, "#define check_module_count(num, mod, parentmod) \\\n"
                    "    {if ((int)num<0) {opp_error(\"Negative module vector size %%s[%%d] in compound module %%s\", \\\n"
                    "                          mod,(int)num,parentmod);return;}}\n");
        fprintf (f, "#define check_gate_count(num, mod, gate, parentmod) \\\n"
                    "    {if ((int)num<0) {opp_error(\"Negative gate vector size %%s.%%s[%%d] in compound module %%s\", \\\n"
                    "                          mod,gate,(int)num,parentmod);return;}}\n");
        fprintf (f, "#define check_loop_bounds(lower, upper, parentmod) \\\n"
                    "    {if ((int)lower<0) \\\n"
                    "        {opp_error(\"Bad loop bounds (%%d..%%d) in compound module %%s\", \\\n"
                    "                 (int)lower,(int)upper,parentmod);return;}}\n");
        fprintf (f, "#define check_module_index(index,modvar,modname,parentmod) \\\n"
                    "    {if (index<0 || index>=modvar[0]->size()) {opp_error(\"Bad submodule index %%s[%%d] in compound module %%s\", \\\n"
                    "          modname,(int)index,parentmod);return;}}\n");
        fprintf (f, "#define check_channel_params(delay, err, channel) \\\n"
                    "    {if ((double)delay<0.0) \\\n"
                    "        {opp_error(\"Negative delay value %%lf in channel %%s\",(double)delay,channel);return;} \\\n"
                    "     if ((double)err<0.0 || (double)err>1.0) \\\n"
                    "        {opp_error(\"Incorrect error value %%lf in channel %%s\",(double)err,channel);return;}}\n");
        fprintf (f, "#define check_modtype(modtype, modname) \\\n"
                    "    {if ((modtype)==NULL) {opp_error(\"Simple module type definition %%s not found\", \\\n"
                    "                                     modname);return;}}\n");
        fprintf (f, "#define check_function(funcptr, funcname) \\\n"
                    "    {if ((funcptr)==NULL) {opp_error(\"Function %%s not found\", \\\n"
                    "                                     funcname);return;}}\n");
        fprintf (f, "#define check_gate(gateindex, modname, gatename) \\\n"
                    "    {if ((int)gateindex==-1) {opp_error(\"Gate %%s.%%s not found\",modname,gatename);return;}}\n");
        fprintf (f, "#define check_anc_param(ptr,parname,compoundmod) \\\n"
                    "    {if ((ptr)==NULL) {opp_error(\"Unknown ancestor parameter named %%s in compound module %%s\", \\\n"
                    "                                parname,compoundmod);return;}}\n");
        fprintf (f, "#define check_param(ptr,parname) \\\n"
                    "    {if ((ptr)==NULL) {opp_error(\"Unknown parameter named %%s\", \\\n"
                    "                                parname);return;}}\n");
        fprintf (f, "#ifndef __cplusplus\n"
                    "#  error Compile as C++!\n"
                    "#endif\n"
                    "#ifdef __BORLANDC__\n"
                    "#  if !defined(__FLAT__) && !defined(__LARGE__)\n"
                    "#    error Compile as 16-bit LARGE model or 32-bit DPMI!\n"
                    "#  endif\n"
                    "#endif\n\n");
        fprintf (f, "// Disable warnings about unused variables:\n"
                    "#ifdef _MSC_VER\n"
                    "#  pragma warning(disable:4101)\n"
                    "#endif\n"
                    "#ifdef __BORLANDC__\n"
                    "#  pragma warn -waus\n"
                    "#  pragma warn -wuse\n"
                    "#endif\n"
                    "// for GCC, seemingly there's no way to emulate the -Wunused command-line\n"
                    "// flag from a source file...\n\n");
        fprintf (f, "// Version check\n"
                    "#define NEDC_VERSION " NEDC_VERSION_HEX "\n"
                    "#if (NEDC_VERSION!=OMNETPP_VERSION)\n"
                    "#    error Version mismatch between OMNeT++ sim library and nedc used to create this file!\n"
                    "#endif\n");
        fprintf (f, "\n");
}

void do_include (char *fname)
{
        name_type namestr;

        if (!firstpass)
        {
                jar_free (fname);
                return;
        }

        /* cut off quotes */
        jar_strcpy (namestr, fname + 1);
        namestr [jar_strlen (namestr) - 1] = 0;

        /* cut off .ned extension */
        if (jar_strcmp (".ned", namestr + jar_strlen (namestr) - 4) == 0)
                namestr [jar_strlen (namestr) - 4] = 0;

        /* include in list if not already there */
        if (!nl_find (include_list, namestr))
                nl_add (&include_list, namestr);

        jar_free (fname);
}

void do_channel (char *cname, char *delay, char *error, char *datarate)
{
        char title [256];
        expr_type value;

        if (firstpass)
        {
                if (nl_find (channel_list, cname))
                {
                        sprintf (errstr,
                                "Duplicate channel name \"%s\"\n",
                                cname);
                        adderr;
                }
                nl_add (&channel_list, cname);
                jar_free (cname);
                jar_free (delay);
                jar_free (error);
                jar_free (datarate);
                return;
        }

        sprintf (title, "channel definition: %s", cname);
        print_sub_remark (yyout, title);

        if (delay)
        {
            fprintf (yyout, "static cPar *%s__delay()\n", cname);
            fprintf (yyout, "{\n");
            print_temp_vars( yyout);
            get_expression(delay ? delay : "0", yyout, value);
            fprintf (yyout, "\tcPar *p = new cPar(\"%s delay\");\n",cname);
            fprintf (yyout, "\t*p = %s;\n",value);
            fprintf (yyout, "\treturn p;\n");
            fprintf (yyout, "}\n\n");
        }

        if (error)
        {
            fprintf (yyout, "static cPar *%s__error()\n", cname);
            fprintf (yyout, "{\n");
            print_temp_vars( yyout );
            get_expression(error ? error: "0", yyout, value);
            fprintf (yyout, "\tcPar *p = new cPar(\"%s error\");\n",cname);
            fprintf (yyout, "\t*p = %s;\n",value);
            fprintf (yyout, "\treturn p;\n");
            fprintf (yyout, "}\n\n");
        }

        if (datarate)
        {
            fprintf (yyout, "static cPar *%s__datarate()\n", cname);
            fprintf (yyout, "{\n");
            print_temp_vars( yyout );
            get_expression(datarate ? datarate : "0", yyout, value);
            fprintf (yyout, "\tcPar *p = new cPar(\"%s datarate\");\n",cname);
            fprintf (yyout, "\t*p = %s;\n",value);
            fprintf (yyout, "\treturn p;\n");
            fprintf (yyout, "}\n\n");
        }

        fprintf(yyout, "Define_Link( %s, %s%s, %s%s, %s%s);\n\n",
                cname,
                (delay ?    cname:"NULL"),  (delay    ? "__delay"   :""),
                (error ?    cname:"NULL"),  (error    ? "__error"   :""),
                (datarate ? cname:"NULL"),  (datarate ? "__datarate":"")
               );

        jar_free (cname);
        jar_free (delay);
        jar_free (error);
        jar_free (datarate);
}

void do_system (char *stname )
{
        if (firstpass)
        {
                /* jar_free (stname); */
                  /* a call to do_systemmodule() from the same ebnf rule
                   *  will do it  --VA
                   */
                return;
        }

        fprintf (yyout, "Define_Network( %s, %s_setup);\n\n",
                         stname, stname);

        fprintf (yyout, "void %s_setup()\n", stname);
        fprintf (yyout, "{\n\n");

        print_temp_vars (yyout);

        jar_strcpy (submodule_name, stname);
        sprintf(submodule_var, "%s_mod", stname);

        tmp_open ();

        cmd_new (NULL,0);  /* treat system module as a submodule --VA */
        is_system = 1;   /* --VA */

        /*---
           jar_free (stname);
              --- will be done in do_sub_or_sys() --VA */
}

void end_system (void)
{
        if (firstpass)
                return;

        end_submodule ();

        tmp_close ();
        fprintf (yyout, "\tcheck_error(); check_memory();\n");

       /*  --this code was moved into cSimulation::setupNetwork():  --VA
        * print_remark (yyout, "match gate pairs on different machines");
        * fprintf (yyout, "\tif (simulation.netInterface()!=NULL)\n"
        *                "\t\tsimulation.netInterface()->setup_connections();\n");
        */
        fprintf (yyout, "}\n\n");

        is_system = 0;   /* --VA */
}

/*------------------------------------------------------------*/

void open_yyin (char *namestr)
{
        int i;
        name_type path;

        /* try to open 'namestr' */
        yyin = fopen (namestr, "r");
        if (yyin)
                return;

        /* try 'namestr' with all include paths as prefixes */
        for (i = 1; i <= nl_count (path_list); i++)
        {
                nl_get (path_list, i, path);
                jar_strcat (path, namestr);
                yyin = fopen (path, "r");
                if (yyin)
                        return;
        }
}

int do_firstpass (char *root_fname)
{
        int i;
        int perr;
        name_type ned_fname;

        mdl_init ();
        cmd_init ();
        nl_add (&include_list, root_fname);

        for( i=1; i<=nl_count(include_list); i++)
        {
            /* note: include list will grow as 'include' directives are
             * processed in input files
             */

            /* open include file i */
            nl_get (include_list, i, fname);
            sprintf (ned_fname, "%s.ned", fname);

            strcpy(current_fname, ned_fname); /*global var*/

            if (i == 1)
                yyin = fopen (ned_fname, "r");  /* main file */
            else
                open_yyin (ned_fname); /* include file: use include paths */

            yyout = stdout;  /* in fact, jar won't output much */

            if (verbose) printf ("%s - 1st pass\n", ned_fname);
            if (yyin == NULL)
            {
                fprintf(stderr, "Can't open file: %s\n", ned_fname);
                return 1;
            }

            /* actual processing */
            firstpass = 1;
            perr = runparse ();
            fclose (yyin);

            /* error handling */
            if (perr)
            {
                fprintf(stderr, "Error(s) while processing file %s\n",ned_fname);
                return perr;
            }
        }
        return 0;
}

int do_secondpass (char *root_fname)
{
        int i;
        int perr;
        name_type ned_fname, cc_fname;

        /* create output file name */
        sprintf (cc_fname, "%s%s", root_fname, suffix);
        if (verbose) printf("target file: %s\n", cc_fname);

        /*
         * open .cc file for output
         */
        yyout = fopen (cc_fname, "w");
        if (yyout == NULL)
        {
                fprintf(stderr, "Can't open file for write: %s\n", cc_fname);
                return 1;
        }

        /*
         * compile main file
         */
        print_header (yyout);
        /* Following code would compile each file in include list (in reverse order)
         *  for( i=nl_count(include_list); i>0; i--)
         * Instead, we just compile main file (1st in include list)
         */
        i = 1;
        {
            /* open include file i */
            nl_get (include_list, i, fname);
            sprintf (ned_fname, "%s.ned", fname);

            strcpy(current_fname, ned_fname); /*global var*/

            if (i == 1)
                yyin = fopen (ned_fname, "r");  /* main file */
            else
                open_yyin (ned_fname); /* include file: use include paths */

            if (verbose) printf ("%s - 2nd pass\n", ned_fname);
            if (yyin == NULL)
            {
                fprintf(stderr,"Can't open file: %s\n", ned_fname);
                return 1;
            }

            fprintf(yyout,"//--------------------------------------------\n");
            fprintf(yyout,"// Following code generated from: %s\n", ned_fname);
            fprintf(yyout,"//--------------------------------------------\n\n");

            /* actual processing */
            firstpass = 0;
            perr = runparse ();
            fclose (yyin);

            /* error handling */
            if (perr)
            {
                    fprintf(stderr,"Error %d while compiling file %s.\n", perr,ned_fname);
                    fclose (yyout);
                    remove (cc_fname);
                    return perr;
            }
            if (no_err)
            {
                    fprintf(stderr,"%d errors while compiling file %s.\n", no_err,ned_fname);
                    fclose (yyout);
                    remove (cc_fname);
                    return 1;
            }
        }
        fclose (yyout);

        /*
        * cleanup
        */
        nl_empty (&include_list);
        nl_empty (&channel_list);
        nl_empty (&for_list);
        mdl_empty ();
        cmd_new ("",0);

        return 0;
}

int compilefile (char *root_fname)
{
        int perr;
        name_type root_fname2;

        /* root_fname2: root_fname without .ned extension */
        jar_strcpy( root_fname2, root_fname);
        if (jar_strcmp(".ned", root_fname2+jar_strlen(root_fname2)-4) == 0)
                root_fname2[ jar_strlen(root_fname2) - 4 ] = 0;

        /*
         * first pass: build include file list and collect declarations
         */
        perr = do_firstpass( root_fname2 );
        if (perr)  return perr;

        /*
         * second pass: generate code
         */
        return do_secondpass( root_fname2 );
}

int main (int argc, char *argv [])
{
        int i;
        int perr, nexterr;

        nl_init (&include_list);
        nl_init (&channel_list);
        nl_init (&for_list);
        nl_init (&machine_list);
        nl_init (&path_list);

        perr = 0;
        if (argc < 2)
        {
                /*
                * print help text
                */
                printf( "NEDC " NEDC_VERSION " - part of OMNeT++. (c) 1992-2001 Andras Varga, TU Budapest\n"
                        "See the license for distribution terms and warranty disclaimer.\n"
                        "\n"
                        "Network Description Compiler.\n"
                        "Usage: nedc [-v] [-I <dir> -I ...] [-s <suffix>] <nedfile1> <nedfile2> ...\n"
                        "  -v          verbose\n"
                        "  -I <dir>    add directory to include path\n"
                        "  -s <suffix> output file suffix (defaults to: _n.cc)\n"
                        "\n"
                      );
        }
        else
        {
                /*
                *   process command line switches
                *   call compilefile() for ned file args
                */
                verbose = 0;
                for (i = 1; i < argc; i++)
                {
                        if (jar_strcmp (argv [i], "-v") == 0)
                        {
                                verbose = 1;
                        }
                        else if (jar_strcmp (argv [i], "-I") == 0)
                        {
                                i++;
                                if (i < argc)
                                {
                                        jar_strcpy (fname, argv [i]);
                                        jar_strcat (fname, SLASH);
                                        nl_add (&path_list, fname);
                                }
                        }
                        else if (argv[i][0]=='-' && toupper(argv[i][1])=='I')
                        {
                                jar_strcpy (fname, argv[i]+2);
                                jar_strcat (fname, SLASH);
                                nl_add (&path_list, fname);
                        }
                        else if (jar_strcmp (argv [i], "-s") == 0)
                        {
                                i++;
                                if (i < argc)
                                {
                                        strncpy(suffix, argv[i], 32);
                                        suffix[31]='\0';
                                }
                        }
                        else if (argv[i][0]=='-' && toupper(argv[i][1])=='s')
                        {
                                strncpy(suffix, argv[i]+2, 32);
                                suffix[31]='\0';
                        }
                        else
                        {
                                nexterr = compilefile( argv[i] );
                                if (nexterr)
                                        perr = nexterr;
                        }
                }
                remove (TMP_FNAME);
        }
        return perr;
}

