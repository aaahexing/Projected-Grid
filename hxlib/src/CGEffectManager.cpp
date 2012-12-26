#include "CGEffectManager.h"

#include <cstdio>
#include <string>
#include <cstdlib>

void CGEffectManager::checkForCgError(const std::string &info) {
	CGerror error;
	const char *error_inf = cgGetLastErrorString(&error);
	if (error != CG_NO_ERROR) {
		fprintf(stderr, "%s : %s\n", info.c_str(), error_inf);
		if (error == CG_COMPILER_ERROR) {
			fprintf(stderr, "%s\n", cgGetLastListing(context));
		}
	}
}

void CGEffectManager::validateEffect() {
	for(technique = cgGetFirstTechnique(effect); technique; technique = cgGetNextTechnique(technique)) {
		if (cgValidateTechnique(technique) == CG_FALSE) {
			fprintf(stderr, "Technique '%s' is not valid. Skipping.\n", cgGetTechniqueName(technique));
			checkForCgError("validateEffect");
		}
	}
}

CGEffectManager::CGEffectManager() {
	context = cgCreateContext();
	cgGLSetManageTextureParameters(context, CG_TRUE);
}

CGEffectManager::~CGEffectManager() {
	cgDestroyContext(context);
}

void CGEffectManager::loadEffectFromFile(const char *effect_file) {
	effect = cgCreateEffectFromFile(context, effect_file, NULL);
	if (!effect) {
		fprintf(stderr, "Failed to load effect from file '%s'. Skipping.\n", effect_file);
		checkForCgError(std::string("loadEffectFromFile ") + effect_file);
	}
	// Set a name for the effect to make it convenient for retrieve the effect itself later 
	if (cgSetEffectName(effect, effect_file) == CG_FALSE) {
		fprintf(stderr, "Failed to set effect name to '%s'.\n", effect_file);
		checkForCgError(std::string("loadEffectFromFile ") + effect_file);
	}
	validateEffect();
}