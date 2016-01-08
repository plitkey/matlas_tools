/*
===============================================================================

  FILE:  mat2las.cpp

  CONTENTS:

    Matlab mex gateway function for using LASlib writer from Matlab

  PROGRAMMERS:

    paula.litkey@fgi.fi - www.fgi.fi

  COPYRIGHT:

    (c) 2014, Finnish Geodetic Institute

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    17 March 2014 -- First public matlas_tools version

===============================================================================
*/


#include "mex.h"
#include <time.h>
#include <math.h>
#include "laswriter.hpp"
//#include "laswaveform13reader.hpp"
#include "lasdefinitions.hpp"
#include "mex_lasz_fun_fgi.hpp"
#include "mex_lasz_fun_fgi.cpp"


void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    char *fname=NULL;
    int next = 0;
    LASpoint *point;
    LASheader *header = 0;
    mwSize    nc, nr;
    int       nfields;
    int argc = 0;
    int last = 1;
    char **argv;
    char *parse_string;
    const mxArray *field_array_ptr;
    const mxArray *ea_array_ptr;
    BOOL header_provided = false;
    I64 offset[3];
    I64 orig_x_offset, orig_y_offset, orig_z_offset;
    fgi_options* fgiopts;
    fgiopts = new fgi_options();
    
    
    if (nrhs == 0) {
        mexPrintf ("usage: mat2las(data_struct,parsestring)\n");
        return;
    }
    if (!mxIsStruct(prhs[0])) mexErrMsgTxt ("First arg must contain Matlab data structure\n");
    if (mxIsStruct(prhs[1])) {
        last++;
        header_provided = true;
    }
    if (!mxIsChar(prhs[last])) mexErrMsgTxt ("Last arg must contain the parse string for lastools.\n");
    if (nlhs !=0) mexErrMsgTxt ("No outputs needed.\n");
    
    parse_string = mxArrayToString(prhs[last]);
    get_argc(argc,parse_string);
    argv = (char **) mxCalloc(argc+1, sizeof(char *)); // Add one to 'argc' for command line compatibility
    
    set_argv(argv,parse_string);
    mxFree(parse_string);
    argc++;
    nfields = mxGetNumberOfFields(prhs[0]);
    fgiparse(argc, argv, fgiopts);
    
    header = new LASheader;
    header->clean();
    point = new LASpoint;
    if (header_provided)    {
        read_mex_hdr(header,point,prhs[1],prhs[0]);
     }
    else {
        read_mex(header,point,prhs[0]);
    }
    
    if (fgiopts->set_offset) {
        mexPrintf("Setting offset\n");
        header->x_offset = fgiopts->x_offset;
        header->y_offset = fgiopts->y_offset;
        header->z_offset = fgiopts->z_offset;        
    }
    if (fgiopts->set_scale)
    {
        mexPrintf("Setting scale\n");
        header->x_scale_factor = fgiopts->x_scale_factor;
        header->y_scale_factor = fgiopts->y_scale_factor;
        header->z_scale_factor = fgiopts->z_scale_factor;
        if (!fgiopts->set_offset) {
            if (F64_IS_FINITE(header->min_x) && F64_IS_FINITE(header->max_x))
                offset[0] = ((I64)((header->min_x + header->max_x)/header->x_scale_factor/20000000))*10000000*header->x_scale_factor;
            else
                offset[0] = 0;
            
            if (F64_IS_FINITE(header->min_y) && F64_IS_FINITE(header->max_y))
                offset[1] = ((I64)((header->min_y + header->max_y)/header->y_scale_factor/20000000))*10000000*header->y_scale_factor;
            else
                offset[1] = 0;
            
            if (F64_IS_FINITE(header->min_z) && F64_IS_FINITE(header->max_z))
                offset[2] = ((I64)((header->min_z + header->max_z)/header->z_scale_factor/20000000))*10000000*header->z_scale_factor;
            else
                offset[2] = 0;
            orig_x_offset = header->x_offset;
            orig_y_offset = header->y_offset;
            orig_z_offset = header->z_offset;
            if (header->x_offset != offset[0])
            {
                header->x_offset = offset[0];
            }
            if (header->y_offset != offset[1])
            {
                header->y_offset = offset[1];
            }
            if (header->z_offset != offset[2])
            {
                header->z_offset = offset[2];
            }
        }
    }

    if (!header->check()) mexErrMsgTxt("header not valid!!\n");
    LASwriteOpener laswriteopener;
    if (!laswriteopener.parse(argc, argv)) mexErrMsgTxt("laswriteopener parse error!!\n");
    LASwriter* laswriter = laswriteopener.open(header);
    if (laswriter == 0) mexErrMsgTxt("ERROR: could not open laswriter\n");
    if (!header_provided) {
        if (!laswriter->update_header(header, false, true) ) {
            //use_inventory = false, update_extra_bytes = true
            mexWarnMsgTxt("Writer header update not succesfull\n");
        }
    }
    field_array_ptr = mxGetFieldByNumber(prhs[0], 0, 0); // pointer to struct mxArray holding points
    nr = mxGetM(field_array_ptr);
    nc = mxGetN(field_array_ptr);
    mexPrintf("Looping %d points\n",nr);
    while (next < nr) 
    {
        copy_point_arr_mex(header,point,prhs[0],next);
        next++;
            laswriter->update_inventory(point);
        laswriter->write_point(point);
    }
        laswriter->update_header(header,1,1);
    laswriter->close();
    delete laswriter;
    delete point;
    point = 0;
    delete fgiopts;
    mxFree(argv);    
}


