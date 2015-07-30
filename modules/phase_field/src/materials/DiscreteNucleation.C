/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DiscreteNucleation.h"
#include "DiscreteNucleationMap.h"

template<>
InputParameters validParams<DiscreteNucleation>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Free energy contribution for nucleating discrete particles");
  params.addRequiredCoupledVar("op_names", "List of variables to force to a target concentration value");
  params.addRequiredParam<UserObjectName>("map", "DiscreteNucleationMap user object");
  params.addRequiredParam<std::vector<Real> >("op_values", "List of target concentration values");
  params.addParam<Real>("penalty", 20.0, "Penalty factor for enforcing the target concentrations");
  return params;
}

DiscreteNucleation::DiscreteNucleation(const InputParameters & params) :
    DerivativeFunctionMaterialBase(params),
    _nvar(coupledComponents("op_names")),
    _op_index(_nvar),
    _op_values(getParam<std::vector<Real> >("op_values")),
    _penalty(getParam<Real>("penalty")),
    _map(getUserObject<DiscreteNucleationMap>("map"))
{
  // check inputs
  if (_nvar != _op_values.size())
    mooseError("The op_names and op_values parameter vectors must have the same number of entries");
  if (_nvar != _args.size())
    mooseError("Internal error.");

  // get libMesh variable numbers
  for (unsigned int i = 0; i < _nvar; ++i)
    _op_index[i] = argIndex(coupled("op_names", i));
}

void
DiscreteNucleation::computeProperties()
{
  // check if a nucleation event list is available for the current element
  const std::vector<char> & nucleus = _map.nuclei(_current_elem);

  // calculate penalty
  unsigned int ne_num = 0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // clear penalty value
    if (_prop_F)
      (*_prop_F)[_qp] = 0.0;

    for (unsigned int i = 0; i < _nvar; ++i)
    {
      const unsigned ii = _op_index[i];

      // sum up penalty contributions
      if (nucleus[_qp])
      {
        Real dc = (*_args[ii])[_qp] - _op_values[i];
        if (_prop_F)
          (*_prop_F)[_qp] += dc * dc;

        // first derivative
        if (_prop_dF[ii])
          (*_prop_dF[ii])[_qp] = 2.0 * dc * _penalty;
      }
      else if (_prop_dF[ii])
        (*_prop_dF[ii])[_qp] = 0.0;

      // second derivatives
      for (unsigned int jj = ii; jj < _nvar; ++jj)
      {
        if (_prop_d2F[ii][jj])
          (*_prop_d2F[ii][jj])[_qp] = (!nucleus[_qp] || ii != jj) ? 0.0 : 2.0 * _penalty;

        // third derivatives
        if (_third_derivatives)
          for (unsigned int kk = jj; kk < _nvar; ++kk)
            if (_prop_d3F[ii][jj][kk])
              (*_prop_d3F[ii][jj][kk])[_qp] = 0.0;
      }
    }

    // apply penalty factor
    if (nucleus[_qp] && _prop_F)
      (*_prop_F)[_qp] *= _penalty;
  }
}
