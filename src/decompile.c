/*
===========================================================================
Copyright (C) 1996-2002, Valve LLC. All rights reserved.
Copyright (C) 2023 Toodles

This product contains software technology licensed from Id
Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
All Rights Reserved.

Use, distribution, and modification of this source code and/or resulting
object code is restricted to non-commercial enhancements to products from
Valve LLC.  All other use, distribution, or modification is prohibited
without written permission from Valve LLC.
===========================================================================
*/

#include "pch.h"

bool g_dragdrop_pause = false;

void decomp_mdl (
    const char *mdlname,
    const char *qcname,
    const char *cd,
    const char *cdtexture,
    const char *cdanim,
    const char *qcdir,
    const char *smddir);

void decomp_spr (
    const char *sprname,
    const char *qcname,
    const char *cd,
    const char *qcdir,
    const char *bmpdir);

void decomp_wad (
    const char *wadname,
    const char *bmpdir,
    const char *pattern);

void decomp_bsptex (
    const char *bspname,
    const char *bmpdir,
    const char *pattern);

void info_mdl (
    const char *mdlname,
    const char *args);

void decomp_pause (void)
{
    if (!g_dragdrop_pause)
        return;
    fprintf (stdout, "\nPress Enter to exit...");
    fflush (stdout);
    getchar ();
}

static void print_help (void)
{
    fprintf (stdout, "Usage: mdldec [options...] <input {*.mdl | *.spr | *.wad | *.bsp}>... [<output {directory | *.qc}>]\n\n");
    fprintf (stdout, "Drag-and-drop: drop one or more model files onto mdldec.exe.\n");
    fprintf (stdout, "Output is written next to each input file.\n\n");
    fprintf (stdout, "Options:\n");
    fprintf (stdout, "\t-help\t\t\tDisplay this message and exit.\n\n");

    fprintf (stdout,
"\t-cd <path>\t\tSets the data path. Defaults to \".\".\n\
\t\t\t\tIf set, data will be placed relative to root path.\n\n");

    fprintf (stdout,
"\t-cdtexture <path>\tSets the texture path, relative to data path.\n\
\t\t\t\tDefaults to \"./maps_8bit\" for models, and \"./bmp\"\n\
\t\t\t\tfor sprites, WADs, and BSPs.\n\n");

    fprintf (stdout,
"\t-cdanim <path>\t\tSets the animation path, relative to data path.\n\
\t\t\t\tDefaults to \"./anims\".\n\n");

    fprintf (stdout,
"\t-pattern <string>\tIf set, only textures containing the matching\n\
\t\t\t\tsubstring will be extracted from WADs and BSPs.\n\n");

    fprintf (stdout,
"\t-info [<string>]\tFile info will be printed. No decompiling will occur.\n\
\n\
\t\t\t\tA comma separated list of following arguments may be\n\
\t\t\t\tadded to print extra info: \"acts\" \"events\" \"bodygroups\"\n\
\n\
\t\t\t\tIf the input file is a WAD or BSP, the optional string\n\
\t\t\t\twill instead act identically to the \"-pattern\" option.\n\n");

    fprintf (stdout,
"\t-pause\t\t\tAlways pause before exit (for drag-and-drop).\n\n");

    fprintf (stdout,
"\t-nopause\t\tNever pause before exit (for scripts).\n\n");
}

static bool is_input_ext (const char *path)
{
    char *name, *ext;
    filebase ((char *)path, &name, &ext);
    return !strcasecmp (ext, ".mdl")
        || !strcasecmp (ext, ".spr")
        || !strcasecmp (ext, ".wad")
        || !strcasecmp (ext, ".bsp");
}

static int getargs (
    int argc,
    char **argv,
    char **cd,
    bool *havecd,
    char **cdtexture,
    char **cdanim,
    char **wadpattern,
    bool *info_mode,
    char **info_args,
    bool *force_pause,
    bool *force_nopause)
{
    if (argc < 2)
    {
        print_help ();
        decomp_pause ();
        exit (0);
    }

    int i;

    *info_mode = false;
    *info_args = NULL;

    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            break;

        if (!strcmp (argv[i], "-help"))
        {
            print_help ();
            exit (0);
        }
        else if (!strcmp (argv[i], "-info"))
        {
            *info_mode = true;
            /* Optional comma list immediately after -info. */
            if (i + 1 < argc && argv[i + 1][0] != '-'
                && !is_input_ext (argv[i + 1]))
            {
                *info_args = argv[i + 1];
                ++i;
            }
        }
        else if (!strcmp (argv[i], "-cd"))
        {
            *cd = argv[i + 1];
            *havecd = true;
            fprintf (stdout, "Data path set to: \"%s\"\n", *cd);
            ++i;
        }
        else if (!strcmp (argv[i], "-cdtexture"))
        {
            *cdtexture = argv[i + 1];
            fprintf (stdout, "Texture path set to: \"%s\"\n", *cdtexture);
            ++i;
        }
        else if (!strcmp (argv[i], "-cdanim"))
        {
            *cdanim = argv[i + 1];
            fprintf (stdout, "Animation path set to: \"%s\"\n", *cdanim);
            ++i;
        }
        else if (!strcmp (argv[i], "-pattern"))
        {
            *wadpattern = argv[i + 1];
            fprintf (stdout, "WAD search pattern set to: \"%s\"\n", *wadpattern);
            ++i;
        }
        else if (!strcmp (argv[i], "-pause"))
        {
            *force_pause = true;
        }
        else if (!strcmp (argv[i], "-nopause"))
        {
            *force_nopause = true;
        }
        else
        {
            fprintf (stdout, "Unknown option: \"%s\"\n", argv[i]);
        }
    }

    if (i >= argc)
    {
        error (1, "No input file provided\n");
    }

    return i;
}

/* Directory portion of input path (without trailing slash).
   Returns heap string. "." when input has no directory. */
static char *input_dirname (const char *in)
{
    const char *last = NULL;
    const char *c = in;
    while (*c)
    {
        if (*c == '/' || *c == '\\')
            last = c;
        ++c;
    }
    if (!last)
        return strdup (".");
    if (last == in)
        return strdup ("\\");
    {
        size_t len = (size_t)(last - in);
        char *dir = (char *)memalloc (len + 1, 1);
        memcpy (dir, in, len);
        dir[len] = '\0';
        return dir;
    }
}

static void getdirs (
    char *in,
    char *out,
    char **qcdir,
    char **qcname,
    bool havecd)
{
    if (havecd)
    {
        *qcdir = ".";
    }

    if (!out) /* Put files in sub directory NEXT TO input (drag-drop safe). */
    {
        *qcname = strdup (skippath (in));
        stripext (*qcname);

        if (!havecd)
        {
            char *indir = input_dirname (in);
            if (!strcmp (indir, "."))
            {
                *qcdir = strdup (*qcname);
            }
            else
            {
                *qcdir = appenddir (indir, *qcname);
            }
            free (indir);
        }
        return;
    }

    char *name, *ext;

    filebase (out, &name, &ext);

    if (*ext) /* QC name provided. Put files in root directory. */
    {
        *qcname = strdup (out);
        stripext (*qcname);

        if (!havecd)
        {
            *qcdir = strdup (out);
            stripfilename (*qcdir);
        }
    }
    else /* Put files in sub directory. */
    {
        *qcname = strdup (skippath (in));
        stripext (*qcname);

        if (!havecd)
        {
            *qcdir = appenddir (out, *qcname);
        }
    }
}

static void decompile_one (
    char *in,
    char *out,
    char *cd,
    bool havecd,
    char *cdtexture_default,
    char *cdanim_default,
    char *wadpattern)
{
    char *qcdir, *qcname;

    getdirs (in, out, &qcdir, &qcname, havecd);

    char *smddir = appenddir (qcdir, cd);

    char *name, *ext;
    filebase (in, &name, &ext);
    if (!strcasecmp (ext, ".spr"))
    {
        char *cdtexture = cdtexture_default ? cdtexture_default : "./bmp";
        char *bmpdir = appenddir (qcdir, cdtexture);
        decomp_spr (in, skippath (qcname), cdtexture, qcdir, bmpdir);
        free (bmpdir);
    }
    else if (!strcasecmp (ext, ".wad"))
    {
        char *cdtexture = cdtexture_default ? cdtexture_default : "./bmp";
        char *bmpdir = appenddir (qcdir, cdtexture);
        decomp_wad (in, bmpdir, wadpattern);
        free (bmpdir);
    }
    else if (!strcasecmp (ext, ".bsp"))
    {
        char *cdtexture = cdtexture_default ? cdtexture_default : "./bmp";
        char *bmpdir = appenddir (qcdir, cdtexture);
        decomp_bsptex (in, bmpdir, wadpattern);
        free (bmpdir);
    }
    else
    {
        char *cdtexture = cdtexture_default ? cdtexture_default : "./maps_8bit";
        char *cdanim = cdanim_default ? cdanim_default : "./anims";
        decomp_mdl (in, skippath (qcname), cd, cdtexture, cdanim, qcdir, smddir);
    }

    free (smddir);
    free (qcname);

    if (!havecd)
    {
        free (qcdir);
    }
}

int main (int argc, char **argv)
{
    char *cd = ".";
    bool havecd = false;
    char *cdtexture = NULL;
    char *cdanim = NULL;
    char *wadpattern = NULL;
    bool info_mode = false;
    char *info_args = NULL;
    bool force_pause = false;
    bool force_nopause = false;

    int i = getargs (argc, argv, &cd, &havecd, &cdtexture, &cdanim,
        &wadpattern, &info_mode, &info_args, &force_pause, &force_nopause);

    int ninputs = argc - i;

    /* Drag-and-drop detection: no options before first file. */
    if (i == 1)
        g_dragdrop_pause = true;
    if (force_pause)
        g_dragdrop_pause = true;
    if (force_nopause)
        g_dragdrop_pause = false;

    /* -info: apply to every input file. */
    if (info_mode)
    {
        for (int k = i; k < argc; ++k)
        {
            /* Skip a stray output-looking arg only in classic 2-arg form. */
            if (ninputs == 2 && k == i + 1 && !is_input_ext (argv[k]))
                break;
            info_mdl (argv[k], info_args ? info_args : wadpattern);
        }
        decomp_pause ();
        return 0;
    }

    /* Decide between classic <in> [out] and multi-file drag-drop. */
    if (ninputs == 1)
    {
        decompile_one (argv[i], NULL, cd, havecd, cdtexture, cdanim, wadpattern);
    }
    else if (ninputs == 2 && !is_input_ext (argv[i + 1]))
    {
        /* Classic: second arg is output dir or .qc file. */
        decompile_one (argv[i], argv[i + 1], cd, havecd, cdtexture, cdanim, wadpattern);
    }
    else
    {
        /* Multi-file drag-and-drop: every arg is an input. */
        fprintf (stdout, "Decompiling %d files...\n", ninputs);
        for (int k = i; k < argc; ++k)
        {
            fprintf (stdout, "\n--- [%d/%d] \"%s\" ---\n", k - i + 1, ninputs, argv[k]);
            decompile_one (argv[k], NULL, cd, havecd, cdtexture, cdanim, wadpattern);
        }
    }

    fprintf (stdout, "\nDone.\n");
    decomp_pause ();

    return 0;
}
