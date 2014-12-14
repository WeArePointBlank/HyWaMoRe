/*
	This reversing effort was made possible by Twili, someguy and myself. 

	Special thanks to DogPolice for code review.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "hywa.h"
#define IsValidSize(x, s, t) (!(x > s / sizeof(t)))

uint32
getUInt32(uint8 *data) {
	return 0;
}

float
getFloat(uint8 *data) {
	return 0.0;
}

uint8 *
getBytes(uint8 *data, uint bytes) {
	return NULL;
}

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
		group[i].sub_group = malloc(sizeof(SubGroup *)*group[i].sub_group_count);

		for(j = 0; j < group[i].sub_group_count; j++) {
			SubGroup *sub_group = malloc(sizeof(SubGroup));
			fread(&sub_group->metadata.name, 1, 8, fh);
			fread(&sub_group->metadata.size, 1, 4, fh);
			sub_group->data = malloc(sizeof(uint8)*sub_group->metadata.size - sizeof(GroupMetaData));
			fread(sub_group->data, 1, sub_group->metadata.size, fh);
			group[i].sub_group[i] = sub_group;
		}
	}
}

/* Read geometry group */
G1MG *
read_G1MG(Group *group) {
	uint32 offset = 0;
	G1MG *g1mg = malloc(sizeof(G1MG));
	uint8 *data = group[0].sub_group[3]->data;

	g1mg->metadata.name = getBytes(data+offset, 8);
	offset += 8;
	g1mg->metadata.size = getUInt32(data+offset);
	offset += 4;
	g1mg->platform = getBytes(data+offset, 8);
	offset += 0x8 + 0x18; //Skip 0x18 bytes since we don't know what they are.
	g1mg->sub_chunk_count = getUInt32(data+offset);
	offset += 0x4;

	SubChunk *sub_chunk = malloc(sizeof(SubChunk)*g1mg->sub_chunk_count);
	g1mg->sub_chunk = sub_chunk;
	int i;
	for(i = 0; i < g1mg->sub_chunk_count; i++) {
		sub_chunk[i].metadata.type = getUInt32(data+offset);
		offset += 0x4;
		sub_chunk[i].metadata.size = getUInt32(data+offset);
		offset += 0x4;
		offset += sub_chunk[i].metadata.size - 0x8;
	}

	return g1mg;
} 

VertexArray *
read_vertex_arrays(SubChunk *vertex_sub_chunk) {
	uint32 offset = 0;
	uint8 *data = vertex_sub_chunk->data;
	uint32 vertex_arrays = getUInt32(data+offset);
	offset += 0x4;
	uint32 _a = 0;
	offset += 0x4;
	uint32 vertex_element_size = getUInt32(data+offset);
	offset += 0x4;
	uint32 vertices = getUInt32(data);
	offset += 0x4;
	uint32 _b = 0;
	offset += 0x4;

	int i, j;
	VertexArray *vertex_array = malloc(sizeof(VertexArray)*vertex_arrays);
	for(i = 0; i < vertex_arrays; i++) {
		vertex_array->entry = malloc(sizeof(VertexEntry *)*vertex_entries);
		for(m = 0; m < vertex_entries; m++) {
			vertex_array->entry[m] = malloc(sizeof(VertexEntry));
			vertex_array->entry[m]->x = getFloat(data+offset);
			offset += 0x4;
			vertex_array->entry[m]->y = getFloat(data+offset);
			offset += 0x4;
			vertex_array->entry[m]->z = getFloat(data+offset);
			offset += 0x4;
			offset += vertex_array->vertex_entry->size - 0xC;
		}
	}
}

HyruleWarriorsModel *
read_model(char *file_path) {
	FILE *file_handle = fopen(file_path, "rb");

	FileHeader *file_header = read_file_header(fh);
	GroupMetaMetaData *groups_meta_meta_data = read_groups_meta_meta_data(fh, file_header->group_count);
	Group *groups = read_groups(fh, file_header->group_count);
	read_sub_groups(fh, groups, file_header->group_count);

	fclose(fh);

	G1MG *g1mg = read_G1MG(groups);
	VertexArray *vertex_arrays = read_vertex_arrays(g1mg->sub_chunk[3]);

	HyruleWarriorsModel *hyrule_warriors_model = malloc(sizeof(HyruleWarriorsModel));
	Geometry *geometry = malloc(sizeof(Geometry));
	hyrule_warriors_model->geometry->vertex_arrays = vertex_arrays;
	return hyrule_warriors_model;
}

char *
hy_wa_model_to_obj(HyruleWarriorsModel *hyrule_warriors_model) {
	return NULL;
}


int
main(int argc, char *argv[]) {
	HyruleWarriorsModel *hyrule_warriors_model = read_model(argv[1]);
	char *obj_text = hy_wa_model_to_obj(hyrule_warriors_model);

	return 0;
}





