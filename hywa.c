/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "hywa.h"
#define IsValidSize(x, s, t) (!(x > s / sizeof(t)))

uint32
getUInt32(uint8 *data) {
	return *((uint32 *)data);
}

float
getFloat(uint8 *data) {
	return *((float *)data);
}

char *
getChar(uint8 *data, uint chars) {
	char *string = malloc(sizeof(char)*chars);
	int i;
	for(i = 0; i < chars; i++) {
		string[i] = (char)data[i];
	}
	return string;
}

uint
getVariableWidthUInt(uint8 *data, uint length_in_bits) {
	switch(length_in_bits) {
		case 0x8:
			return *data;
		case 0x10:
			return *((uint16 *)data);
		case 0x20:
			return *((uint32 *)data);
		default:
			return 0;
	}
	return 0;
}

FileHeader *
read_file_header(FILE *fh) {
	uint8 buffer[4];
	FileHeader *file_header = malloc(sizeof(FileHeader));
	fread(buffer, 1, 4, fh);
	file_header->group_count = *((uint32 *)buffer);
	return file_header;
}

GroupMetaMetaData *
read_groups_meta_meta_data(FILE *fh, uint32 group_count) {
	uint8 buffer[4];
	GroupMetaMetaData *group_meta_meta_data = malloc(sizeof(GroupMetaMetaData)*group_count);

	int i;
	for(i = 0; i < group_count; i++) {
		fread(buffer, 1, 4, fh);
		group_meta_meta_data[i].start = getUInt32(buffer);
		fread(bufer, 1, 4, fh);
		group_meta_meta_data[i].size = getUInt32(buffer);
	}

	return group_meta_meta_data;
}

Group *
read_groups(FILE *fh, uint32 group_count) {
	uint8 buffer[8];
	Group *group = malloc(sizeof(Group)*group_count);

	int i;
	for(i = 0; i < group_count; i++) {
		fseek(fh, group_meta_meta_data[i].start, SEEK_SET);
		GroupMetaData *metadata = malloc(sizeof(GroupMetaData));
		group[i].metadata = metadata;
		fread(buffer, 1, 8, fh);
		group[i].metadata->name = getChar(buffer, 8);
		fread(buffer, 1, 4, fh);
		group[i].metadata->size = getUInt32(buffer);
		fread(buffer, 1, 4, fh);
		group[i].size = getUInt32(buffer);
		fread(buffer, 1, 4, fh);
		group[i]._a = getUInt32(buffer);
		fread(buffer, 1, 4, fh);
		group[i].sub_group_count = getUInt32(buffer);
		group[i].sub_group_start = ftell(fh);
	}

	return group;
}

void
read_sub_groups(FILE *fh, Group *group, uint32 group_count) {
	uint8 buffer[8];
	int i, j;
	for(i = 0; i < group_count; i++) {
		fseek(fh, group[i].sub_group_start, SEEK_SET);
		group[i].sub_group = malloc(sizeof(SubGroup *)*group[i].sub_group_count);

		for(j = 0; j < group[i].sub_group_count; j++) {
			SubGroup *sub_group = malloc(sizeof(SubGroup));
			fread(buffer, 1, 8, fh);
			sub_group->metadata.name = getChar(buffer, 8);
			fread(buffer, 1, 4, fh);
			sub_group->metadata.size = getUInt32(buffer);
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

	g1mg->metadata.name = getChar(data+offset, 8);
	offset += 8;
	g1mg->metadata.size = getUInt32(data+offset);
	offset += 4;
	g1mg->platform = getChar(data+offset, 8);
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

	int i, m;
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

FaceGroup *
read_face_groups(SubChunk *face_sub_chunk) {
	uint32 offset = 0;
	uint8 *data = face_sub_chunk->data;
	uint32 face_groups = getUInt32(data+offset);
	offset += 4;

	int i, m;
	uint a, b, c;
	FaceGroup *face_group = malloc(sizeof(FaceGroup)*face_groups);
	for(i = 0; i < face_groups; i++) {
		face_group[i].indices = getUInt32(data+offset);
		offset += 4;
		face_group[i].index_length_in_bits = getUInt32(data+offset);
		offset += 4;
		face_group[i]._a = 0;
		offset += 4;

		Triangle *triangle = malloc(sizeof(Triangle)*face_group->indices);
		a = getVariableWidthUInt(data+offset, face_group[i].index_length_in_bits);
		offset += face_group[i].index_length_in_bits / 8;
		b = getVariableWidthUInt(data+offset, face_group[i].index_length_in_bits);
		offset += face_group[i].index_length_in_bits / 8;
		for(m = 0; m < face_group->indices; m++) {
			c = getVariableWidthUInt(data+offset, face_group[i].index_length_in_bits);
			offset += face_group[i].index_length_in_bits / 8;
			if(a != c && b != c && a != c) {
				triangle[i].a = a;
				triangle[i].b = b;
				triangle[i].c = c;
			}
			a = b;
			b = c;
		}
		face_group[i].triangle = triangle;
	}

	return face_group;
}

Meshes *
read_meshes(SubChunk *mesh_sub_chunk, VertexArray *vertex_arrays, FaceGroup *face_groups) {
	uint32 offset = 0;
	uint8 *data = mesh_sub_chunk->data;
	uint32 total_meshes = getUInt32(data+offset);
	offset += 0x4;

	int i;
	uint32 geometry_index = 0;
	uint32 vertex_index = 0;
	uint32 triangle_index = 0;

	Mesh *mesh = malloc(sizeof(Mesh)*total_meshes);
	for(i = 0; i < total_meshes; i++) {
		offset += 0x4 + 0x18;
		geometry_index = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].vertex_array = vertex_arrays[geometry_index];
		mesh[i].face_group = face_groups[geometry_index];
		offset += 0x8;
		mesh[i].vertex_index = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].vertices = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].triangle_index = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].triangles = getUInt32(data+offset);
		offset += 0x4;
	}
	Meshes *meshes = malloc(sizeof(Meshes));
	meshes->mesh = mesh;
	meshes->meshes = total_meshes;
	return mesh;
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
	FaceGroup *face_groups = read_face_groups(g1mg->sub_chunk[6]);
	Meshes *meshes = read_meshes(g1mg->sub_chunk[7]);

	HyruleWarriorsModel *hyrule_warriors_model = malloc(sizeof(HyruleWarriorsModel));
	hyrule_warriors_model->meshes = meshes;
	return hyrule_warriors_model;
}

void
hy_wa_model_to_obj(HyruleWarriorsModel *hwm) {
	int i, v, t;
	for(i = 0; i < hwm->meshes.amount; i++) {
		for(v = 0; v < hwm->meshes.mesh[i]->vertices; v++) {
			VertexEntry *vertex = hwm->meshes.mesh[i].vertex_array->entry[hwm->meshes.mesh[i].vertex_index+v];
			printf(stdout, "v %f %f %f\n", vertex->x, vertex->y, vertex->z);
		}

		for(t = 0; t < hwm->meshes.mesh[i]->triangles; t++) {
			Triangle *triangle = hwm->meshes.mesh[i].face_group->triangle[hwm->meshes.mesh[i].triangle_index+t];
			printf(stdout, "f %i %i %i\n", triangle->a, triangle->b, triangle->c);
		}
	}
	return;
}


int
main(int argc, char *argv[]) {
	HyruleWarriorsModel *hyrule_warriors_model = read_model(argv[1]);
	hy_wa_model_to_obj(hyrule_warriors_model);
	return 0;
}





