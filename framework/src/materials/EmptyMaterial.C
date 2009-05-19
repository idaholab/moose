#include "EmptyMaterial.h"

template<>
Parameters valid_params<EmptyMaterial>()
{}

void
EmptyMaterial::computeProperties()
{
}
