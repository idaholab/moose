//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StrainGradDispDerivatives.h"
#include "RankTwoTensor.h"

registerMooseObject("PhaseFieldApp", StrainGradDispDerivatives);

InputParameters
StrainGradDispDerivatives::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Provide the constant derivatives of strain w.r.t. the displacement gradient components.");
  params.addCoupledVar("displacement_gradients",
                       "List of displacement gradient component variables");
  return params;
}

StrainGradDispDerivatives::StrainGradDispDerivatives(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _nvar(coupledComponents("displacement_gradients")),
    _dstrain(_nvar)
{
  switch (_nvar)
  {
    case 1:
      _gdim = 1;
      break;

    case 4:
      _gdim = 2;
      break;

    case 9:
      _gdim = 3;
      break;

    default:
      mooseError("Supply 1, 4, or 9 displacement_gradient component variables");
  }

  if (_gdim > LIBMESH_DIM)
    mooseError("Too many gradient component variables for the current LIBMESH_DIM");

  for (unsigned int i = 0; i < _nvar; ++i)
    _dstrain[i] = &declarePropertyDerivative<RankTwoTensor>(
        "elastic_strain", coupledName("displacement_gradients", i));
}

void
StrainGradDispDerivatives::computeQpProperties()
{
  unsigned int i = 0;
  for (unsigned int j = 0; j < _gdim; ++j)
    for (unsigned int k = 0; k < _gdim; ++k)
    {
      (*_dstrain[i])[_qp].zero();
      (*_dstrain[i])[_qp](j, k) += 0.5;
      (*_dstrain[i])[_qp](k, j) += 0.5;
      ++i;
    }
}
