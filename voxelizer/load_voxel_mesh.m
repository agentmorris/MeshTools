%
% function [header, voxels] = load_voxel_mesh(filename, read_border);
%
% Written by Chris Sewell and Dan Morris, 2006.
%
% Loads a voxel mesh as output by the voxelizer program.
%
% filename: the .voxels file you want to read
%
% header: a struct containing metadata about the voxel file
%
% voxels: a struct containing voxel data as N-length vectors, where N
% is the number of voxels in the file.  The most important elements in
% this struct are x, y, and z, which represent the positions of each voxel.
%
% Example usage (uses the default filename generated if you download voxelizer,
% run it, and click "voxelize"):
%
% [header,voxels] = load_voxel_mesh('dragon_vrip_res3.ply.v80.voxels');
% plot3(voxels.x,voxels.y,voxels.z,'.');
%

function [header, voxels] = load_voxel_mesh(filename);

fid = fopen(filename,'r');

if (fid == -1)
  fprintf(1,'Error opening file %s\n',filename);
  return;
end

fprintf(1,'Loading %s...\n',filename);

header = struct;

header.header_size = fread(fid,1,'int32');
header.num_objects = fread(fid,1,'int32');
header.object_header_size = fread(fid,1,'int32');
header.voxel_struct_size = fread(fid,1,'int32');

header.num_voxels = fread(fid,1,'int32');
header.voxel_resolution = fread(fid,3,'int32');
header.voxel_size = fread(fid,3,'float32');
header.model_scale_factor = fread(fid,1,'float32');
header.model_offset = fread(fid,3,'float32');
header.zero_coordinate = fread(fid,3,'float32');
header.has_texture = fread(fid,1,'uchar');

namestr = fread(fid,260,'uint8')';
null_index = min(find(namestr==0));
namestr = namestr(1:null_index-1);
header.texture_filename = char(namestr);

NUM_MODIFIERS = 5;
NUM_COLUMNS = 17 + NUM_MODIFIERS*4;
DATA_ALLOCATION_CHUNK = 100000;
data = zeros(DATA_ALLOCATION_CHUNK,NUM_COLUMNS);

i = 0;
resolution_product = header.voxel_resolution(1)*header.voxel_resolution(2)*header.voxel_resolution(3);

for (c=1:resolution_product)
    
  exists = fread(fid,1,'uchar');
   
  if (exists == '2')    
    i = i+1; 
    
    % Allocate more space if necessary
     if (i > size(data,1))
       data = [data; zeros(DATA_ALLOCATION_CHUNK,NUM_COLUMNS)];
     end 
    
     data(i,1) = fread(fid,1,'int16');  
     data(i,2) = fread(fid,1,'int16');
     data(i,3) = fread(fid,1,'int16');
     
     data(i,4) = fread(fid,1,'uchar');
  
     data(i,5) = fread(fid,1,'float32');
     data(i,6) = fread(fid,1,'float32');
   
     data(i,7) = fread(fid,1,'uchar');
   
     data(i,8) = fread(fid,1,'float32');
     data(i,9) = fread(fid,1,'float32');
     data(i,10) = fread(fid,1,'float32');
   
     data(i,11) = fread(fid,1,'uchar');
   
     data(i,12) = fread(fid,1,'float32');
   
     data(i,13) = fread(fid,1,'float32');
     data(i,14) = fread(fid,1,'float32');
     data(i,15) = fread(fid,1,'float32');
   
     data(i,16) = fread(fid,1,'uchar');
     
     for j=1:NUM_MODIFIERS 
         data(i,16+j) = fread(fid,1,'float32');
     end
       
     for j=1:NUM_MODIFIERS*3
         data(i,16+NUM_MODIFIERS+j) = fread(fid,1,'float32');
     end
     
     data(i,16+4*NUM_MODIFIERS+1) = fread(fid,1,'uchar');
         
  end

  if (mod(c,floor(resolution_product/10))==0)
    fprintf(1, '%.1f%% done...\n', 100.0*c/resolution_product);
  end
  
end

data = data(1:i,:);

voxels = struct;
voxels.i = data(:,1);
voxels.j = data(:,2);
voxels.k = data(:,3);
voxels.x = (voxels.i - header.voxel_resolution(1)/2) .* header.voxel_size(1);
voxels.y = (voxels.j - header.voxel_resolution(2)/2) .* header.voxel_size(2);
voxels.z = (voxels.k - header.voxel_resolution(3)/2) .* header.voxel_size(3);
header.global_pos = header.zero_coordinate+header.voxel_size./2.0 + header.voxel_size.*header.voxel_resolution./2.0;
voxels.global_x = header.global_pos(1) + voxels.x;
voxels.global_y = header.global_pos(2) + voxels.y;
voxels.global_z = header.global_pos(3) + voxels.z;
voxels.has_texture = data(:,4);
voxels.u = data(:,5);
voxels.v = data(:,6);
voxels.has_normal = data(:,7);
voxels.normal = data(:,8:10);
voxels.has_distance = data(:,11);
voxels.distance_to_surface = data(:,12);
voxels.distance_gradient = data(:,13:15);
voxels.num_modifiers = data(:,16);
voxels.distance_to_modifier = data(:,17:16+NUM_MODIFIERS);
voxels.modifier_gradient = data(:,16+NUM_MODIFIERS+1:16+4*NUM_MODIFIERS);
voxels.is_on_border = data(:,16+4*NUM_MODIFIERS+1);
voxels.index = voxels.i * (header.voxel_resolution(2) * header.voxel_resolution(3)) + voxels.j * header.voxel_resolution(3) + voxels.k;

fclose(fid);
