function[]=compile_matlas_tools()
%
%   CONTENTS:
% 
%    Compile las2mat.cpp and mat2las.cpp in linux if the lasTools is installed
%    in subfolder 'lastools' of the current directory (/matlas_tools)
% 
%   PROGRAMMERS:
% 
%     paula.litkey@fgi.fi - www.fgi.fi
% 
%   COPYRIGHT:
% 
%     (c) 2014, Finnish Geodetic Institute
% 
%     This is free software; you can redistribute and/or modify it under the
%     terms of the GNU Lesser General Licence as published by the Free Software
%     Foundation. See the COPYING file for more information.
% 
%     This software is distributed WITHOUT ANY WARRANTY and without even the
%     implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
% 
%   CHANGE HISTORY:
% 
%     18 March 2014 -- First public matlas_tools version
%

p = pwd;
liblibpath = [p '/lastools/LASlib/lib/'];
libincpath = [p '/lastools/LASlib/inc'];
zippath = [p '/lastools/LASzip/'];
srcpath = [p '/lastools/src'];
incl_string = [liblibpath 'liblas.a -I' libincpath ' -I' srcpath ' -I' zippath 'inc -I' zippath 'src'];
C1 = ['mex las2mat.cpp ' incl_string];
C2 = ['mex mat2las.cpp ' incl_string];
%fprintf(1,'%s\n',C1);
%fprintf(1,'%s\n',C2);
try
    eval(C1);
catch
    error('Check the paths and the instruction and the lastools version!\n');
        %keyboard;
end
%end
try
    eval(C2);
catch
    error('Check the paths and the instruction!\n');
    %keyboard;
end

