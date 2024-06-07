#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#define CHUNK_W 256
#define CHUNK_H 240
#define CHUNK_D 1
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

class Voxel;

class Chunk {
public:
	Voxel* voxels;
	Chunk();
	~Chunk();
};

#endif
