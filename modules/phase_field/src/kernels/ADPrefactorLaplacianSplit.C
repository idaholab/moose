//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPrefactorLaplacianSplit.h"

registerMooseObject("PhaseFieldApp", ADPrefactorLaplacianSplit);

InputParameters
ADPrefactorLaplacianSplit::validParams()
{
  InputParameters params = ADLaplacianSplit::validParams();
  params.addClassDescription("Laplacian split with a prefactor.");
  params.addRequiredParam<Real>("prefactor", "prefactor of the Laplacian operator"); 
  params.addParam<MaterialPropertyName>("density_value", "1.0", "density of the fluid mixture");  
  return params;
}

ADPrefactorLaplacianSplit::ADPrefactorLaplacianSplit(const InputParameters & parameters)
  : ADLaplacianSplit(parameters), _prefactor(getParam<Real>("prefactor")), _rho_val(getADMaterialProperty<Real>("density_value"))
{

}

ADRealGradient
ADPrefactorLaplacianSplit::precomputeQpResidual()
{
  return (_prefactor/_rho_val[_qp]) * ADLaplacianSplit::precomputeQpResidual();
}

