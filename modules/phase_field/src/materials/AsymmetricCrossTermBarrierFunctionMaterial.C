//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AsymmetricCrossTermBarrierFunctionMaterial.h"

registerMooseObject("PhaseFieldApp", AsymmetricCrossTermBarrierFunctionMaterial);

InputParameters
AsymmetricCrossTermBarrierFunctionMaterial::validParams()
{
  InputParameters params = CrossTermBarrierFunctionBase::validParams();
  params.addClassDescription(
      "Free energy contribution asymmetric across interfaces between arbitrary pairs of phases.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "hi_names", "Switching Function Materials that provide h(eta_i)");
  return params;
}

AsymmetricCrossTermBarrierFunctionMaterial::AsymmetricCrossTermBarrierFunctionMaterial(
    const InputParameters & parameters)
  : CrossTermBarrierFunctionBase(parameters), _h(_num_eta), _dh(_num_eta), _d2h(_num_eta)
{
  // switching functions
  const std::vector<MaterialPropertyName> & hi_names =
      getParam<std::vector<MaterialPropertyName>>("hi_names");
  if (hi_names.size() != _num_eta)
    paramError("hi_names", "The number of hi_names must be equal to the number of coupled etas");

  for (unsigned int i = 0; i < _num_eta; ++i)
  {
    _h[i] = &getMaterialProperty<Real>(hi_names[i]);
    _dh[i] = &getMaterialPropertyDerivative<Real>(hi_names[i], _eta_names[i]);
    _d2h[i] = &getMaterialPropertyDerivative<Real>(hi_names[i], _eta_names[i], _eta_names[i]);
  }
}

void
AsymmetricCrossTermBarrierFunctionMaterial::computeQpProperties()
{
  // Initialize properties to zero before accumulating
  CrossTermBarrierFunctionBase::computeQpProperties();

  // Sum the components of our W_ij matrix to get constant used in our g function
  for (unsigned int i = 0; i < _num_eta; ++i)
    for (unsigned int j = i + 1; j < _num_eta; ++j)
    {
      // readable aliases
      const Real ni = (*_eta[i])[_qp];
      const Real nj = (*_eta[j])[_qp];

      const Real Wij = _W_ij[_num_eta * i + j];
      const Real Wji = _W_ij[_num_eta * j + i];

      const Real hi = (*_h[i])[_qp];
      const Real hj = (*_h[j])[_qp];
      const Real dhi = (*_dh[i])[_qp];
      const Real dhj = (*_dh[j])[_qp];
      const Real d2hi = (*_d2h[i])[_qp];
      const Real d2hj = (*_d2h[j])[_qp];

      // raw barrier term and derivatives
      Real B, dBi, dBj, d2Bii, d2Bjj, d2Bij;
      switch (_g_order)
      {
        case 0: // SIMPLE
          B = 16.0 * ni * ni * nj * nj;
          dBi = 16.0 * 2.0 * ni * nj * nj;
          dBj = 16.0 * 2.0 * ni * ni * nj;
          d2Bii = 16.0 * 2.0 * nj * nj;
          d2Bjj = 16.0 * 2.0 * ni * ni;
          d2Bij = 16.0 * 4.0 * ni * nj;
          break;

        case 1: // LOW
          B = 4.0 * ni * nj;
          dBi = 4.0 * nj;
          dBj = 4.0 * ni;
          d2Bii = 0.0;
          d2Bjj = 0.0;
          d2Bij = 4.0;
          break;

        default:
          mooseError("Internal error");
      }

      _prop_g[_qp] += (Wij * hi + Wji * hj) * B;
      // first derivatives
      (*_prop_dg[i])[_qp] += (Wij * hi + Wji * hj) * dBi + (Wij * dhi) * B;
      (*_prop_dg[j])[_qp] += (Wij * hi + Wji * hj) * dBj + (Wji * dhj) * B;
      // second derivatives (diagonal)
      (*_prop_d2g[i][i])[_qp] +=
          (Wij * hi + Wji * hj) * d2Bii + 2 * (Wij * dhi) * dBi + (Wij * d2hi) * B;
      (*_prop_d2g[j][j])[_qp] +=
          (Wij * hi + Wji * hj) * d2Bjj + 2 * (Wji * dhj) * dBj + (Wji * d2hj) * B;
      // second derivatives (off-diagonal)
      (*_prop_d2g[i][j])[_qp] =
          (Wij * hi + Wji * hj) * (d2Bij) + (Wji * dhj) * dBi + (Wij * dhi) * dBj;
    }
}
