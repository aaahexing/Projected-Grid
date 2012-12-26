#ifndef __CGEFFECTMANAGER_H__
#define __CGEFFECTMANAGER_H__

#include <string>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

class CGEffectManager {
	// The CG context
	CGcontext context;
	// Current effect & technique
	CGpass pass;
	CGeffect effect;
	CGtechnique technique;
protected:
	void checkForCgError(const std::string &info);
	void validateEffect();
public:
	CGEffectManager();

	~CGEffectManager();

	inline void registerStates() {
		cgGLRegisterStates(context);
	}
	inline void activeEffect(const char *effect_name) {
		effect = cgGetNamedEffect(context, effect_name);
		if (!effect) checkForCgError(std::string("activeEffect ") + effect_name);
	}
	inline void activeTechnique(const char *technique_name) {
		technique = cgGetNamedTechnique(effect, technique_name);
		if (!technique) checkForCgError(std::string("activeTechnique ") + technique_name);
	}
	inline void activeFirstTechnique() {
		technique = cgGetFirstTechnique(effect);
		if (!technique) checkForCgError(std::string("activeTechnique first technique"));
	}
	inline void activeNextTechnique() {
		technique = cgGetNextTechnique(technique);
		if (!technique) activeFirstTechnique();
	}
	inline bool techniqueRunning() const {
		return pass != NULL;
	}
	inline void techniqueInitFirstPass() {
		pass = cgGetFirstPass(technique);
	}
	inline void techniqueApply() {
		cgSetPassState(pass);
	}
	inline void techniqueNextPass() {
		cgResetPassState(pass);
		pass = cgGetNextPass(pass);
	}
	inline CGpass getCurrentPass() {
		return pass;
	}
	inline CGprogram getPassProgram(CGdomain domain) {
		CGprogram program = cgGetPassProgram(pass, domain);
		if (!program) checkForCgError(std::string("getPassProgram"));
		return program;
	}
	inline CGparameter getPassParameter(CGdomain domain, const char *para_name) {
		CGprogram program = getPassProgram(domain);
		CGparameter param = cgGetNamedParameter(program, para_name);
		if (!param) checkForCgError(std::string("getPassParameter ") + para_name);
		return param;
	}
	inline CGparameter getEffectParameter(const char *para_name) {
		CGparameter param = cgGetNamedEffectParameter(effect, para_name);
		if (!param) checkForCgError(std::string("getEffectParameter ") + para_name);
		return param;
	}
	inline int getParameterBufferOffset(CGparameter parameter) {
		int param_offset = cgGetParameterBufferOffset(parameter);
		if (!param_offset) checkForCgError(std::string("getParameterBufferOffset"));
		return param_offset;
	}
	// Set effect parameters

	
	void loadEffectFromFile(const char *effect_file);
};

#endif	// __CGEFFECTMANAGER_H__