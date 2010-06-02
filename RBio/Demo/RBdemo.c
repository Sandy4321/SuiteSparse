/* ========================================================================== */
/* === RBio/Demo/RBdemo.c: C demo =========================================== */
/* ========================================================================== */

/* Copyright 2009, Timothy A. Davis, All Rights Reserved.
   Refer to RBio/Doc/license.txt for the RBio license. */

/* This program reads a Rutherford/Boeing file from stdin and uses it
   to test the RBio C-callable functions. */

#include "RBio.h"
#define Int UF_long
#define ID "%ld"

#define SLEN 4096

int main (int argc, char **argv)
{
    double xr, xz, xmin, xmax ;
    double *Ax, *Az ;
    Int nrow, ncol, mkind, skind, *Ap, *Ai, i, *Zp, *Zi, asize, mkind2, skind2,
        znz, j, p, status, njumbled, nzeros, build_upper, zero_handling, fem,
        xsize, nelnz, nnz ;
    char title [73], key [9], mtype [4], mtype2 [4], *filename ;

    UFconfig config ;
    config.malloc_memory = malloc ;
    config.free_memory = free ;

    /* read from stdin, or from file given by first argument */
    filename = (argc > 1) ? argv [1] : NULL ;

    title [0] = '\0' ;
    build_upper = 1 ;
    zero_handling = 2 ;     /* extract zeros */
    status = RBread (filename, build_upper, zero_handling, title, key, mtype,
        &nrow, &ncol, &mkind, &skind, &asize, &znz,
        &Ap, &Ai, &Ax, &Az, &Zp, &Zi, &config) ;

    printf ("\n===========================================================\n") ;
    printf ("title: [%s]\n", title) ;
    printf ("status " ID "\n", status) ;
    if (status != RBIO_OK)
    {
        printf ("RBread Error! "ID"\n", status) ;
        UFfree (Ap, &config) ;
        UFfree (Ai, &config) ;
        UFfree (Ax, &config) ;
        UFfree (Az, &config) ;
        UFfree (Zp, &config) ;
        UFfree (Zi, &config) ;

        if (filename != NULL)
        {
            status = RBreadraw (filename, title, key, mtype, &nrow, &ncol, &nnz,
                &nelnz, &mkind, &skind, &fem, &xsize, &Ap, &Ai, &Ax, NULL) ;
            if (status != RBIO_OK)
            {
                printf ("RBreadraw error! status "ID"\n", status) ;
                return (1) ;
            }
            printf ("raw nrow "ID" ncol "ID" nnz "ID" nelnz "ID" xsize "ID"\n",
                nrow, ncol, nnz, nelnz, xsize) ;
            UFfree (Ap, &config) ;
            UFfree (Ai, &config) ;
            UFfree (Ax, &config) ;
            return (0) ;
        }
        return (1) ;
    }

    printf ("key [%s] nrow "ID" ncol "ID "\n", key, nrow, ncol) ;
    printf ("mkind "ID" skind "ID" asize "ID" znz "ID"\n",
        mkind, skind, asize, znz) ;
    printf ("mtype from file:               [%s]\n", mtype) ;

    if (Ap [ncol] < 20)
    {
        for (j = 0 ; j < ncol ; j++)
        {
            printf ("\ncolumn "ID":\n", j) ;
            for (p = Ap [j] ; p < Ap [j+1] ; p++)
            {
                i = Ai [p] ;
                RBget_entry (mkind, Ax, Az, p, &xr, &xz) ;
                printf ("  "ID" : %g", i, xr) ;
                if (xz != 0) printf (" + (%g)i\n", xz) ;
                printf ("\n") ;
            }
        }
    }

    status = RBok (nrow, ncol, asize, Ap, Ai, Ax, Az, NULL, mkind,
        &njumbled, &nzeros);
    printf ("(A) RBok status "ID" "ID" "ID"\n", status, njumbled, nzeros) ;
    if (status != RBIO_OK || njumbled != 0 || nzeros != 0)
    {
        printf ("RBok Error!\n") ;
        return (1) ;
    }

    status = RBok (nrow, ncol, asize, Zp, Zi, NULL, NULL, NULL, 1,
        &njumbled, &nzeros) ;
    printf ("(Z) RBok status "ID" "ID" "ID"\n", status, njumbled, nzeros) ;
    if (status != RBIO_OK || njumbled != 0 || nzeros != 0)
    {
        printf ("RBok Error (Z)!\n") ;
        return (1) ;
    }

    status = RBwrite ("temp.rb", title, key, nrow, ncol, Ap, Ai, Ax, Az, Zp, Zi,
        mkind, mtype2, NULL) ;
    if (status != RBIO_OK)
    {
        printf ("RBwrite error!\n") ;
    }

    status = RBkind (nrow, ncol, Ap, Ai, Ax, Az, mkind, &mkind2, &skind2,
        mtype2, &xmin, &xmax, NULL, NULL) ;
    if (status != RBIO_OK)
    {
        printf ("RBkind error!\n") ;
        return (1) ;
    }

    printf ("mtype from examining matrix:   [%s]\n", mtype2) ;

    UFfree (Ap, &config) ;
    UFfree (Ai, &config) ;
    UFfree (Ax, &config) ;
    UFfree (Az, &config) ;
    UFfree (Zp, &config) ;
    UFfree (Zi, &config) ;

    zero_handling = 1 ;     /* prune zeros */
    status = RBread ("temp.rb", build_upper, zero_handling, title, key, mtype,
        &nrow, &ncol, &mkind, &skind, &asize, &znz,
        &Ap, &Ai, &Ax, NULL, NULL, NULL, &config) ;
    if (status != 0)
    {
        UFfree (Ap, &config) ;
        UFfree (Ai, &config) ;
        UFfree (Ax, &config) ;
        printf ("RBread Error! "ID"\n", status) ;
        return (1) ;
    }

    status = RBkind (nrow, ncol, Ap, Ai, Ax, NULL, mkind, &mkind2, &skind2,
        mtype2, &xmin, &xmax, NULL, NULL) ;
    if (status != RBIO_OK)
    {
        printf ("RBkind error!\n") ;
    }
    printf ("mtype after reading from file: [%s]\n", mtype2) ;

    UFfree (Ap, NULL) ;
    UFfree (Ai, NULL) ;
    UFfree (Ax, NULL) ;

    status = RBreadraw ("temp.rb", title, key, mtype, &nrow, &ncol, &nnz,
        &nelnz, &mkind, &skind, &fem, &xsize, &Ap, &Ai, &Ax, NULL) ;
    if (status != RBIO_OK)
    {
        printf ("RBreadraw error!\n") ;
        return (1) ;
    }
    printf ("raw nrow "ID" ncol "ID" nnz "ID" nelnz "ID" xsize "ID"\n",
        nrow, ncol, nnz, nelnz, xsize) ;

    if (fem)
    {
        printf ("FEM matrix\n") ;
    }
    else
    {
        status = RBkind (nrow, ncol, Ap, Ai, Ax, NULL, mkind, &mkind2, &skind2,
            mtype2, &xmin, &xmax, NULL, NULL) ;
        if (status != RBIO_OK)
        {
            printf ("RBkind error!\n") ;
        }
        printf ("mtype from raw read from file: [%s]\n", mtype2) ;
    }

    UFfree (Ap, NULL) ;
    UFfree (Ai, NULL) ;
    UFfree (Ax, NULL) ;

    /* read in just the pattern */
    status = RBread ("temp.rb", build_upper, zero_handling, title, key, mtype,
        &nrow, &ncol, &mkind, &skind, &asize, &znz,
        &Ap, &Ai, NULL, NULL, NULL, NULL, &config) ;

    status = RBok (nrow, ncol, asize, Ap, Ai, Ax, Az, NULL, mkind,
        &njumbled, &nzeros);
    printf ("(P) RBok status "ID" "ID" "ID"\n", status, njumbled, nzeros) ;
    if (status != RBIO_OK || njumbled != 0 || nzeros != 0)
    {
        printf ("RBok Error!\n") ;
        return (1) ;
    }

    status = RBkind (nrow, ncol, Ap, Ai, NULL, NULL, mkind, &mkind2, &skind2,
        mtype2, &xmin, &xmax, NULL, NULL) ;
    if (status != RBIO_OK)
    {
        printf ("RBkind error!\n") ;
        return (1) ;
    }
    printf ("mtype of pattern:              [%s]\n", mtype2) ;


    UFfree (Ap, NULL) ;
    UFfree (Ai, NULL) ;

    /* read in just the lower triangular part for symmetric case */
    build_upper = 0 ;
    zero_handling = 0 ;     /* ignore zeros */
    status = RBread ("temp.rb", build_upper, zero_handling, title, key, mtype,
        &nrow, &ncol, &mkind, &skind, &asize, &znz,
        &Ap, &Ai, &Ax, NULL, NULL, NULL, &config) ;
    if (status != RBIO_OK)
    {
        printf ("RBread error! "ID"\n", status) ;
        return (1) ;
    }

    status = RBkind (nrow, ncol, Ap, Ai, Ax, NULL, mkind, &mkind2, &skind2,
        mtype2, &xmin, &xmax, NULL, NULL) ;
    if (status != RBIO_OK)
    {
        printf ("RBkind error!\n") ;
        return (1) ;
    }
    printf ("mtype of tril(A):              [%s]\n", mtype2) ;

    status = RBok (nrow, ncol, asize, Ap, Ai, Ax, NULL, NULL, mkind,
        &njumbled, &nzeros);
    printf ("(L) RBok status "ID" "ID" "ID"\n", status, njumbled, nzeros) ;
    if (status != RBIO_OK || njumbled != 0)
    {
        printf ("RBok Error!\n") ;
        return (1) ;
    }

    UFfree (Ap, NULL) ;
    UFfree (Ai, NULL) ;
    UFfree (Ax, NULL) ;

    printf ("RBdemo OK\n") ;
    return (0) ;
}
