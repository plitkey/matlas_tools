# -*- coding: utf-8 -*-

"""
Created on Wed Feb 26 11:58:47 2014
  FILE:  copy_change_files.py

  CONTENTS:
    Helper function to make changes in LASlib cpp and hpp files for use in 
    Windows with Matlab mex

  COPYRIGHT:

    (c) 2014, Finnish Geodetic Institute

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    18 March 2014 -- First matlas_tools version

@author: paula.litkey@fgi.fi - www.fgi.fi
"""
import os
import shutil
def insert_line(olines,key,addline) :
    # inserts line after the keyline                       
    for line in olines :
        if key in line :
            olines.insert(olines.index(line)+1,"\r\n")
            olines.insert(olines.index(line)+2,addline)
            break

def change_line(olines,key,new_word,one_only) :
    for line in olines :
        if key in line :
            olines[olines.index(line)] = line.replace(key,new_word)
            if one_only :
                break
                    
def mkw_dirs(wdir0) :
    wdirh = os.path.join(wdir0,"inc")
    if not os.path.exists(wdirh) :
        os.makedirs(wdirh)
    #print(wdirh)
    wdirs = os.path.join(wdir0,"src")
    if not os.path.exists(wdirs) :
        os.makedirs(wdirs)
    #print(wdirs)
    return wdirh, wdirs
    
def copy_nochange(laszpath,writedh,writeds) :
    
    cdir = os.path.join(laszpath, "LASzip","dll")
    shutil.copy(os.path.join(cdir,"laszip_dll.h"), writedh)
    shutil.copy(os.path.join(cdir,"laszip_dll.c"), writeds)
    cdir = os.path.join(laszpath,"src")
    shutil.copy(os.path.join(cdir,"geoprojectionconverter.hpp"), writedh)
    shutil.copy(os.path.join(cdir,"geoprojectionconverter.cpp"), writeds)

def make_changes_dll(laztpath,wdirh,wdirs) :
    import datetime

    #laztpath = "/home/pli/matlab_work/matlas1/lastools/"
    filelist_add_hd = ["lasdefinitions.hpp", "mydefs.hpp", "laszip.hpp"]
    cdirs = [os.path.join(laztpath, "LASzip","src"), os.path.join(laztpath,"LASlib","src"), os.path.join(laztpath,"LASlib","inc")]

    #wdir0 = "/home/pli/python_kokeita/dll_stabbed/"
    
    clkeyword = "class "
    dllkeyword = "class LASZIP_DLL " 
    nword = "class CPL_DLL "
    stabline = "    " + datetime.datetime.now().strftime("%d %b %Y") + " -- FGI matlas tools, source modified for matlas compilation in Windows"
    mykeyword = "#if defined(_MSC_VER) && (_MSC_VER < 1300)" 
    mynword = "#if defined(_MSC_VER)"
 
    for cdir in cdirs :
        listing = os.listdir(cdir)
        for cur_file in listing :
            cfile = os.path.join(cdir,cur_file)
            #print(cfile)  
            if cur_file.endswith(".hpp") or cur_file.endswith(".cpp") :                
                with open(cfile, "r") as f :
                    olines = f.readlines()
                if cur_file.endswith(".hpp") :
                    wdir = wdirh
                elif cur_file.endswith(".cpp") :
                    wdir = wdirs

                if any(dllkeyword in s for s in olines) or any(clkeyword in s for s in olines) \
                        or (cur_file in filelist_add_hd) or "mydefs.hpp" in cur_file :
                    
                    if cur_file in filelist_add_hd :
                        #insert_line(olines,"*/",'#include "export_symb.h"\r\n')
                        insert_line(olines,"*/",'#include "export_symb.h"\n')
                    if "mydefs" in cur_file :
                        change_line(olines,mykeyword,mynword,True)
                    if any(dllkeyword in s for s in olines) :
                        change_line(olines,dllkeyword,nword,False)
                    elif any(clkeyword in s for s in olines) :
                        change_line(olines,clkeyword,nword,False)
                    insert_line(olines,"CHANGE HISTORY",stabline)
                    print("Changing file: " + cur_file)
                    #print(cfile)

                    wfile = os.path.join(wdir,cur_file)
                    with open(wfile, "w") as fw :
                        fw.writelines(olines)
                                
                else:
                    shutil.copy(cfile, wdir)

    gfile = os.path.join(wdirh,"geoprojectionconverter.hpp")
    with open(gfile, "r") as f :
        olines = f.readlines()
    insert_line(olines,"*/",'#include "export_symb.h"\n')    
    change_line(olines,clkeyword,nword,False)
    insert_line(olines,"CHANGE HISTORY",stabline)
    with open(gfile, "w") as fw :
        fw.writelines(olines)
    gfile = os.path.join(wdirs,"geoprojectionconverter.cpp")
    with open(gfile, "r") as f :
        olines = f.readlines()
    change_line(olines,clkeyword,nword,False)
    insert_line(olines,"CHANGE HISTORY",stabline)
    with open(gfile, "w") as fw :
        fw.writelines(olines)
        
def make_changes_sta(laztpath,wdirh,wdirs):
    import datetime
    cdirs = [os.path.join(laztpath, "LASzip","src"), os.path.join(laztpath,"LASlib","src"), os.path.join(laztpath,"LASlib","inc")]
        
    stabline = "    " + datetime.datetime.now().strftime("%d %b %Y") + " -- FGI matlas tools, source modified for matlas compilation in Windows"
    mykeyword = "#if defined(_MSC_VER) && (_MSC_VER < 1300)" 
    mynword = "#if defined(_MSC_VER)"

    for cdir in cdirs:
        listing = os.listdir(cdir)  
    
        for cur_file in listing:
            cfile = os.path.join(cdir,cur_file)
            if cdir == cdirs[0] and "mydefs.hpp" in cur_file :
                with open(cfile, "r") as f:
                    olines = f.readlines()                    
                change_line(olines,mykeyword,mynword,True)
                insert_line(olines,"CHANGE HISTORY",stabline)
                print("Changing file: " + cur_file)                        
                wfile = os.path.join(wdirh,cur_file)
                with open(wfile, "w") as fw :
                    fw.writelines(olines)
                                
            elif cur_file.endswith(".hpp") : 
                shutil.copy(cfile, wdirh)
            elif cur_file.endswith(".cpp") :
                shutil.copy(cfile, wdirs)
       


def main():
    import sys
    statonly = False
    if len(sys.argv) < 2 :
        print("Usage: copy_change_files.py lastoolspath opt")
        print("Static changes example : copy_change_files.py C:\mydir\lastools 1")
        print("dynamic changes example: copy_change_files.py C:\mydir\lastools 0")
        sys.exit(1)

    lastoolsdir = sys.argv[1]               
    if not os.path.exists(lastoolsdir) :
        print("Lastools directory not valid!")
        sys.exit(1)
            
    if len(sys.argv) >2 :
        if int(sys.argv[2])==1 :
            statonly = True

    if statonly:
        print("Making stat changes\n")
        writedir = os.path.join(os.getcwd(),"stat") #sys.argv[2]
        if not os.path.exists(writedir) :
            os.makedirs(writedir)
        writedh,writeds = mkw_dirs(writedir)
        copy_nochange(lastoolsdir,writedh,writeds)
        make_changes_sta(lastoolsdir,writedh,writeds)
    else:
        print("Making dll changes\n")
        writedir = os.path.join(os.getcwd(),"dll") #sys.argv[2]
        if not os.path.exists(writedir) :
            os.makedirs(writedir)
        writedh,writeds = mkw_dirs(writedir)
        copy_nochange(lastoolsdir,writedh,writeds)
        shutil.copy(os.path.join(os.getcwd(),"export_symb.h"), writedh)
        make_changes_dll(lastoolsdir,writedh,writeds)        
                
if __name__ == "__main__" :
    main()

