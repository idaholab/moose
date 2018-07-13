//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeAdvection.h"

registerMooseObject("MooseApp", ConservativeAdvection);

template <>
InputParameters
validParams<ConservativeAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} u$ which in its weak "
                             "form is given by: $(-\\nabla \\psi_i, \\vec{v} u)$.");
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  MooseEnum upwinding_type("none full", "none");
  params.addParam<MooseEnum>("upwinding_type",
                             upwinding_type,
                             "Type of upwinding used.  None: Typically results in overshoots and "
                             "undershoots, but numerical diffusion is minimised.  Full: Overshoots "
                             "and undershoots are avoided, but numerical diffusion is large");
  return params;
}

ConservativeAdvection::ConservativeAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _velocity(getParam<RealVectorValue>("velocity")),
    _upwinding(getParam<MooseEnum>("upwinding_type").getEnum<UpwindingType>()),
    _u_nodal(_var.dofValues()),
    _upwind_node(0),
    _dtotal_mass_out(0)
{
}

Real
ConservativeAdvection::negSpeedQp() const
{
  return -_grad_test[_i][_qp] * _velocity;
}

Real
ConservativeAdvection::computeQpResidual()
{
  // This is the no-upwinded version
  // It gets called via Kernel::computeResidual()
  return negSpeedQp() * _u[_qp];
}

Real
ConservativeAdvection::computeQpJacobian()
{
  // This is the no-upwinded version
  // It gets called via Kernel::computeJacobian()
  return negSpeedQp() * _phi[_j][_qp];
}

void
ConservativeAdvection::computeResidual()
{
  switch (_upwinding)
  {
    case UpwindingType::none:
      Kernel::computeResidual();
      break;
    case UpwindingType::full:
      fullUpwind(JacRes::CALCULATE_RESIDUAL);
      break;
  }
}

void
ConservativeAdvection::computeJacobian()
{
  switch (_upwinding)
  {
    case UpwindingType::none:
      Kernel::computeJacobian();
      break;
    case UpwindingType::full:
      fullUpwind(JacRes::CALCULATE_JACOBIAN);
      break;
  }
}

void
ConservativeAdvection::fullUpwind(JacRes res_or_jac)
{
  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
    if (ke.n() == 0)
      // this removes a problem encountered in
      // the initial timestep when
      // use_displaced_mesh=true
      return;
    _local_ke.resize(ke.m(), ke.n());
    _local_ke.zero();
  }

  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  // Compute the outflux from each node.  Even if we are computing the Jacobian we still need this
  // to see which nodes are upwind and which are downwind
  _local_re.resize(num_nodes); // if this is positive at the node, mass (or whatever the Variable
                               // represents) is flowing out of the node
  _local_re.zero();
  _upwind_node.resize(num_nodes);
  for (_i = 0; _i < num_nodes; ++_i)
  {
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * negSpeedQp();
    _upwind_node[_i] = (_local_re(_i) >= 0.0);
  }

  // Variables used to ensure mass conservation
  Real total_mass_out = 0.0;
  Real total_in = 0.0;
  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
    _dtotal_mass_out.assign(num_nodes, 0.0);

  for (unsigned int n = 0; n < num_nodes; ++n)
  {
    if (_upwind_node[n])
    {
      if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
      {
        if (_test.size() == _phi.size())
          /* u at node=n depends only on the u at node=n, by construction.  For
           * linear-lagrange variables, this means that Jacobian entries involving the derivative
           * will only be nonzero for derivatives wrt variable at node=n.  Hence the
           * (n, n) in the line below.  The above "if" statement catches other variable types
           * (eg constant monomials)
           */
          _local_ke(n, n) += _local_re(n);

        _dtotal_mass_out[n] += _local_ke(n, n);
      }
      _local_re(n) *= _u_nodal[n];
      total_mass_out += _local_re(n);
    }
    else                        // downwind node
      total_in -= _local_re(n); // note the -= means the result is positive
  }

  // Conserve mass over all phases by proportioning the total_mass_out mass to the inflow nodes,
  // weighted by their proto_flux values
  for (unsigned int n = 0; n < num_nodes; ++n)
  {
    if (!_upwind_node[n]) // downwind node
    {
      if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
        for (_j = 0; _j < _phi.size(); _j++)
          _local_ke(n, _j) += _local_re(n) * _dtotal_mass_out[_j] / total_in;
      _local_re(n) *= total_mass_out / total_in;
    }
  }

  // Add the result to the residual and jacobian
  if (res_or_jac == JacRes::CALCULATE_RESIDUAL)
  {
    DenseVector<Number> & re = _assembly.residualBlock(_var.number());
    re += _local_re;

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
    }
  }

  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
    ke += _local_ke;

    if (_has_diag_save_in)
    {
      unsigned int rows = ke.m();
      DenseVector<Number> diag(rows);
      for (unsigned int i = 0; i < rows; i++)
        diag(i) = _local_ke(i, i);

      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }
}
