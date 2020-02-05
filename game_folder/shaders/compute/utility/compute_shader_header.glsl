#version 430
#define group_size 8
#define block_size 64

layout(local_size_x = group_size, local_size_y = group_size) in;

