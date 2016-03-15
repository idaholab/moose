/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrossTermBarrierFunctionMaterial.h"

template<>
InputParameters validParams<CrossTermBarrierFunctionMaterial>()
{
  InputParameters params = validParams<CrossTermBarrierFunctionBase>();
  params.addClassDescription("");
  return params;
}

CrossTermBarrierFunctionMaterial::CrossTermBarrierFunctionMaterial(const InputParameters & parameters) :
    CrossTermBarrierFunctionBase(parameters)
{
  // error out if W_ij is not symmetric
  for (unsigned int i = 0; i < _num_eta; ++i)
    for (unsigned int j = 0; j < i; ++j)
      if (_W_ij[_num_eta * i + j] != _W_ij[_num_eta * j + i])
        mooseError("Please supply a symmetric W_ij matrix for ");
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

      // barrier function value
      _prop_g[_qp] += Wij * (ni * ni * nj * nj + (ni - 1) * (ni - 1) * (nj - 1) * (nj - 1));

      // first derivative
      (*_prop_dg[i])[_qp] += Wij * (2 * ni * nj * nj + 2 * (ni - 1) * (nj - 1) * (nj - 1));

      // second derivative
      (*_prop_d2g[i][j])[_qp] +=  Wij * (4 * ni * nj + 4 * (ni - 1) * (nj - 1));
    }
}
