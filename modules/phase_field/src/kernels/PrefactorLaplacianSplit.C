//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrefactorLaplacianSplit.h"

registerMooseObject("PhaseFieldApp", PrefactorLaplacianSplit);

InputParameters
PrefactorLaplacianSplit::validParams()
{
  InputParameters params = LaplacianSplit::validParams();
  params.addClassDescription("Laplacian split with a prefactor.");
  params.addRequiredParam<Real>("prefactor", "prefactor of the Laplacian operator"); 
  return params;
}

PrefactorLaplacianSplit::PrefactorLaplacianSplit(const InputParameters & parameters)
  : LaplacianSplit(parameters), _prefactor(getParam<Real>("prefactor"))
{
}

RealGradient
PrefactorLaplacianSplit::precomputeQpResidual()
{
  return _prefactor * LaplacianSplit::precomputeQpResidual();
}

Real
PrefactorLaplacianSplit::computeQpOffDiagJacobian(unsigned int jvar)
{
  return _prefactor * LaplacianSplit::computeQpOffDiagJacobian(jvar);
}
