/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "MechanicalContactConstraint.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "AuxiliarySystem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"

#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "ContactAugLagMulProblem.h"
#include "Executioner.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<MechanicalContactConstraint>()
{
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  InputParameters params = validParams<NodeFaceConstraint>();
  params.addRequiredParam<BoundaryName>("boundary", "The master boundary");
  params.addRequiredParam<BoundaryName>("slave", "The slave boundary");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");

  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addRequiredCoupledVar("nodal_area", "The nodal area");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("penalty_slip",
                        1e8,
                        "The penalty to apply on sliping direction.  This can vary depending on "
                        "the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "capture_tolerance", 0, "Normal distance from surface within which nodes are captured");

  params.addParam<Real>("penetration_tolerance", 1e-9, "The tolerance of the distance function");
  params.addParam<Real>("stickking_tolerance", 1e-9, "The tolerance of the sticking");

  params.addParam<Real>("frictionalforce_tolerance", 1e-9, "The tolerance of the frictionalforce");

  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<Real>("tension_release",
                        0.0,
                        "Tension release threshold.  A node in contact "
                        "will not be released if its tensile load is below "
                        "this value.  No tension release if negative.");

  params.addParam<std::string>("formulation", "default", "The contact formulation");
  params.addParam<bool>(
      "normalize_penalty",
      false,
      "Whether to normalize the penalty parameter with the nodal area for penalty contact.");
  params.addParam<bool>("master_slave_jacobian",
                        true,
                        "Whether to include jacobian entries coupling master and slave nodes.");
  params.addParam<bool>(
      "connected_slave_nodes_jacobian",
      true,
      "Whether to include jacobian entries coupling nodes connected to slave nodes.");
  params.addParam<bool>("non_displacement_variables_jacobian",
                        true,
                        "Whether to include jacobian "
                        "entries coupling with "
                        "variables that are not "
                        "displacement variables.");
  params.addParam<unsigned int>("stick_lock_iterations",
                                std::numeric_limits<unsigned int>::max(),
                                "Number of times permitted to switch between sticking and slipping "
                                "in a solution before locking node in a sticked state.");
  params.addParam<Real>("stick_unlock_factor",
                        1.5,
                        "Factor by which frictional capacity must be "
                        "exceeded to permit stick-locked node to slip "
                        "again.");
  return params;
}

MechanicalContactConstraint::MechanicalContactConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _displaced_problem(parameters.get<FEProblemBase *>("_fe_problem_base")->getDisplacedProblem()),
    _component(getParam<unsigned int>("component")),
    _model(ContactMaster::contactModel(getParam<std::string>("model"))),
    _formulation(ContactMaster::contactFormulation(getParam<std::string>("formulation"))),
    _normalize_penalty(getParam<bool>("normalize_penalty")),
    _penalty(getParam<Real>("penalty")),
    _penalty_slip(1e8),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _tension_release(getParam<Real>("tension_release")),
    _capture_tolerance(getParam<Real>("capture_tolerance")),
    _penetration_tolerance(getParam<Real>("penetration_tolerance")),
    _stickking_tolerance(getParam<Real>("stickking_tolerance")),
    _frictionalforce_tolerance(getParam<Real>("frictionalforce_tolerance")),
    _stick_lock_iterations(getParam<unsigned int>("stick_lock_iterations")),
    _stick_unlock_factor(getParam<Real>("stick_unlock_factor")),
    _update_contact_set(true),
    _residual_copy(_sys.residualGhosted()),
    _mesh_dimension(_mesh.dimension()),
    _vars(3, libMesh::invalid_uint),
    _nodal_area_var(getVar("nodal_area", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution()),
    _master_slave_jacobian(getParam<bool>("master_slave_jacobian")),
    _connected_slave_nodes_jacobian(getParam<bool>("connected_slave_nodes_jacobian")),
    _non_displacement_vars_jacobian(getParam<bool>("non_displacement_variables_jacobian"))
{
  _overwrite_slave_residual = false;

  if (isParamValid("displacements"))
  {
    // modern parameter scheme for displacements
    for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
      _vars[i] = coupled("displacements", i);
  }
  else
  {
    // Legacy parameter scheme for displacements
    if (isParamValid("disp_x"))
      _vars[0] = coupled("disp_x");
    if (isParamValid("disp_y"))
      _vars[1] = coupled("disp_y");
    if (isParamValid("disp_z"))
      _vars[2] = coupled("disp_z");

    mooseDeprecated("use the `displacements` parameter rather than the `disp_*` parameters (those "
                    "will go away with the deprecation of the Solid Mechanics module).");
  }

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));

  if (_formulation == CF_TANGENTIAL_PENALTY && _model != CM_COULOMB)
    mooseError("The 'tangential_penalty' formulation can only be used with the 'coulomb' model");

  if (_model == CM_GLUED)
    _penetration_locator.setUpdate(false);

  if (_friction_coefficient < 0)
    mooseError("The friction coefficient must be nonnegative");

  if (parameters.isParamValid("penalty_slip"))
    _penalty_slip = parameters.get<Real>("penalty_slip");
  else
    _penalty_slip = parameters.get<Real>("penalty");
}

void
MechanicalContactConstraint::timestepSetup()
{
  if (_component == 0)
  {
    updateContactSet(true);

    _update_contact_set = false;
  }
}

void
MechanicalContactConstraint::jacobianSetup()
{
  if (_component == 0)
  {
    if (_update_contact_set)
      updateContactSet();
    _update_contact_set = true;
  }
}

void
MechanicalContactConstraint::updateLagMul(bool beginning_of_step)
{

  for (auto & pinfo_pair : _penetration_locator._penetration_info)
  {
    const dof_id_type slave_node_num = pinfo_pair.first;
    PenetrationInfo * pinfo = pinfo_pair.second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force) / nodalArea(*pinfo);

    const Real distance = pinfo->_normal * (pinfo->_closest_point - _mesh.nodeRef(slave_node_num));

    if (!pinfo->isCaptured() &&
        MooseUtils::absoluteFuzzyGreaterEqual(distance, 0.0, _capture_tolerance))
      pinfo->capture();

    else if (_model != CM_GLUED && pinfo->isCaptured() && _tension_release >= 0.0 &&
             -contact_pressure >= _tension_release && pinfo->_locked_this_step < 2)
    {
      pinfo->release();
      pinfo->_contact_force.zero();
    }

    if (pinfo->isCaptured())
    {
      if (_model == CM_FRICTIONLESS)
        pinfo->_lagrange_multiplier -= getPenalty(*pinfo, _penalty) * distance;

      if (_model == CM_COULOMB)
      {

        RealVectorValue distance_vec =
            (pinfo->_normal * (_mesh.nodeRef(slave_node_num) - pinfo->_closest_point)) *
            pinfo->_normal;

        Real penalty = getPenalty(*pinfo, _penalty);
        RealVectorValue pen_force =
            penalty * distance_vec + pinfo->_lagrange_multiplier * pinfo->_normal;

        pinfo->_lagrange_multiplier += penalty * distance_vec * pinfo->_normal;

        if (beginning_of_step)
        {
          RealVectorValue _lm_slip_init(0.0, 0.0, 0.0);
          pinfo->_lagrange_multiplier_slip = _lm_slip_init;
          //  pinfo->_mech_status = PenetrationInfo::MS_STICKING;
        }
        else
        {
          // Frictional capacity
          const Real capacity(_friction_coefficient *
                              (pen_force * pinfo->_normal < 0 ? -pen_force * pinfo->_normal : 0));

          RealVectorValue contact_force_normal(
              (pinfo->_lagrange_multiplier + (pen_force * pinfo->_normal)) * pinfo->_normal);

          RealVectorValue tangential_inc_slip =
              pinfo->_incremental_slip -
              (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;
          // RealVectorValue tangential_inc_slip = (pinfo->_total_slip - (pinfo->_incremental_slip *
          // pinfo->_normal) * pinfo->_normal )
          //                                     - (pinfo->_total_slip_old - (pinfo->_total_slip_old
          //                                     * pinfo->_normal_old) * pinfo->_normal_old ) ;

          Real penalty_slip = getPenalty(*pinfo, _penalty_slip);

          RealVectorValue inc_pen_force_tangential =
              pinfo->_lagrange_multiplier_slip + penalty_slip * tangential_inc_slip;

          RealVectorValue tau_old = pinfo->_contact_force_old -
                                    pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old);
          // Elastic predictor
          RealVectorValue contact_force_tangential = inc_pen_force_tangential + tau_old;

          // Tangential magnitude of elastic predictor
          const Real tan_mag(contact_force_tangential.norm());

          if (tan_mag > capacity)
            pinfo->_lagrange_multiplier_slip =
                -tau_old + capacity * contact_force_tangential / tan_mag;
          else
            pinfo->_lagrange_multiplier_slip += penalty_slip * tangential_inc_slip;
        }
      }
    }
  }
}

bool
MechanicalContactConstraint::haveAugLM()
{

  if (_formulation == CF_AUGMENTED_LAGRANGE)
    return true;

  return false;
}

bool MechanicalContactConstraint::contactConverged() // const NumericVector<Number> & solution)
{

  Real contactResidual = 0.0;

  Real converged = 0.0;

  for (auto & pinfo_pair : _penetration_locator._penetration_info)
  {
    const dof_id_type slave_node_num = pinfo_pair.first;
    PenetrationInfo * pinfo = pinfo_pair.second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    const Real distance = pinfo->_normal * (pinfo->_closest_point - _mesh.nodeRef(slave_node_num));

    if (pinfo->isCaptured())
    {

      if (contactResidual < std::abs(distance))
        contactResidual = std::abs(distance);

      if (contactResidual > _penetration_tolerance)
      {
        _console << "penetration enforcement not satisfied \n";
        printf("L inf norm of penetration is %4.3e\n", contactResidual);
        converged = 1;
        break;
      }

      if (_model == CM_COULOMB)
      {

        RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) *
                                             pinfo->_normal);
        RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);

        RealVectorValue tangential_inc_slip =
            pinfo->_incremental_slip - (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

        // RealVectorValue tangential_inc_slip = (pinfo->_total_slip - (pinfo->_incremental_slip *
        // pinfo->_normal) * pinfo->_normal )
        //                                    - (pinfo->_total_slip_old - (pinfo->_total_slip_old *
        //                                    pinfo->_normal_old) * pinfo->_normal_old ) ;

        const Real tan_mag(contact_force_tangential.norm());
        const Real tangential_inc_slip_mag = tangential_inc_slip.norm();

        RealVectorValue distance_vec =
            (pinfo->_normal * (_mesh.nodeRef(slave_node_num) - pinfo->_closest_point)) *
            pinfo->_normal;

        Real penalty = getPenalty(*pinfo, _penalty);
        RealVectorValue pen_force =
            penalty * distance_vec + pinfo->_lagrange_multiplier * pinfo->_normal;

        // Frictional capacity
        const Real capacity(_friction_coefficient *
                            (pen_force * pinfo->_normal < 0 ? -pen_force * pinfo->_normal : 0));

        if (tangential_inc_slip_mag > _stickking_tolerance)
        {
          if (tan_mag < capacity)
          {
            _console << "slipped too far \n";
            printf("tangential_force is %4.2f capacity\n", tan_mag / capacity);
            converged = 2;
            break;
          }
        }

        if (tan_mag > (1 + _frictionalforce_tolerance) * capacity)
        {
          _console << "frictional force exceeds frictional limit \n";
          printf("tangential_force is %4.2f capacity\n", tan_mag / capacity);
          converged = 3;
          break;
        }
      }
    }
  }

  _communicator.max(converged);

  if (converged > 0.0)
    return false;

  return true;
}

void
MechanicalContactConstraint::updateContactSet(bool beginning_of_step)
{
  for (auto & pinfo_pair : _penetration_locator._penetration_info)
  {
    const dof_id_type slave_node_num = pinfo_pair.first;
    PenetrationInfo * pinfo = pinfo_pair.second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    if (beginning_of_step)
    {
      if (_app.getExecutioner()->lastSolveConverged())
      {
        pinfo->_contact_force_old = pinfo->_contact_force;
        pinfo->_normal_old = pinfo->_normal;
        pinfo->_total_slip_old = pinfo->_total_slip;
        pinfo->_accumulated_slip_old = pinfo->_accumulated_slip;
        pinfo->_frictional_energy_old = pinfo->_frictional_energy;
        pinfo->_mech_status_old = pinfo->_mech_status;
      }
      else if (pinfo->_mech_status_old == PenetrationInfo::MS_NO_CONTACT &&
               pinfo->_mech_status != PenetrationInfo::MS_NO_CONTACT)
      {
        // The penetration info object could be based on a bad state so delete it
        delete pinfo_pair.second;
        pinfo_pair.second = NULL;
        continue;
      }

      pinfo->_locked_this_step = 0;
      pinfo->_stick_locked_this_step = 0;
      pinfo->_starting_elem = pinfo->_elem;
      pinfo->_starting_side_num = pinfo->_side_num;
      pinfo->_starting_closest_point_ref = pinfo->_closest_point_ref;
      // updateLagMul(true);
    }
    pinfo->_incremental_slip_prev_iter = pinfo->_incremental_slip;

    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force) / nodalArea(*pinfo);
    const Real distance = pinfo->_normal * (pinfo->_closest_point - _mesh.nodeRef(slave_node_num));

    // Capture
    if (!pinfo->isCaptured() &&
        MooseUtils::absoluteFuzzyGreaterEqual(distance, 0.0, _capture_tolerance))
    {
      pinfo->capture();

      // Increment the lock count every time the node comes back into contact from not being in
      // contact.
      if (_formulation == CF_KINEMATIC || _formulation == CF_TANGENTIAL_PENALTY)
        ++pinfo->_locked_this_step;
    }
    // Release
    else if (_model != CM_GLUED && pinfo->isCaptured() && _tension_release >= 0.0 &&
             -contact_pressure >= _tension_release && pinfo->_locked_this_step < 2)
    {
      pinfo->release();
      pinfo->_contact_force.zero();
    }
  }
}

bool
MechanicalContactConstraint::shouldApply()
{
  bool in_contact = false;

  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL && pinfo->isCaptured())
    {
      in_contact = true;

      // This computes the contact force once per constraint, rather than once per quad point and
      // for
      // both master and slave cases.
      if (_component == 0)
        computeContactForce(pinfo);
    }
  }

  return in_contact;
}

void
MechanicalContactConstraint::computeContactForce(PenetrationInfo * pinfo)
{

  const Node * node = pinfo->_node;

  // Build up residual vector
  RealVectorValue res_vec;
  for (unsigned int i = 0; i < _mesh_dimension; ++i)
  {
    dof_id_type dof_number = node->dof_number(0, _vars[i], 0);
    res_vec(i) = _residual_copy(dof_number);
  }

  RealVectorValue distance_vec(_mesh.nodeRef(node->id()) - pinfo->_closest_point);
  const Real penalty = getPenalty(*pinfo, _penalty);
  const Real penalty_slip = getPenalty(*pinfo, _penalty_slip);

  RealVectorValue pen_force(penalty * distance_vec);

  switch (_model)
  {
    case CM_FRICTIONLESS:
      switch (_formulation)
      {
        case CF_KINEMATIC:
          pinfo->_contact_force = -pinfo->_normal * (pinfo->_normal * res_vec);
          break;

        case CF_PENALTY:
          pinfo->_contact_force = pinfo->_normal * (pinfo->_normal * pen_force);
          break;

        case CF_AUGMENTED_LAGRANGE:
          pinfo->_contact_force =
              (pinfo->_normal *
               (pinfo->_normal * (pen_force + pinfo->_lagrange_multiplier * pinfo->_normal)));
          break;

        default:
          mooseError("Invalid contact formulation");
          break;
      }
      pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
      break;
    case CM_COULOMB:
      switch (_formulation)
      {
        case CF_KINEMATIC:
        {
          // Frictional capacity
          const Real capacity(_friction_coefficient *
                              (res_vec * pinfo->_normal > 0 ? res_vec * pinfo->_normal : 0));

          // Normal and tangential components of predictor force
          pinfo->_contact_force = -res_vec;
          RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) *
                                               pinfo->_normal);
          RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);

          RealVectorValue tangential_inc_slip =
              pinfo->_incremental_slip -
              (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

          // Magnitude of tangential predictor force
          const Real tan_mag(contact_force_tangential.size());
          const Real tangential_inc_slip_mag = tangential_inc_slip.size();
          const Real slip_tol = capacity / penalty;
          pinfo->_slip_tol = slip_tol;

          if ((tangential_inc_slip_mag > slip_tol || tan_mag > capacity) &&
              (pinfo->_stick_locked_this_step < _stick_lock_iterations ||
               tan_mag > capacity * _stick_unlock_factor))
          {
            if (pinfo->_stick_locked_this_step >= _stick_lock_iterations)
              pinfo->_stick_locked_this_step = 0;

            // Check for scenario where node has slipped too far in a previous iteration
            // for special treatment (only done if the incremental slip is non-trivial)
            bool slipped_too_far = false;
            RealVectorValue slip_inc_direction;
            if (tangential_inc_slip_mag > slip_tol)
            {
              slip_inc_direction = tangential_inc_slip / tangential_inc_slip_mag;
              Real slip_dot_tang_force = slip_inc_direction * contact_force_tangential;
              if (slip_dot_tang_force < capacity)
                slipped_too_far = true;
            }

            if (slipped_too_far) // slip back along slip increment
              pinfo->_contact_force = contact_force_normal + capacity * slip_inc_direction;
            else
            {
              if (tan_mag > 0) // slip along tangential force direction
                pinfo->_contact_force =
                    contact_force_normal + capacity * contact_force_tangential / tan_mag;
              else // treat as frictionless
                pinfo->_contact_force = contact_force_normal;
            }
            if (capacity == 0)
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
          {
            if (pinfo->_mech_status != PenetrationInfo::MS_STICKING &&
                pinfo->_mech_status != PenetrationInfo::MS_NO_CONTACT)
              ++pinfo->_stick_locked_this_step;
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
          }
          break;
        }

        case CF_PENALTY:
        {
          distance_vec = pinfo->_incremental_slip +
                         (pinfo->_normal * (_mesh.nodeRef(node->id()) - pinfo->_closest_point)) *
                             pinfo->_normal;
          pen_force = penalty * distance_vec;

          // Frictional capacity
          // const Real capacity( _friction_coefficient * (pen_force * pinfo->_normal < 0 ?
          // -pen_force * pinfo->_normal : 0) );
          const Real capacity(_friction_coefficient *
                              (res_vec * pinfo->_normal > 0 ? res_vec * pinfo->_normal : 0));

          // Elastic predictor
          pinfo->_contact_force =
              pen_force + (pinfo->_contact_force_old -
                           pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old));
          RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) *
                                               pinfo->_normal);
          RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);

          // Tangential magnitude of elastic predictor
          const Real tan_mag(contact_force_tangential.norm());

          if (tan_mag > capacity)
          {
            pinfo->_contact_force =
                contact_force_normal + capacity * contact_force_tangential / tan_mag;
            if (capacity == 0)
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
          break;
        }

        case CF_AUGMENTED_LAGRANGE:
        {
          distance_vec =
              (pinfo->_normal * (_mesh.nodeRef(node->id()) - pinfo->_closest_point)) *
              pinfo->_normal; // + (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

          pen_force = penalty * distance_vec + pinfo->_lagrange_multiplier * pinfo->_normal;

          // Frictional capacity
          const Real capacity(_friction_coefficient *
                              (pen_force * pinfo->_normal < 0 ? -pen_force * pinfo->_normal : 0));

          RealVectorValue contact_force_normal((pen_force * pinfo->_normal) * pinfo->_normal);

          RealVectorValue tangential_inc_slip =
              pinfo->_incremental_slip -
              (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

          // RealVectorValue tangential_inc_slip = (pinfo->_total_slip - (pinfo->_incremental_slip *
          // pinfo->_normal) * pinfo->_normal )
          //                                    - (pinfo->_total_slip_old - (pinfo->_total_slip_old
          //                                    * pinfo->_normal_old) * pinfo->_normal_old ) ;

          RealVectorValue inc_pen_force_tangential =
              pinfo->_lagrange_multiplier_slip + penalty_slip * tangential_inc_slip;

          // Elastic predictor
          RealVectorValue contact_force_tangential =
              inc_pen_force_tangential +
              (pinfo->_contact_force_old -
               pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old));

          // Tangential magnitude of elastic predictor
          const Real tan_mag(contact_force_tangential.norm());

          if (tan_mag > capacity)
          {
            pinfo->_contact_force =
                contact_force_normal + capacity * contact_force_tangential / tan_mag;
            if (capacity == 0)
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
          {
            pinfo->_contact_force = contact_force_normal + contact_force_tangential;
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
          }

          break;
        }

        case CF_TANGENTIAL_PENALTY:
        {
          // Frictional capacity (kinematic formulation)
          const Real capacity = _friction_coefficient * std::max(res_vec * pinfo->_normal, 0.0);

          // Normal component of contact force (kinematic formulation)
          RealVectorValue contact_force_normal((-res_vec * pinfo->_normal) * pinfo->_normal);

          // Predictor tangential component of contact force (penalty formulation)
          RealVectorValue inc_pen_force_tangential = penalty * pinfo->_incremental_slip;
          RealVectorValue contact_force_tangential =
              inc_pen_force_tangential +
              (pinfo->_contact_force_old -
               pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old));

          // Magnitude of tangential predictor
          const Real tan_mag(contact_force_tangential.norm());

          if (tan_mag > capacity)
          {
            pinfo->_contact_force =
                contact_force_normal + capacity * contact_force_tangential / tan_mag;
            if (capacity == 0.0)
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
          {
            pinfo->_contact_force = contact_force_normal + contact_force_tangential;
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
          }
          break;
        }

        default:
          mooseError("Invalid contact formulation");
          break;
      }
      break;

    case CM_GLUED:
      switch (_formulation)
      {
        case CF_KINEMATIC:
          pinfo->_contact_force = -res_vec;
          break;

        case CF_PENALTY:
          pinfo->_contact_force = pen_force;
          break;

        case CF_AUGMENTED_LAGRANGE:
          pinfo->_contact_force =
              pen_force + pinfo->_lagrange_multiplier * distance_vec / distance_vec.norm();
          break;

        default:
          mooseError("Invalid contact formulation");
          break;
      }
      pinfo->_mech_status = PenetrationInfo::MS_STICKING;
      break;

    default:
      mooseError("Invalid or unavailable contact model");
      break;
  }
}

Real
MechanicalContactConstraint::computeQpSlaveValue()
{
  return _u_slave[_qp];
}

Real
MechanicalContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  Real resid = pinfo->_contact_force(_component);

  switch (type)
  {
    case Moose::Slave:
      if (_formulation == CF_KINEMATIC)
      {
        RealVectorValue distance_vec(*_current_node - pinfo->_closest_point);
        const Real penalty = getPenalty(*pinfo, _penalty);
        RealVectorValue pen_force(penalty * distance_vec);

        if (_model == CM_FRICTIONLESS)
          resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;

        else if (_model == CM_COULOMB)
        {
          distance_vec = distance_vec - pinfo->_incremental_slip;
          pen_force = penalty * distance_vec;
          if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
              pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
            resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;
          else
            resid += pen_force(_component);
        }
        else if (_model == CM_GLUED)
          resid += pen_force(_component);
      }
      else if (_formulation == CF_TANGENTIAL_PENALTY && _model == CM_COULOMB)
      {
        RealVectorValue distance_vec(*_current_node - pinfo->_closest_point);
        const Real penalty = getPenalty(*pinfo, _penalty);
        RealVectorValue pen_force(penalty * distance_vec);
        resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;
      }
      return _test_slave[_i][_qp] * resid;

    case Moose::Master:
      return _test_master[_i][_qp] * -resid;
  }

  return 0.0;
}

Real
MechanicalContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  const Real penalty = getPenalty(*pinfo, _penalty);
  const Real penalty_slip = getPenalty(*pinfo, _penalty_slip);

  switch (type)
  {
    case Moose::SlaveSlave:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                     (_phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                         pinfo->_normal(_component) * pinfo->_normal(_component);
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_COULOMB:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
                }
                return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                       (_phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                           pinfo->_normal(_component) * pinfo->_normal(_component);
              }
              else
              {
                const Real curr_jac = (*_jacobian)(
                    _current_node->dof_number(0, _vars[_component], 0), _connected_dof_indices[_j]);
                return -curr_jac + _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp];
              }
            }

            case CF_PENALTY:

            case CF_AUGMENTED_LAGRANGE:
            {
              Real normal_comp = _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;

              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
              {
                tang_comp += _phi_slave[_j][_qp] * penalty_slip * _test_slave[_i][_qp] *
                             (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              }

              return normal_comp + tang_comp;
            }

            case CF_TANGENTIAL_PENALTY:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              Real normal_comp = -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                                 (_phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                                     pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));

              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              const Real curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                                 _connected_dof_indices[_j]);
              return -curr_jac + _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp];
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp];

            default:
              mooseError("Invalid contact formulation");
          }
        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::SlaveMaster:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              Node * curr_master_node = _current_master->get_node(_j);

              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) =
                    (*_jacobian)(dof_number, curr_master_node->dof_number(0, _vars[_component], 0));
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                     (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                         pinfo->_normal(_component) * pinfo->_normal(_component);
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_COULOMB:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                Node * curr_master_node = _current_master->get_node(_j);

                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number,
                                            curr_master_node->dof_number(0, _vars[_component], 0));
                }
                return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                       (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                           pinfo->_normal(_component) * pinfo->_normal(_component);
              }
              else
              {
                Node * curr_master_node = _current_master->get_node(_j);
                const Real curr_jac =
                    (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                 curr_master_node->dof_number(0, _vars[_component], 0));
                return -curr_jac - _phi_master[_j][_qp] * penalty * _test_slave[_i][_qp];
              }
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
            {
              Real normal_comp = -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;

              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
              {
                tang_comp = -_phi_master[_j][_qp] * penalty_slip * _test_slave[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              }

              return normal_comp + tang_comp;
            }

            case CF_TANGENTIAL_PENALTY:
            {
              Node * curr_master_node = _current_master->get_node(_j);

              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) =
                    (*_jacobian)(dof_number, curr_master_node->dof_number(0, _vars[_component], 0));
              }
              Real normal_comp = -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                                 (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                                     pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));

              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              Node * curr_master_node = _current_master->get_node(_j);
              const Real curr_jac =
                  (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                               curr_master_node->dof_number(0, _vars[_component], 0));
              return -curr_jac - _phi_master[_j][_qp] * penalty * _test_slave[_i][_qp];
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp];

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterSlave:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                     _test_master[_i][_qp];
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_test_master[_i][_qp] * penalty * _phi_slave[_j][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
                }
                return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                       _test_master[_i][_qp];
              }
              else
              {
                const Real slave_jac = (*_jacobian)(
                    _current_node->dof_number(0, _vars[_component], 0), _connected_dof_indices[_j]);
                return slave_jac * _test_master[_i][_qp];
              }
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
            {
              Real normal_comp = -_phi_slave[_j][_qp] * penalty * _test_master[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;

              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
              {
                tang_comp = -_phi_slave[_j][_qp] * penalty_slip * _test_master[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              }

              return normal_comp + tang_comp;
            }

            case CF_TANGENTIAL_PENALTY:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              Real normal_comp =
                  pinfo->_normal(_component) * (pinfo->_normal * jac_vec) * _test_master[_i][_qp];

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = -_test_master[_i][_qp] * penalty * _phi_slave[_j][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));

              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              const Real slave_jac = (*_jacobian)(
                  _current_node->dof_number(0, _vars[_component], 0), _connected_dof_indices[_j]);
              return slave_jac * _test_master[_i][_qp];
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_test_master[_i][_qp] * penalty * _phi_slave[_j][_qp];

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterMaster:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
              return 0.0;

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_COULOMB:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                Node * curr_master_node = _current_master->get_node(_j);

                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number,
                                            curr_master_node->dof_number(0, _vars[_component], 0));
                }
                return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                       (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                           pinfo->_normal(_component) * pinfo->_normal(_component);
              }
              else
              {
                Node * curr_master_node = _current_master->get_node(_j);
                const Real curr_jac =
                    (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                 curr_master_node->dof_number(0, _vars[_component], 0));
                return -curr_jac + _phi_master[_j][_qp] * penalty * _test_slave[_i][_qp];
              }
            }

            case CF_PENALTY:
            case CF_TANGENTIAL_PENALTY:

            case CF_AUGMENTED_LAGRANGE:
            {
              Real normal_comp = _phi_master[_j][_qp] * penalty * _test_master[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;

              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
              {
                tang_comp = _phi_master[_j][_qp] * penalty_slip * _test_master[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              }

              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
              return 0.0;

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
                return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] *
                       pinfo->_normal(_component) * pinfo->_normal(_component);
              else
                return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp];

            case CF_TANGENTIAL_PENALTY:
            {
              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              return tang_comp; // normal component is zero
            }

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0.0;
}

Real
MechanicalContactConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                      unsigned int jvar)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  const Real penalty = getPenalty(*pinfo, _penalty);
  // const Real penalty_slip = getPenalty(*pinfo,_penalty_slip);

  unsigned int coupled_component;
  Real normal_component_in_coupled_var_dir = 1.0;
  if (getCoupledVarComponent(jvar, coupled_component))
    normal_component_in_coupled_var_dir = pinfo->_normal(coupled_component);

  switch (type)
  {
    case Moose::SlaveSlave:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                     (_phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                         pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_COULOMB:
        {
          if ((_formulation == CF_KINEMATIC || _formulation == CF_TANGENTIAL_PENALTY) &&
              (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
               pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
          {
            RealVectorValue jac_vec;
            for (unsigned int i = 0; i < _mesh_dimension; ++i)
            {
              dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
              jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
            }
            return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                   (_phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                       pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          }
          else if ((_formulation == CF_PENALTY) &&
                   (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                    pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

          else if (_formulation == CF_AUGMENTED_LAGRANGE)
          {

            Real normal_comp = _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] *
                               pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              return normal_comp;
          }
          else
          {
            const Real curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                               _connected_dof_indices[_j]);
            return -curr_jac;
          }
        }

        case CM_GLUED:
        {
          const Real curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                             _connected_dof_indices[_j]);
          return -curr_jac;
        }
        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::SlaveMaster:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              Node * curr_master_node = _current_master->get_node(_j);

              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) =
                    (*_jacobian)(dof_number, curr_master_node->dof_number(0, _vars[_component], 0));
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                     (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                         pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_COULOMB:
          if ((_formulation == CF_KINEMATIC || _formulation == CF_TANGENTIAL_PENALTY) &&
              (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
               pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
          {
            Node * curr_master_node = _current_master->get_node(_j);

            RealVectorValue jac_vec;
            for (unsigned int i = 0; i < _mesh_dimension; ++i)
            {
              dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
              jac_vec(i) =
                  (*_jacobian)(dof_number, curr_master_node->dof_number(0, _vars[_component], 0));
            }
            return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                   (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) *
                       pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          }
          else if ((_formulation == CF_PENALTY) &&
                   (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                    pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

          else if (_formulation == CF_AUGMENTED_LAGRANGE)
          {

            Real normal_comp = -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] *
                               pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              return normal_comp;
          }
          else
            return 0.0;

        case CM_GLUED:
          return 0;

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterSlave:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                     _test_master[_i][_qp];
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_test_master[_i][_qp] * penalty * _phi_slave[_j][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
                }
                return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                       _test_master[_i][_qp];
              }
              else
              {
                const Real slave_jac = (*_jacobian)(
                    _current_node->dof_number(0, _vars[_component], 0), _connected_dof_indices[_j]);
                return slave_jac * _test_master[_i][_qp];
              }
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
                return -_test_master[_i][_qp] * penalty * _phi_slave[_j][_qp] *
                       pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

              return 0.0;
            }
            case CF_TANGENTIAL_PENALTY:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
                }
                return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                       _test_master[_i][_qp];
              }
              else
                return 0.0;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              const Real slave_jac = (*_jacobian)(
                  _current_node->dof_number(0, _vars[_component], 0), _connected_dof_indices[_j]);
              return slave_jac * _test_master[_i][_qp];
            }

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return 0.0;

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::MasterMaster:
      switch (_model)
      {
        case CM_FRICTIONLESS:
          switch (_formulation)
          {
            case CF_KINEMATIC:
              return 0.0;

            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }

        case CM_COULOMB:
        {
          if (_formulation == CF_AUGMENTED_LAGRANGE)
          {

            Real normal_comp = _phi_master[_j][_qp] * penalty * _test_master[_i][_qp] *
                               pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              return normal_comp;
          }
          else if (_formulation == CF_PENALTY &&
                   (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                    pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          else
            return 0.0;
        }

        case CM_GLUED:
          if ((_formulation == CF_PENALTY || _formulation == CF_AUGMENTED_LAGRANGE) &&
              (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
               pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          else
            return 0.0;

        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0.0;
}

Real
MechanicalContactConstraint::nodalArea(PenetrationInfo & pinfo)
{
  const Node * node = pinfo._node;

  dof_id_type dof = node->dof_number(_aux_system.number(), _nodal_area_var->number(), 0);

  Real area = (*_aux_solution)(dof);
  if (area == 0.0)
  {
    if (_t_step > 1)
      mooseError("Zero nodal area found");
    else
      area = 1.0; // Avoid divide by zero during initialization
  }
  return area;
}

Real
MechanicalContactConstraint::getPenalty(PenetrationInfo & pinfo, Real penalty_param)
{

  Real penalty = penalty_param;
  if (_normalize_penalty)
    penalty *= nodalArea(pinfo);

  return penalty;
}

void
MechanicalContactConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), _var.number());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SlaveSlave);

  if (_master_slave_jacobian)
  {
    DenseMatrix<Number> & Ken =
        _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());
    if (Ken.m() && Ken.n())
      for (_i = 0; _i < _test_slave.size(); _i++)
        for (_j = 0; _j < _phi_master.size(); _j++)
          Ken(_i, _j) += computeQpJacobian(Moose::SlaveMaster);

    _Kne.resize(_test_master.size(), _connected_dof_indices.size());
    for (_i = 0; _i < _test_master.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpJacobian(Moose::MasterSlave);
  }

  if (Knn.m() && Knn.n())
    for (_i = 0; _i < _test_master.size(); _i++)
      for (_j = 0; _j < _phi_master.size(); _j++)
        Knn(_i, _j) += computeQpJacobian(Moose::MasterMaster);
}

void
MechanicalContactConstraint::computeOffDiagJacobian(unsigned int jvar)
{
  getConnectedDofIndices(jvar);

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());

  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), jvar);

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  if (_master_slave_jacobian)
  {
    DenseMatrix<Number> & Ken =
        _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);
    for (_i = 0; _i < _test_slave.size(); _i++)
      for (_j = 0; _j < _phi_master.size(); _j++)
        Ken(_i, _j) += computeQpOffDiagJacobian(Moose::SlaveMaster, jvar);

    _Kne.resize(_test_master.size(), _connected_dof_indices.size());
    if (_Kne.m() && _Kne.n())
      for (_i = 0; _i < _test_master.size(); _i++)
        // Loop over the connected dof indices so we can get all the jacobian contributions
        for (_j = 0; _j < _connected_dof_indices.size(); _j++)
          _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::MasterSlave, jvar);
  }

  for (_i = 0; _i < _test_master.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      Knn(_i, _j) += computeQpOffDiagJacobian(Moose::MasterMaster, jvar);
}

void
MechanicalContactConstraint::getConnectedDofIndices(unsigned int var_num)
{
  unsigned int component;
  if (getCoupledVarComponent(var_num, component) || _non_displacement_vars_jacobian)
  {
    if (_master_slave_jacobian && _connected_slave_nodes_jacobian)
      NodeFaceConstraint::getConnectedDofIndices(var_num);
    else
    {
      _connected_dof_indices.clear();
      MooseVariable & var = _sys.getVariable(0, var_num);
      _connected_dof_indices.push_back(var.nodalDofIndex());
    }
  }

  _phi_slave.resize(_connected_dof_indices.size());
  // dof_id_type current_node_var_dof_index = _sys.getVariable(0, _vars[component]).nodalDofIndex();
  dof_id_type current_node_var_dof_index = _sys.getVariable(0, var_num).nodalDofIndex();
  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to the dof associated with this node
  // and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == current_node_var_dof_index)
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }
}

bool
MechanicalContactConstraint::getCoupledVarComponent(unsigned int var_num, unsigned int & component)
{
  component = std::numeric_limits<unsigned int>::max();
  bool coupled_var_is_disp_var = false;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
  {
    if (var_num == _vars[i])
    {
      coupled_var_is_disp_var = true;
      component = i;
      break;
    }
  }
  return coupled_var_is_disp_var;
}
