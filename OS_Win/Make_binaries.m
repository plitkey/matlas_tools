function Make_binaries(LibOpt)
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This is a make file to compile las2mat and mat2las binaries from precompiled libraries.
% Run the file in this directory after copying.
% In case of errors, please consult the document files in "Documentation" folder.
%
% - ep, 2013
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Directories, files, and compiler options
SourceDir  = ['..\matlas_source\'];
OutPutDir =  ['.\Binaries'];
% Header directory depends on linking (static / dynamic), see lines below

% HeaderDir  = ['-I.\Code\SrcStat\LIBinc -I.\Code\SrcStat\ZIPsrc'];

% Check computer architecture
CA = computer('arch');
if strcmp(CA,'win64')
    LibraryDir = ['-L.\Libraries\Win64'];
    opts = ['-O -largeArrayDims -outdir ',OutPutDir,'\Win64'];
elseif strcmp(CA,'win32')
    LibraryDir = ['-L.\Libraries\Win32'];
    opts = ['-O -outdir ',OutPutDir,'\Win32'];
else
    error('Non-Windows architecture detected. Compilation aborted.')
end
    
RdrFile = 'las2mat.cpp';
WrtFile = 'mat2las.cpp';

%% Erase old binaries from the 'Binaries' directory
if exist('.\Binaries\','dir')
    delete .\Binaries\*.*
else
    error('''Binaries'' folder missing! Aborting compilation.')
end

%% Library linking

if nargin < 1; LibOpt = 'static'; end

if strcmp(LibOpt,'static')
    % Static linking
    libfile = '-lStatLIB';
    HeaderDir  = ['-I.\Src2Lib\stat\inc\'];
elseif strcmp(LibOpt,'dynamic')
    % Dynamic linking
    libfile = '-lDynLIB';
    HeaderDir  = ['-I.\Src2Lib\dll\inc\'];
    if strcmp(CA,'win64')
        system('copy .\Libraries\Win64\DynLIB.dll .\Binaries\DynLIB.dll'); % Copy .dll into the binary directory to run it
    else
        system('copy .\Libraries\Win32\DynLIB.dll .\Binaries\DynLIB.dll'); % Copy .dll into the binary directory to run it
    end
    disp([char(13),'Library file DynLIB.dll was copied into .\Binaries'])
else
    error('Unknown linking option. Possible linking options are ''static'' and ''dynamic''.')
end

%% Compile

disp([char(13),'Computer architecture detected as... ', CA])
disp(['Selected linking option... ', LibOpt])

tic
disp([char(13),'Compiling reader into... ', OutPutDir])
eval(['mex ',opts,' ',SourceDir,RdrFile,' ',HeaderDir,' ',LibraryDir,' ',libfile])
disp(['Reader compiled in ',num2str(toc,'%.2f'),' seconds.',])

tic
disp(['Compiling writer into... ', OutPutDir])
eval(['mex ',opts,' ',SourceDir,WrtFile,' ',HeaderDir,' ',LibraryDir,' ',libfile])
disp(['Writer compiled in ',num2str(toc,'%.2f'),' seconds.',char(13)])