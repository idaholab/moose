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
#include <map>

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
  params.addParam<bool>("elasticity_tensor_is_ad",
                        false,
                        "Whether the elasticity tensor material property was declared as an AD "
                        "property.");
  return params;
}

InputParameters
LMWeightedGapUserObject::validParams()
{
  InputParameters params = WeightedGapUserObject::validParams();
  params.addClassDescription(
      "Provides the mortar normal Lagrange multiplier for constraint enforcement.");
  addNodalNormalCoupling(params);
  params.suppressParameter<bool>("ghost_point_neighbors");
  params += LMWeightedGapUserObject::newParams();
  return params;
}

LMWeightedGapUserObject::LMWeightedGapUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    _derive_c_from_elasticity(getParam<bool>("derive_c_from_elasticity")),
    _elasticity_tensor_is_ad(getParam<bool>("elasticity_tensor_is_ad")),
    _dof_to_derived_c(declareRestartableData<std::unordered_map<dof_id_type, std::array<Real, 2>>>(
        "dof_to_derived_c")),
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

    // This FE object is shared by all fullNodalIntegrals() evaluations.
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
    if (_elasticity_tensor_is_ad)
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

void
LMWeightedGapUserObject::verifyLagrange(const MooseVariable & var,
                                        const std::string & var_param_name,
                                        const MooseVariable & reference_var,
                                        const std::string & reference_var_param_name) const
{
  verifyLagrange(var, var_param_name);

  if (var.feType() != reference_var.feType())
    paramError(var_param_name,
               "'",
               var_param_name,
               "' must use the same finite element type (order and family) as '",
               reference_var_param_name,
               "'.");

  if (var.useDual() != reference_var.useDual())
    paramError(var_param_name,
               "'",
               var_param_name,
               "' must use the same dual/standard basis setting as '",
               reference_var_param_name,
               "'.");
}

const VariableTestValue &
LMWeightedGapUserObject::test() const
{
  return _use_petrov_galerkin ? _aux_lm_var->phiLower() : _lm_var->phiLower();
}

const ADVariableValue &
LMWeightedGapUserObject::contactPressure() const
{
  // Under node-based scaling, the multiplier is zhat_j = kappa_j*lambda_j, and with derived
  // physical scaling the stored value is y_j with zhat_j = D_j y_j.
  // nodalContactPressure() and getNormalContactPressure() return the physical pressure lambda_j.
  return _lm_var->adSlnLower();
}

void
LMWeightedGapUserObject::initialize()
{
  WeightedGapUserObject::initialize();
  initializeNodalScaling();
}

void
LMWeightedGapUserObject::finalize()
{
  WeightedGapUserObject::finalize();
  finalizeNodalScaling();
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

const VariableTestValue &
LMWeightedGapUserObject::tractionBasis() const
{
  return _lm_var->phiLower();
}

ADReal
LMWeightedGapUserObject::nodalContactPressure(const Node & node) const
{
  const auto sys_num = _lm_var->sys().number();
  const auto var_num = _lm_var->number();
  mooseAssert(node.n_dofs(sys_num, var_num),
              "The Lagrange multiplier must have a degree of freedom at this secondary node.");

  // Seed the lower-dimensional nodal Lagrange multiplier derivative explicitly.
  const auto dof_index = node.dof_number(sys_num, var_num, 0);
  ADReal nodal_pressure = (*_lm_var->sys().currentSolution())(dof_index);
  if (Moose::doDerivatives(_subproblem, _sys))
    Moose::derivInsert(nodal_pressure.derivatives(), dof_index, 1.);
  // Recover the physical pressure lambda_j = D_j y_j / kappa_j (Popp 2013 eq. 39 for kappa_j).
  // nodalScale() is keyed by displaced-mesh node pointers, so resolve the supplied node id through
  // that mesh.
  const auto * const dof = _subproblem.mesh().nodePtr(node.id());
  return nodal_pressure * derivedNormalScale(dof) / nodalScale(dof);
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
  // Recover the physical pressure lambda_j = D_j y_j / kappa_j (Popp 2013 eq. 39 for kappa_j).
  // nodalScale() is keyed by the displaced-mesh node pointer, and callers may pass a different
  // pointer of the same id, so map through the mesh to match.
  const auto * const dof = _subproblem.mesh().nodePtr(node->id());
  return (*_lm_var->sys().currentSolution())(dof_number)*derivedNormalScale(dof) / nodalScale(dof);
}

Real
LMWeightedGapUserObject::derivedNormalScale(const DofObject * const dof) const
{
  return findValue(_dof_to_derived_c, dof->id(), std::array<Real, 2>{{1, 1}})[0];
}

Real
LMWeightedGapUserObject::derivedTangentialScale(const DofObject * const dof) const
{
  return findValue(_dof_to_derived_c, dof->id(), std::array<Real, 2>{{1, 1}})[1];
}

void
LMWeightedGapUserObject::finalizeDerivedC()
{
  if (!_derive_c_from_elasticity)
    return;

  // Every process that touches a node's mortar segments uses that node's scale, and a node's set of
  // touching processes can grow as the contact surfaces slide, so gather the partial sums for newly
  // covered nodes on every process. Every process sums them in the same order, which makes the
  // resulting scales identical everywhere.
  std::vector<dof_id_type> ids;
  std::vector<Real> normal_sums, tangential_sums, weight_sums;
  for (const auto & [id, sums] : _derived_c_sums)
  {
    ids.push_back(id);
    normal_sums.push_back(sums[0]);
    tangential_sums.push_back(sums[1]);
    weight_sums.push_back(sums[2]);
  }
  _derived_c_sums.clear();
  _communicator.allgather(ids, /*identical_buffer_sizes=*/false);
  _communicator.allgather(normal_sums, /*identical_buffer_sizes=*/false);
  _communicator.allgather(tangential_sums, /*identical_buffer_sizes=*/false);
  _communicator.allgather(weight_sums, /*identical_buffer_sizes=*/false);

  std::map<dof_id_type, std::array<Real, 3>> global_sums;
  for (const auto i : index_range(ids))
  {
    auto & [normal_sum, tangential_sum, weight_sum] = global_sums[ids[i]];
    normal_sum += normal_sums[i];
    tangential_sum += tangential_sums[i];
    weight_sum += weight_sums[i];
  }

  for (const auto & [id, sums] : global_sums)
  {
    const auto & [normal_sum, tangential_sum, weight_sum] = sums;
    // A node whose covered test function integral vanishes gets its scales from a later coverage
    if (weight_sum <= 0)
      continue;
    const std::array<Real, 2> scales{{normal_sum / weight_sum, tangential_sum / weight_sum}};
    for (const auto scale : scales)
      if (!std::isfinite(scale) || scale <= 0)
        mooseError(
            "The physical contact stiffness scales at node ", id, " must be finite and positive.");
    _dof_to_derived_c.emplace(id, scales);
  }
}

void
LMWeightedGapUserObject::accumulateDerivedCIfNeeded()
{
  if (!_derive_c_from_elasticity || _dof_to_derived_c.count(_lower_secondary_elem->node_id(_i)))
    return;
  if (_elasticity_tensor_is_ad)
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

  // The acoustic tensor Q_ij = C_ikjl n_k n_l gives the stiffness of a body against a traction
  // wave along the contact normal n: n.Q.n is the normal (constrained) modulus, and t.Q.t is the
  // shear modulus for a tangent t. The tangential modulus is averaged over the tangent plane,
  // (tr Q - n.Q.n) / (dim - 1) with the trace over the mesh dimensions, which is independent of
  // the tangent basis; the friction law needs a single tangential constant per node so that the
  // regularized slip stays parallel to the slip velocity.
  const RealVectorValue & n = _normals[_i];
  const auto dim = _subproblem.mesh().dimension();
  const auto acoustic_moduli = [&n, dim](const auto & C)
  {
    Real normal = 0, trace = 0;
    for (const auto i : make_range(3))
      for (const auto j : make_range(3))
      {
        Real Q_ij = 0;
        for (const auto k : make_range(3))
          for (const auto l : make_range(3))
            Q_ij += MetaPhysicL::raw_value(C(i, k, j, l)) * n(k) * n(l);
        normal += n(i) * Q_ij * n(j);
        if (i == j && i < dim)
          trace += Q_ij;
      }
    return std::array<Real, 2>{{normal, (trace - normal) / (dim - 1)}};
  };
  const auto [C_nn_sec, C_tt_sec] = acoustic_moduli((*sec_ptr)[_qp]);
  const auto [C_nn_pri, C_tt_pri] = acoustic_moduli((*pri_ptr)[_qp]);

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
  for (const auto modulus : {C_nn_sec, C_tt_sec, C_nn_pri, C_tt_pri})
    if (!std::isfinite(modulus) || modulus <= 0.0)
      mooseError("The normal and shear acoustic stiffnesses on both contact bodies must be finite "
                 "and positive when physical contact scaling is used.");
  if (!std::isfinite(secondary_face_measure) || secondary_face_measure <= 0.0 ||
      !std::isfinite(primary_face_measure) || primary_face_measure <= 0.0 ||
      !std::isfinite(secondary_parent_volume) || secondary_parent_volume <= 0.0 ||
      !std::isfinite(primary_parent_volume) || primary_parent_volume <= 0.0)
    mooseError("The normal contact lengths on both contact bodies must be finite and positive ",
               "when physical contact scaling is used.");

  const Real h_secondary = secondary_parent_volume / secondary_face_measure;
  const Real h_primary = primary_parent_volume / primary_face_measure;
  // The two adjacent element layers act as springs in series
  const Real normal_scale = 1.0 / (h_secondary / C_nn_sec + h_primary / C_nn_pri);
  const Real tangential_scale = 1.0 / (h_secondary / C_tt_sec + h_primary / C_tt_pri);
  const Real test_weight = (*_test)[_i][_qp] * _qp_factor;
  auto & [normal_sum, tangential_sum, weight_sum] =
      _derived_c_sums[_lower_secondary_elem->node_id(_i)];
  normal_sum += normal_scale * test_weight;
  tangential_sum += tangential_scale * test_weight;
  weight_sum += test_weight;
}

template void LMWeightedGapUserObject::fetchElasticityTensorProperties<false>(const std::string &,
                                                                              const std::string &);
template void LMWeightedGapUserObject::fetchElasticityTensorProperties<true>(const std::string &,
                                                                             const std::string &);
template void LMWeightedGapUserObject::accumulateDerivedC<false>();
template void LMWeightedGapUserObject::accumulateDerivedC<true>();
