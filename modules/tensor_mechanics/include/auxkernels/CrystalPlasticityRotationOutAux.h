//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CRYSTALPLASTICITYROTATIONOUTAUX_H
#define CRYSTALPLASTICITYROTATIONOUTAUX_H

#include "AuxKernel.h"
#include "FiniteStrainCrystalPlasticity.h"

class CrystalPlasticityRotationOutAux;

template <>
InputParameters validParams<CrystalPlasticityRotationOutAux>();

class CrystalPlasticityRotationOutAux : public AuxKernel
{
public:
  CrystalPlasticityRotationOutAux(const InputParameters & parameters);
  virtual ~CrystalPlasticityRotationOutAux() {}

protected:
  virtual Real computeValue();

private:
  std::string _rotout_file_name;
  unsigned int _out_freq;
  const MaterialProperty<RankTwoTensor> & _update_rot;
};

#endif // CRYSTALPLASTICITYROTATIONOUTAUX_H//
