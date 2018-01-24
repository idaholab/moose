/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef VARCOUPLINGMATERIALEIGEN_H_
#define VARCOUPLINGMATERIALEIGEN_H_

#include "Material.h"

class VarCouplingMaterialEigen;

template <>
InputParameters validParams<VarCouplingMaterialEigen>();

/**
 * A material that couples a variable
 */
class VarCouplingMaterialEigen : public Material
{
public:
  VarCouplingMaterialEigen(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const VariableValue & _var;
  const VariableValue & _var_old;
  std::string _propname;
  MaterialProperty<Real> & _mat;
  MaterialProperty<Real> & _mat_old;
};

#endif // VARCOUPLINGMATERIALEIGEN_H
