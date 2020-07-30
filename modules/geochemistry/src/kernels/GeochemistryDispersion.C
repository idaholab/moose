//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryDispersion.h"

registerMooseObject("GeochemistryApp", GeochemistryDispersion);

InputParameters
GeochemistryDispersion::validParams()
{
  InputParameters params = AnisotropicDiffusion::validParams();
  params.addCoupledVar("porosity", 1.0, "Porosity");
  params.addClassDescription("Kernel describing grad(porosity * tensor_coeff * "
                             "grad(concentration)), where porosity is an AuxVariable (or just "
                             "a real number), tensor_coeff is the hydrodynamic dispersion tensor "
                             "and concentration is the 'variable' for this Kernel");
  return params;
}

GeochemistryDispersion::GeochemistryDispersion(const InputParameters & parameters)
  : AnisotropicDiffusion(parameters), _porosity(coupledValue("porosity"))
{
}

Real
GeochemistryDispersion::computeQpResidual()
{
  return _porosity[_qp] * AnisotropicDiffusion::computeQpResidual();
}

Real
GeochemistryDispersion::computeQpJacobian()
{
  return _porosity[_qp] * AnisotropicDiffusion::computeQpJacobian();
}
