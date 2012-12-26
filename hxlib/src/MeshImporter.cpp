#include "MeshImporter.h"

#include "MeshBuilder.h"

bool MeshImporterOBJ::import(const char *mesh_path, MeshBuilder *mesh_builder) {
	FILE *mesh_reader = fopen(mesh_path, "r");
	if (mesh_reader == NULL) {
		fprintf(stderr, "Cannot read model '%s'!\n", mesh_path);
		return false;
	}
	char mark[64], data[3096];
	mesh_builder->startMesh();
	while (fscanf(mesh_reader, "%s", mark) != EOF) {
		fgets(data, sizeof(data), mesh_reader);
		if (!strcmp(mark, "#")) {
			// comments, safe to ignore
		} else if (!strcmp(mark, "v")) {
			glm::vec3 position;
			sscanf(data, "%f%f%f", &position.x, &position.y, &position.z);
			mesh_builder->addPosition(position);
		} else if (!strcmp(mark, "vt")) {
			glm::vec2 texcoord;
			sscanf(data, "%f%f", &texcoord.x, &texcoord.y);
			mesh_builder->addTexcoord(texcoord);
		} else if (!strcmp(mark, "f")) {
			std::vector<int> v_lst;
			std::vector<int> vt_lst;
			for (char *part = strtok(data, " "); part; part = strtok(NULL, " ")) {
				int ii = 0, len = strlen(part), fi[3] = {0};
				for (int i = 0; i < len; ++i, ++ii) {
					fi[ii] = atoi(part + i);
					for (int vv = fi[ii]; vv > 0; vv /= 10)
						++i;
				}
				// The indices of positions must be valid
				if (fi[0] > 0) {
					v_lst.push_back(fi[0] - 1);
				}
				if (fi[1] > 0) {
					vt_lst.push_back(fi[1] - 1);
				}
			}
			if (!vt_lst.empty()) {
				mesh_builder->addFace(v_lst, vt_lst);
			} else {
				mesh_builder->addFace(v_lst);
			}
		}
	}
	mesh_builder->finishMesh();
	fclose(mesh_reader);
	return true;
}

MeshImporter* MeshImporterFactory::createImporter(const char *importer_type) {
	if (!strcmp(importer_type, "obj")) {
		return new MeshImporterOBJ();
	} else {
		fprintf(stderr, "Cannot create instance of '%s' importer!\n", importer_type);
		return NULL;
	}
}

bool MeshImporterFactory::import(const char *mesh_path, MeshBuilder *mesh_builder) {
	bool succeed = true;
	char ext[16] = "obj";
	MeshImporter *mesh_importer = NULL;
	// @todo: parse the path to get right extension name
	// Create the corresponding importer
	if (!strcmp(ext, "obj")) {
		mesh_importer = new MeshImporterOBJ();
	} else {
		fprintf(stderr, "Models with extension '%s' not supported yet.\n", ext);
		succeed = false;
	}
	// Check the validness of the importer
	if (succeed && mesh_importer == NULL) {
		fprintf(stderr, "Cannot create the corresponding importer for mesh '%s'\n", mesh_path);
		succeed = false;
	}
	if (succeed && !mesh_importer->import(mesh_path, mesh_builder)) {
		fprintf(stderr, "Failed to import model '%s'\n", mesh_path);
		succeed = false;
	}
	if (mesh_importer) {
		if (!strcmp(ext, "obj"))
			delete static_cast<MeshImporterOBJ*>(mesh_importer);
		mesh_importer = NULL;
	}
	return succeed;
}
