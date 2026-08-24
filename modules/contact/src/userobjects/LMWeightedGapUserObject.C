//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMWeightedGapUserObject.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "MortarContactUtils.h"
#include "AutomaticMortarGeneration.h"

#include "libmesh/fe.h"

#include <cmath>

registerMooseObject("ContactApp", LMWeightedGapUserObject);

InputParameters
LMWeightedGapUserObject::newParams()
{
  auto params = emptyInputParameters();
  params.addRequiredCoupledVar(
      "lm_variable", "The Lagrange multiplier variable representing the contact pressure.");
  params.addParam<bool>(
      "use_petrov_galerkin", false, "Whether to use the Petrov-Galerkin approach.");
  params.addCoupledVar("aux_lm",
                       "Auxiliary Lagrange multiplier variable that is utilized together with the "
                       "Petrov-Galerkin approach.");
  params.addParam<bool>(
      "use_nodal_scaling",
      false,
      "Whether to apply the node-based Lagrange-multiplier scaling of Popp et al. (2013) to "
      "improve the conditioning of the linear system when secondary elements are only partially "
      "covered (edge dropping). See the documentation for the current limitations.");
  params.addParam<bool>("derive_c_from_elasticity",
                        false,
                        "Compute a physical stiffness per normal length from the contact surface "
                        "material properties and adjacent element depths.");
  params.addParam<std::string>(
      "secondary_base_name",
      "",
      "Base name prefix of the elasticity_tensor material property on the secondary body. "
      "Must match the base_name used on the secondary material block that computes the "
      "elasticity tensor.");
  params.addParam<std::string>(
      "primary_base_name",
      "",
      "Base name prefix of the elasticity_tensor material property on the primary body. "
      "Must match the base_name used on the primary material block that computes the "
      "elasticity tensor.");
  params.addParam<bool>(
      "use_automatic_differentiation",
      false,
      "Whether the elasticity tensor material property was declared as an AD property. "
      "Set to true if the material block uses an AD elasticity tensor.");
  return params;
}

InputParameters
LMWeightedGapUserObject::validParams()
{
  InputParameters params = WeightedGapUserObject::validParams();
  params.addClassDescription(
      "Provides the mortar normal Lagrange multiplier for constraint enforcement.");
  params.set<bool>("allow_nodal_normal_derivatives") = true;
  params += LMWeightedGapUserObject::newParams();
  return params;
}

LMWeightedGapUserObject::LMWeightedGapUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    _derive_c_from_elasticity(getParam<bool>("derive_c_from_elasticity")),
    _use_automatic_differentiation(getParam<bool>("use_automatic_differentiation")),
    _lm_var(getVar("lm_variable", 0)),
    _use_petrov_galerkin(getParam<bool>("use_petrov_galerkin")),
    _aux_lm_var(isCoupled("aux_lm") ? getVar("aux_lm", 0) : nullptr),
    _use_nodal_scaling(getParam<bool>("use_nodal_scaling"))
{
  checkInput(_lm_var, "lm_variable");
  verifyLagrange(*_lm_var, "lm_variable");

  if (_use_petrov_galerkin && ((!isParamValid("aux_lm")) || _aux_lm_var == nullptr))
    paramError("use_petrov_galerkin",
               "We need to specify an auxiliary variable `aux_lm` while using the Petrov-Galerkin "
               "approach");

  if (_use_petrov_galerkin && _aux_lm_var->useDual())
    paramError("aux_lm",
               "Auxiliary LM variable needs to use standard shape function, i.e., set `use_dual = "
               "false`.");

  if (_use_nodal_scaling)
  {
    // kappa_j's denominator int_e N_j is zero at TRI6 and TET10 vertices and negative at QUAD8 and
    // HEX20 corners, and its numerator has to be integrated with the same basis, so every variable
    // entering kappa_j has to be first order.
    if (_lm_var->feType().order != FIRST || _disp_x_var->feType().order != FIRST ||
        (_use_petrov_galerkin && _aux_lm_var->feType().order != FIRST))
      paramError("use_nodal_scaling",
                 "Node-based scaling is only implemented for first-order Lagrange multiplier and "
                 "displacement variables.");

    // Scaling rescales the partially covered multiplier DOFs that the default treatment zeroes.
    if (!getParam<bool>("correct_edge_dropping"))
      paramError("use_nodal_scaling",
                 "Node-based scaling requires 'correct_edge_dropping = true'.");

    // Built once here rather than per element in fullNodalIntegrals().
    const FEType fe_type(FIRST, LAGRANGE);
    const auto lower_dim = _subproblem.mesh().dimension() - 1;
    _nodal_scaling_fe = FEBase::build(lower_dim, fe_type);
    _nodal_scaling_qrule = std::make_unique<QGauss>(lower_dim, fe_type.default_quadrature_order());
    _nodal_scaling_fe->attach_quadrature_rule(_nodal_scaling_qrule.get());
  }

  if (_derive_c_from_elasticity)
  {
    const std::string sec_base = getParam<std::string>("secondary_base_name");
    const std::string pri_base = getParam<std::string>("primary_base_name");
    const std::string sec_name =
        sec_base.empty() ? "elasticity_tensor" : sec_base + "_elasticity_tensor";
    const std::string pri_name =
        pri_base.empty() ? "elasticity_tensor" : pri_base + "_elasticity_tensor";
    if (_use_automatic_differentiation)
      fetchElasticityTensorProperties<true>(sec_name, pri_name);
    else
      fetchElasticityTensorProperties<false>(sec_name, pri_name);
  }
}

void
LMWeightedGapUserObject::checkInput(const MooseVariable * const var,
                                    const std::string & var_param_name) const
{
  if (isCoupledConstant(var_param_name))
    paramError(var_param_name,
               "The Lagrange multiplier variable must be an actual variable and not a constant.");
  else if (!var)
    paramError(var_param_name,
               "The Lagrange multiplier variables must be provided and be actual variables.");
}

void
LMWeightedGapUserObject::verifyLagrange(const MooseVariable & var,
                                        const std::string & var_param_name) const
{
  if (var.feType().family != LAGRANGE)
    paramError(var_param_name, "The Lagrange multiplier variables must be of Lagrange type");
}

const VariableTestValue &
LMWeightedGapUserObject::test() const
{
  return _use_petrov_galerkin ? _aux_lm_var->phiLower() : _lm_var->phiLower();
}

const ADVariableValue &
LMWeightedGapUserObject::contactPressure() const
{
  return (_use_nodal_scaling || _derive_c_from_elasticity) ? _scaled_contact_pressure
                                                           : _lm_var->adSlnLower();
}

void
LMWeightedGapUserObject::initialize()
{
  WeightedGapUserObject::initialize();
  initializeNodalScaling();
  clearDerivedC();
}

void
LMWeightedGapUserObject::finalize()
{
  WeightedGapUserObject::finalize();
  finalizeNodalScaling();
  if (_derive_c_from_elasticity && _derived_c_needs_update)
    finalizeDerivedC();
}

void
LMWeightedGapUserObject::computeQpIProperties()
{
  WeightedGapUserObject::computeQpIProperties();
  computeQpINodalScaling();
  accumulateDerivedCIfNeeded();
}

void
LMWeightedGapUserObject::initializeNodalScaling()
{
  if (!_use_nodal_scaling)
    return;

  _dof_to_covered_fraction_sum.clear();
  _dof_to_nodal_scale.clear();
  _elem_to_full_nodal_integral.clear();
  _dof_to_full_normalization.clear();
  _full_normalization_elems.clear();
}

void
LMWeightedGapUserObject::computeQpINodalScaling()
{
  if (!_use_nodal_scaling)
    return;

  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));

  // Numerator of kappa_j (Popp 2013 eq. 36): covered fraction (int_{e_int} N_j)/(int_e N_j) summed
  // over adjacent elements. Both use the standard N_j (fePhiLower), not the dual test, so kappa_j
  // is 1 at full coverage in every coordinate system.
  const auto & std_phi = _assembly.fePhiLower<Real>(_disp_x_var->feType());
  const auto & full_integrals = fullNodalIntegrals(_lower_secondary_elem);
  _dof_to_covered_fraction_sum[dof] += std_phi[_i][_qp] * _qp_factor / full_integrals[_i];

  // Coverage-independent divisor for `normalize_c` (see normalizeCDivisor()): sum the full-element
  // integral int_e N_j once per distinct adjacent element, not once per quadrature point, since it
  // does not depend on qp and multiple mortar segments may cover the same element.
  if (_full_normalization_elems[dof].insert(_lower_secondary_elem->id()).second)
    _dof_to_full_normalization[dof] += full_integrals[_i];
}

void
LMWeightedGapUserObject::finalizeNodalScaling()
{
  if (!_use_nodal_scaling)
    return;

  // Reduce the processor-local numerators (the thread sums only rank-owned elements).
  // send_data_back = true: non-owner ranks also need kappa_j for the primary-side coupling.
  Moose::Mortar::Contact::communicateRealObject(_dof_to_covered_fraction_sum,
                                                _subproblem.mesh(),
                                                _nodal,
                                                _communicator,
                                                /*send_data_back=*/true);

  // Same reduction for the normalize_c divisor sum; also needed on non-owner ranks.
  Moose::Mortar::Contact::communicateRealObject(_dof_to_full_normalization,
                                                _subproblem.mesh(),
                                                _nodal,
                                                _communicator,
                                                /*send_data_back=*/true);

  // Divide by n_j^e (adjacent-element count, including fully dropped neighbors) for the eq. 36
  // mean; the fully ghosted mortar interface makes that count global on every process.
  const auto & nodes_to_secondary_elem = amg().nodesToSecondaryElem();
  for (const auto & [dof, fraction_sum] : _dof_to_covered_fraction_sum)
    _dof_to_nodal_scale[dof] =
        fraction_sum / libmesh_map_find(nodes_to_secondary_elem, dof->id()).size();
}

const std::vector<Real> &
LMWeightedGapUserObject::fullNodalIntegrals(const Elem * const elem)
{
  const auto it = _elem_to_full_nodal_integral.find(elem->id());
  if (it != _elem_to_full_nodal_integral.end())
    return it->second;

  // A separate finite element avoids reinit of the shared mortar-segment state.
  const std::vector<Real> & JxW = _nodal_scaling_fe->get_JxW();
  const std::vector<Point> & q_points = _nodal_scaling_fe->get_xyz();
  const std::vector<std::vector<Real>> & phi = _nodal_scaling_fe->get_phi();
  _nodal_scaling_fe->reinit(elem);

  std::vector<Real> integrals(phi.size(), 0);
  for (const auto qp : make_range(_nodal_scaling_qrule->n_points()))
  {
    Real coord;
    coordTransformFactor(_subproblem, elem->subdomain_id(), q_points[qp], coord);
    for (const auto j : index_range(integrals))
      integrals[j] += phi[j][qp] * JxW[qp] * coord;
  }

  return _elem_to_full_nodal_integral.emplace(elem->id(), std::move(integrals)).first->second;
}

void
LMWeightedGapUserObject::reinit()
{
  if (!_use_nodal_scaling && !_derive_c_from_elasticity)
    return;

  // The stored multiplier is scaled (zhat_j = kappa_j lambda_j, and with derived physical scaling
  // zhat_j = D_j y_j for the stored y_j); interpolate the physical pressure
  // sum_j Phi_j (D_j y_j/kappa_j) for the coupling (Popp 2013 eq. 39), cached once per segment.
  // Phi_j is Real, so the multiplier derivatives are seeded exactly.
  const auto & phi = _lm_var->phiLower();
  const Elem * const lower_elem = _assembly.lowerDElem();
  const auto sys_num = _lm_var->sys().number();
  const auto var_num = _lm_var->number();
  const auto & current_solution = *_lm_var->sys().currentSolution();

  mooseAssert(phi.size(), "The Lagrange multiplier should have lower-dimensional shape functions");
  const std::size_t n_qp = phi[0].size();
  _scaled_contact_pressure.resize(n_qp);
  for (const auto qp : make_range(n_qp))
    _scaled_contact_pressure[qp] = 0;

  // Loop over the multiplier's shape functions, which on a second-order mesh are fewer than the
  // element's nodes
  for (const auto j : index_range(phi))
  {
    const Node * const node = lower_elem->node_ptr(j);
    const auto dof_index = node->dof_number(sys_num, var_num, /*component=*/0);
    ADReal lm_value = current_solution(dof_index);
    Moose::derivInsert(lm_value.derivatives(), dof_index, 1.);
    const ADReal physical_pressure = lm_value * derivedPressureScale(node) / nodalScale(node);
    for (const auto qp : make_range(n_qp))
      _scaled_contact_pressure[qp] += phi[j][qp] * physical_pressure;
  }
}

Real
LMWeightedGapUserObject::getNormalContactPressure(const Node * const node) const
{
  const auto sys_num = _lm_var->sys().number();
  const auto var_num = _lm_var->number();
  if (!node->n_dofs(sys_num, var_num))
    mooseError("No degrees of freedom for the Lagrange multiplier at the node. If this is being "
               "called from an aux kernel make sure that your aux variable has the same order as "
               "your Lagrange multiplier");

  const auto dof_number = node->dof_number(sys_num, var_num, /*component=*/0);
  // Recover the physical pressure lambda_j = D_j y_j / kappa_j (Popp 2013 eq. 39 for kappa_j;
  // D_j is 1 unless the physical stiffness scale is derived). nodalScale() is keyed by the
  // displaced-mesh node pointer, and callers may pass a different pointer of the same id, so map
  // through the mesh to match -- exactly as getNormalGap() does above.
  const auto * const dof = _subproblem.mesh().nodePtr(node->id());
  const Real raw_value = (*_lm_var->sys().currentSolution())(dof_number);
  return raw_value * derivedPressureScale(dof) / nodalScale(dof);
}

const ADVariableValue &
LMWeightedGapUserObject::scaledLowerSln(const MooseVariableFE<Real> & lm_var,
                                        ADVariableValue & cache) const
{
  const auto & phi = lm_var.phiLower();
  const auto & dof_indices = lm_var.dofIndicesLower();
  const auto & solution = *lm_var.sys().currentSolution();
  const auto n_qp = phi.size() == 0 ? 0 : phi[0].size();
  cache.resize(n_qp);
  for (const auto qp : make_range(n_qp))
  {
    ADReal value = 0;
    for (const auto i : index_range(dof_indices))
    {
      const auto dof_index = dof_indices[i];
      ADReal dof_value = solution(dof_index);
      Moose::derivInsert(dof_value.derivatives(), dof_index, 1.);
      const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(i));
      const Real scale = derivedPressureScale(dof);
      value += phi[i][qp] * scale * dof_value;
    }
    cache[qp] = value;
  }
  return cache;
}

void
LMWeightedGapUserObject::timestepSetup()
{
  WeightedGapUserObject::timestepSetup();
  _derived_c_needs_update = true;
}

void
LMWeightedGapUserObject::meshChanged()
{
  WeightedGapUserObject::meshChanged();
  _derived_c_needs_update = true;
}

void
LMWeightedGapUserObject::clearDerivedC()
{
  if (_derived_c_needs_update)
    _dof_to_derived_c.clear();
}

void
LMWeightedGapUserObject::finalizeDerivedC()
{
  Moose::Mortar::Contact::communicateVelocities(_dof_to_derived_c,
                                                _subproblem.mesh(),
                                                _nodal,
                                                _communicator,
                                                /*send_data_back=*/false);
  for (auto & [dof, scale_and_weight] : _dof_to_derived_c)
  {
    auto & [scale, weight] = scale_and_weight;
    if (!std::isfinite(weight) || weight <= 0.0)
      mooseError("The mortar normalization for physical contact scaling at DOF ",
                 dof->id(),
                 " must be finite and positive.");
    scale /= weight;
    if (!std::isfinite(scale) || scale <= 0.0)
      mooseError("The physical contact stiffness scale at DOF ",
                 dof->id(),
                 " must be finite and positive.");
  }
  _derived_c_needs_update = false;
}

void
LMWeightedGapUserObject::accumulateDerivedCIfNeeded()
{
  if (!_derive_c_from_elasticity || !_derived_c_needs_update)
    return;
  if (_use_automatic_differentiation)
    accumulateDerivedC<true>();
  else
    accumulateDerivedC<false>();
}

template <bool is_ad>
void
LMWeightedGapUserObject::fetchElasticityTensorProperties(const std::string & sec_name,
                                                         const std::string & pri_name)
{
  if constexpr (is_ad)
  {
    _elasticity_tensor_secondary_ad = &getGenericMaterialProperty<RankFourTensor, true>(sec_name);
    _elasticity_tensor_primary_ad =
        &getGenericNeighborMaterialProperty<RankFourTensor, true>(pri_name);
  }
  else
  {
    _elasticity_tensor_secondary = &getGenericMaterialProperty<RankFourTensor, false>(sec_name);
    _elasticity_tensor_primary =
        &getGenericNeighborMaterialProperty<RankFourTensor, false>(pri_name);
  }
}

template <bool is_ad>
void
LMWeightedGapUserObject::accumulateDerivedC()
{
  const GenericMaterialProperty<RankFourTensor, is_ad> * sec_ptr;
  const GenericMaterialProperty<RankFourTensor, is_ad> * pri_ptr;
  if constexpr (is_ad)
  {
    sec_ptr = _elasticity_tensor_secondary_ad;
    pri_ptr = _elasticity_tensor_primary_ad;
  }
  else
  {
    sec_ptr = _elasticity_tensor_secondary;
    pri_ptr = _elasticity_tensor_primary;
  }

  const RealVectorValue & n = _normals[_i];
  GenericReal<is_ad> C_nn_sec_generic = 0;
  GenericReal<is_ad> C_nn_pri_generic = 0;
  for (const auto a : make_range(3))
    for (const auto b : make_range(3))
      for (const auto c : make_range(3))
        for (const auto d : make_range(3))
        {
          const Real w = n(a) * n(b) * n(c) * n(d);
          C_nn_sec_generic += w * (*sec_ptr)[_qp](a, b, c, d);
          C_nn_pri_generic += w * (*pri_ptr)[_qp](a, b, c, d);
        }

  const Real C_nn_sec = MetaPhysicL::raw_value(C_nn_sec_generic);
  const Real C_nn_pri = MetaPhysicL::raw_value(C_nn_pri_generic);
  const auto & reference_mesh = _fe_problem.mesh();
  const auto * const reference_secondary_face = reference_mesh.elemPtr(_lower_secondary_elem->id());
  const auto * const reference_primary_face = reference_mesh.elemPtr(_lower_primary_elem->id());
  const auto * const reference_secondary_parent =
      reference_mesh.elemPtr(_lower_secondary_elem->interior_parent()->id());
  const auto * const reference_primary_parent =
      reference_mesh.elemPtr(_lower_primary_elem->interior_parent()->id());
  const Real secondary_face_measure = reference_secondary_face->volume();
  const Real primary_face_measure = reference_primary_face->volume();
  const Real secondary_parent_volume = reference_secondary_parent->volume();
  const Real primary_parent_volume = reference_primary_parent->volume();
  if (!std::isfinite(C_nn_sec) || C_nn_sec <= 0.0 || !std::isfinite(C_nn_pri) || C_nn_pri <= 0.0)
    mooseError("The normal acoustic stiffness on both contact bodies must be finite and positive ",
               "when physical contact scaling is used.");
  if (!std::isfinite(secondary_face_measure) || secondary_face_measure <= 0.0 ||
      !std::isfinite(primary_face_measure) || primary_face_measure <= 0.0 ||
      !std::isfinite(secondary_parent_volume) || secondary_parent_volume <= 0.0 ||
      !std::isfinite(primary_parent_volume) || primary_parent_volume <= 0.0)
    mooseError("The normal contact lengths on both contact bodies must be finite and positive ",
               "when physical contact scaling is used.");

  const Real h_secondary = secondary_parent_volume / secondary_face_measure;
  const Real h_primary = primary_parent_volume / primary_face_measure;
  const Real physical_scale = 1.0 / (h_secondary / C_nn_sec + h_primary / C_nn_pri);
  const Real test_weight = (*_test)[_i][_qp] * _qp_factor;
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));
  auto & [scale_sum, weight_sum] = _dof_to_derived_c[dof];
  scale_sum += physical_scale * test_weight;
  weight_sum += test_weight;
}

template void LMWeightedGapUserObject::fetchElasticityTensorProperties<false>(const std::string &,
                                                                              const std::string &);
template void LMWeightedGapUserObject::fetchElasticityTensorProperties<true>(const std::string &,
                                                                             const std::string &);
template void LMWeightedGapUserObject::accumulateDerivedC<false>();
template void LMWeightedGapUserObject::accumulateDerivedC<true>();
