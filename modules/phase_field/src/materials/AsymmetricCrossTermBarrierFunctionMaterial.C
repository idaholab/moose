/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AsymmetricCrossTermBarrierFunctionMaterial.h"

template<>
InputParameters validParams<AsymmetricCrossTermBarrierFunctionMaterial>()
{
  InputParameters params = validParams<CrossTermBarrierFunctionBase>();
  params.addClassDescription("Free energy contribution asymmetric across interfaces between arbitrary pairs of phases.");
  params.addParam<std::vector<MaterialPropertyName> >("hi_names", "Switching Function Materials that provide h(eta_i)");
  return params;
}

AsymmetricCrossTermBarrierFunctionMaterial::AsymmetricCrossTermBarrierFunctionMaterial(const InputParameters & parameters) :
    CrossTermBarrierFunctionBase(parameters),
    _h(_num_eta),
    _dh(_num_eta),
    _d2h(_num_eta)
{
  // switching functions
  const std::vector<MaterialPropertyName> & hi_names = getParam<std::vector<MaterialPropertyName> >("hi_names");
  if (hi_names.size() != _num_eta)
    mooseError("The number of coupled etas must be equal to the number of hi_names in AsymmetricCrossTermBarrierFunctionMaterial " << name());

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
      const Real ni = (*_eta[i])[_qp];
      const Real nj = (*_eta[j])[_qp];

      const Real Wij = _W_ij[_num_eta * i + j];
      const Real Wji = _W_ij[_num_eta * j + i];

      switch (_g_order)
      {
        case 0: // SIMPLE
          _prop_g[_qp] += 16.0 * (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * (ni * ni * nj * nj);
          // first derivatives
          (*_prop_dg[i])[_qp] += 16.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * (2 * ni * nj * nj)
                                         + (Wij * (*_dh[i])[_qp]) * (ni * ni * nj * nj));
          (*_prop_dg[j])[_qp] += 16.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * (2 * ni * ni * nj)
                                         + (Wji * (*_dh[j])[_qp]) * (ni * ni * nj * nj));
          // second derivatives (diagonal)
          (*_prop_d2g[i][i])[_qp] += 16.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * (2 * nj * nj)
                                             + 2 * (Wij * (*_dh[i])[_qp]) * (2 * ni * nj * nj)
                                             + (Wij * (*_d2h[i])[_qp]) * (ni * ni * nj * nj));
          (*_prop_d2g[j][j])[_qp] += 16.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * (2 * ni * ni)
                                             + 2 * (Wji * (*_dh[j])[_qp]) * (2 * ni * ni * nj)
                                             + (Wji * (*_d2h[j])[_qp]) * (ni * ni * nj * nj));
          // second derivatives (off-diagonal)
          (*_prop_d2g[i][j])[_qp] = 16.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * (4 * ni * nj)
                                             + (Wji * (*_dh[j])[_qp]) * (2 * ni * nj * nj)
                                             + (Wij * (*_dh[i])[_qp]) * (2 * ni * ni * nj));
          break;

        case 1: // LOW
          _prop_g[_qp] += 4.0 * (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * (ni * nj);
          // first derivatives
          (*_prop_dg[i])[_qp] += 4.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * nj
                                         + (Wij * (*_dh[i])[_qp]) * (ni * nj));
          (*_prop_dg[j])[_qp] += 4.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp]) * ni
                                         + (Wji * (*_dh[j])[_qp]) * (ni * nj));
          // second derivatives (diagonal)
          (*_prop_d2g[i][i])[_qp] += 4.0 * (  2 * (Wij * (*_dh[i])[_qp]) * nj
                                            + (Wij * (*_d2h[i])[_qp]) * ni * nj);
          (*_prop_d2g[j][j])[_qp] += 4.0 * (  2 * (Wji * (*_dh[j])[_qp]) * ni
                                            + (Wji * (*_d2h[j])[_qp]) * ni * nj);
          // second derivatives (off-diagonal)
          (*_prop_d2g[i][j])[_qp] = 4.0 * (  (Wij * (*_h[i])[_qp] + Wji * (*_h[j])[_qp])
                                            + (Wji * (*_dh[j])[_qp]) * nj
                                            + (Wij * (*_dh[i])[_qp]) * ni);
          break;

        default:
          mooseError("Internal error");
      }
    }
}
