#pragma once

#include "THMObject.h"

class ComponentGroup;

template <>
InputParameters validParams<ComponentGroup>();

/**
 * Group of components. Use only for parsing input files
 */
class ComponentGroup : public THMObject
{
public:
  ComponentGroup(const InputParameters & parameters);
};
