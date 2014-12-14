typedef struct {
} Skeleton;

typedef struct {
	uint32 group_count;
	GroupMetaData *group_meta_data;
} FileHeader;

typedef struct {
	uint32 start;
	uint32 size;
	Group *group;
} GroupMetaMetaData;

typedef struct {
	char name[8];
	uint32 size;
} GroupMetaData;

/* Entry */
typedef struct {
	GroupMetaData metadata;
	uint32 size;
	uint32 _a;
	uint32 sub_group_count;
	uint32 sub_group_start;
	SubGroup **sub_group;
} Group;

typedef struct {
	GroupMetaData metadata;
	uint8 *data;
} SubGroup;

/*** Sub-groups ***/
/* Unknown */
typedef struct {
	GroupMetaData metadata;
} G1MF;

/* Skeleton */
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
	char platform[8];
	uint32 _a;
	uint32 _b;
	uint32 _c;
	uint32 _d;
	uint32 _e;
	uint32 _f;
	uint32 sub_chunk_count;
} G1MG;

typedef struct {
	uint32 type;
	uint32 size;
} ChunkMetaData;

typedef struct {
	ChunkMetaData metadata;
	uint8 *data;
} SubChunk;

typedef struct {
	uint32 vertex_array_count;
	VertexArray *vertex_array;
} VertexSubChunk;

typedef struct {
	float x, y, z;
	uint8 *_unknown;
} VertexEntry;

typedef struct {
	uint32 _a;
	uint32 vertex_entry_size;
	uint32 vertex_entries;
	uint32 _b;
	VertexEntry **entry;
} VertexArray;


