//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Q2PSaturationFlux.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

// C++ includes
#include <iostream>

registerMooseObject("RichardsApp", Q2PSaturationFlux);

InputParameters
Q2PSaturationFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "fluid_density",
      "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredCoupledVar("porepressure_variable",
                               "The variable representing the porepressure");
  params.addRequiredParam<UserObjectName>(
      "fluid_relperm",
      "A RichardsRelPerm UserObject (eg RichardsRelPermPowerGas) that defines the "
      "fluid relative permeability as a function of the saturation Variable.");
  params.addRequiredParam<Real>("fluid_viscosity", "The fluid dynamic viscosity");
  params.addClassDescription(
      "Flux according to Darcy-Richards flow.  The Variable of this Kernel must be the saturation");
  return params;
}

Q2PSaturationFlux::Q2PSaturationFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _pp(coupledValue("porepressure_variable")),
    _grad_pp(coupledGradient("porepressure_variable")),
    _pp_nodal(coupledDofValues("porepressure_variable")),
    _pp_var(coupled("porepressure_variable")),
    _relperm(getUserObject<RichardsRelPerm>("fluid_relperm")),
    _viscosity(getParam<Real>("fluid_viscosity")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _num_nodes(0),
    _mobility(0),
    _dmobility_dp(0),
    _dmobility_ds(0)
{
}

void
Q2PSaturationFlux::prepareNodalValues()
{
  _num_nodes = _pp_nodal.size();

  Real density;
  Real ddensity_dp;
  Real relperm;
  Real drelperm_ds;

  _mobility.resize(_num_nodes);
  _dmobility_dp.resize(_num_nodes);
  _dmobility_ds.resize(_num_nodes);

  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    density = _density.density(_pp_nodal[nodenum]);      // fluid density at the node
    ddensity_dp = _density.ddensity(_pp_nodal[nodenum]); // d(fluid density)/dP at the node
    relperm = _relperm.relperm(
        _var.dofValues()[nodenum]); // relative permeability of the fluid at node nodenum
    drelperm_ds = _relperm.drelperm(_var.dofValues()[nodenum]); // d(relperm)/dsat

    // calculate the mobility and its derivatives wrt P and S
    _mobility[nodenum] = density * relperm / _viscosity;
    _dmobility_dp[nodenum] = ddensity_dp * relperm / _viscosity;
    _dmobility_ds[nodenum] = density * drelperm_ds / _viscosity;
  }
}

Real
Q2PSaturationFlux::computeQpResidual()
{
  // note this is not the complete residual:
  // the upwind mobility parts get added in computeResidual
  return _grad_test[_i][_qp] *
         (_permeability[_qp] * (_grad_pp[_qp] - _density.density(_pp[_qp]) * _gravity[_qp]));
}

void
Q2PSaturationFlux::computeResidual()
{
  upwind(true, false, 0);
}

void
Q2PSaturationFlux::computeJacobian()
{
  upwind(false, true, _var.number());
}

void
Q2PSaturationFlux::computeOffDiagJacobian(const unsigned int jvar)
{
  upwind(false, true, jvar);
}

Real
Q2PSaturationFlux::computeQpJac(unsigned int dvar)
{
  // this is just the derivative of the flux WITHOUT the upstream mobility terms
  // Those terms get added in during computeJacobian()
  if (dvar == _pp_var)
    return _grad_test[_i][_qp] *
           (_permeability[_qp] *
            (_grad_phi[_j][_qp] - _density.ddensity(_pp[_qp]) * _gravity[_qp] * _phi[_j][_qp]));
  else
    return 0;
}

void
Q2PSaturationFlux::upwind(bool compute_res, bool compute_jac, unsigned int jvar)
{
  if (compute_jac && !(jvar == _var.number() || jvar == _pp_var))
    return;

  // calculate the mobility values and their derivatives
  prepareNodalValues();

  // compute the residual without the mobility terms
  // Even if we are computing the jacobian we still need this
  // in order to see which nodes are upwind and which are downwind
  prepareVectorTag(_assembly, _var.number());

  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  if (compute_jac)
  {
    prepareMatrixTag(_assembly, _var.number(), jvar);
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJac(jvar);
  }

  // Now perform the upwinding by multiplying the residuals at the
  // upstream nodes by their mobilities
  //
  // The residual for the kernel is the darcy flux.
  // This is
  // R_i = int{mobility*flux_no_mob} = int{mobility*grad(pot)*permeability*grad(test_i)}
  // for node i.  where int is the integral over the element.
  // However, in fully-upwind, the first step is to take the mobility outside the
  // integral, which was done in the _local_re calculation above.
  //
  // NOTE: Physically _local_re(_i) is a measure of fluid flowing out of node i
  // If we had left in mobility, it would be exactly the mass flux flowing out of node i.
  //
  // This leads to the definition of upwinding:
  // ***
  // If _local_re(i) is positive then we use mobility_i.  That is
  // we use the upwind value of mobility.
  // ***
  //
  // The final subtle thing is we must also conserve fluid mass: the total mass
  // flowing out of node i must be the sum of the masses flowing
  // into the other nodes.

  // FIRST:
  // this is a dirty way of getting around precision loss problems
  // and problems at steadystate where upwinding oscillates from
  // node-to-node causing nonconvergence.
  // I'm not sure if i actually need to do this in moose.  Certainly
  // in cosflow it is necessary.
  // I will code a better algorithm if necessary
  bool reached_steady = true;
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    if (_local_re(nodenum) >= 1E-20)
    {
      reached_steady = false;
      break;
    }
  }
  reached_steady = false;

  // DEFINE VARIABLES USED TO ENSURE MASS CONSERVATION
  // total mass out - used for mass conservation
  Real total_mass_out = 0;
  // total flux in
  Real total_in = 0;

  // the following holds derivatives of these
  std::vector<Real> dtotal_mass_out;
  std::vector<Real> dtotal_in;
  if (compute_jac)
  {
    dtotal_mass_out.assign(_num_nodes, 0);
    dtotal_in.assign(_num_nodes, 0);
  }

  // PERFORM THE UPWINDING!
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    if (_local_re(nodenum) >= 0 || reached_steady) // upstream node
    {
      if (compute_jac)
      {
        for (_j = 0; _j < _phi.size(); _j++)
          _local_ke(nodenum, _j) *= _mobility[nodenum];
        if (jvar == _var.number())
          // deriv wrt S
          _local_ke(nodenum, nodenum) += _dmobility_ds[nodenum] * _local_re(nodenum);
        else
          // deriv wrt P
          _local_ke(nodenum, nodenum) += _dmobility_dp[nodenum] * _local_re(nodenum);
        for (_j = 0; _j < _phi.size(); _j++)
          dtotal_mass_out[_j] += _local_ke(nodenum, _j);
      }
      _local_re(nodenum) *= _mobility[nodenum];
      total_mass_out += _local_re(nodenum);
    }
    else
    {
      total_in -= _local_re(nodenum); // note the -= means the result is positive
      if (compute_jac)
        for (_j = 0; _j < _phi.size(); _j++)
          dtotal_in[_j] -= _local_ke(nodenum, _j);
    }
  }

  // CONSERVE MASS
  // proportion the total_mass_out mass to the inflow nodes, weighting by their _local_re values
  if (!reached_steady)
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
      if (_local_re(nodenum) < 0)
      {
        if (compute_jac)
          for (_j = 0; _j < _phi.size(); _j++)
          {
            _local_ke(nodenum, _j) *= total_mass_out / total_in;
            _local_ke(nodenum, _j) +=
                _local_re(nodenum) * (dtotal_mass_out[_j] / total_in -
                                      dtotal_in[_j] * total_mass_out / total_in / total_in);
          }
        _local_re(nodenum) *= total_mass_out / total_in;
      }

  // ADD RESULTS TO RESIDUAL OR JACOBIAN
  if (compute_res)
  {
    accumulateTaggedLocalResidual();

    if (_has_save_in)
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }

  if (compute_jac)
  {
    accumulateTaggedLocalMatrix();
    if (_has_diag_save_in && jvar == _var.number())
    {
      const unsigned int rows = _local_ke.m();
      DenseVector<Number> diag(rows);
      for (unsigned int i = 0; i < rows; i++)
        diag(i) = _local_ke(i, i);

      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }
}
