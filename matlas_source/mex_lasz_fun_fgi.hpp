/*
===============================================================================

  FILE:  mex_lasz_fun_fgi.hpp

  CONTENTS:

    Helper variables and functions for using LASlib reader and writer from 
    Matlab, this file is used by mex_lasz_fun.cpp, mat2las.cpp and las2mat.cpp

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
#include <math.h>
#include <string.h>
#include "laswriter.hpp"
#include "lasreader.hpp"
#ifndef mwIndex
#define mwIndex int
#endif


        
const char *hdr_field_names[] = {"file_signature","file_source_ID","global_encoding",
"project_ID_GUID_data_1","project_ID_GUID_data_2","project_ID_GUID_data_3",
"project_ID_GUID_data_4","version_major","version_minor","system_identifier",
"generating_software","file_creation_day","file_creation_year","header_size",
"offset_to_point_data","number_of_variable_length_records","point_data_format",
"point_data_record_length","number_of_point_records","number_of_points_by_return",
"x_scale_factor","y_scale_factor","z_scale_factor","x_offset","y_offset",
"z_offset","user_data_after_header_size","min_x","min_y","min_z","max_x","max_y","max_z"};

const char *pnt_field_names[] = {"x","y","z","intensity", "return_number",
        "number_of_returns","scan_direction_flag",
        "edge_of_flight_line","classification","scan_angle_rank",
        "user_data","point_source_ID"};
const char *att_field_names[] = {"name","type", "description","scale","offset"}; 
const char *LASvlr_evrl_fields[] = {"reserved","user_id", "record_id",
        "record_length_after_header","description","data"};
const char *LASvlr_geo_keys_fields[] = {"key_directory_version", "key_revision", 
        "minor_revision", "number_of_keys"};

const char *LASvlr_key_entry_fields[] = {"key_id","tiff_tag_location","count",
        "value_offset"};

const char *LASvlr_classification_fields[] = {"class_number", "description"};

const char *LASvlr_wave_packet_fields[] = {"data"};

const char *LASvlr_lastiling_fields [] = {"level","level_index","implicit_levels : 30","buffer : 1","reversible : 1","min_x", "max_x", "min_y", "max_y"};

const char *LASvlr_lasoriginal_fields [] = { "number_of_point_records", "number_of_points_by_return", "max_x"," min_x", "max_y", "min_y", "max_z" "min_z"};

const char *name_table[10] = { "unsigned char", "char", "unsigned short", "short", "unsigned long", "long", "unsigned long long", "long long", "float", "double" };

int startday[13] = {-1, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

// **********************************************************************
class fgi_options
{
public:
    BOOL extra_pass;
    BOOL set_scale;
    BOOL set_offset;
    BOOL header;
    BOOL populate;
    F64 x_scale_factor;
    F64 y_scale_factor;
    F64 z_scale_factor;
    F64 x_offset;
    F64 y_offset;
    F64 z_offset;
    inline BOOL set_scale_factor(F64 scale[3]) {
        this->x_scale_factor = scale[0];
        this->y_scale_factor = scale[1];
        this->z_scale_factor = scale[2];
        return TRUE; }
    inline BOOL set_offset_vals(F64 offset[3]) {
        this->x_offset = offset[0];
        this->y_offset = offset[1];
        this->z_offset = offset[2];
        return TRUE; }

    inline BOOL print() {
        mexPrintf("extra_pass: %d, set_scale: %d, no_header: %d, npop: %d \n",this->extra_pass,this->set_scale,this->header,this->populate);
        return TRUE;
    }
    
    fgi_options()
    {
        extra_pass = false;
        set_scale = false;
        set_offset = false;
        header = true;
        populate = false;
        x_scale_factor = 0.01;
        y_scale_factor = 0.01;
        z_scale_factor = 0.01;
        x_offset = 0.0;
        y_offset = 0.0;
        z_offset = 0.0;
    };
    
};


//**********************************************************************
// functions for getting parse string from Matlab
// Add +1 to argc
void add_argc(int j, int cs, int& num);
void get_argc(int& a, char* str1);
// Create argument vector for passing it in following parser
void set_argv(char** A, char* str2);
void usage(bool wait);
static double taketime();
//static void byebye(bool error=false, bool wait=false);
//
//initialization for reading
//void initialize_plhs1(LASheader *header, LASpoint *point, mxArray *plhs1, double **pntrArr);
//**********************************************************************
// Helper functions
int maxcoord(double *xmax, double *x, mwSize len);
int mincoord(double *xmin, double *x, mwSize len);
long double2long(double d);
//**********************************************************************
// fgi parser
BOOL fgiparse(int argc, char *argv[], fgi_options *fgiopts);
//**********************************************************************
// Functions for reading
void copy_vlr_struct(mxArray *tmpvlr_1, LASheader *header, int vlrid);
void get_header_fields(LASheader *header, mxArray *mexhdr, I32 *eao);
void copy_mex_point_arr(LASreader *lasreader, int next, double *p1, double *p2, 
        double *p3, double *p4, double *p5, double *p6, double *p7, 
        double *p8, double *p9, double *p10, double *p11, double *p12, 
        double *p13, double *p14);
void copy_extra_attribute(LASreader *lasreader, double *ep1,int next, I32 attribute_array_offset, I32 index);
//**********************************************************************
// Functions for writing
void read_mex(LASheader *header, LASpoint *point, const mxArray *mexdata);
void read_mex_hdr(LASheader *header, LASpoint *point, const mxArray *mexhdr, const mxArray *mexdata);
void add_attributes_to_header(LASheader *header,const mxArray *extra_field_array_ptr, bool hdr_info);
void put_attributes(LASheader *header, LASpoint *point, double *pntr, int next, int i);
void copy_field(LASheader *header, LASpoint *point, const mxArray *pnt, int next,mwIndex index,int field_index,const char  *field_name);
void copy_point_arr_mex(LASheader *header, LASpoint *point, const mxArray *pnt, int next);
