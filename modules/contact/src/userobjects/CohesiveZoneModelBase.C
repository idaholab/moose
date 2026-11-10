//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneModelBase.h"

#include "WeightedGapUserObject.h"
#include "AutomaticMortarGeneration.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseException.h"
#include "ADUtils.h"
#include "MortarContactUtils.h"
#include "CohesiveZoneModelTools.h"

#include "libmesh/enum_to_string.h"
#include "libmesh/fe_base.h"
#include "libmesh/fe_interface.h"
#include "libmesh/fe_map.h"
#include "libmesh/quadrature.h"
#include "libmesh/quadrature_gauss.h"

#include <cmath>
#include <set>

InputParameters
CohesiveZoneModelBase::validParams()
{
  InputParameters params = MortarUserObject::validParams();
  params.addClassDescription(
      "Base class for mortar-based cohesive zone models. It runs on the undisplaced mortar "
      "interface and measures the displacement jump between the bonded material points in a local "
      "frame that follows the deformed secondary surface.");
  params.addRequiredCoupledVar("disp_x", "The x displacement variable");
  params.addRequiredCoupledVar("disp_y", "The y displacement variable");
  params.addCoupledVar("disp_z", "The z displacement variable");
  params.set<bool>("use_displaced_mesh") = false;
  params.set<bool>("interpolate_normals") = false;
  params.addParam<bool>(
      "use_dual",
      false,
      "Whether to weight the nodal mortar integrals and interpolate the traction with the dual "
      "basis of the displacement shape functions. The dual basis has a positive integral for "
      "every node, which the standard basis of TRI6 and QUAD8 faces does not.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // The local frame at a node depends on the displacements of all secondary faces incident to it
  WeightedGapUserObject::addNodalNormalCoupling(params);
  params.suppressParameter<bool>("ghost_point_neighbors");

  // The cohesive zone model does not apply contact. These parameters configured the contact of an
  // earlier version; a nonzero penalty or friction is rejected in the constructor, because dropping
  // it silently would remove the contact the input asked for.
  const std::string contact_uo_message =
      "The cohesive zone model does not apply contact. Set this parameter on a "
      "PenaltyWeightedGapUserObject (or a PenaltyFrictionUserObject for friction) and point the "
      "mortar contact constraints at it.";
  params.addDeprecatedParam<Real>(
      "penalty", "The normal contact penalty, which is not applied.", contact_uo_message);
  params.addDeprecatedParam<Real>("penalty_friction",
                                  "The tangential contact penalty, which is not applied.",
                                  contact_uo_message);
  params.addDeprecatedParam<Real>("friction_coefficient",
                                  "The friction coefficient, which is not applied.",
                                  contact_uo_message);
  params.addDeprecatedParam<VariableName>("secondary_variable",
                                          "The contact constraint variable, which is not used.",
                                          contact_uo_message);
  params.addDeprecatedParam<bool>(
      "use_physical_gap", "The contact gap measure, which is not used.", contact_uo_message);
  params.addDeprecatedParam<std::vector<VariableName>>(
      "displacements",
      "The displacement variables, which are not used.",
      "The cohesive zone model couples the displacements through 'disp_x', 'disp_y' and 'disp_z'.");
  return params;
}

CohesiveZoneModelBase::CohesiveZoneModelBase(const InputParameters & parameters)
  : MortarUserObject(parameters),
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
    _dt(_fe_problem.dt()),
    _use_dual(getParam<bool>("use_dual")),
    _epsilon_tolerance(1.0e-40),
    _dof_to_damage(declareRestartableData<std::unordered_map<dof_id_type, std::pair<ADReal, Real>>>(
        "dof_do_damage", std::unordered_map<dof_id_type, std::pair<ADReal, Real>>{}))
{
  if (_use_dual)
    _mci_assembly.activateDual();

  // The traction is applied on every process contributing to a node's integrals (see finalize()),
  // so those processes need the point-neighbor couplings of nonlocal secondary elements
  _fe_problem
      .getMortarInterface(std::make_pair(_primary_id, _secondary_id),
                          std::make_pair(_primary_subdomain_id, _secondary_subdomain_id),
                          /*on_displaced=*/false)
      .requestNonlocalSecondaryCouplings();

  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "The cohesive zone model pairs the bonded material points on the undisplaced mortar "
               "interface, so 'use_displaced_mesh' must be false.");

  if (getParam<bool>("interpolate_normals"))
    paramError("interpolate_normals",
               "The cohesive zone model uses the nodal normals and cannot interpolate them.");

  if (_subproblem.mesh().dimension() == 3 && !_has_disp_z)
    paramError("disp_z", "The z displacement is required for a three-dimensional interface.");

  // A zero penalty or friction applied no contact, so only a nonzero one changes the results
  if (isParamValid("penalty") && getParam<Real>("penalty") != 0)
    paramError("penalty",
               "The cohesive zone model does not apply normal contact. Add a "
               "PenaltyWeightedGapUserObject with this penalty and point the normal mortar "
               "contact constraints at it.");
  for (const auto & param : {"penalty_friction", "friction_coefficient"})
    if (isParamValid(param) && getParam<Real>(param) != 0)
      paramError(param,
                 "The cohesive zone model does not apply frictional contact. Add a "
                 "PenaltyFrictionUserObject with this parameter and point the mortar contact "
                 "constraints at it. Its friction does not depend on the cohesive damage.");
}

void
CohesiveZoneModelBase::initialSetup()
{
  MortarUserObject::initialSetup();
  _test = _use_dual ? &_mci_assembly.feDualPhiLower<Real>(_disp_x_var->feType())
                    : &_disp_x_var->phiLower();

  const std::array<std::pair<const MooseVariable *, const char *>, 3> displacement_variables{
      {{_disp_x_var, "disp_x"}, {_disp_y_var, "disp_y"}, {_disp_z_var, "disp_z"}}};
  for (const auto & [variable, parameter_name] : displacement_variables)
    if (variable)
    {
      if (!variable->isNodal())
        paramError(parameter_name,
                   "The cohesive zone model requires a nodal displacement variable.");
      if (&variable->sys() != &_sys)
        paramError(parameter_name,
                   "The cohesive zone model requires the displacement variables to be in the "
                   "nonlinear system that assembles its traction.");
    }

  // The interface frame differentiates the geometric map of each secondary face at its nodes
  for (const Elem * const face :
       _subproblem.mesh().getMesh().active_subdomain_elements_ptr_range(_secondary_subdomain_id))
  {
    const FEType fe_type(face->default_order(), FEMap::map_fe_type(*face));
    if (FEInterface::n_shape_functions(fe_type, face) != face->n_nodes())
      mooseError("The geometric map of secondary mortar element ",
                 face->id(),
                 " does not have one shape function per element node, which the cohesive zone "
                 "interface frame requires.");
  }

  // The nodal jump and strengths are mortar integrals divided by the integral of the nodal test
  // function. For the standard basis that integral is zero at TRI6 vertices and negative at QUAD8
  // corners, so check every secondary face type once on its reference element.
  std::set<std::pair<ElemType, unsigned int>> checked_face_types;
  const auto lower_dim = _subproblem.mesh().dimension() - 1;
  const auto & test_fe_type = _disp_x_var->feType();
  for (const Elem * const face :
       _subproblem.mesh().getMesh().active_subdomain_elements_ptr_range(_secondary_subdomain_id))
  {
    if (FEInterface::n_shape_functions(test_fe_type, face) != face->n_nodes())
      paramError("disp_x",
                 "The cohesive zone model requires one nodal displacement shape function for "
                 "each secondary face node, but the ",
                 Utility::enum_to_string(face->type()),
                 " secondary faces of this mesh do not have that property at the active finite "
                 "element order.");

    if (!checked_face_types.emplace(face->type(), face->p_level()).second)
      continue;

    const auto reference_face = Elem::build(face->type());
    reference_face->set_p_level(face->p_level());
    std::vector<std::unique_ptr<Node>> reference_nodes;
    for (const auto k : make_range(reference_face->n_nodes()))
    {
      reference_nodes.push_back(Node::build(reference_face->master_point(k), k));
      reference_face->set_node(k, reference_nodes.back().get());
    }

    // The integrand is a single shape function on the reference face; twice its order integrates
    // it exactly with a margin.
    auto fe = FEBase::build(lower_dim, test_fe_type);
    const auto effective_order = test_fe_type.order.get_order() + face->p_level();
    QGauss qrule(lower_dim, static_cast<Order>(2 * effective_order));
    fe->attach_quadrature_rule(&qrule);
    const auto & phi = _use_dual ? fe->get_dual_phi() : fe->get_phi();
    const auto & JxW = fe->get_JxW();
    fe->reinit(reference_face.get());

    for (const auto j : index_range(phi))
    {
      Real nodal_integral = 0;
      for (const auto qp : index_range(JxW))
        nodal_integral += phi[j][qp] * JxW[qp];
      // Relative to the reference face measure, so that round-off of an exactly zero integral
      // (TRI6 vertices) is caught; TOLERANCE (1e-6) is far below the smallest positive nodal
      // integral of the supported faces (1/36 of the face for standard QUAD9 corners, 1/20 for
      // dual QUAD8 corners).
      if (nodal_integral <= TOLERANCE * reference_face->volume())
        paramError("disp_x",
                   "The cohesive zone model normalizes nodal mortar integrals by the integral of "
                   "each nodal test function, which is not positive for the ",
                   Utility::enum_to_string(face->type()),
                   " secondary faces of this mesh at the active finite element order.",
                   _use_dual ? " Use a face type and order whose nodal test functions all have "
                               "positive integrals."
                             : " Set 'use_dual = true' to use the dual basis, whose nodal "
                               "integrals are positive, or use first-order faces.");
    }
  }

  // The nodal integrals run over the mortar segments only, that is over the part of each secondary
  // face that pairs with the primary surface. Second-order test functions, standard and dual alike,
  // are negative on part of the face, so the covered integral of a vertex function is not positive
  // once its support is more than a quarter uncovered, which flips the sign of the jump.
  // First-order test functions are nonnegative, so a partly covered first-order face is safe. The
  // pairing is fixed on the reference mesh, so this needs checking only once.
  if (test_fe_type.order > FIRST)
  {
    const auto & segment_info = amg().mortarSegmentMeshElemToInfo();
    for (const auto & [secondary_id, segments] : amg().secondariesToMortarSegments())
    {
      const Elem & face = *_subproblem.mesh().elemPtr(secondary_id);
      Real covered_measure = 0;
      for (const Elem * const segment : segments)
        if (libmesh_map_find(segment_info, segment).primary_elem)
          covered_measure += segment->volume();
      // The relative tolerances of the edge-dropping test of AutomaticMortarGeneration: in 3D the
      // mortar segments lie on the linearized face, whose measure differs from the curved face's
      const Real tolerance = _subproblem.mesh().dimension() == 3 ? 0.1 : TOLERANCE;
      if (covered_measure < (1 - tolerance) * face.volume())
        paramError("disp_x",
                   "Secondary face ",
                   secondary_id,
                   " of the cohesive zone interface is only partly covered by the primary surface. "
                   "The nodal mortar integrals of second-order test functions are not positive on "
                   "a partly covered face. Use first-order elements where the secondary surface "
                   "extends beyond the primary surface.");
    }
  }
}

void
CohesiveZoneModelBase::initialize()
{
  _dof_to_normalization.clear();
  _dof_to_weighted_displacements.clear();
  _dof_to_weighted_reference_offset.clear();
  _dof_to_interface_displacement_jump.clear();
  _dof_to_czm_traction.clear();
  _dof_to_global_czm_traction.clear();
}

void
CohesiveZoneModelBase::execute()
{
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test->size(); ++_i)
      computeQpIProperties();
  }
}

void
CohesiveZoneModelBase::computeQpProperties()
{
  // Trim the derivatives with respect to interior node displacements, whose shape functions vanish
  // on the face
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  const std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_disp{
      {_primary_disp_x[_qp], _primary_disp_y[_qp], _has_disp_z ? (*_primary_disp_z)[_qp] : 0}};
  std::array<ADReal, 3> secondary_disp{{_secondary_disp_x[_qp],
                                        _secondary_disp_y[_qp],
                                        _has_disp_z ? (*_secondary_disp_z)[_qp] : 0}};

  trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, primary_disp, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, secondary_disp, true);

  _qp_factor = _JxW_msm[_qp] * _coord[_qp];

  for (const auto component : make_range(3))
    _qp_displacement(component) =
        (primary_disp[component] - secondary_disp[component]) * _qp_factor;

  // On the undisplaced mortar interface these are the reference positions of the paired points
  _qp_reference_offset = (_phys_points_primary[_qp] - _phys_points_secondary[_qp]) * _qp_factor;
}

void
CohesiveZoneModelBase::computeQpIProperties()
{
  const auto * const dof = cast_ptr<const DofObject *>(_lower_secondary_elem->node_ptr(_i));
  const auto test = (*_test)[_i][_qp];

  _dof_to_weighted_displacements[dof] += test * _qp_displacement;
  _dof_to_weighted_reference_offset[dof] += test * _qp_reference_offset;
  _dof_to_normalization[dof] += test * _qp_factor;
}

void
CohesiveZoneModelBase::finalize()
{
  // The traction is applied by every process that holds a contributing mortar segment, so the
  // summed nodal integrals are sent back to all contributors
  const auto & mesh = _subproblem.mesh();
  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_normalization, mesh, /*nodal=*/true, _communicator, /*send_data_back=*/true);
  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_weighted_displacements, mesh, /*nodal=*/true, _communicator, /*send_data_back=*/true);
  Moose::Mortar::Contact::communicateRealObject(_dof_to_weighted_reference_offset,
                                                mesh,
                                                /*nodal=*/true,
                                                _communicator,
                                                /*send_data_back=*/true);
}

void
CohesiveZoneModelBase::reinit()
{
  const auto n_qp = _qrule_msm->n_points();
  for (auto & traction : _czm_interpolated_traction)
  {
    traction.resize(n_qp);
    for (const auto qp : make_range(n_qp))
      traction[qp] = 0;
  }

  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    // The nodal traction is computed once per execution and reused by every segment of the node
    auto it = _dof_to_global_czm_traction.find(node);
    if (it == _dof_to_global_czm_traction.end())
    {
      // Nodes without mortar segments carry no traction
      if (!_dof_to_normalization.count(node))
        continue;

      const auto rotation = computeLocalDisplacementJump(*node, _normals[i]);
      computeCZMTraction(node);
      computeGlobalTraction(node, rotation);
      it = _dof_to_global_czm_traction.find(node);
    }

    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(n_qp))
      for (const auto component : make_range(3))
        _czm_interpolated_traction[component][qp] += test_i[qp] * it->second(component);
  }
}

ADRankTwoTensor
CohesiveZoneModelBase::computeLocalDisplacementJump(const Node & node,
                                                    const RealVectorValue & orientation_normal)
{
  using std::isfinite;

  const auto [reference_frame, rotation] = computeInterfaceFrames(node, orientation_normal);

  const auto & weighted_reference_offset =
      libmesh_map_find(_dof_to_weighted_reference_offset, &node);
  const ADRealVectorValue weighted_jump =
      rotation.transpose() *
          (libmesh_map_find(_dof_to_weighted_displacements, &node) + weighted_reference_offset) -
      reference_frame.transpose() * weighted_reference_offset;

  for (const auto i : make_range(3))
  {
    if (!isfinite(MetaPhysicL::raw_value(weighted_jump(i))))
      throw MooseException(
          "The cohesive zone displacement jump at secondary node ", node.id(), " is not finite.");
    for (const auto j : make_range(3))
      if (!isfinite(MetaPhysicL::raw_value(rotation(i, j))))
        throw MooseException(
            "The cohesive zone interface frame at secondary node ", node.id(), " is not finite.");
  }

  _dof_to_interface_displacement_jump[&node] = weighted_jump;
  return rotation;
}

std::pair<RankTwoTensor, ADRankTwoTensor>
CohesiveZoneModelBase::computeInterfaceFrames(const Node & node,
                                              const RealVectorValue & orientation_normal) const
{
  using std::sqrt;

  const auto dim = _subproblem.mesh().dimension();

  RealVectorValue reference_area_vector_sum;
  ADRealVectorValue area_vector_sum;
  Real reference_area_magnitude_sum = 0;
  Real area_magnitude_sum = 0;
  ADRankTwoTensor surface_gradient;
  Real surface_weight_sum = 0;

  for (const Elem * const face : libmesh_map_find(amg().nodesToSecondaryElem(), node.id()))
  {
    const Point master_point = face->master_point(face->get_node_index(&node));
    const FEType fe_type(face->default_order(), FEMap::map_fe_type(*face));
    const auto n_shape_functions = face->n_nodes();
    mooseAssert(FEInterface::n_shape_functions(fe_type, face) == n_shape_functions,
                "initialSetup checks one geometric shape function per face node.");

    // Face tangents at the node on the reference and on the deformed positions
    std::array<RealVectorValue, 2> reference_tangents;
    std::array<ADRealVectorValue, 2> tangents;
    for (const auto k : make_range(n_shape_functions))
    {
      const auto face_node_position = deformedPosition(face->node_ref(k));
      for (const auto a : make_range(dim - 1))
      {
        const auto dphi = FEInterface::shape_deriv(fe_type, face, k, a, master_point);
        reference_tangents[a] += dphi * face->point(k);
        tangents[a] += dphi * face_node_position;
      }
    }

    const RealVectorValue reference_area_vector =
        dim == 2 ? RealVectorValue(reference_tangents[0](1), -reference_tangents[0](0), 0)
                 : reference_tangents[0].cross(reference_tangents[1]);
    const ADRealVectorValue area_vector =
        dim == 2 ? ADRealVectorValue(tangents[0](1), -tangents[0](0), 0)
                 : tangents[0].cross(tangents[1]);

    // Orient every face by the mortar nodal normal on the reference mesh, so that the deformed
    // normal follows the reference one without flipping
    const Real orientation = reference_area_vector * orientation_normal < 0 ? -1 : 1;
    reference_area_vector_sum += orientation * reference_area_vector;
    reference_area_magnitude_sum += reference_area_vector.norm();
    area_vector_sum += orientation * area_vector;
    area_magnitude_sum += MetaPhysicL::raw_value(area_vector.norm());

    if (dim == 3)
    {
      // Surface deformation gradient of the face, g_a (x) G^a with the dual reference tangents
      // G^a = M^-1_ab G_b and M_ab = G_a . G_b
      const Real m00 = reference_tangents[0] * reference_tangents[0];
      const Real m01 = reference_tangents[0] * reference_tangents[1];
      const Real m11 = reference_tangents[1] * reference_tangents[1];
      const Real det = m00 * m11 - m01 * m01;
      const std::array<RealVectorValue, 2> dual_tangents{
          {(m11 * reference_tangents[0] - m01 * reference_tangents[1]) / det,
           (m00 * reference_tangents[1] - m01 * reference_tangents[0]) / det}};

      const Real weight = reference_area_vector.norm();
      for (const auto a : make_range(2))
        for (const auto i : make_range(3))
          for (const auto j : make_range(3))
            surface_gradient(i, j) += weight * tangents[a](i) * dual_tangents[a](j);
      surface_weight_sum += weight;
    }
  }

  // The reference frame uses the same face average as the deformed frame, so both coincide on
  // the reference mesh and differ by R for a rigid motion R. The reference faces cancel only on a
  // mesh that is folded at the node, which no time step cut can repair; the relative tolerance is
  // the one of the deformed check below.
  if (reference_area_vector_sum.norm() <= TOLERANCE * reference_area_magnitude_sum)
    mooseError("The secondary surface of the mesh is folded at cohesive zone node ",
               node.id(),
               ": the incident reference face normals cancel.");
  const RealVectorValue reference_normal = reference_area_vector_sum.unit();
  const auto reference_frame =
      CohesiveZoneModelTools::computeReferenceRotation<false>(reference_normal, dim);

  // The face area vectors cancel only on a folded surface. The check is relative to the summed
  // face area magnitudes, so it is independent of the mesh size; TOLERANCE (1e-6) admits the
  // round-off of the sum and rejects normals whose direction the round-off would dominate.
  const auto area_vector_norm = area_vector_sum.norm();
  if (MetaPhysicL::raw_value(area_vector_norm) <= TOLERANCE * area_magnitude_sum)
    throw MooseException("The secondary surface is folded at cohesive zone node ",
                         node.id(),
                         ": the incident face normals cancel.");
  const ADRealVectorValue normal = area_vector_sum / area_vector_norm;

  // In 2D the tangent is the normal rotated by +90 degrees, as is the reference tangent of
  // computeReferenceRotation. On an unfolded surface this is the direction of (I - n n) F_s T1.
  if (dim == 2)
    return {reference_frame,
            ADRankTwoTensor::initializeFromColumns(
                normal, ADRealVectorValue(-normal(1), normal(0), 0), ADRealVectorValue(0, 0, 1))};

  surface_gradient /= surface_weight_sum;
  const ADRealVectorValue stretched_tangent_one = surface_gradient * reference_frame.column(1);
  const ADRealVectorValue stretched_tangent_two = surface_gradient * reference_frame.column(2);

  // Orthonormal basis of the deformed tangent plane aligned with F_s T1, with the handedness of
  // the reference frame, T2 = N x T1
  const ADRealVectorValue projected_tangent_one =
      stretched_tangent_one - (normal * stretched_tangent_one) * normal;
  const auto a11 = projected_tangent_one.norm();
  // F_s is dimensionless, so a11 is the in-plane stretch of the unit reference tangent T1 and the
  // absolute tolerance is a stretch. Below TOLERANCE (1e-6) the surface is crushed or folded at
  // this node, and the derivative of the normalization below, of magnitude 1 / a11, would exceed
  // 1e6.
  if (MetaPhysicL::raw_value(a11) <= TOLERANCE)
    throw MooseException("The secondary surface is folded at cohesive zone node ",
                         node.id(),
                         ": the reference tangent has no in-plane image.");
  const ADRealVectorValue basis_one = projected_tangent_one / a11;
  const ADRealVectorValue basis_two = normal.cross(basis_one);

  // In this basis the in-plane surface stretch A_ab = basis_a . F_s T_b has A_21 = 0 and A_11 > 0.
  // Its polar rotation turns basis_one by psi with cos(psi) = (A_11 + A_22) / r and
  // sin(psi) = (A_21 - A_12) / r, r the norm of the numerators. It is defined while
  // det(A) = A_11 A_22 > 0, i.e. while the surface is not inverted in its plane. On the reference
  // mesh A is symmetric positive definite, so psi = 0 and the frame is the reference frame.
  const auto a12 = basis_one * stretched_tangent_two;
  const auto a22 = basis_two * stretched_tangent_two;
  // det(A) = a11 a22 is the in-plane area ratio and a11 > TOLERANCE, so a22 <= TOLERANCE (1e-6)
  // means that the surface has lost its area or is inverted in its plane at this node
  if (MetaPhysicL::raw_value(a22) <= TOLERANCE)
    throw MooseException(
        "The secondary surface is inverted in its plane at cohesive zone node ", node.id(), ".");
  const auto polar_cos = a11 + a22;
  const auto polar_sin = -a12;
  const auto polar_norm = sqrt(polar_cos * polar_cos + polar_sin * polar_sin);
  const ADRealVectorValue tangent_one =
      (polar_cos * basis_one + polar_sin * basis_two) / polar_norm;

  return {reference_frame,
          ADRankTwoTensor::initializeFromColumns(normal, tangent_one, normal.cross(tangent_one))};
}

ADPoint
CohesiveZoneModelBase::deformedPosition(const Node & node) const
{
  ADPoint point = node;
  const bool do_derivatives = Moose::doDerivatives(_subproblem, _sys);

  const std::array<const MooseVariable *, 3> displacement_variables{
      {_disp_x_var, _disp_y_var, _disp_z_var}};
  for (const auto component : index_range(displacement_variables))
    if (const auto * const variable = displacement_variables[component])
    {
      const auto sys_num = variable->sys().number();
      const auto var_num = variable->number();
      // Enriched elements can have mapping nodes without displacement degrees of freedom. These
      // nodes keep their reference position.
      if (!node.n_dofs(sys_num, var_num))
        continue;

      const auto dof = node.dof_number(sys_num, var_num, 0);
      point(component) += (*variable->sys().currentSolution())(dof);
      if (do_derivatives)
        Moose::derivInsert(point(component).derivatives(), dof, 1.0);
    }

  return point;
}

void
CohesiveZoneModelBase::computeGlobalTraction(const Node * const node,
                                             const ADRankTwoTensor & rotation)
{
  using std::isfinite;

  const ADRealVectorValue traction = rotation * libmesh_map_find(_dof_to_czm_traction, node);
  for (const auto i : make_range(3))
    if (!isfinite(MetaPhysicL::raw_value(traction(i))))
      throw MooseException(
          "The cohesive zone traction at secondary node ", node->id(), " is not finite.");

  _dof_to_global_czm_traction[node] = traction;
}

void
CohesiveZoneModelBase::timestepSetup()
{
  // save off damage from the last timestep
  for (auto & map_pr : _dof_to_damage)
  {
    auto & [damage, old_damage] = map_pr.second;
    old_damage = {MetaPhysicL::raw_value(damage)};
    damage = old_damage;
  }
}

const ADVariableValue &
CohesiveZoneModelBase::czmGlobalTraction(const unsigned int i) const
{
  mooseAssert(i < 3,
              "Internal error in czmGlobalTraction. Index exceeds the traction vector size.");

  return _czm_interpolated_traction[i];
}
