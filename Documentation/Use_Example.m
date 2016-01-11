%   CONTENTS:
% 
%    Example use of matlas_tools gateway to LASlib, the example expects the
%    user is working in Matlab and the current directory (/matlas_tools/Documentation)
% 
%   PROGRAMMERS:
% 
%     paula.litkey@fgi.fi, eetu.puttonen@fgi.fi - www.fgi.fi
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
%     24 March 2014 -- First public matlas_tools version
%     11 January 2016 -- Version with waveform read
%

%% Set example folders

% This folder
def_folder = pwd;
data_folder = fullfile(pwd,'Example_files');
disp(['Resulting files will be written in ',data_folder,char(10)])

% LasTools folder on computer
lastools_folder = [];% e.g. "fullfile('Mydrive:','MyData','lastools')"

% Binary folder (matlas_tools)
if strfind(computer('arch'),'win64')
    % Default location of binaries in Windows
    run_folder = fullfile('..','OS_Win','Binaries','Win64');
elseif strfind(computer('arch'),'win32')
    % Default location of binaries in Windows
    run_folder = fullfile('..','OS_Win','Binaries','Win32');
else 
    % Default location of binaries in Linux (AFTER configuring and running
    % both the make and the compilation files in directory '../OS_Lin/')
    run_folder = fullfile('..','matlas_source');
end

cd(run_folder);

%% Check that the lastools path is correct and the example file exists

% Error string
err_str = 'FILE NOT FOUND. Check that your LasTools path is correct! To see how to write .las files from scratch, go to example 6. \n';

housename = fullfile(lastools_folder,'data','house.laz');
if ~exist(housename,'file')
    fprintf(1,err_str);
    cd(def_folder); % Return to Documentation folder
    return
end

%% Example 1: read and plot .las file

parsestr = ['-i ' housename];
[hdr0,str0] = las2mat(parsestr);
hdr0 % View header structure
str0 % View point structure

figure(1);
hold on
plot3(str0.x,str0.y,str0.z,'k.','MarkerSize',1)
axis('equal')

%% Example 2: usage of lastools filtering options
parsestr2 = ['-i ' housename ' -drop_middle'];
[hdr01,str01] = las2mat(parsestr2);

figure();
plot3(str01.x,str01.y,str01.z,'.','MarkerSize',1)
hold on % plot the filtered points just to check 

% Clip circle from data
parsestr22 = ['-i ' housename ' -keep_circle 309250 6143470 10'];
[hdr02,str02] = las2mat(parsestr22);

% Plot the clipped area on top of other data
plot3(str02.x,str02.y,str02.z,'c.','MarkerSize',1)
axis('equal')

%% Example 3: Read several datasets into the same structure with 'list of files' option

lakename = fullfile(lastools_folder,'data','lake.laz');
if ~exist(lakename,'file')
    fprintf(1,err_str);
    cd(def_folder); % Return to Documentation folder
    return
end

fusaname = fullfile(lastools_folder,'data','fusa.laz');
if ~exist(fusaname,'file')
    fprintf(1,err_str);
    cd(def_folder); % Return to Documentation folder
    return
end

% Create list of files from example datasets
wf1 = fullfile(data_folder,'test1.txt');
fid = fopen(wf1,'wt');
pp = pwd; % current path
fprintf(fid,'%s\n',housename);
fprintf(fid,'%s\n',lakename);
fprintf(fid,'%s\n',fusaname);
fclose(fid);

% Read the list of files
parsestr3 = ['-lof ',fullfile(data_folder,'test1.txt'),' -merged'];
[hdr1,str1] = las2mat(parsestr3);

% Plot results
figure();
plot3(str1.x,str1.y,str1.z,'.','MarkerSize',1);
% the figure should show three line like things quite far away from
% eachother
%% Example 4: Edit point structure and write it into a new .las file

% Edit header of the Example 1 (run first)
hdr0.system_identifier = 'MATLAB fiddling 12122013';

% Plot intensity histogram
figure; 
clf;
hist(str0.intensity,100);

% Print point classes with high intensity
str0.classification(str0.intensity>1e4)

% Print different classes present in this data
unique(str0.classification)

% Assign new class for high intensity points
str0.classification(str0.intensity>1e4) = 7; % 7: withheld point/low noise point
str0.classification(str0.intensity>1e4)
cl7 = find(str0.classification==7); % there were no other class 7 points

% Plot newly assigned class on top of other data
figure(1); % Figure from Example 1
plot3(str0.x(cl7),str0.y(cl7),str0.z(cl7),'r.');
view([-37.5 30])
% There it is, unidentified highly reflective tiny object.

% After modification, save data into a new file
temp_file = fullfile(data_folder,'house_edited1.laz');
parsestring_out_hdr = ['-o ',temp_file];
mat2las(str0,hdr0,parsestring_out_hdr);

% Example of save without header (this is meant to be used if the data was
% not in las format previously (there is no read header available)
temp_file = fullfile(data_folder,'house_edited2.laz');
parsestring_out_no_hdr = ['-o ',temp_file];
mat2las(str0,parsestring_out_no_hdr);

% Read the saved files to check if this works
[h01,s01] = las2mat(['-i ',fullfile(data_folder,'house_edited1.laz')]);
[h02,s02] = las2mat(['-i ',fullfile(data_folder,'house_edited2.laz')]);
h01
h02

% Notice the difference in the headers!
hdr01.variable_length_records

%% Example 5: Adding of user defined extra bytes to a point structure

%[hdr0,str0] = las2mat(['-i ' housename]);

% Creation of metadata for two user defined extra bytes
attributes(1).name = 'random1';
attributes(1).type = 9;
attributes(1).description = 'example1';
attributes(1).scale = 0.0001;
attributes(1).offset = 0;

attributes(2).name = 'random2';
attributes(2).type = 9;
attributes(2).description = 'example2';
attributes(2).scale = 0.1;
attributes(2).offset = 0;

% Add the above information of attributes to the data structure
% NOTE: This step is required for extra byte writing with NO header!
str0.attribute_info = attributes;

% Add (random) extra data to the point data structure
tmp = rand(1,size(str0.x,1));
str0.attributes = [tmp; tmp];

temp_file = fullfile(data_folder,'house_extra.laz');
parsestring_extra = ['-o ',temp_file];
mat2las(str0,parsestring_extra);

% Read the written file and compare with the original data
[h02,s02] = las2mat(['-i ', fullfile(data_folder,'house_extra.laz')]);
h02
s02
sum(s02.attributes(:) - str0.attributes(:))
% here the user fell to the sin of too high precision, but hopefully the real 
% user is better aware of the extra attirbute data types and scaling 

%% These examples show a few examples of writing new .las files from scratch in MATLAB
% -ep 2013

%% Example 6: Create a new .las file with minimum information

% A -----------------------------------------------------------------------

% Create minimum point structure required for writing ---------------------
% Minimum requirement is to have x,y and z fields

estr6 = struct(...
    'x', rand(100000,1)*3000, ...
    'y', rand(100000,1)*5000, ...
    'z', rand(100000,1)*100, ...
    'intensity', rand(100000,1));
% Write
mat2las(estr6,['-o ',fullfile(data_folder,'Write_las_example_6a.las')])
% Read
[thdr_1a,tstr_1a] = las2mat(['-i ',fullfile(data_folder,'Write_las_example_6a.las')]);

% B -----------------------------------------------------------------------

% Create basic header structure -------------------------------------------
% In this example, the scale is changed from the previous eexample. If there is no 
% additional information in the header, it is better not to use it. 
ehdr6 = struct(...
    'file_signature', 'LASF', ...
    'header_size', 227, ...
    'point_data_format', 0, ...
    'point_data_record_length', 36, ...
    'number_of_point_records', 100000, ...
    'number_of_points_by_return', [100000 0 0 0 0], ...
    'x_scale_factor', 1.0000e-04, ...
    'y_scale_factor', 1.0000e-04, ...
    'z_scale_factor', 1.0000e-04, ...
    'x_offset', 0, ...
    'y_offset', 0, ...
    'z_offset', 0, ...
    'user_data_after_header_size', 0, ...
    'min_x', min(estr6.x), ...
    'min_y', min(estr6.y), ...
    'min_z', min(estr6.z), ...
    'max_x', max(estr6.x), ...
    'max_y', max(estr6.y), ...
    'max_z', max(estr6.z) ...
    );
% Write
mat2las(estr6,ehdr6,['-o ',fullfile(data_folder,'Write_las_example_6b.las')])
% Read
[thdr_6b,tstr_6b] = las2mat(['-i ',fullfile(data_folder,'Write_las_example_6b.las')]);

%% Example 7: Create a .las file with a more completed header structure

% The main idea is to write with header, if you have already loaded the
% data and there is a header ready. 

% Create basic header structure -------------------------------------------
ehdr7 = struct(...
    'file_signature', 'LASF', ...
    'file_source_id', 0, ...
    'global_encoding', 0, ...
    'project_ID_GUID_data_1', 0, ... 
    'project_ID_GUID_data_2', 0, ...
    'project_ID_GUID_data_3', 0, ...
    'project_ID_GUID_data_4', [0 0 0 0], ...
    'version_major', 1, ...
    'version_minor', 2, ...
    'system_identifier', '', ...
    'generating_software', 'Matlab', ...
    'file_creation_day', 268, ...
    'file_creation_year', 2013, ...
    'header_size', 227, ...
    'offset_to_point_data', 227, ...
    'number_of_variable_length_records', 0, ...
    'point_data_format', 0, ...
    'point_data_record_length', 36, ...
    'number_of_point_records', 100000, ...
    'number_of_points_by_return', [100000 0 0 0 0], ...
    'x_scale_factor', 1.0000e-04, ...
    'y_scale_factor', 1.0000e-04, ...
    'z_scale_factor', 1.0000e-04, ...
    'x_offset', 0, ...
    'y_offset', 0, ...
    'z_offset', 0, ...
    'user_data_after_header_size', 0, ...
    'min_x', 0.0000, ...
    'min_y', 1.0000, ...
    'min_z', 0.0000, ...
    'max_x', 1.0000, ...
    'max_y', 0.0000, ...
    'max_z', 1.0000 ...
    );

% Create basic point structure --------------------------------------------

estr7 = struct(...
    'x', rand(100000,1), ...
    'y', rand(100000,1), ...
    'z', rand(100000,1), ...
    'intensity', rand(100000,1), ...
    'return_number', ones(100000,1), ...
    'number_of_returns', ones(100000,1), ...
    'scan_direction_flag', zeros(100000,1), ...
    'edge_of_flight_line', zeros(100000,1), ...
    'classification', zeros(100000,1), ...
    'scan_angle_rank', zeros(100000,1), ...
    'user_data', zeros(100000,1), ...
    'point_source_ID', zeros(100000,1) ...
    );

% OPTIONAL: Add possible extra information

% RGB ---------------------------------------------------------------------



% RGB ends ----------------------------------------------------------------

% extra attributes --------------------------------------------------------
% Available datatypes U8 I8 U16 I16 U32 I32 F32 F64 with corresponding type 
% codenumbers [1,2,3,4,5,6,9,10] 
        
ea_temp_hdr(1,1) = struct(...
    'name', 'Extra attribute 1', ...
    'type', 4, ... 
    'description', 'The first example attribute', ...
    'scale', 0.01, ...
    'offset', 0 ...
    );

ea_temp_hdr(1,2) = struct(...
    'name', 'Extra attribute 2', ...
    'type', 4, ...
    'description', 'The second example attribute', ...
    'scale', 0.01, ...
    'offset', 0 ...
    );

% Add attributes in the main header 
ehdr7.number_attributes = length(ea_temp_hdr);
ehdr7.attributes = ea_temp_hdr;
ehdr7.attribute_array_offsets =  [0 2]; % 
% Add attributes in the point structure
estr7.attributes = rand(100000,2);
% extra attributes end ----------------------------------------------------

% Write data on disk ------------------------------------------------------
mat2las(estr7,ehdr7,['-o ',fullfile(data_folder,'Write_las_example_7.las')])

% Validate results --------------------------------------------------------
[thdr7,tstr7] = las2mat(['-i ',fullfile(data_folder,'Write_las_example_7.las')]);
rmse7_xyz =  [sqrt(sum((estr7.x - tstr7.x).^2,1)./size(estr7.x,1)), ...
             sqrt(sum((estr7.x - tstr7.x).^2,1)./size(estr7.x,1)), ...
             sqrt(sum((estr7.x - tstr7.x).^2,1)./size(estr7.x,1))]

if isfield(estr7,'attributes')
    rmse7_extra = sqrt(sum((estr7.attributes-tstr7.attributes).^2,1)./size(estr7.attributes,1))
end
         
%% Example 8: Create a .las file without a header structure 

% Create a basic point structure ------------------------------------------

estr8 = struct(...
    'x', rand(100000,1), ...
    'y', rand(100000,1), ...
    'z', rand(100000,1), ...
    'intensity', rand(100000,1), ...
    'return_number', ones(100000,1), ...
    'number_of_returns', ones(100000,1), ...
    'scan_direction_flag', zeros(100000,1), ...
    'edge_of_flight_line', zeros(100000,1), ...
    'classification', zeros(100000,1), ...
    'scan_angle_rank', zeros(100000,1), ...
    'user_data', zeros(100000,1), ...
    'point_source_ID', zeros(100000,1) ...
    );

% OPTIONAL: Add possible extra information

% extra attributes --------------------------------------------------------

% NOTE! To add extra attributes without a header structure, the extra
% attribute information must be included in point structure

ea_hdr_info(1,1) = struct(...
    'name', 'Extra attribute 1', ...
    'type', 4, ... % Available numbers [1,2,3,4,5,6,9,10]
    'description', 'The first example attribute', ...
    'scale', 0.01, ...
    'offset', 0 ...
    );

ea_hdr_info(1,2) = struct(...
    'name', 'Extra attribute 2', ...
    'type', 4, ... % Available numbers [1,2,3,4,5,6,9,10]
    'description', 'The second example attribute', ...
    'scale', 0.01, ...
    'offset', 0 ...
    );

% Add the above information of the extra attributes to the data structure
estr8.attribute_info = ea_hdr_info;

% Add (random) extra data to the point data structure
estr8.attributes = rand(size(estr8.x,1),2);

% extra attributes ends ---------------------------------------------------

% Write data on disk ------------------------------------------------------
mat2las(estr8,['-o ', fullfile(data_folder,'Write_las_example_8.las')])

% Validate results --------------------------------------------------------
[thdr8,tstr8] = las2mat(['-i ',fullfile(data_folder,'Write_las_example_8.las')]);

% NOTE! Scaling options default to 0.01 precision if not given during writing!
rmse8_xyz =  [sqrt(sum((estr8.x - tstr8.x).^2,1)./size(estr8.x,1)), ...
             sqrt(sum((estr8.x - tstr8.x).^2,1)./size(estr8.x,1)), ...
             sqrt(sum((estr8.x - tstr8.x).^2,1)./size(estr8.x,1))]
         
if isfield(estr8,'attributes')         
    rmse8_extra = sqrt(sum((estr8.attributes-tstr8.attributes).^2,1)./size(estr8.attributes,1))
end        

% Write data on disk, add coordinate precision manually with the option 'fgi_scale'

mat2las(estr8,['-o ',fullfile(data_folder,'Write_las_example_8b.las'),' -fgi_scale 0.0001 0.0001 0.0001'])

% Validate results --------------------------------------------------------
[thdr8b,tstr8b] = las2mat(['-i ',fullfile(data_folder,'Write_las_example_8b.las')]);

rmse8b_xyz =  [sqrt(sum((estr8.x - tstr8b.x).^2,1)./size(estr8.x,1)), ...
              sqrt(sum((estr8.x - tstr8b.x).^2,1)./size(estr8.x,1)), ...
              sqrt(sum((estr8.x - tstr8b.x).^2,1)./size(estr8.x,1))]
         
if isfield(estr8,'attributes')         
    rmse8b_extra = sqrt(sum((estr8.attributes-tstr8b.attributes).^2,1)./size(estr8.attributes,1))
end

%% Return to the original folder
cd(def_folder)

%% Waveform read: CAUTION waveform in las format is many times not ok according to the LAS specification
% for example, waveform might be save for each extracted point, and as a result, the same wavepacket is recorded 
% multiple times, the Xt,Yt,Zt anchor, however is typically the first return, using any other return as the anchor
% will result in oddities.
% 
% use only small plots, large areas will crash Matlab 
%
% the str structure will have fields wavepacket and wavepacket_info and the header has field wave_packet_descriptors
%
%
% example of plotting the waveform and the system extracted points
%
xyz(:,1) = str.x; %-min(str.x); % Scale coordinates to easier values
xyz(:,2) = str.y; %-min(str.y);
xyz(:,3) = str.z;
plot3(xyz(:,1),xyz(:,2),xyz(:,3),'.');
hold on;
XYZT(:,1) = str.wave_pos_packet_fields.Xt;
XYZT(:,2) = str.wave_pos_packet_fields.Yt;
XYZT(:,3) = str.wave_pos_packet_fields.Zt;
index = str.wave_pos_packet_fields.Index;
location = str.wave_pos_packet_fields.Location(ids);
for i = 1:size(str.wavepacket,1)
    wf_vec = double(str.new_wavepacket(ii,:));
    if ~isempty(wf_vec),
       wf_index = index(ii);
       if i>1 && length(wf_vec)==length(wf_vec_old) && all(wf_vec_old == wf_vec), % same wf recorded for all extracted points!
          fprintf(1,'Skipping wf %d\n',i);
          continue;
       end
                
       if wf_index == 0,
          %keyboard;
          timestep = 1000; 
       else
          timestep = double(str.wf_header.wave_packet_descriptors(wf_index).TemporalSpacing);
       end
                
       zwf = find(wf_vec,1,'last');
       wf_vec = wf_vec(1:zwf);
       wftime = 0:timestep:length(wf_vec)*timestep-1;
       wftime = wftime*.5; % distance times 2 -> time*.5 !! check if location prints ok !!
       tmptime = (repmat(wftime',1,3) - location(i)); %*.5; % time halving here!! if above not ok try this!!!
       wf1 = tmptime .* repmat(XYZT(i,:),length(tmptime),1); % displacement from anchor point
       wf0 = ones(size(wftime,2),1)*xyz(i,:); % anchor point
       %keyboard;
       if sum(wf_vec)~=0,
           newPoints = wf_vec > NOISELEVEL;
           npnt = wf0(newPoints,:)-wf1(newPoints,:); % position of the wf units % minus!!! 02.02.11!!!
           if ~isempty(npnt) && length(npnt) > 2, % larger dim at least 3
		plot3(npnt(:,1),npnt(:,2),npnt(:,3),'.','MarkerSize',1)
           end
       end
    end
 end
