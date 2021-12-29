
#ifndef WKBIGLOBBY_DEBUGF_H
#define WKBIGLOBBY_DEBUGF_H

#include "Config.h"
#define debugf(fmt, ...) if(Config::isDevConsoleEnabled()) printf("%s:%d: " fmt, __FUNCTION__ , __LINE__, __VA_ARGS__);

#endif //WKBIGLOBBY_DEBUGF_H
