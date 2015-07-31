/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATSOURCEPD_H
#define HEATSOURCEPD_H

#include "Kernel.h"

//Forward Declarations
class ColumnMajorMatrix;
class SymmElasticityTensor;
class SymmTensor;

class HeatSourcePD : public Kernel
{
public:

  HeatSourcePD(const std::string & name, InputParameters parameters);
  virtual ~HeatSourcePD() {}

protected:
  virtual void computeResidual();
  virtual Real computeQpResidual();

  const Real _power_density;

  const MaterialProperty<Real> & _bond_volume;
  const MaterialProperty<Real> & _bond_status;
  Function & _function;

private:

  const bool _temp_coupled;
  const unsigned int _temp_var;

};

template<>
InputParameters validParams<HeatSourcePD>();

#endif //HEATSOURCEPD_H
