//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeAdvection.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", ConservativeAdvection);
registerMooseObject("MooseApp", ADConservativeAdvection);

template <bool is_ad>
InputParameters
ConservativeAdvectionTempl<is_ad>::generalParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} u$ which in its weak "
                             "form is given by: $(-\\nabla \\psi_i, \\vec{v} u)$.");
  MooseEnum upwinding_type("none full", "none");
  params.addParam<MooseEnum>("upwinding_type",
                             upwinding_type,
                             "Type of upwinding used.  None: Typically results in overshoots and "
                             "undershoots, but numerical diffusion is minimized.  Full: Overshoots "
                             "and undershoots are avoided, but numerical diffusion is large");
  params.addParam<MaterialPropertyName>("advected_quantity",
                                        "An optional material property to be advected. If not "
                                        "supplied, then the variable will be used.");
  return params;
}

template <>
InputParameters
ConservativeAdvectionTempl<false>::validParams()
{
  InputParameters params = generalParams();
  params.addCoupledVar("velocity", "Velocity vector");
  params.deprecateCoupledVar("velocity", "velocity_variable", "12/31/2025");
  params.addParam<MaterialPropertyName>("velocity_material", "Velocity vector given as a material");
  return params;
}

template <bool is_ad>
InputParameters
ConservativeAdvectionTempl<is_ad>::validParams()
{
  InputParameters params = generalParams();
  params.addCoupledVar("velocity_variable", "Velocity vector given as a variable");
  params.addParam<MaterialPropertyName>("velocity", "Velocity vector given as a material");
  params.deprecateParam("velocity", "velocity_material", "12/31/2025");
  return params;
}

template <bool is_ad>
ConservativeAdvectionTempl<is_ad>::ConservativeAdvectionTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _velocity(this->isParamValid("velocity_variable")
                  ? &this->template coupledGenericVectorValue<is_ad>("velocity_variable")
                  : (this->isParamValid("velocity_material")
                         ? &this->template getGenericMaterialProperty<RealVectorValue, is_ad>(
                                    "velocity_material")
                                .get()
                         : nullptr)),
    _adv_quant(
        isParamValid("advected_quantity")
            ? this->template getGenericMaterialProperty<Real, is_ad>("advected_quantity").get()
            : _u),
    _upwinding(
        this->template getParam<MooseEnum>("upwinding_type").template getEnum<UpwindingType>()),
    _u_nodal(_var.dofValues()),
    _upwind_node(0),
    _dtotal_mass_out(0)
{
  if (_upwinding != UpwindingType::none && this->isParamValid("advected_quantity"))
    paramError(
        "advected_quantity",
        "Upwinding is not compatable with an advected quantity that is not the primary variable.");

  if (!_velocity ||
      (this->isParamValid("velocity_variable") && this->isParamValid("velocity_material")))
    paramError("velocity_variable",
               "Either velocity_variable or velocity_material must be specificied, not both, nor "
               "velocity.");

  if (this->_has_diag_save_in)
    paramError("diag_save_in",
               "_local_ke not computed for global AD indexing. Save-in is deprecated anyway. Use "
               "the tagging system instead.");
}

template <bool is_ad>
GenericReal<is_ad>
ConservativeAdvectionTempl<is_ad>::negSpeedQp() const
{
  return -_grad_test[_i][_qp] * (*_velocity)[_qp];
}

template <bool is_ad>
GenericReal<is_ad>
ConservativeAdvectionTempl<is_ad>::computeQpResidual()
{
  // This is the no-upwinded version
  // It gets called via GenericKernel<is_ad>::computeResidual()
  return negSpeedQp() * _adv_quant[_qp];
}

template <>
Real
ConservativeAdvectionTempl<false>::computeQpJacobian()
{
  // This is the no-upwinded version
  // It gets called via GenericKernel<is_ad>::computeJacobian()
  return negSpeedQp() * _phi[_j][_qp];
}

template <bool is_ad>
Real
ConservativeAdvectionTempl<is_ad>::computeQpJacobian()
{
  mooseError("Internal error, should never get here when using AD");
  return 0.0;
}

template <bool is_ad>
void
ConservativeAdvectionTempl<is_ad>::computeResidual()
{
  switch (_upwinding)
  {
    case UpwindingType::none:
      GenericKernel<is_ad>::computeResidual();
      break;
    case UpwindingType::full:
      fullUpwind(JacRes::CALCULATE_RESIDUAL);
      break;
  }
}

template <bool is_ad>
void
ConservativeAdvectionTempl<is_ad>::computeJacobian()
{
  switch (_upwinding)
  {
    case UpwindingType::none:
      GenericKernel<is_ad>::computeJacobian();
      break;
    case UpwindingType::full:
      fullUpwind(JacRes::CALCULATE_JACOBIAN);
      break;
  }
}

template <bool is_ad>
void
ConservativeAdvectionTempl<is_ad>::fullUpwind(JacRes res_or_jac)
{
  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  // Even if we are computing the Jacobian we still need to compute the outflow from each node to
  // see which nodes are upwind and which are downwind
  prepareVectorTag(this->_assembly, _var.number());

  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
    prepareMatrixTag(this->_assembly, _var.number(), _var.number());

  // Compute the outflux from each node and store in _local_re
  // If _local_re is positive at the node, mass (or whatever the Variable represents) is flowing out
  // of the node
  _upwind_node.resize(num_nodes);
  for (_i = 0; _i < num_nodes; ++_i)
  {
    for (_qp = 0; _qp < this->_qrule->n_points(); _qp++)
      _local_re(_i) += this->_JxW[_qp] * this->_coord[_qp] * MetaPhysicL::raw_value(negSpeedQp());
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
  // weighted by their local_re values
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
    accumulateTaggedLocalResidual();

    if (this->_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (const auto & var : this->_save_in)
        var->sys().solution().add_vector(_local_re, var->dofIndices());
    }
  }

  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
    accumulateTaggedLocalMatrix();
}

template class ConservativeAdvectionTempl<false>;
template class ConservativeAdvectionTempl<true>;
