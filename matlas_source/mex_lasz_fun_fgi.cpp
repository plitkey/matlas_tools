/*
===============================================================================

  FILE:  mex_lasz_fun_fgi.cpp

  CONTENTS:

    Helper functions for using LASlib reader and writer from Matlab, 
    this file is used by mat2las.cpp and las2mat.cpp

  PROGRAMMERS:

    paula.litkey@fgi.fi, eetu.puttonen@fgi.fi - www.fgi.fi

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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef mwIndex
#define mwIndex int
#endif

#ifdef _WIN32
        const bool WI=true;
#else
        const bool WI=false;
#endif

//**********************************************************************
// functions for getting parse string from Matlab - ep
// Add +1 to argc
void add_argc(int j, int cs, int& num)
{
  if (j-cs > 1) // skip consequent spaces and quotations
  {
	num++;
  }
}
// Get number of arguments
void get_argc(int& a, char* str1)
{
  int j = 0, cs = -1;
  bool lock = false;
  while (str1[j])
  {
    switch (str1[j])
	{
	  case ' ':	    
	    if (lock == false) // if lock is off, then get difference between the index and the last marker location
		  {
			add_argc(j,cs,a);
			cs = j;
		  }
		break;
	  case '"':	    
	    if (lock)
		{
		  add_argc(j,cs,a);
		  lock = false; // turn lock off, don't change the marker
		  cs = j;
		}
		else if (lock == false) // if lock is not on, turn it on, and set marker to the location
		{
		  lock = true;
		  cs = j;
		}
		break;
	}
    j++;		
  }
  add_argc(j,cs,a);
 
  if (lock)
  {
    mexWarnMsgTxt("Quotes were not closed, check your input!");
  }
  
}

// Create argument vector for passing it in following parser
void set_argv(char** A, char* str2)
{
  int i = 0, j = 0, cs = -1;
  int k;
  bool lock = false;
  char *str_tmp;
  const char *H;
  
  if (i == 0)
  {
    H = (const char *) mxCalloc(13, sizeof(char *));
	H = "Hello world.";
    A[i++] =(char*) H;
  }
 
  while (str2[j])

  {
    switch (str2[j])
	{
	  case ' ':	    
	    if (lock == false) // if lock is off, then get difference between the current index and the last marker location
		{
		  if (j-cs > 1)
		  {
		    str_tmp = (char *) mxCalloc(128, sizeof(char *));
		    for (k = 0; k < j-(cs+1); k++)
			  {				
				str_tmp[k] = str2[(cs+1)+k];
			  }			
			  str_tmp[k] = '\0';			  			  
			  A[i++] = str_tmp;
			  //mexPrintf("Input %d, %s\n",i,str_tmp);
		  }
		  cs = j; // Update marker
		}
		break;
	  case '"':	    
	    if (lock)
		{
		  lock = false; // turn lock off, update marker		   
		  if (j-cs > 1)
		  {
		    str_tmp = (char *) mxCalloc(128, sizeof(char *));
			for (k = 0; k < j-(cs+1); k++)
			{
			  //mexPrintf("Input %d = <%c>\n",k,str2[(cs+1)+k]);
			  str_tmp[k] = str2[(cs+1)+k];
			}			
			str_tmp[k] = '\0';
   		    A[i++] = str_tmp;
			//mexPrintf("Input %d, %s\n",i,str_tmp);
		  }			
		  cs = j;
		}
		else if (lock == false) // if lock is not on, turn it on, and update marker
		{
		  lock = true;
		  cs = j;
		}
		break;
	}
    j++;		
  }
  
  // Get the last argument from the end of the string
  if (j-cs >= 1)
  {       
      str_tmp = (char *) mxCalloc(128, sizeof(char));
	for (k = 0; k < j-(cs); k++)
    {	    
	  str_tmp[k] = str2[(cs+1)+k];
	} 	
    str_tmp[k] = '\0';	  
	A[i++] = str_tmp;
    //mexPrintf("LAST\t");
	//mexPrintf("Input %d, %s\n",i,str_tmp);
  }    
}

//**********************************************************************
// helper functions -pli
int maxcoord(double *xmax, double *x, mwSize len)
{
    int Ret = 0;
    while (Ret < len)    {
        if (*(x+Ret) > *xmax)   {
            *xmax = *(x+Ret);
        }
        Ret++;
    } 
    return(Ret);
}

int mincoord(double *xmin, double *x, mwSize len)
{
    int Ret = 0;
    while (Ret < len)    {
        if (*(x+Ret) < *xmin) {
            *xmin = *(x+Ret);
        }
        Ret++;
    }
    return(Ret);
}
long double2long(double d)
{
  //long l = static_cast<long>(floor(d));
    long l = (long) floor(d);
  return l;
}

//*********************************************** ***********************
// FGI parser -pli
// remove only arguments which start with "fgi"
//
BOOL fgiparse(int argc, char *argv[], fgi_options *fgiopts)
{
    int go = 1;
    for (int i = 1; i < argc; i++)
    {
        //mexPrintf("argv[i]: %s\n",argv[i]);
        if (argv[i][0] == '\0')
        {
            continue;
        }
        else if (strcmp(argv[i],"-i") == 0)
        {
            if ((i+1) >= argc)
            {
                fprintf(stderr,"ERROR: '%s' needs at least 1 argument: file_name or wild_card\n", argv[i]);
                return FALSE;
            }
            i+=1;
            do
            {
                if (strstr(argv[i],".txt") != 0 || strstr(argv[i],".asc") != 0)
                {
                    go = 0;
                    if (WI) {
                        mexErrMsgTxt("Use Matlab load function for .txt (fscan for .asc) files in Windows!");
                    }
                    fgiopts->extra_pass = true;
                    fgiopts->header = false;
                }
                i+=1;
            } while (i < argc && *argv[i] != '-' && go);
            i-=1;
        }
        
        else if (strstr(argv[i],"-keep") != 0 || strstr(argv[i],"-drop") != 0 || strstr(argv[i],"-clip") != 0)
        {
            // filtered data, need extra pass for header update
            fgiopts->extra_pass = true;
        }
        else if (strcmp(argv[i],"-lof") == 0 )
        {
            fgiopts->extra_pass = true;
        }
        else if (strcmp(argv[i],"-populate") == 0 )
        {
            fgiopts->populate = true;
        }
        else if (strcmp(argv[i],"-fgi_scale") == 0)
        {
            // in write without header, set scale to other than default 0.01
            fgiopts->set_scale = true;
            *argv[i]='\0';
            if ((i+3) >= argc)
            {
                mexErrMsgTxt("ERROR: -fgi_scale needs 3 scale arguments: x y z\n");               
            }
            F64 scale_factor[3];
            i++;
            sscanf(argv[i], "%lf", &(scale_factor[0]));
            *argv[i]='\0';
            i++;
            sscanf(argv[i], "%lf", &(scale_factor[1]));
            *argv[i]='\0'; 
            i++;
            sscanf(argv[i], "%lf", &(scale_factor[2]));
            *argv[i]='\0';
            fgiopts->set_scale_factor(scale_factor);
        }
        else if (strcmp(argv[i],"-fgi_offset") == 0)
        {
            // in write without header, set offset
            fgiopts->set_offset = true;
            *argv[i]='\0';
            if ((i+3) >= argc)
            {
                mexErrMsgTxt("ERROR: -fgi_offset needs 3 scale arguments: x y z\n");
               
            }
            F64 offset[3];
            i++;
            sscanf(argv[i], "%lf", &(offset[0]));
            *argv[i]='\0';
            i++;
            sscanf(argv[i], "%lf", &(offset[1 ]));
            *argv[i]='\0';
            i++;
            sscanf(argv[i], "%lf", &(offset[2]));
            *argv[i]='\0';
            fgiopts->set_offset_vals(offset);
        }
    }
}
//**********************************************************************
// Functions for reading -pli
// 
void copy_vlr_struct(mxArray *tmpvlr_1, LASheader *header, int vlrid) {
    mxArray *tmpU16_1, *tmpU16_2, *tmpU16_3, *tmpI8, *tmpU8;
    I8  *prI8;
    U16 *prU1, *prU2, *prU3;
    U8  *prU8;
    
    tmpU16_1 = mxCreateNumericMatrix(1,1,mxUINT16_CLASS,mxREAL);
    prU1 = (U16 *)mxGetData(tmpU16_1);
    *prU1 = header->vlrs[vlrid].reserved;
    mxSetFieldByNumber(tmpvlr_1,vlrid,0,tmpU16_1);
    
    tmpI8 = mxCreateNumericMatrix(1,16,mxINT8_CLASS,mxREAL);
    prI8 = (I8 *)mxGetData(tmpI8);
    prI8 = header->vlrs[vlrid].user_id;
    mxSetFieldByNumber(tmpvlr_1,vlrid,1,tmpI8);
    
    tmpU16_2 = mxCreateNumericMatrix(1,1,mxUINT16_CLASS,mxREAL);
    prU2 = (U16 *)mxGetData(tmpU16_2);
    *prU2 = header->vlrs[vlrid].record_id;
    mxSetFieldByNumber(tmpvlr_1,vlrid,2,tmpU16_2);
    tmpU16_3 = mxCreateNumericMatrix(1,1,mxUINT16_CLASS,mxREAL);
    prU3 = (U16 *)mxGetData(tmpU16_3);
    *prU3 = header->vlrs[vlrid].record_length_after_header;
    mxSetFieldByNumber(tmpvlr_1,vlrid,3,tmpU16_3);
    mxSetFieldByNumber(tmpvlr_1,vlrid,4,mxCreateString((char*)(header->vlrs[vlrid].description)));
    if (header->vlrs[vlrid].data != 0){
        tmpU8 = mxCreateNumericMatrix(1,header->vlrs[vlrid].record_length_after_header,mxUINT8_CLASS,mxREAL);
        mxSetFieldByNumber(tmpvlr_1,vlrid,5,tmpU8);
        U8 *tmp = (U8 *) mxGetData(tmpU8);
        try
        {
            memcpy(tmp,header->vlrs[vlrid].data,header->vlrs[vlrid].record_length_after_header*sizeof(U8));
        }
        catch (...)
        {
            mexWarnMsgTxt("vlrs length not correct\n");
        }
    }
}
void copy_wf_vlr_struct(mxArray *tmpvlr_2, LASvlr_wave_packet_descr *vlr_wave_packet_descr, int vlrid) {
    mxArray *tmpU321, *tmpU322, *tmpU81, *tmpU82;
    U32 *prU321, *prU322;
    U8  *prU81, *prU82;
    tmpU81 = mxCreateNumericMatrix(1,1,mxUINT8_CLASS,mxREAL);
    prU81 = (U8 *)mxGetData(tmpU81);
    *prU81 = vlr_wave_packet_descr->getBitsPerSample();
    mxSetFieldByNumber(tmpvlr_2,vlrid,0,tmpU81);
    tmpU82 = mxCreateNumericMatrix(1,1,mxUINT8_CLASS,mxREAL);
    prU82 = (U8 *)mxGetData(tmpU82);
    *prU82 = vlr_wave_packet_descr->getCompressionType();
    mxSetFieldByNumber(tmpvlr_2,vlrid,1,tmpU82);
    tmpU321 = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    prU321 = (U32 *)mxGetData(tmpU321);
    *prU321 = vlr_wave_packet_descr->getNumberOfSamples();
    if (prU321==0) {
        *prU321 = 80;
    }
    mxSetFieldByNumber(tmpvlr_2,vlrid,2,tmpU321);
    tmpU322 = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
    prU322 = (U32 *)mxGetData(tmpU322);
    *prU322 = vlr_wave_packet_descr->getTemporalSpacing();
    mxSetFieldByNumber(tmpvlr_2,vlrid,3,tmpU322);
    mxSetFieldByNumber(tmpvlr_2,vlrid,4,mxCreateDoubleScalar(vlr_wave_packet_descr->getDigitizerGain()));
    mxSetFieldByNumber(tmpvlr_2,vlrid,5,mxCreateDoubleScalar(vlr_wave_packet_descr->getDigitizerOffset()));    
}
int get_header_fields(LASheader *header, mxArray *mexhdr) {
    mxArray *tmp, *tmpA_1, *tmpA_2;
    mxArray *tmpD_1, *tmpD_2, *tmpD_3, *tmpD_4, *tmpD_5, *tmpD_6;
    mxArray *tmpD_7, *tmpD_8, *tmpD_9, *tmpD_10, *tmpD_11, *tmpD_12;
    mxArray *tmpD_13, *tmpD_14, *tmpD_15, *tmpD_16, *tmpD_17;
    mxArray *tmpf64_1, *tmpf64_2,*tmpf64_3, *tmpf64_4, *tmpf64_5;
    mxArray *tmpf64_6, *tmpf64_7, *tmpf64_8,*tmpf64_9, *tmpf64_10, *tmpf64_11;
    mxArray *tmpf64_12, *tmpf64_13, *tmpf64_14;
    mxArray *tmpvlr_1, *tmpvlr_2;
    char    *wfstr;
    int    WFDATAMAXSAMPLES=0;
    int nvlrs = header->number_of_variable_length_records;
    
    if (strncmp(header->file_signature, "LASF",4) != 0) {
        mexPrintf("File signature is not 'LASF'... aborting");
        return(WFDATAMAXSAMPLES);
    }
    /*These fields are in all headers;*/ 
    mxSetField(mexhdr, 0, "file_signature", mxCreateString(header->file_signature));
    tmpD_1 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_1) = (double)header->version_major;
    mxSetField(mexhdr, 0, "version_major", tmpD_1);
    tmpD_2 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_2) = (double)header->version_minor;
    mxSetField(mexhdr, 0, "version_minor", tmpD_2);
    tmpD_3 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_3) = (double)header->project_ID_GUID_data_1;
    mxSetField(mexhdr, 0, "project_ID_GUID_data_1", tmpD_3);
    tmpD_4 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_4) = (double)header->project_ID_GUID_data_2;
    mxSetField(mexhdr, 0, "project_ID_GUID_data_2", tmpD_4);
    tmpD_5 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_5) = (double)header->project_ID_GUID_data_3;
    mxSetField(mexhdr, 0, "project_ID_GUID_data_3", tmpD_5);
    tmpD_6 = mxCreateDoubleMatrix(1,4,mxREAL);
    double *pdata = mxGetPr(tmpD_6);
    for (int i = 0; i<4; i++) {
        pdata[i] = (double)header->project_ID_GUID_data_4[i];
    }
    mxSetField(mexhdr, 0, "project_ID_GUID_data_4", tmpD_6);
    tmpD_7 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_7) = (double)header->global_encoding;
    mxSetField(mexhdr, 0, "global_encoding", tmpD_7);
    tmpD_8 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_8) = (double)header->file_source_ID;
    mxSetField(mexhdr, 0, "file_source_ID", tmpD_8);
    mxSetField(mexhdr,0,"system_identifier",mxCreateString(header->system_identifier));
    mxSetField(mexhdr,0,"generating_software",mxCreateString(header->generating_software));
    tmpD_9 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_9) = (double)header->file_creation_day;
    mxSetField(mexhdr, 0, "file_creation_day", tmpD_9);
    tmpD_10 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_10) = (double)header->file_creation_year;
    mxSetField(mexhdr, 0, "file_creation_year", tmpD_10);
    tmpD_11 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_11) = (double)header->header_size;
    mxSetField(mexhdr, 0, "header_size", tmpD_11);
    tmpD_12 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_12) = (double)header->offset_to_point_data;
    mxSetField(mexhdr, 0, "offset_to_point_data", tmpD_12);
    tmpD_13 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_13) = (double)header->number_of_variable_length_records;
    mxSetField(mexhdr, 0, "number_of_variable_length_records", tmpD_13);
    tmpD_14 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_14) = (double)header->point_data_format;
    mxSetField(mexhdr, 0, "point_data_format", tmpD_14);
    tmpD_15 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpD_15) = (double)header->point_data_record_length;
    mxSetField(mexhdr, 0, "point_data_record_length", tmpD_15);
    tmpD_16 = mxCreateDoubleMatrix(1,1,mxREAL);    
    *mxGetPr(tmpD_16) = (double)header->number_of_point_records;
    mxSetField(mexhdr, 0, "number_of_point_records", tmpD_16);
    
    tmp = mxCreateDoubleMatrix(1,5,mxREAL);
    double *nprdata = mxGetPr(tmp);
    for (int j = 0; j<5; j++) {
        nprdata[j] = (double)header->number_of_points_by_return[j];
    }
    mxSetField(mexhdr, 0, "number_of_points_by_return", tmp);
    //
    tmpf64_1 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_1) = header->x_scale_factor;
    mxSetField(mexhdr, 0, "x_scale_factor", tmpf64_1);
    tmpf64_2 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_2) = header->y_scale_factor;
    mxSetField(mexhdr, 0, "y_scale_factor", tmpf64_2);
    tmpf64_3 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_3) = header->z_scale_factor;
    mxSetField(mexhdr, 0, "z_scale_factor", tmpf64_3);
    tmpf64_4 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_4) = header->x_offset;
    mxSetField(mexhdr, 0, "x_offset", tmpf64_4);
    tmpf64_5 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_5) = header->y_offset;
    mxSetField(mexhdr, 0, "y_offset", tmpf64_5);
    tmpf64_6 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_6) = header->z_offset;
    mxSetField(mexhdr, 0, "z_offset", tmpf64_6);
    tmpf64_7 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_7) = (double)header->user_data_after_header_size;
    mxSetField(mexhdr, 0, "user_data_after_header_size", tmpf64_7);
    tmpf64_8 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_8) = header->max_x;
    mxSetField(mexhdr, 0, "max_x", tmpf64_8);
    tmpf64_9 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_9) = header->max_y;
    mxSetField(mexhdr, 0, "max_y", tmpf64_9);
    tmpf64_10 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_10) = header->max_z;
    mxSetField(mexhdr, 0, "max_z", tmpf64_10);
    tmpf64_11 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_11) = header->min_x;
    mxSetField(mexhdr, 0, "min_x", tmpf64_11);
    tmpf64_12 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_12) = header->min_y;
    mxSetField(mexhdr, 0, "min_y", tmpf64_12);
    tmpf64_13 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(tmpf64_13) = header->min_z;
    mxSetField(mexhdr, 0, "min_z", tmpf64_13);
    if (nvlrs >0)   {
        LASattribute  *extra; //, *ext;
        LASvlr_wave_packet_descr* vlr_wave_packet_descr;
        mxAddField(mexhdr,"variable_length_records");
        tmpvlr_1 = mxCreateStructMatrix(1,nvlrs,6,LASvlr_evrl_fields);
        mxSetField(mexhdr, 0, "variable_length_records", tmpvlr_1);
        vlr_wave_packet_descr = (LASvlr_wave_packet_descr*)mxMalloc(sizeof(LASvlr_wave_packet_descr*));
        if (!vlr_wave_packet_descr) {
            mexErrMsgTxt("could not allocate vlr_wave_packet_descr\n");
        }
        int wfi = 0;
        for (int i = 0; i < nvlrs; i++)
        {
            //mexPrintf("variable length header record %d of %d:\012", i+1, (int)header->number_of_variable_length_records);
            //mexPrintf("  reserved             %d\012", header->vlrs[i].reserved);
            //mexPrintf("  user ID              '%s'\012", header->vlrs[i].user_id);
            //mexPrintf("  record ID            %d\012", header->vlrs[i].record_id);
            //mexPrintf("  length after header  %d\012", header->vlrs[i].record_length_after_header);
            //mexPrintf("  description          '%s'\012", header->vlrs[i].description);           
            copy_vlr_struct(tmpvlr_1,header,i);
            if ((header->vlrs[i].record_id >= 100) && (header->vlrs[i].record_id < 355)) // WavePacketDescriptor
            {
                
                wfi++;
                
                vlr_wave_packet_descr = (LASvlr_wave_packet_descr*)header->vlrs[i].data;
                //mexPrintf("  index %d bits/sample %d compression %d samples %u temporal %u gain %lg, offset %lg\012", header->vlrs[i].record_id-99, vlr_wave_packet_descr->getBitsPerSample(), vlr_wave_packet_descr->getCompressionType(), vlr_wave_packet_descr->getNumberOfSamples(), vlr_wave_packet_descr->getTemporalSpacing(), vlr_wave_packet_descr->getDigitizerGain(), vlr_wave_packet_descr->getDigitizerOffset());
                if (WFDATAMAXSAMPLES<vlr_wave_packet_descr->getNumberOfSamples()) {
                WFDATAMAXSAMPLES = vlr_wave_packet_descr->getNumberOfSamples();
                }
            }            
        }
        if (WFDATAMAXSAMPLES) {
            mxAddField(mexhdr,"wave_packet_descriptors");
            tmpvlr_2 = mxCreateStructMatrix(1,wfi,6,LASvlr_wave_packet_descriptor_fields);
            mxSetField(mexhdr, 0, "wave_packet_descriptors", tmpvlr_2);
            int iwf = 0;
            for (int i = 0; i < nvlrs; i++) {
                if ((header->vlrs[i].record_id >= 100) && (header->vlrs[i].record_id < 355)) // WavePacketDescriptor
                {
                    vlr_wave_packet_descr = (LASvlr_wave_packet_descr*)header->vlrs[i].data;
                    copy_wf_vlr_struct(tmpvlr_2,vlr_wave_packet_descr,iwf);
                    iwf++;
                }
            }
                        
        }
        
        //--------------------------------------------------------------------------------------------------
        if ( header->number_attributes > 0) {
            mwSize dims = header->number_attributes;
            extra = (LASattribute*)mxMalloc(sizeof(LASattribute)*dims);
            if (!extra) {
               mexErrMsgTxt("could not allocate attributes\n");
            }
            mxAddField(mexhdr,"number_attributes");
            tmpf64_14 = mxCreateDoubleMatrix(1,1,mxREAL);
            *mxGetPr(tmpf64_14) = header->number_attributes;
            mxSetField(mexhdr, 0, "number_attributes", tmpf64_14);
            mxAddField(mexhdr,"attributes");
            tmpA_1 = mxCreateStructMatrix(1,dims,5,att_field_names);
            mxSetField(mexhdr, 0, "attributes", tmpA_1);
            int VRLid = 0;
            int k = 0;
            while (VRLid < nvlrs){
                if (header->vlrs[VRLid].record_id == 4) // ExtraBytes
                {
                    extra = (LASattribute*) header->vlrs[VRLid].data;
                    while (k < header->number_attributes) {
                        mxSetFieldByNumber(tmpA_1,k,0,mxCreateString((char*)(extra[k].name)));
                        mxSetFieldByNumber(tmpA_1,k,1,mxCreateDoubleScalar(extra[k].data_type));
                        mxSetFieldByNumber(tmpA_1,k,2,mxCreateString((char*)(extra[k].description)));
                        mxSetFieldByNumber(tmpA_1,k,3,mxCreateDoubleScalar(extra[k].scale[0]));
                        mxSetFieldByNumber(tmpA_1,k,4,mxCreateDoubleScalar(extra[k].offset[0]));
                        k++;
                    }
                }
                VRLid++;
            }
            /*if (extra) {
                mxFree(extra); 
                
            }*/
        }
        /*if (vlr_wave_packet_descr) {
            mxFree(vlr_wave_packet_descr);
        }*/
        
    }
    return(WFDATAMAXSAMPLES);
}
void copy_mex_point_arr(LASreader *lasreader, int next, double *p1, double *p2, 
        double *p3, double *p4, double *p5, double *p6, double *p7, 
        double *p8, double *p9, double *p10, double *p11, double *p12, 
        double *p13, double *p14)
 {
    *(p1+next) = (double)lasreader->point.get_x();
    *(p2+next) = (double)lasreader->point.get_y();
    *(p3+next) = (double)lasreader->point.get_z();
    *(p4+next) = (double)lasreader->point.intensity;
    *(p5+next) = (double)lasreader->point.return_number;
    *(p6+next) = (double)lasreader->point.number_of_returns;
    *(p7+next) = (double)lasreader->point.scan_direction_flag;
    *(p8+next) = (double)lasreader->point.edge_of_flight_line;
    //*(p9+next) = (double)lasreader->point.classification;
    if (lasreader->header.point_data_format > 5)
    {
        if (lasreader->point.get_extended_classification())
        {
            *(p9+next) = (double)lasreader->point.get_extended_classification();
        }
        else
        {
            *(p9+next) = (double)lasreader->point.get_classification();
        }
    }
    else
    {
        *(p9+next) = (double)lasreader->point.get_classification();
    }
    
    *(p10+next) = (double)lasreader->point.scan_angle_rank;
    *(p11+next) = (double)lasreader->point.user_data;
    *(p12+next) = (double)lasreader->point.point_source_ID;
    if (lasreader->point.have_gps_time)
    {
        *(p13+next) = (double)lasreader->point.gps_time;
    }
    if (lasreader->point.have_rgb)
    {
        *(p14+next) = (double)lasreader->point.rgb[0];
        *(p14+lasreader->header.number_of_point_records+next) = (double)lasreader->point.rgb[1];
        *(p14+lasreader->header.number_of_point_records*2+next) = (double)lasreader->point.rgb[2];

        if (lasreader->point.have_nir)
        {
            *(p14+lasreader->header.number_of_point_records*3+next) = (double)lasreader->point.rgb[3];
        }
    }
    
}
void copy_attribute(LASreader *lasreader, double *ep1,int next, I32 attribute_array_offset, I32 index) {
    if (lasreader->header.attributes[index].data_type == 1)
    {
        U8 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else if (lasreader->header.attributes[index].data_type == 2)
    {
        I8 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else if (lasreader->header.attributes[index].data_type == 3)
    {
        U16 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else if (lasreader->header.attributes[index].data_type == 4)
    {
        I16 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else if (lasreader->header.attributes[index].data_type == 5)
    {
        U32 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else if (lasreader->header.attributes[index].data_type == 6)
    {
        I32 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else if (lasreader->header.attributes[index].data_type == 9)
    {
        F32 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else if (lasreader->header.attributes[index].data_type == 10)
    {
        F64 value;
        lasreader->point.get_attribute(attribute_array_offset, value);
        *(ep1+next) = value * 1.0;
    }
    else
    {
        mexErrMsgTxt("ERROR: extra attribute type not valid.\n");
    }
    if (lasreader->header.attributes[index].has_scale() || lasreader->header.attributes[index].has_offset())
    {
        *(ep1+next) = (lasreader->header.attributes[index].scale[0]* *(ep1+next) + lasreader->header.attributes[index].offset[0]) * 1.0;
    }

}
void copy_wf_pos_struct(LASreader *lasreader,int next, double *p1, double *p2, 
        double *p3, double *p4, double *p5, double *p6, double *p7) {

    *(p1+next) = (double)lasreader->point.wavepacket.getXt();
    *(p2+next) = (double)lasreader->point.wavepacket.getYt();    
    *(p3+next) = (double)lasreader->point.wavepacket.getZt();
    *(p4+next) = (double)lasreader->point.wavepacket.getIndex();    
    *(p5+next) = (double)lasreader->point.wavepacket.getOffset();
    *(p6+next) = (double)lasreader->point.wavepacket.getSize();
    *(p7+next) = (double)lasreader->point.wavepacket.getLocation();    
}
// void copy_mex_wf_point_arr(LASreader *lasreader,int next,double *pwfp, int wfs_num)
// {
// //     mexPrintf("Copy wf point: %d\t%d\t",next,wfs_num);
// //     mexPrintf("ID: %d\t", lasreader->point.wavepacket.getIndex());
//     *(pwfp+next) = (double)lasreader->point.wavepacket.getXt();
//     *(pwfp+wfs_num+next) = (double)lasreader->point.wavepacket.getYt();
//     *(pwfp+wfs_num*2+next) = (double)lasreader->point.wavepacket.getZt();
//     *(pwfp+wfs_num*3+next) = (double)lasreader->point.wavepacket.getIndex(); 
//     *(pwfp+wfs_num*4+next) = (double)lasreader->point.wavepacket.getOffset(); 
//     *(pwfp+wfs_num*5+next) = (double)lasreader->point.wavepacket.getSize(); 
//     *(pwfp+wfs_num*6+next) = (double)lasreader->point.wavepacket.getLocation();
// }
void copy_waveform(LASwaveform13reader *laswaveform13reader, int next, double *wfd, int wfs_num)
{
  U32 i;

  if (laswaveform13reader->nbits == 8)
  {
    for (i = 0; i < laswaveform13reader->nsamples; i++)
    {
      //*(wfd+laswaveform13reader->nsamples*next+i) = (double)laswaveform13reader->samples[i];
      *(wfd+wfs_num*i+next) = (double)laswaveform13reader->samples[i];
      //mexPrintf("\t%d",laswaveform13reader->samples[i]);
    }
  }
  else if (laswaveform13reader->nbits == 16)
  {
    for (i = 0; i < laswaveform13reader->nsamples; i++)
    {
      //*(wfd+laswaveform13reader->nsamples*next+i) = (double)((U16*)laswaveform13reader->samples)[i];
      *(wfd+wfs_num*i+next) = (double)((U16*)laswaveform13reader->samples)[i];
      //mexPrintf("\t%d",laswaveform13reader->samples[i]);
    }
  }
  else if (laswaveform13reader->nbits == 32)
  {
    for (i = 0; i < laswaveform13reader->nsamples; i++)
    {
      //*(wfd+laswaveform13reader->nsamples*next+i) = (double)((I32*)laswaveform13reader->samples)[i];
      *(wfd+wfs_num*i+next) = (double)((I32*)laswaveform13reader->samples)[i];
      //mexPrintf("\t%d",laswaveform13reader->samples[i]);
    }
  }
  else
  {
      mexPrintf("doing nothing in copy waveform!\n");
  }
  //mexPrintf("\n");
}

//**********************************************************************
// Functions for writing -pli
// 

// Write without header - header is filled using the data
void read_mex(LASheader *header, LASpoint *point, const mxArray *mexdata) {
    const mxArray *field_array_ptr;
    mxArray *dateArr[1];
    double *datapr;
    double *datepr;
    int extranext;
    int index,strlen;
    U16  file_creation_day = 0;
    int ok = 0;
    double *xmax, *ymax, *zmax;
    double *xmin, *ymin, *zmin;
    double  datevec[6];
    int    extra_a_num, extra_ai_num;
    mwSize     nc, nr;
    bool    rgb = false;
    bool    time = false;
    bool    extra_a = false;
    bool    extra_ai = false;
    bool    hdr_info = false;
    int     allcoords = 0;
    mwSize  total_num_of_elements, number_of_fields, field_index, npoints;
    mwSize  total_num_of_extra_elements;
    const char* field_name;
    char temp[32];
    // copy from txt2las
    xmax =(double*) mxCalloc(1,sizeof(double));
    if (!xmax)
        mexErrMsgTxt("Memory error");
    ymax =(double*) mxCalloc(1,sizeof(double));
    if (!ymax)
        mexErrMsgTxt("Memory error");
    zmax =(double*) mxCalloc(1,sizeof(double));
    if (!zmax)
        mexErrMsgTxt("Memory error");
    xmin =(double*) mxCalloc(1,sizeof(double));
    if (!xmin)
        mexErrMsgTxt("Memory error");
    ymin =(double*) mxCalloc(1,sizeof(double));
    if (!ymin)
        mexErrMsgTxt("Memory error");
    zmin =(double*) mxCalloc(1,sizeof(double));
    if (!zmin)
        mexErrMsgTxt("Memory error");

    // clean header
    header->clean();
    // set creation date
    mexCallMATLAB(1,dateArr, 0,NULL, "clock");
    datepr = mxGetPr(dateArr[0]);    
    file_creation_day = startday[double2long(datepr[1])] + double2long(datepr[2]);
    if (((double2long(*datepr)%4) == 0) && (double2long(*(datepr+1)) > 2)) file_creation_day++;
    header->file_creation_day = (U16) file_creation_day;
    header->file_creation_year = (U16) double2long(datepr[0]);
    strlen = sprintf(temp,"fgi_mat2las lastools v %d\n", LAS_TOOLS_VERSION);
    strncpy(header->system_identifier, temp, 32);

    total_num_of_elements = mxGetNumberOfElements(mexdata);
    number_of_fields = mxGetNumberOfFields(mexdata);
    /* Walk through each structure element. */
    for (field_index=0; field_index<number_of_fields; field_index++)  {
        field_name = mxGetFieldNameByNumber(mexdata,field_index);
        //mexPrintf("%s %s %s %d\n", "FIELD: ", field_name, "INDEX :", field_index);
        field_array_ptr = mxGetFieldByNumber(mexdata, 0, field_index);
        if(field_array_ptr == NULL) {
            mexPrintf("%s%d\t%s%d\n", "FIELD: ", field_index+1, "STRUCT INDEX :", index+1);
            mexErrMsgIdAndTxt( "MATLAB:pointstruct:fieldEmpty","Above field is empty!");
        }
        datapr = mxGetPr(field_array_ptr);
        nr = mxGetM(field_array_ptr);
        nc = mxGetN(field_array_ptr);
        //mexPrintf("next: %d\t field_index: %d\t index: %d\n",0,field_index,index);
        if (strcmp(field_name,"x")==0) {
            allcoords++;
            *xmax = 0;
            ok = maxcoord(xmax,datapr,nr);
            if (ok<nr) mexErrMsgTxt("not all points were looped in max search");
            *xmin = *xmax;
            ok = 0;
            ok = mincoord(xmin,datapr,nr);
            if (ok<nr) mexErrMsgTxt("not all points were looped in min search");
            header->max_x = (F64) *xmax;
            header->min_x = (F64) *xmin;
            // x field is usually of the right size to fill number of points
            header->number_of_point_records = nr;
            npoints = nr;
        }
        else if (strcmp(field_name,"y")==0) {
            allcoords++;
            *ymax = 0;
            ok = maxcoord(ymax,datapr,nr);
            if (ok<nr) mexErrMsgTxt("not all points were looped in max search");
            *ymin = *ymax;
            ok = 0;
            ok = mincoord(ymin,datapr,nr);
            if (ok<nr) mexErrMsgTxt("not all points were looped in min search");
            header->max_y = (F64) *ymax;
            header->min_y = (F64) *ymin;            
        }
        else if (strcmp(field_name,"z")==0) {
            allcoords++;
            *zmax = 0;
            ok = maxcoord(zmax,datapr,nr);
            if (ok<nr) mexErrMsgTxt("not all points were looped in max search");
            *zmin = *zmax;
            ok = 0;
            ok = mincoord(zmin,datapr,nr);
            if (ok<nr) mexErrMsgTxt("not all points were looped in min search");
            header->max_z = (F64) *zmax;
            header->min_z = (F64) *zmin;            
        }
        else if (strcmp(field_name,"gps_time")==0) {
            time = true;
        }
        else if (strcmp(field_name,"rgb")==0) {
            rgb = true;
        }
        
        else if (strcmp(field_name,"attributes")==0) {
            extra_a = true;
            extra_a_num = field_index;
        }
        else if (strcmp(field_name,"attribute_info")==0) {
            extra_ai = true;
            extra_ai_num = field_index;
        }
        
    }
    if (allcoords < 3) mexErrMsgTxt("all x,y and z fields not found in data!");
    if (time)
    {
        if (rgb)
        {
            header->point_data_format = 3;
            header->point_data_record_length = 34;
        }
        else
        {
            header->point_data_format = 1;
            header->point_data_record_length = 28;
        }
    }
    else
    {
        if (rgb)
        {
            header->point_data_format = 2;
            header->point_data_record_length = 26;
        }
        else
        {
            header->point_data_format = 0;
            header->point_data_record_length = 20;
        }
    }
    
    if (extra_a && extra_ai) {
        const mxArray *extra_field_array_ptr;
        header->clean_attributes();
        field_name = mxGetFieldNameByNumber(mexdata,extra_ai_num);
        //mexPrintf("%s %s\n", "FIELD: ", field_name);        
        extra_field_array_ptr = mxGetFieldByNumber(mexdata, 0, extra_ai_num);
        add_attributes_to_header(header,extra_field_array_ptr,hdr_info);   
    }
    else if (extra_a || extra_ai) {
        mexErrMsgTxt("Both fields 'attributes' and 'attribute_info'"
                "needed in matlab data structure to write attributes!\n");
    }
    header->update_extra_bytes_vlr();
    //mexPrintf("Header num extra: %d\n",header->number_attributes);
    point->init(header, header->point_data_format, header->point_data_record_length,header);
    point->zero();
}
// Write with header - header is used for info
void read_mex_hdr(LASheader *header, LASpoint *point, const mxArray *mexhdr, const mxArray *mexdata) {
    
    const mxArray *field_array_ptr;
    const mxArray *reserved_array_ptr;
    const mxArray *user_id_array_ptr;
    const mxArray *record_id_array_ptr;
    const mxArray *rec_len_array_ptr;
    const mxArray *descr_array_ptr;    
    const mxArray *data_array_ptr;
    mxArray *dateArr[1];
    double *datapr;
    double  *datepr;
    U16  file_creation_day = 0;
    int extranext;
    int index,strlen;
    int    extra_a_num;
    mwSize     nc, nr;
    bool    rgb = false;
    bool    time = false;
    bool    extra_a = false;
    bool    hdr_info = true;
    mwSize  total_num_of_elements, number_of_fields, field_index, npoints;
    mwSize  total_num_of_extra_elements;
    const char* field_name;
    char       user_id[16]; //=NULL;
    char        temp[32];
    int        ifield, nfields;
    int         nvlrs = 0;
    //U32 original_offset = 0;
    mwSize     NStructElems;
    // clean header
    header->clean();
    nfields = mxGetNumberOfFields(mexhdr);
    NStructElems = mxGetNumberOfElements(mexhdr);
    total_num_of_elements = mxGetNumberOfElements(mexhdr);
    number_of_fields = mxGetNumberOfFields(mexhdr);
    
    mexCallMATLAB(1,dateArr, 0,NULL, "clock");
    datepr = mxGetPr(dateArr[0]);    
    file_creation_day = startday[double2long(datepr[1])] + double2long(datepr[2]);
    if (((double2long(*datepr)%4) == 0) && (double2long(*(datepr+1)) > 2)) file_creation_day++;
    header->file_creation_day = (U16) file_creation_day;
    header->file_creation_year = (U16) double2long(datepr[0]);
    strlen = sprintf(temp,"fgi_mat2las lastools v %d\n", LAS_TOOLS_VERSION);
    strncpy(header->system_identifier, temp, 32);
    /* Walk through each structure element. */
    for (index=0; index<total_num_of_elements; index++)  {
        
        /* For the given index, walk through each field. */
        for (field_index=0; field_index<number_of_fields; field_index++)  {
            field_name = mxGetFieldNameByNumber(mexhdr,field_index);
            field_array_ptr = mxGetFieldByNumber(mexhdr, index, field_index);
            // only needed fields are replaced, header is updated later
            if (strcmp(field_name,"file_source_ID")==0) {
                header->file_source_ID = (U16) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"global_encoding")==0) {
                header->global_encoding = (U16) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"version_major")==0) {
                header->version_major = (U8) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"version_minor")==0) {
                header->version_minor = (U8) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"header_size")==0) {
                header->header_size = (U16) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"offset_to_point_data")==0) {
                //fixing this with old information could cause trouble
                //header->offset_to_point_data = (U32) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"number_of_variable_length_records")==0) {
               header->number_of_variable_length_records = (U32) mxGetScalar(field_array_ptr);
               nvlrs = (int)header->number_of_variable_length_records;
            }
            else if (strcmp(field_name,"point_data_format")==0) {                
                header->point_data_format = (U8) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"point_data_record_length")==0) {
                header->point_data_record_length = (U16) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"number_of_point_records")==0) {
                header->number_of_point_records = (U32) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"x_scale_factor")==0) {
                header->x_scale_factor = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"y_scale_factor")==0) {
                header->y_scale_factor = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"z_scale_factor")==0) {
                header->z_scale_factor = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"x_offset")==0) {
                header->x_offset = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"y_offset")==0) {
                header->y_offset = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"z_offset")==0) {
                header->z_offset = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"max_x")==0) {
                header->max_x = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"max_y")==0) {
                header->max_y = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"max_z")==0) {
                header->max_z = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"min_x")==0) {
                header->min_x = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"min_y")==0) {
                header->min_y = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"min_z")==0) {
                header->min_z = (F64) mxGetScalar(field_array_ptr);
            }
            
            else if (strcmp(field_name,"number_attributes")==0) {
                header->number_attributes = (F64) mxGetScalar(field_array_ptr);
            }
            else if (strcmp(field_name,"attributes")==0) {
                extra_a = true;
                extra_a_num = field_index;
            }
            else if (strcmp(field_name,"variable_length_records")==0 & nvlrs>0){                
                for (int i = 0; i < nvlrs; i++)  {
                    // first pass to determine if other than extra attribute vlrs exist
                    record_id_array_ptr = mxGetFieldByNumber(field_array_ptr, i, 2);
                    if (mxGetScalar(record_id_array_ptr) != 4 )  {// extra attributes added later!
                        header->clean_vlrs();
                    }
                }
                for (int i = 0; i < nvlrs; i++)
                {
                    //mexPrintf("VLRS %d\n",i);
                    reserved_array_ptr = mxGetFieldByNumber(field_array_ptr, i, 0);
                    user_id_array_ptr = mxGetFieldByNumber(field_array_ptr, i, 1);
                    record_id_array_ptr = mxGetFieldByNumber(field_array_ptr, i, 2);
                    rec_len_array_ptr = mxGetFieldByNumber(field_array_ptr, i, 3);
                    descr_array_ptr = mxGetFieldByNumber(field_array_ptr, i, 4);
                    data_array_ptr = mxGetFieldByNumber(field_array_ptr, i, 5);
                    
                    if (mxGetScalar(record_id_array_ptr) != 4 )  {// extra attributes added later!                        
                        if (user_id_array_ptr != 0){
                            try
                            {
                                memcpy(user_id, mxGetData(user_id_array_ptr),16*sizeof(I8));
                            }
                            catch (...)
                            {
                                mexWarnMsgTxt("vlrs user_id copy not succesful\n");
                            }
                        }
                        header->add_vlr((const char*)user_id ,  (U16) mxGetScalar(record_id_array_ptr), (U16) mxGetScalar(rec_len_array_ptr), (U8*) mxGetData(data_array_ptr));
                        mxGetString(descr_array_ptr,header->vlrs[i].description,32);
                    }
                }
                if (header->number_of_variable_length_records != nvlrs) mexWarnMsgTxt("variable length record number changed!");
            }            
            if (field_array_ptr == NULL) {
                mexPrintf("\tEmpty Field\n");
            }
        }

    }
    
    if (extra_a) {
        const mxArray *extra_field_array_ptr;
        if (mxGetFieldNumber(mexdata,"attributes") < 0) {
            mexErrMsgTxt("Both header and data need 'attributes' field"
                    "in matlab structure to write attributes!\n");
        }
        
        header->clean_attributes();
        extra_field_array_ptr = mxGetFieldByNumber(mexhdr, 0, extra_a_num);
        add_attributes_to_header(header,extra_field_array_ptr,hdr_info);
    }
    if (header->user_data_in_header !=0) {
        header->user_data_in_header = 0;
        if (header->offset_to_point_data > header->header_size) {
            header->offset_to_point_data =header->offset_to_point_data - *(U32 *)header->user_data_in_header;
        }
    }

    header->update_extra_bytes_vlr();
    if (!header->check()) mexErrMsgTxt("header not valid!!\n");
    point->init(header, header->point_data_format, header->point_data_record_length,header);
    point->zero();
}

void add_attributes_to_header(LASheader *header,const mxArray *extra_field_array_ptr,bool hdr_info)
{
    const mxArray *name_array_ptr;
    const mxArray *type_array_ptr;
    const mxArray *descr_array_ptr;
    const mxArray *scale_array_ptr;
    const mxArray *offset_array_ptr;    
    const char* field_name;
    char    namestr[32];
    char    descrstr[32];
    I32     type = 0;
    U16     extra_bytes_size = 0;
    mwSize     nc, nr;
    mwSize number_of_fields, field_index;
    mxClassID  category;
    nr = mxGetM(extra_field_array_ptr);
    nc = mxGetN(extra_field_array_ptr);
    /* Walk through each structure element. */

    category = mxGetClassID(extra_field_array_ptr);
    if (category != mxSTRUCT_CLASS) mexErrMsgTxt("No struct Here!");
    LASattribute  *extra;
    extra = (LASattribute*)mxMalloc(sizeof(LASattribute)*nc);
    if (!extra) {
       mexErrMsgTxt("could not allocate extra attributes\n");
    }
    
    name_array_ptr = mxGetFieldByNumber(extra_field_array_ptr, 0, 0);

    for (int eind=0; eind<nc; eind++)  {
        number_of_fields = mxGetNumberOfFields(extra_field_array_ptr);
        name_array_ptr = mxGetFieldByNumber(extra_field_array_ptr, eind, 0);
        type_array_ptr = mxGetFieldByNumber(extra_field_array_ptr, eind, 1);
        descr_array_ptr = mxGetFieldByNumber(extra_field_array_ptr, eind, 2);
        mxGetString(name_array_ptr,namestr,32);
        mxGetString(descr_array_ptr,descrstr,32);
        type = mxGetScalar(type_array_ptr);
        extra[eind] = LASattribute(type-1,namestr,descrstr);
        if (number_of_fields == 4) {
            scale_array_ptr = mxGetFieldByNumber(extra_field_array_ptr, eind, 3);
            extra[eind].set_scale((F64) mxGetScalar(scale_array_ptr));
        }
        if (number_of_fields == 5) {
            scale_array_ptr = mxGetFieldByNumber(extra_field_array_ptr, eind, 3);
            offset_array_ptr = mxGetFieldByNumber(extra_field_array_ptr, eind, 4);
            extra[eind].set_scale((F64) mxGetScalar(scale_array_ptr));
            extra[eind].set_offset((F64) mxGetScalar(offset_array_ptr));
        }
        extra_bytes_size += extra[eind].get_size();
        if(header->add_attribute((const LASattribute) extra[eind]) < 0) {
            mexPrintf("Attribute %d\t",eind);
            mexWarnMsgTxt("Attribute adding problem!");
        }
    }
    header->update_extra_bytes_vlr();
    if (!hdr_info)  {
        header->point_data_record_length += extra_bytes_size;
    }
    mxFree(extra);
}
//copy from laslib lasreader_txt.cpp LASreaderTXT::parse_extra_attribute function
void put_attributes(LASheader *header, LASpoint *point, double *pntr, int next, int index)
{
    
  if (index >= header->number_attributes)
  {
    mexErrMsgTxt("Attribute adding problem!");
  }
  if (header->attributes[index].data_type == 1)
  {
    I32 temp_i;
    F64 temp_d;
    temp_d = *(pntr+next);
    if (header->attributes[index].has_offset())
    {
      temp_d -= header->attributes[index].offset[0];
    }
    if (header->attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header->attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < U8_MIN || temp_i > U8_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type U8 is %d. clamped to [%d %d] range.\n", index, temp_i, U8_MIN, U8_MAX);
      point->set_attribute(header->get_attribute_start(index), U8_CLAMP(temp_i));
    }
    else
    {
      point->set_attribute(header->get_attribute_start(index), (U8)temp_i);
    }
  }
  else if (header->attributes[index].data_type == 2)
  {
    I32 temp_i;
    F64 temp_d;
    temp_d = *(pntr+next);
    if (header->attributes[index].has_offset())
    {
      temp_d -= header->attributes[index].offset[0];
    }
    if (header->attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header->attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < I8_MIN || temp_i > I8_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type I8 is %d. clamped to [%d %d] range.\n", index, temp_i, I8_MIN, I8_MAX);
      point->set_attribute(header->get_attribute_start(index), I8_CLAMP(temp_i));
    }
    else
    {
      point->set_attribute(header->get_attribute_start(index), (I8)temp_i);
    }
  }
  else if (header->attributes[index].data_type == 3)
  {
    I32 temp_i;
    F64 temp_d;
    temp_d = *(pntr+next);
    if (header->attributes[index].has_offset())
    {
      temp_d -= header->attributes[index].offset[0];
    }
    if (header->attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header->attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < U16_MIN || temp_i > U16_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type U16 is %d. clamped to [%d %d] range.\n", index, temp_i, U16_MIN, U16_MAX);
      point->set_attribute(header->get_attribute_start(index), U16_CLAMP(temp_i));
    }
    else
    {
      point->set_attribute(header->get_attribute_start(index), (U16)temp_i);
    }
  }
  else if (header->attributes[index].data_type == 4)
  {
    I32 temp_i;
    F64 temp_d;
    temp_d = *(pntr+next);
    if (header->attributes[index].has_offset())
    {
      temp_d -= header->attributes[index].offset[0];
    }
    if (header->attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header->attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    if (temp_i < I16_MIN || temp_i > I16_MAX)
    {
      fprintf(stderr, "WARNING: attribute %d of type I16 is %d. clamped to [%d %d] range.\n", index, temp_i, I16_MIN, I16_MAX);
      point->set_attribute(header->get_attribute_start(index), I16_CLAMP(temp_i));
    }
    else
    {
      point->set_attribute(header->get_attribute_start(index), (I16)temp_i);
    }
  }
  else if (header->attributes[index].data_type == 5)
  {
    U32 temp_u;
    F64 temp_d;
    temp_d = *(pntr+next);
    if (header->attributes[index].has_offset())
    {
      temp_d -= header->attributes[index].offset[0];
    }
    if (header->attributes[index].has_scale())
    {
      temp_u = U32_QUANTIZE(temp_d/header->attributes[index].scale[0]);
    }
    else
    {
      temp_u = U32_QUANTIZE(temp_d);
    }
    point->set_attribute(header->get_attribute_start(index), temp_u);
  }
  else if (header->attributes[index].data_type == 6)
  {
    I32 temp_i;
    F64 temp_d;
    temp_d = *(pntr+next);
    if (header->attributes[index].has_offset())
    {
      temp_d -= header->attributes[index].offset[0];
    }
    if (header->attributes[index].has_scale())
    {
      temp_i = I32_QUANTIZE(temp_d/header->attributes[index].scale[0]);
    }
    else
    {
      temp_i = I32_QUANTIZE(temp_d);
    }
    point->set_attribute(header->get_attribute_start(index), temp_i);
  }
  else if (header->attributes[index].data_type == 9)
  {
    F32 temp_f;
    temp_f = (F32) *(pntr+next);
    point->set_attribute(header->get_attribute_start(index), temp_f);
  }
  else if (header->attributes[index].data_type == 10)
  {
    F64 temp_d;
    temp_d = *(pntr+next);
    point->set_attribute(header->get_attribute_start(index), temp_d);
  }
  else
  {
    fprintf(stderr, "WARNING: attribute %d not (yet) implemented.\n", index);
  }
}


void copy_field(LASheader *header, LASpoint *point, const mxArray *pnt, int next, mwIndex index,int field_index,const char  *field_name)  {
    const mxArray *field_array_ptr;
    double *datapr;
    int extranext;
    mwSize     nc, nr;
    field_array_ptr = mxGetFieldByNumber(pnt, index, field_index);
    if(field_array_ptr == NULL) {
        mexPrintf("%s%d\t%s%d\n", "FIELD: ", field_index+1, "STRUCT INDEX :", index+1);
        mexErrMsgIdAndTxt( "MATLAB:pointstruct:fieldEmpty","Above field is empty!");
    }
    datapr = mxGetPr(field_array_ptr);
    nr = mxGetM(field_array_ptr);
    nc = mxGetN(field_array_ptr);
    if (strcmp(field_name,"x")==0) {
        point->set_x((F64) *(datapr + next));
    }
    else if (strcmp(field_name,"y")==0) {
        point->set_y((F64) *(datapr + next));        
    }
    else if (strcmp(field_name,"z")==0) {
        point->set_z((F64) *(datapr + next));
    }
    else if (strcmp(field_name,"intensity")==0) {
        point->intensity = (U16) *(datapr + next);
    }
    else if (strcmp(field_name,"return_number")==0) {
        point->return_number = (U8) *(datapr + next);
    }
    else if (strcmp(field_name,"number_of_returns")==0) {
        point->number_of_returns = (U8) *(datapr + next);
    }
    else if (strcmp(field_name,"scan_direction_flag")==0) {
        point->scan_direction_flag = (U8) *(datapr + next);
    }
    else if (strcmp(field_name,"edge_of_flight_line")==0) {
        point->edge_of_flight_line = (U8) *(datapr + next);
    }
    else if (strcmp(field_name,"classification")==0) {
        point->classification = (U8) *(datapr + next);
    }
    else if (strcmp(field_name,"scan_angle_rank")==0) {
        point->scan_angle_rank = (U8) *(datapr + next);
    }
    else if (strcmp(field_name,"user_data")==0) {
        point->user_data = (U8) *(datapr + next);
    }
    else if (strcmp(field_name,"point_source_ID")==0) {
        point->point_source_ID = (U16) *(datapr + next);
    }
    else if (strcmp(field_name,"gps_time")==0) {
        point->gps_time = *(datapr + next);        
    }
    else if (strcmp(field_name,"rgb")==0) {
        int M = (point->have_nir ? 4 : 3);
        for (int i=0; i<M; i++) {
            point->rgb[i] = (U16) *(datapr + i*header->number_of_point_records + next);
        }
    }

    else if (strcmp(field_name,"attributes")==0) {
        int ind;
        for (ind=0; ind<header->number_attributes; ind++) {
            extranext = ind*header->number_of_point_records + next;
            put_attributes(header,point,datapr,extranext,ind);
        }
    }

}


void copy_point_arr_mex(LASheader *header, LASpoint *point, const mxArray *pnt, int next)
{

    int number_of_fields, field_index;
    mwSize total_num_of_elements;
    const char  *field_name;
    mwIndex index;
    total_num_of_elements = mxGetNumberOfElements(pnt);
    number_of_fields = mxGetNumberOfFields(pnt);
    /* Walk through each structure element. */
    for (index=0; index<total_num_of_elements; index++)  {
        /* For the given index, walk through each field. */
        for (field_index=0; field_index<number_of_fields; field_index++)  {
            field_name = mxGetFieldNameByNumber(pnt,field_index);
            copy_field(header,point,pnt,next,index,field_index,field_name);
        }   
        
    }
}


