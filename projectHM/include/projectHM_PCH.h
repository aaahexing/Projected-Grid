#pragma once

#ifndef __PROJECTHM_PCH_H__
#define __PROJECTHM_PCH_H__

#include <map>
#include <set>
#include <queue>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <Windows.h>

using std::cout;
using std::cerr;
using std::endl;

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/matrix_operation.hpp"

const float PI = acos(-1.0);
const float Infinity = std::numeric_limits<float>::infinity();

#define projectHM_DEBUG

#endif	/* __PROJECTHM_PCH_H__ */