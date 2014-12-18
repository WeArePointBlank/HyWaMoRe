typedef struct {
	char *name;
	uint32 size;
} GroupMetaData;

typedef struct {
	GroupMetaData metadata;
	uint8 *data;
} SubGroup;

typedef struct {
	GroupMetaData metadata;
	uint32 size;
	uint32 _a;
	uint32 sub_group_count;
	uint32 sub_group_start;
	SubGroup *sub_group;
} Group;

typedef struct {
	uint32 start;
	uint32 size;
	Group *group;
} GroupMetaMetaData;

typedef struct {
	uint32 group_count;
	GroupMetaMetaData *group_meta_meta_data;
} FileHeader;


typedef struct {
	uint32 type;
	uint32 size;
} ChunkMetaData;

typedef struct {
	ChunkMetaData metadata;
	uint8 *data;
} SubChunk;

/*** Sub-groups ***/
/* Unknown */
typedef struct {
	GroupMetaData metadata;
} G1MF;

/* Skeleton */
typedef struct {
} Skeleton;

typedef struct {
	GroupMetaData metadata;
	Skeleton skele;
} G1MS;

/* Bone matrices */
typedef struct {
	GroupMetaData metadata;
} G1MM;

/* Geometry */
typedef struct {
	GroupMetaData metadata;
	char *platform;
	uint32 _a;
	uint32 _b;
	uint32 _c;
	uint32 _d;
	uint32 _e;
	uint32 _f;
	uint32 sub_chunk_count;
	SubChunk *sub_chunk;
} G1MG;



typedef struct {
	float x, y, z;
	uint8 *_unknown;
} VertexEntry;

typedef struct {
	uint32 _a;
	uint32 vertex_entry_size;
	uint32 vertex_entries;
	uint32 _b;
	VertexEntry *entry;
} VertexArray;

typedef struct {
	uint32 vertex_array_count;
	VertexArray *vertex_array;
} VertexSubChunk;

typedef struct {
	uint a, b, c;
} Triangle;

typedef struct {
	uint triangles;
	Triangle *triangle;
} TriangleGroup;

typedef struct {
	uint groups;
	TriangleGroup *group;
} TriangleGroups;

typedef struct {
	uint32 indices;
	uint32 index_length_in_bits;
	uint32 _a;
	uint16 *index;
} FaceGroup;

typedef struct {
	VertexArray *vertex_array;
	uint32 vertex_index;
	uint32 vertices;
	FaceGroup *face_group;
	uint32 indices_index;
	uint32 indices;
} Mesh;

typedef struct {
	uint amount;
	Mesh *mesh;
} Meshes;

typedef struct {
	Meshes *meshes;
} HyruleWarriorsModel;
