/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
