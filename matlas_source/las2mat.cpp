/*
===============================================================================

  FILE:  mat2las.cpp

  CONTENTS:

    Matlab mex gateway function for using LASlib writer from Matlab
    (test.laz can be found in lastools/data/)
    >> [hdr,str] = las2mat('-i test.laz')
    >> parsestring = '-i test.laz -clip 2483789 366396 2483809 366416 -keep_class 2 ';
    >> [hdr,str] = las2mat(parsestring)


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

    18 March 2014 -- First public matlas_tools version

===============================================================================
*/

#include "mex.h"
//#include <time.h>
#include <math.h>
#include "lasreader.hpp"
#include "lasdefinitions.hpp"
#include "lasutility.hpp"
#include "mex_lasz_fun_fgi.hpp" 
#include "mex_lasz_fun_fgi.cpp" 


void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    int next = 0, Nextra = 0, extranext = 0;
    
    int NUMBER_OF_HDRFIELDS = 33;
    int NUMBER_OF_PNTFIELDS = 12;
    I32 attribute_array_offsets[10];
    LASpoint *point;
    mxArray *tmpD_1, *tmpD_2, *tmpD_3, *tmpD_4, *tmpD_5, *tmpD_6;
    mxArray *tmpD_7, *tmpD_8, *tmpD_9, *tmpD_10, *tmpD_11, *tmpD_12;
    mxArray *tmpD_13, *tmpD_14, *tmpD_15;
    mxArray *tmpI32; 
    double *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9, *p10;
    double *p11, *p12, *p13, *p14;
    double *pe1;
    int argc = 0;
    char **argv;
    char *parse_string;
    fgi_options* fgiopts;
    fgiopts = new fgi_options();
    
    if (nrhs == 0) {
        mexPrintf ("usage: las2mat(lastools parsestring)\n");
        return;
    }
    if (!mxIsChar(prhs[0])) mexErrMsgTxt ("first arg must contain the parse string for lastools.\n");
    if (nlhs !=2) mexErrMsgTxt ("Two outputs needed.( header and data struct )\n");
    
    /* copy the string data from prhs[2] into a C string "parse_string".    */
    parse_string = mxArrayToString(prhs[0]);
    get_argc(argc,parse_string);
    argv = (char **) mxCalloc(argc+1, sizeof(char *)); // Add one to 'argc' for command line compatibility
    
    // Fill argv
    set_argv(argv,parse_string);
    mxFree(parse_string);
    argc++;
    fgiparse(argc, argv, fgiopts);
    LASreadOpener lasreadopener;
    if (!fgiopts->header && !fgiopts->populate)
    {
        mexPrintf("no header detected!\n");
        lasreadopener.set_populate_header(true);
    }
    if (!lasreadopener.parse(argc, argv)) mexErrMsgTxt("lasreadopener parse error!!\n");
    LASreader *lasreader = lasreadopener.open();
    if (!lasreader) mexErrMsgTxt("LASREADER Error! could not open lasreader!");
    lasreader->header.clean_laszip(); // re-created if needed in write 

    if (fgiopts->extra_pass)
    {
        // extra pass to precompute the header information
        // store the inventory for the header
        LASinventory lasinventory;        
        while (lasreader->read_point())  lasinventory.add(&lasreader->point);
        
        lasreader->close();
        if (!lasreadopener.reopen(lasreader))
        {
            mexPrintf("ERROR: could not reopen '%s' for main pass\n", lasreadopener.get_file_name());
            mexErrMsgTxt("LASREADER: could not reopen");
        }

        lasreader->header.number_of_point_records = lasinventory.number_of_point_records;
        for (int i = 0; i < 5; i++) lasreader->header.number_of_points_by_return[i] = lasinventory.number_of_points_by_return[i+1];
        lasreader->header.max_x = lasreader->header.get_x(lasinventory.max_X);
        lasreader->header.min_x = lasreader->header.get_x(lasinventory.min_X);
        lasreader->header.max_y = lasreader->header.get_y(lasinventory.max_Y);
        lasreader->header.min_y = lasreader->header.get_y(lasinventory.min_Y);
        lasreader->header.max_z = lasreader->header.get_z(lasinventory.max_Z);
        lasreader->header.min_z = lasreader->header.get_z(lasinventory.min_Z);
    }

    delete fgiopts;
    LASheader *header = &(lasreader->header);
    if (!header) mexErrMsgTxt("LASREADER: Unable to fetch header for file");
    header->clean_laszip();
    plhs[0] = mxCreateStructMatrix(1, 1, NUMBER_OF_HDRFIELDS, hdr_field_names);
    get_header_fields(header, plhs[0]);
    plhs[1] = mxCreateStructMatrix(1, 1, NUMBER_OF_PNTFIELDS, pnt_field_names);
    
    point = new LASpoint;
    point->init(&lasreader->header, lasreader->header.point_data_format, lasreader->header.point_data_record_length);
    tmpD_1 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_2 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_3 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_4 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_5 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_6 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_7 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_8 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_9 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_10 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_11 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    tmpD_12 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    
    p1 = mxGetPr(tmpD_1);
    p2 = mxGetPr(tmpD_2);
    p3 = mxGetPr(tmpD_3);
    p4 = mxGetPr(tmpD_4);
    p5 = mxGetPr(tmpD_5);
    p6 = mxGetPr(tmpD_6);
    p7 = mxGetPr(tmpD_7);
    p8 = mxGetPr(tmpD_8);
    p9 = mxGetPr(tmpD_9);
    p10 = mxGetPr(tmpD_10);
    p11 = mxGetPr(tmpD_11);
    p12 = mxGetPr(tmpD_12);
    
    mxSetField(plhs[1], 0, "x", tmpD_1);
    mxSetField(plhs[1], 0, "y", tmpD_2);
    mxSetField(plhs[1], 0, "z", tmpD_3);
    mxSetField(plhs[1], 0, "intensity", tmpD_4);
    mxSetField(plhs[1], 0, "return_number", tmpD_5);
    mxSetField(plhs[1], 0, "number_of_returns", tmpD_6);
    mxSetField(plhs[1], 0, "scan_direction_flag", tmpD_7);
    mxSetField(plhs[1], 0, "edge_of_flight_line", tmpD_8);
    mxSetField(plhs[1], 0, "classification", tmpD_9);
    mxSetField(plhs[1], 0, "scan_angle_rank", tmpD_10);
    mxSetField(plhs[1], 0, "user_data", tmpD_11);
    mxSetField(plhs[1], 0, "point_source_ID", tmpD_12);
    if (point->have_gps_time) {
        mxAddField(plhs[1],"gps_time");
        tmpD_13 = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
        mxSetField(plhs[1], 0, "gps_time", tmpD_13);
        p13 = mxGetPr(tmpD_13);
    }
    if (point->have_nir && point->have_rgb) {
        mxAddField(plhs[1],"rgb");
        tmpD_14 = mxCreateDoubleMatrix(header->number_of_point_records,4,mxREAL);
        mxSetField(plhs[1], 0, "rgb", tmpD_14);
        p14 = mxGetPr(tmpD_14);
    }
    else if (point->have_rgb){
        mxAddField(plhs[1],"rgb");
        tmpD_14 = mxCreateDoubleMatrix(header->number_of_point_records,3,mxREAL);
        mxSetField(plhs[1], 0, "rgb", tmpD_14);
        p14 = mxGetPr(tmpD_14);
        
    }
    //if (point->have_wavepacket) {
    //    mxAddField(plhs[1],"wavepacket");
    //    tmpwp = mxCreateDoubleMatrix(header->number_of_point_records,1,mxREAL);
    //    mxSetField(plhs[1], 0, "wavepacket", tmpwp);
    //    pwp = mxGetPr(tmpwp);
    //}
    
    if (header->number_attributes >0) { // init variables to read extra attributes
        mxArray *tmpD_15;
        Nextra = header->number_attributes;
        mxAddField(plhs[1],"attributes");
        tmpD_15 = mxCreateDoubleMatrix(header->number_of_point_records,Nextra,mxREAL);
        mxSetField(plhs[1], 0, "attributes", tmpD_15);
        pe1 = mxGetPr(tmpD_15);
        
        mxAddField(plhs[0],"attribute_array_offsets");
        tmpI32 = mxCreateDoubleMatrix(1,header->number_attributes,mxREAL);
        double *pI32 = mxGetPr(tmpI32);
        for (int i=0; i<header->number_attributes; i++) {
            attribute_array_offsets[i] = lasreader->header.get_attribute_start(i);
            *(pI32+i) = lasreader->header.get_attribute_start(i);
        }
        mxSetField(plhs[0], 0, "attribute_array_offsets", tmpI32);
    }
    while (lasreader->read_point())
    {
        
        copy_mex_point_arr(lasreader,next,p1,p2,p3,
                p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14);
        
        //if (point->have_wavepacket) {  
        //}
        
        if (header->number_attributes >0) {
            I32 index;
            for (index=0; index<header->number_attributes; index++) {
                extranext = index*header->number_of_point_records+next;
                copy_attribute(lasreader,pe1,extranext,attribute_array_offsets[index],index);
                if (extranext > Nextra*header->number_of_point_records) {
                    mexErrMsgTxt("Overflow!!");
                }
            }
        }
        next++;
        
    }
    // close the reader
    lasreader->close();
    delete lasreader;
    delete point;
    point = 0;
    mxFree(argv);
}


