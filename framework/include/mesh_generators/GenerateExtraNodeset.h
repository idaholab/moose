//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERATEEXTRANODESET_H
#define GENERATEEXTRANODESET_H

#include "MeshGenerator.h"

// Forward declarations
class GenerateExtraNodeset;

template <>
InputParameters validParams<GenerateExtraNodeset>();

/**
 *
 */
class GenerateExtraNodeset : public MeshGenerator
{
public:
  GenerateExtraNodeset(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  std::unique_ptr<MeshBase> & _input;
};

#endif // GENERATEEXTRANODESET_H
