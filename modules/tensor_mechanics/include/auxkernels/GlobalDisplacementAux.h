//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GLOBALDISPLACEMENTAUX_H
#define GLOBALDISPLACEMENTAUX_H

#include "AuxKernel.h"

// Forward Declarations
class GlobalDisplacementAux;
class RankTwoTensor;

template <>
InputParameters validParams<GlobalDisplacementAux>();

class GlobalDisplacementAux : public AuxKernel
{
public:
  GlobalDisplacementAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  std::string _base_name;

  const MaterialProperty<RankTwoTensor> & _global_strain;
  const unsigned int _component;

  RealVectorValue _global_disp;
};
#endif // GLOBALDISPLACEMENTAUX_H
