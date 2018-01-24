//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PRESETNODALBC_H
#define PRESETNODALBC_H

#include "NodalBC.h"

class PresetNodalBC;

template <>
InputParameters validParams<PresetNodalBC>();

/**
 * TODO
 */
class PresetNodalBC : public NodalBC
{
public:
  PresetNodalBC(const InputParameters & parameters);

  void computeValue(NumericVector<Number> & current_solution);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpValue() = 0;
};

#endif /* PRESETBC_H */
