#include "any_hit_functions.h"

using namespace optix;

RT_PROGRAM void any_hit()
{
    phong_shadow();
}
