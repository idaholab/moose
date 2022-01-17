#pragma once

#include "THMObject.h"

/**
 * Group of components. Use only for parsing input files
 */
class ComponentGroup : public THMObject
{
public:
  ComponentGroup(const InputParameters & parameters);

public:
  static InputParameters validParams();
};
