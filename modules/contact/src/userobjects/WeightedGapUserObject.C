//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedGapUserObject.h"
#include "MooseVariableField.h"
#include "SubProblem.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "MortarContactUtils.h"
#include "AutomaticMortarGeneration.h"

#include "libmesh/quadrature.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/fe.h"

#include <limits>

InputParameters
WeightedGapUserObject::validParams()
{
  InputParameters params = MortarUserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.addRequiredCoupledVar("disp_x", "The x displacement variable");
  params.addRequiredCoupledVar("disp_y", "The y displacement variable");
  params.addCoupledVar("disp_z", "The z displacement variable");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<bool>("interpolate_normals") = false;
  params.addParam<bool>(
      "use_nodal_scaling",
      false,
      "Whether to apply the node-based Lagrange-multiplier scaling of Popp, Seitz, Gee & Wall "
      "(2013), CMAME 264:67-80, Sec. 4.2, to improve the conditioning of the linear system when "
      "secondary elements are only partially covered (edge dropping). Requires "
      "'correct_edge_dropping = true'. Supported for frictionless normal Lagrange multiplier "
      "contact (LMWeightedGapUserObject with ComputeWeightedGapLMMechanicalContact) using a "
      "first-order Lagrange multiplier on a replicated mesh, in Cartesian, RZ, or spherical "
      "coordinates, and in parallel.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

WeightedGapUserObject::WeightedGapUserObject(const InputParameters & parameters)
  : MortarUserObject(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _nodal(getVar("disp_x", 0)->feType().family == LAGRANGE),
    _disp_x_var(getVar("disp_x", 0)),
    _disp_y_var(getVar("disp_y", 0)),
    _has_disp_z(isCoupled("disp_z")),
    _disp_z_var(_has_disp_z ? getVar("disp_z", 0) : nullptr),
    _secondary_disp_x(_disp_x_var->adSln()),
    _primary_disp_x(_disp_x_var->adSlnNeighbor()),
    _secondary_disp_y(_disp_y_var->adSln()),
    _primary_disp_y(_disp_y_var->adSlnNeighbor()),
    _secondary_disp_z(_has_disp_z ? &_disp_z_var->adSln() : nullptr),
    _primary_disp_z(_has_disp_z ? &_disp_z_var->adSlnNeighbor() : nullptr),
    _coord(_assembly.mortarCoordTransformation()),
    _use_nodal_scaling(getParam<bool>("use_nodal_scaling"))
{
  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must be true for the WeightedGapUserObject object");
}

void
WeightedGapUserObject::initialSetup()
{
  MortarUserObject::initialSetup();
  _test = &test();

  if (_use_nodal_scaling)
  {
    // Only the LM formulation applies the scaling; other user objects would silently ignore it.
    if (!nodalScalingApplied())
      paramError("use_nodal_scaling",
                 "Node-based scaling is only implemented for the Lagrange multiplier contact "
                 "formulation (LMWeightedGapUserObject); it would be silently ignored here.");

    // Scaling rescales the partially covered multiplier DOFs that the default treatment zeroes.
    if (!getParam<bool>("correct_edge_dropping"))
      paramError("use_nodal_scaling",
                 "Node-based scaling requires 'correct_edge_dropping = true'.");

    // n_j^e (adjacency count) is globally complete only on a replicated mesh; on a distributed mesh
    // kappa_j would be partition dependent. Multi-rank on a replicated mesh is fine.
    if (_subproblem.mesh().isDistributedMesh())
      paramError("use_nodal_scaling",
                 "Node-based scaling is not supported on a distributed mesh; run mortar contact on "
                 "a replicated mesh (Mesh/parallel_type = REPLICATED). Parallel multi-rank "
                 "execution on a replicated mesh is supported.");
  }
}

void
WeightedGapUserObject::computeQpProperties()
{
  // Trim interior node variable derivatives
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_disp{
      {_primary_disp_x[_qp], _primary_disp_y[_qp], _has_disp_z ? (*_primary_disp_z)[_qp] : 0}};
  std::array<ADReal, 3> secondary_disp{{_secondary_disp_x[_qp],
                                        _secondary_disp_y[_qp],
                                        _has_disp_z ? (*_secondary_disp_z)[_qp] : 0}};

  trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, primary_disp, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, secondary_disp, true);

  const ADReal & prim_x = primary_disp[0];
  const ADReal & prim_y = primary_disp[1];
  const ADReal * prim_z = nullptr;
  if (_has_disp_z)
    prim_z = &primary_disp[2];

  const ADReal & sec_x = secondary_disp[0];
  const ADReal & sec_y = secondary_disp[1];
  const ADReal * sec_z = nullptr;
  if (_has_disp_z)
    sec_z = &secondary_disp[2];

  // Compute gap vector
  ADRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

  // Generic displacement for interface problems
  _qp_displacement_nodal(0) = prim_x - sec_x;
  _qp_displacement_nodal(1) = prim_y - sec_y;
  if (_has_disp_z)
    _qp_displacement_nodal(2) = *prim_z - *sec_z;

  _qp_displacement_nodal *= _JxW_msm[_qp] * _coord[_qp];

  gap_vec(0).derivatives() = prim_x.derivatives() - sec_x.derivatives();
  gap_vec(1).derivatives() = prim_y.derivatives() - sec_y.derivatives();
  if (_has_disp_z)
    gap_vec(2).derivatives() = prim_z->derivatives() - sec_z->derivatives();

  // Compute integration point quantities: Normals (geometry) is averaged at the node, but not
  // interpolated within the weak integration.
  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_qp] * _coord[_qp];
}

void
WeightedGapUserObject::computeQpIProperties()
{
  mooseAssert(_normals.size() == _lower_secondary_elem->n_nodes(),
              "Making sure that _normals is the expected size");

  // Get the _dof_to_weighted_gap map
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));

  auto & [weighted_gap, normalization] = _dof_to_weighted_gap[dof];

  weighted_gap += (*_test)[_i][_qp] * _qp_gap_nodal * _normals[_i];
  normalization += (*_test)[_i][_qp] * _qp_factor;

  _dof_to_weighted_displacements[dof] += (*_test)[_i][_qp] * _qp_displacement_nodal;

  if (_use_nodal_scaling)
  {
    // Numerator of kappa_j (Popp 2013 eq. 36): covered fraction (int_{e_int} N_j)/(int_e N_j)
    // summed over adjacent elements, using the standard N_j (displacement basis, fePhiLower) -- not
    // the dual test, so kappa_j = 1 at full coverage in every coordinate system, RZ/spherical
    // included.
    const auto & std_phi = _assembly.fePhiLower<Real>(_disp_x_var->feType());
    _dof_to_covered_fraction_sum[dof] +=
        std_phi[_i][_qp] * _qp_factor / fullNodalIntegrals(_lower_secondary_elem)[_i];
  }
}

void
WeightedGapUserObject::initialize()
{
  _dof_to_weighted_gap.clear();
  _dof_to_weighted_displacements.clear();
  _dof_to_covered_fraction_sum.clear();
  _dof_to_nodal_scale.clear();
  _elem_to_full_nodal_integral.clear();
}

void
WeightedGapUserObject::finalize()
{
  // If the constraint is performed by the owner, then we don't need any data sent back; the owner
  // will take care of it. But if the constraint is not performed by the owner and we might have to
  // do some of the constraining ourselves, then we need data sent back to us
  const bool send_data_back = !constrainedByOwner();
  Moose::Mortar::Contact::communicateGaps(_dof_to_weighted_gap,
                                          _subproblem.mesh(),
                                          _nodal,
                                          /*normalize_c*/ true,
                                          _communicator,
                                          send_data_back);

  if (_use_nodal_scaling)
  {
    // Reduce the processor-local numerators (the thread sums only rank-owned elements).
    // send_data_back = true: non-owner ranks also need kappa_j for the primary-side coupling.
    Moose::Mortar::Contact::communicateRealObject(_dof_to_covered_fraction_sum,
                                                  _subproblem.mesh(),
                                                  _nodal,
                                                  _communicator,
                                                  /*send_data_back=*/true);

    // Divide by n_j^e (adjacent-element count, including fully dropped neighbors) for the eq. 36
    // mean; the map is globally complete on the replicated mesh.
    const auto & nodes_to_secondary_elem = amg().nodesToSecondaryElem();
    for (const auto & [dof, fraction_sum] : _dof_to_covered_fraction_sum)
      _dof_to_nodal_scale[dof] =
          fraction_sum / libmesh_map_find(nodes_to_secondary_elem, dof->id()).size();
  }
}

void
WeightedGapUserObject::execute()
{
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test->size(); ++_i)
      computeQpIProperties();
  }
}

const std::vector<Real> &
WeightedGapUserObject::fullNodalIntegrals(const Elem * const elem)
{
  const auto it = _elem_to_full_nodal_integral.find(elem->id());
  if (it != _elem_to_full_nodal_integral.end())
    return it->second;

  // Integrate int_e N_j (coordinate-weighted) per node with a temporary FE (as in
  // Assembly::elementVolume / NodalArea), avoiding reinit of the shared mortar-segment state.
  const FEType fe_type(FIRST, LAGRANGE);
  std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<Point> & q_points = fe->get_xyz();
  const std::vector<std::vector<Real>> & phi = fe->get_phi();

  QGauss qrule(elem->dim(), fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  fe->reinit(elem);

  std::vector<Real> integrals(phi.size(), 0);
  for (const auto qp : make_range(qrule.n_points()))
  {
    Real coord;
    coordTransformFactor(_subproblem, elem->subdomain_id(), q_points[qp], coord);
    for (const auto j : index_range(integrals))
      integrals[j] += phi[j][qp] * JxW[qp] * coord;
  }

  return _elem_to_full_nodal_integral.emplace(elem->id(), std::move(integrals)).first->second;
}

Real
WeightedGapUserObject::getNormalGap(const Node * const node) const
{
  const auto it = _dof_to_weighted_gap.find(_subproblem.mesh().nodePtr(node->id()));

  // We are returning the physical weighted gap for analysis purposes
  if (it != _dof_to_weighted_gap.end())
    return physicalGap(it->second);
  else
    return 0.0;
}
