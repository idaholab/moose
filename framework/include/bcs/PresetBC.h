//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PRESETBC_H
#define PRESETBC_H

#include "PresetNodalBC.h"

class PresetBC;

template <>
InputParameters validParams<PresetBC>();

/**
 * TODO:
 */
class PresetBC : public PresetNodalBC
{
public:
  PresetBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;

  const Real & _value;
};

#endif /* PRESETBC_H */
