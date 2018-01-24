//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDEXTRANODESET_H
#define ADDEXTRANODESET_H

#include "MeshModifier.h"

// Forward Declaration
class AddExtraNodeset;

template <>
InputParameters validParams<AddExtraNodeset>();

class AddExtraNodeset : public MeshModifier
{
public:
  AddExtraNodeset(const InputParameters & params);

protected:
  virtual void modify() override;
};

#endif // ADDEXTRANODESET_H
