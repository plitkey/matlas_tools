/*--------------------------------------------------------------------
/*
 * Write LAStools lasreader header and points from Matlab struct
 * >> writeLASZ_fgi3(str,hdr,parsestring);
 *  
 * 
 * the parsestring contains the name of the output file:
 * >> parsestring = '-o outQ4.las';
 *
 * fgi 2013
 *--------------------------------------------------------------------*/

//pli & ep 2013

#include "mex.h"
//#include "mwsize.h"
#include <time.h>
#include <math.h>
#include "lasreader.hpp"
#include "laswriter.hpp"
//#include "laswaveform13reader.hpp"
//#include "print_hdr.cpp"
#include "lasdefinitions.hpp"
#include "mex_lasz_fun_fgi.hpp"
#include "mex_lasz_fun_fgi.cpp"


void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    char *fname=NULL;
    int next = 0, rgbnext = 0, Nextra = 0, extranext = 0;
    I32 extra_attribute_array_offsets[10];
    LASpoint *point;
    LASheader *header = 0;
    mxArray *tmpD_14, *tmpD_15;
    mwSize M, NStructElems, NStructElems2;
    mwSize    nc, nr;
    int        ifield, nfields, nfields2, nfieldsh;
    int argc = 0;
    int last = 1;
    char **argv;
    char *parse_string;
    size_t buflen;
    int set_gps_time_endcoding = -1;
    const mxArray *field_array_ptr;
    double *maxclass;
    int *tmpint;
    BOOL header_provided = false;
    I64 offset[3];
    I64 orig_x_offset, orig_y_offset, orig_z_offset;
    fgi_options* fgiopts;
    fgiopts = new fgi_options();
    
    maxclass = (double *)mxCalloc(1,sizeof(double));
    if (!maxclass) 
        mexErrMsgTxt("Memory error");
    *maxclass=0;
    
    if (nrhs == 0) {
        mexPrintf ("usage: writeLASZ_hdrstruct(data_struct,parsestring)\n");
        return;
    }
    if (!mxIsStruct(prhs[0])) mexErrMsgTxt ("First arg must contain Matlab data structure\n");
    if (mxIsStruct(prhs[1])) {
        last++;
        header_provided = true;
    }
    if (!mxIsChar(prhs[last])) mexErrMsgTxt ("Last arg must contain the parse string for lastools.\n");
    if (nlhs !=0) mexErrMsgTxt ("No outputs needed.\n");
    
    /* copy the string data from prhs[2] into a C string "parse_string".    */
    parse_string = mxArrayToString(prhs[last]);
    // Get string arguments
    get_argc(argc,parse_string);
    
    // Allocate argv
    argv = (char **) mxCalloc(argc+1, sizeof(char *)); // Add one to 'argc' for command line compatibility
    
    // Fill argv
    set_argv(argv,parse_string);
    mxFree(parse_string);
    argc++;
    nfields = mxGetNumberOfFields(prhs[0]);
    fgiparse(argc, argv, fgiopts);
    
    //int extra_field_number = mxGetFieldNumber(prhs[0],"extra_attributes");
    //lasreadopener.set_file_name(fname);
    header = new LASheader;
    header->clean();
   
    point = new LASpoint;
    if (header_provided)    {
        maxcoord(maxclass,mxGetPr(mxGetField(prhs[0],0,"classification")),mxGetScalar(mxGetField(prhs[1],0,"number_of_point_records")));
        if (*maxclass > 31 && mxGetScalar(mxGetField(prhs[1],0,"point_data_format"))<6) {
            mexErrMsgTxt ("The selected point data format does not allow for classification codes bigger than 31.\n Change the classification codes or the point data format and the corresponding point record length\n");
        }
        //tmpint = mxGetFieldNumber(prhs[0], 0, "wavepacket");
        if (mxGetFieldNumber(prhs[0], "wavepacket")!= -1) {
            mexPrintf ("Warning: waveform writing not supported!\n");
        }
        read_mex_hdr(header,point,prhs[1],prhs[0]);
    }
    else {
        read_mex(header,point,prhs[0]);
    }
    if (fgiopts->set_offset) {
        header->x_offset = fgiopts->x_offset;
        header->y_offset = fgiopts->y_offset;
        header->z_offset = fgiopts->z_offset;        
    }
    if (fgiopts->set_scale)
    {
        header->x_scale_factor = fgiopts->x_scale_factor;
        header->y_scale_factor = fgiopts->y_scale_factor;
        header->z_scale_factor = fgiopts->z_scale_factor;
        if (!fgiopts->set_offset) {
            // auto_reoffset copied from lasreader_las.cpp
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
            // do we need to change anything
            //mexPrintf("offset: %d, %d, %d\n",offset[0],offset[1],offset[2]);
            //reoffset_x = reoffset_y = reoffset_z = FALSE;
            orig_x_offset = header->x_offset;
            orig_y_offset = header->y_offset;
            orig_z_offset = header->z_offset;
            if (header->x_offset != offset[0])
            {
                header->x_offset = offset[0];
                //reoffset_x = TRUE;
            }
            if (header->y_offset != offset[1])
            {
                header->y_offset = offset[1];
                //reoffset_y = TRUE;
            }
            if (header->z_offset != offset[2])
            {
                header->z_offset = offset[2];
                //reoffset_z = TRUE;
            }
        }
    }
    
    if (!header->check()) mexErrMsgTxt("header not valid!!\n");
    //print_hdr(header);
    //point->init(Hdr, Hdr->point_data_format, Hdr->point_data_record_length);
    LASwriteOpener laswriteopener;
    //mexPrintf("laswriteopener\n");
    if (!laswriteopener.parse(argc, argv)) mexErrMsgTxt("laswriteopener parse error!!\n");
    LASwriter* laswriter = laswriteopener.open(header);
    if (laswriter == 0) mexErrMsgTxt("ERROR: could not open laswriter\n");
    if (!laswriter->update_header(header, false, true) ) { 
        //use_inventory = false, update_extra_bytes = true
        mexWarnMsgTxt("Writer header update not succesfull\n");
    }
    
    field_array_ptr = mxGetFieldByNumber(prhs[0], 0, 0); // pointer to struct mxArray holding points
    nr = mxGetM(field_array_ptr);
    nc = mxGetN(field_array_ptr);
    mexPrintf("Looping %d points\n",nr);
    while (next < nr) 
    {
        copy_point_arr_mex(header,point,prhs[0],next);
        next++;
        //mexPrintf("point->x: %6.2g\n",point->coordinates[0]);
        laswriter->update_inventory(point);
        laswriter->write_point(point);
    }
    laswriter->update_header(header,1,1);
    delete point;
    point = 0;
    laswriter->close();
    //delete header;
    //header = 0;
    delete laswriter;
    mxFree(maxclass);
}


