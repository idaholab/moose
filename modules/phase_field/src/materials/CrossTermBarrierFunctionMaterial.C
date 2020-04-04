//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrossTermBarrierFunctionMaterial.h"

registerMooseObject("PhaseFieldApp", CrossTermBarrierFunctionMaterial);

InputParameters
CrossTermBarrierFunctionMaterial::validParams()
{
  InputParameters params = CrossTermBarrierFunctionBase::validParams();
  params.addClassDescription(
      "Free energy contribution symmetric across interfaces between arbitrary pairs of phases.");
  return params;
}

CrossTermBarrierFunctionMaterial::CrossTermBarrierFunctionMaterial(
    const InputParameters & parameters)
  : CrossTermBarrierFunctionBase(parameters)
{
  // error out if W_ij is not symmetric
  for (unsigned int i = 0; i < _num_eta; ++i)
    for (unsigned int j = 0; j < i; ++j)
      if (_W_ij[_num_eta * i + j] != _W_ij[_num_eta * j + i])
        paramError("W_ij", "Please supply a symmetric W_ij matrix");
}

void
CrossTermBarrierFunctionMaterial::computeQpProperties()
{
  // Initialize properties to zero before accumulating
  CrossTermBarrierFunctionBase::computeQpProperties();

  // Sum the components of our W_ij matrix to get constant used in our g function
  for (unsigned int i = 0; i < _num_eta; ++i)
    for (unsigned int j = i + 1; j < _num_eta; ++j)
    {
      const Real ni = (*_eta[i])[_qp];
      const Real nj = (*_eta[j])[_qp];
      const Real Wij = _W_ij[_num_eta * i + j];

      switch (_g_order)
      {
        case 0: // SIMPLE
          _prop_g[_qp] += 16.0 * Wij * (ni * ni * nj * nj);
          // first derivatives
          (*_prop_dg[i])[_qp] += 16.0 * Wij * (2 * ni * nj * nj);
          (*_prop_dg[j])[_qp] += 16.0 * Wij * (2 * ni * ni * nj);
          // second derivatives (diagonal)
          (*_prop_d2g[i][i])[_qp] += 16.0 * Wij * (2 * nj * nj);
          (*_prop_d2g[j][j])[_qp] += 16.0 * Wij * (2 * ni * ni);
          // second derivatives (off-diagonal)
          (*_prop_d2g[i][j])[_qp] = 16.0 * Wij * (4 * ni * nj);
          break;

        case 1: // LOW
          _prop_g[_qp] += 4.0 * Wij * (ni * nj);
          // first derivatives
          (*_prop_dg[i])[_qp] += 4.0 * Wij * nj;
          (*_prop_dg[j])[_qp] += 4.0 * Wij * ni;
          // second derivatives (diagonal) vanish
          // second derivatives (off-diagonal)
          (*_prop_d2g[i][j])[_qp] = 4.0 * Wij;
          break;

        default:
          mooseError("Internal error");
      }
    }
}
