/*
	brought to you by .blank: success is in range.
	
	contributors:
	Twili (format reversing)
	lee (parsing tool)
	chrrox (format reversing)
	
	thanks to:
	crediar
	DogPolice
	
	
	/@\inere csfer aeov/@\
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#include "hywa.h"

uint32
getUInt32(uint8 *data) {
	return be32toh(*((uint32 *)data));
}

uint16
getUInt16(uint8 *data) {
	return be16toh(*((uint16 *)data));
}

float
getFloat(uint8 *data) {
	float f;
	uint32 td = be32toh(*((uint32 *)data));
	memcpy(&f, &td, sizeof(float));
	return f;
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
			return be16toh(*((uint16 *)data));
		case 0x20:
			return be32toh(*((uint32 *)data));
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
	file_header->group_count = getUInt32(buffer);	
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
		fread(buffer, 1, 4, fh);
		group_meta_meta_data[i].size = getUInt32(buffer);
	}

	return group_meta_meta_data;
}

Group *
read_groups(FILE *fh, GroupMetaMetaData *group_meta_meta_data, uint32 group_count) {
	uint8 buffer[8];
	Group *group = malloc(sizeof(Group)*group_count);

	int i;
	for(i = 0; i < group_count; i++) {
		fseek(fh, group_meta_meta_data[i].start, SEEK_SET);
		fread(buffer, 1, 8, fh);
		group[i].metadata.name = getChar(buffer, 8);
		fread(buffer, 1, 4, fh);
		group[i].metadata.size = getUInt32(buffer);
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
		group[i].sub_group_count -= 1;
		group[i].sub_group = malloc(sizeof(SubGroup)*group[i].sub_group_count);
		for(j = 0; j < group[i].sub_group_count; j++) {
			fread(buffer, 1, 8, fh);
			group[i].sub_group[j].metadata.name = getChar(buffer, 8);
			fread(buffer, 1, 4, fh);
			group[i].sub_group[j].metadata.size = getUInt32(buffer);
			group[i].sub_group[j].data = malloc(sizeof(uint8)*(group[i].sub_group[j].metadata.size - 0xC));
			if(group[i].sub_group[j].data != NULL) {
				fread(group[i].sub_group[j].data, 1, group[i].sub_group[j].metadata.size - 0xC, fh);
			}
		}
	}
}

/* Read geometry group */
G1MG *
read_G1MG(Group *group) {
	uint32 offset = 0;
	G1MG *g1mg = malloc(sizeof(G1MG));
	uint8 *data = group[0].sub_group[3].data;

	g1mg->metadata.name = group[0].sub_group[3].metadata.name;
	g1mg->metadata.size = group[0].sub_group[3].metadata.size;
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
		sub_chunk[i].data = data+offset;
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

	int i, m;
	VertexArray *vertex_array = malloc(sizeof(VertexArray)*vertex_arrays);
	for(i = 0; i < vertex_arrays; i++) {
			vertex_array[i]._a = 0;
			offset += 0x4;
			vertex_array[i].vertex_entry_size = getUInt32(data+offset);
			offset += 0x4;
			vertex_array[i].vertex_entries = getUInt32(data+offset);
			offset += 0x4;
			vertex_array[i]._b = 0;
			offset += 0x4;
			vertex_array[i].entry = malloc(sizeof(VertexEntry)*vertex_array[i].vertex_entries);
			for(m = 0; m < vertex_array[i].vertex_entries; m++) {
				vertex_array[i].entry[m].x = getFloat(data+offset);
				offset += 0x4;
				vertex_array[i].entry[m].y = getFloat(data+offset);
				offset += 0x4;
				vertex_array[i].entry[m].z = getFloat(data+offset);
				offset += 0x4;
				offset += vertex_array[i].vertex_entry_size - 0xC;
			}
	}
	return vertex_array;
}

FaceGroup *
read_face_groups(SubChunk *face_sub_chunk) {
	uint32 offset = 0;
	uint8 *data = face_sub_chunk->data;
	uint32 face_groups = getUInt32(data+offset);
	offset += 4;

	int i, m;
	FaceGroup *face_group = malloc(sizeof(FaceGroup)*face_groups);
	for(i = 0; i < face_groups; i++) {
		face_group[i].indices = getUInt32(data+offset);
		offset += 4;
		face_group[i].index_length_in_bits = getUInt32(data+offset);
		offset += 4;
		face_group[i]._a = 0;
		offset += 4;

		uint16 *index = malloc(sizeof(uint16)*face_group[i].indices);
		for(m = 0; m < face_group[i].indices; m++) {
			index[m] = getUInt16(data+offset);
			offset += 2;
		}
		face_group[i].index = index;
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

	Mesh *mesh = malloc(sizeof(Mesh)*total_meshes);
	for(i = 0; i < total_meshes; i++) {
		offset += 0x4 + 0x18;
		geometry_index = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].vertex_array = &vertex_arrays[geometry_index];
		mesh[i].face_group = &face_groups[geometry_index];
		offset += 0x8;
		mesh[i].vertex_index = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].vertices = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].indices_index = getUInt32(data+offset);
		offset += 0x4;
		mesh[i].indices = getUInt32(data+offset);
		offset += 0x4;
	}
	Meshes *meshes = malloc(sizeof(Meshes));
	meshes->mesh = mesh;
	meshes->amount = total_meshes;
	return meshes;
}

HyruleWarriorsModel *
read_model(char *file_path) {
	FILE *fh = fopen(file_path, "rb");

	FileHeader *file_header = read_file_header(fh);
	GroupMetaMetaData *groups_meta_meta_data = read_groups_meta_meta_data(fh, file_header->group_count);
	Group *groups = read_groups(fh, groups_meta_meta_data, file_header->group_count);
	read_sub_groups(fh, groups, file_header->group_count);

	fclose(fh);

	G1MG *g1mg = read_G1MG(groups);
	VertexArray *vertex_arrays = read_vertex_arrays(&g1mg->sub_chunk[3]);
	FaceGroup *face_groups = read_face_groups(&g1mg->sub_chunk[6]);
	Meshes *meshes = read_meshes(&g1mg->sub_chunk[7], vertex_arrays, face_groups);

	HyruleWarriorsModel *hyrule_warriors_model = malloc(sizeof(HyruleWarriorsModel));
	hyrule_warriors_model->meshes = meshes;
	return hyrule_warriors_model;
}


TriangleGroup *
triangulate_indices(FaceGroup *face_group, uint start_index, uint amount) {
		uint a, b, c, t, m;
		uint face_direction, start_direction = -1;

		TriangleGroup *triangle_group = malloc(sizeof(TriangleGroup));
		triangle_group->triangle = NULL;
		
		a = face_group->index[start_index];
		b = face_group->index[start_index+1];
		face_direction = start_direction;
		t = 0;
		for(m = start_index+2; m < start_index+amount; m++) {
			c = face_group->index[m];
			if(c == 0xFFFF) {
				m++;
				a = face_group->index[m];
				m++;
				b = face_group->index[m];
				face_direction = start_direction;
			} else {
				face_direction *= -1;
				if(a != b && b != c && a != c) {
					triangle_group->triangle = realloc(triangle_group->triangle, sizeof(Triangle)*(t+1));
					if(face_direction > 0) {
						triangle_group->triangle[t].a = a;
						triangle_group->triangle[t].b = b;
						triangle_group->triangle[t].c = c;
					} else {
						triangle_group->triangle[t].a = a;
						triangle_group->triangle[t].b = c;
						triangle_group->triangle[t].c = b;
					}
					t++;
				}
				a = b;
				b = c;
			}
		}
		triangle_group->triangles = t;
		return triangle_group;
}

void
hy_wa_model_to_obj(HyruleWarriorsModel *hwm) {
	int t, m, v; 
	FaceGroup *face_group = NULL;
	VertexEntry *vertex_entry = NULL;
	Triangle *tri = NULL;
	Meshes *meshes = hwm->meshes;
	uint vertex_offset = 1;
	FaceGroup *last_face_group = NULL;

	for(m = 0; m < meshes->amount; m++) {
		for(v = meshes->mesh[m].vertex_index; v < meshes->mesh[m].vertex_index+meshes->mesh[m].vertices; v++) {
			vertex_entry = &meshes->mesh[m].vertex_array->entry[v];
			printf("v %f %f %f\n", vertex_entry->x, vertex_entry->y, vertex_entry->z);
		}
	}
	printf("\n");

	TriangleGroup *triangle_group = NULL;
	last_face_group = meshes->mesh[0].face_group;
	for(m = 0; m < meshes->amount; m++) {
			
			face_group = meshes->mesh[m].face_group;
			if(face_group != last_face_group) {
				vertex_offset += meshes->mesh[m-1].vertex_array->vertex_entries;
			}
			
			printf("g mesh%d\n\n", m);
			triangle_group = triangulate_indices(face_group, meshes->mesh[m].indices_index, meshes->mesh[m].indices);
			for(t = 0; t < triangle_group->triangles; t++) {
				tri = &triangle_group->triangle[t];
				printf("f %d %d %d\n", tri->a+vertex_offset, tri->b+vertex_offset, tri->c+vertex_offset);
			}
			printf("\n");
			
			last_face_group = face_group;
	}
	return;
}


int
main(int argc, char *argv[]) {
	HyruleWarriorsModel *hyrule_warriors_model = read_model(argv[1]);
	hy_wa_model_to_obj(hyrule_warriors_model);
	return 0;
}
