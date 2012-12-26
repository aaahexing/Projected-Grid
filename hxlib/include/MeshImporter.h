#ifndef __MESHIMPORTER_H__
#define __MESHIMPORTER_H__

class MeshBuilder;

class MeshImporter {
public:
	virtual bool import(const char *mesh_path, MeshBuilder *mesh_builder) = 0;
};

class MeshImporterOBJ : public MeshImporter {
public:
	virtual bool import(const char *mesh_path, MeshBuilder *mesh_builder);
};

class MeshImporterFactory {
public:
	static MeshImporter* createImporter(const char *importer_type);
	static bool import(const char *mesh_path, MeshBuilder *mesh_builder);
};

#endif
