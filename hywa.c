#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "hywa.h"

FileHeader *
read_file_header(FILE *fh) {
	FileHeader *file_header = malloc(sizeof(FileHeader));
	fread(&file_header->group_count, 1, 4, fh);

	return file_header;
}

GroupMetaMetaData *
read_groups_meta_meta_data(FILE *fh, uint32 group_count) {
	GroupMetaMetaData *group_meta_meta_data = malloc(sizeof(GroupMetaMetaData)*group_count);
	int i;
	for(i = 0; i < group_count; i++) {
		fread(&group_meta_meta_data[i].start, 1, 4, fh);
		fread(&group_meta_meta_data[i].size, 1, 4, fh);
	}

	return group_meta_meta_data;
}

Group *
read_groups(FILE *fh, uint32 group_count) {
	Group *group = malloc(sizeof(Group)*group_count);
	int i;
	for(i = 0; i < group_count; i++) {
		fseek(fh, group_meta_meta_data[i].start, SEEK_SET);
		GroupMetaData *metadata = malloc(sizeof(GroupMetaData));
		group[i].metadata = metadata;
		fread(&group[i].metadata->name, 1, 8, fh);
		fread(&group[i].metadata->size, 1, 4, fh);
		fread(&group[i].size, 1, 4, fh);
		fread(&group[i]._a, 1, 4, fh);
		fread(&group[i].sub_group_count, 1, 4, fh);
		group[i].sub_group_start = ftell(fh);
	}

	return group;
}

void
read_sub_groups(FILE *fh, Group *group, uint32 group_count) {
	int i, j;
	for(i = 0; i < group_count; i++) {
		fseek(fh, group[i].sub_group_start, SEEK_SET);
		group[i].subgroup = malloc(sizeof(SubGroup *)*group[i].sub_group_count);
		for(j = 0; j < group[i].sub_group_count; j++) {
			SubGroup *sub_group = malloc(sizeof(SubGroup));
			fread(&sub_group->metadata.name, 1, 8, fh);
			fread(&sub_group->metadata.size, 1, 4, fh);
			sub_group->data = malloc(sizeof(uint8)*sub_group->metadata.size - sizeof(GroupMetaData));
			fread(sub_group->data, 1, sub_group->metadata.size, fh);
		}
	}
}

/* Read geometry group */
void
read_G1MG(Group *group) {
	uint32 delta = 0;
	uint8 *data = &group[0].sub_group[3]->data[0x8+0x18];
	uint32 SchunkCount = getUInt32(data);
	delta += 0x4;
	int i;
	for(i = 0; i < SchunkCount; i++) {
		SubChunk *sub_chunk = malloc(sizeof(SubChunk));
		sub_chunk->metadata.type = getUInt32(data+delta);
		delta += 0x4;
		sub_chunk->metadata.size = getUInt32(data+delta);
		delta += 0x4;
		delta += sub_chunk->metadata.size - 0x8;
	}
} 

VertexArray *
read_vertex_array() {
	uint32 vertex_arrays = getUInt32(data);
	uint32 _a = 0;
	uint32 vertex_element_size = getUInt32(data);
	uint32 vertices = getUInt32(data);
	uint32 _b = 0;

	int i, j;
	VertexArray *vertex_array = malloc(sizeof(VertexArray)*vertex_arrays);
	for(i = 0; i < vertex_arrays; i++) {
		vertex_array->entry = malloc(sizeof(VertexEntry *)*vertex_entries);
		for(j = 0; j < vertex_entries; i++) {
			vertex_array->entry[i] = malloc(sizeof(VertexEntry));
			vertex_array->
		}
	}
}









