//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANKFOURAUX_H
#define RANKFOURAUX_H

#include "AuxKernel.h"
#include "RankFourTensor.h"

class RankFourAux;

/**
 * RankFourAux is designed to take the data in the RankFourTensor material
 * property, for example stiffness, and output the value for the
 * supplied indices.
 */

template <>
InputParameters validParams<RankFourAux>();

class RankFourAux : public AuxKernel
{
public:
  RankFourAux(const InputParameters & parameters);

  virtual ~RankFourAux() {}

protected:
  virtual Real computeValue();

private:
  const MaterialProperty<RankFourTensor> & _tensor;
  const unsigned int _i;
  const unsigned int _j;
  const unsigned int _k;
  const unsigned int _l;
};

#endif // RANKFOURAUX_H
