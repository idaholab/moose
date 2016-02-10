/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "MechanicalContactConstraint.h"
#include "SystemBase.h"
#include "PenetrationLocator.h"
#include "Assembly.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

template<>
InputParameters validParams<MechanicalContactConstraint>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<NodeFaceConstraint>();
  params.addRequiredParam<BoundaryName>("boundary", "The master boundary");
  params.addRequiredParam<BoundaryName>("slave", "The slave boundary");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addRequiredCoupledVar("nodal_area", "The nodal area");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("capture_tolerance", 0, "Normal distance from surface within which nodes are captured");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<Real>("tension_release", 0.0, "Tension release threshold.  A node in contact will not be released if its tensile load is below this value.  No tension release if negative.");

  params.addParam<std::string>("formulation", "default", "The contact formulation");
  params.addParam<bool>("normalize_penalty", false, "Whether to normalize the penalty parameter with the nodal area for penalty contact.");
  params.addParam<bool>("master_slave_jacobian", true, "Whether to include jacobian entries coupling master and slave nodes.");
  params.addParam<bool>("connected_slave_nodes_jacobian", true, "Whether to include jacobian entries coupling nodes connected to slave nodes.");
  params.addParam<bool>("non_displacement_variables_jacobian", true, "Whether to include jacobian entries coupling with variables that are not displacement variables.");
  return params;
}

MechanicalContactConstraint::MechanicalContactConstraint(const InputParameters & parameters) :
    NodeFaceConstraint(parameters),
    _component(getParam<unsigned int>("component")),
    _model(contactModel(getParam<std::string>("model"))),
    _formulation(contactFormulation(getParam<std::string>("formulation"))),
    _normalize_penalty(getParam<bool>("normalize_penalty")),
    _penalty(getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _tension_release(getParam<Real>("tension_release")),
    _capture_tolerance(getParam<Real>("capture_tolerance")),
    _update_contact_set(true),
    _residual_copy(_sys.residualGhosted()),
    _x_var(isCoupled("disp_x") ? coupled("disp_x") : libMesh::invalid_uint),
    _y_var(isCoupled("disp_y") ? coupled("disp_y") : libMesh::invalid_uint),
    _z_var(isCoupled("disp_z") ? coupled("disp_z") : libMesh::invalid_uint),
    _mesh_dimension(_mesh.dimension()),
    _vars(_x_var, _y_var, _z_var),
    _nodal_area_var(getVar("nodal_area", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution()),
    _master_slave_jacobian(getParam<bool>("master_slave_jacobian")),
    _connected_slave_nodes_jacobian(getParam<bool>("connected_slave_nodes_jacobian")),
    _non_displacement_vars_jacobian(getParam<bool>("non_displacement_variables_jacobian"))
{
  _overwrite_slave_residual = false;

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(parameters.get<std::string>("normal_smoothing_method"));

  if (_model == CM_GLUED ||
      (_model == CM_COULOMB && _formulation == CF_KINEMATIC))
    _penetration_locator.setUpdate(false);

  if (_friction_coefficient < 0)
    mooseError("The friction coefficient must be nonnegative");
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
MechanicalContactConstraint::updateContactSet(bool beginning_of_step)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator
    it  = _penetration_locator._penetration_info.begin(),
    end = _penetration_locator._penetration_info.end();
  for (; it!=end; ++it)
  {
    const dof_id_type slave_node_num = it->first;
    PenetrationInfo * pinfo = it->second;

    // Skip this pinfo if there are no DOFs on this node.
    if ( ! pinfo || pinfo->_node->n_comp(_sys.number(), _vars(_component)) < 1 )
      continue;

    if (beginning_of_step)
    {
      pinfo->_locked_this_step = 0;
      pinfo->_starting_elem = it->second->_elem;
      pinfo->_starting_side_num = it->second->_side_num;
      pinfo->_starting_closest_point_ref = it->second->_closest_point_ref;
      pinfo->_contact_force_old = pinfo->_contact_force;
      pinfo->_accumulated_slip_old = pinfo->_accumulated_slip;
      pinfo->_frictional_energy_old = pinfo->_frictional_energy;
    }

    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force) / nodalArea(*pinfo);
    const Real distance = pinfo->_normal * (pinfo->_closest_point - _mesh.node(slave_node_num));

    // Capture
    if ( ! pinfo->isCaptured() && MooseUtils::absoluteFuzzyGreaterEqual(distance, 0, _capture_tolerance))
    {
      pinfo->capture();

      // Increment the lock count every time the node comes back into contact from not being in contact.
      if (_formulation == CF_KINEMATIC)
        ++pinfo->_locked_this_step;
    }
    // Release
    else if (_model != CM_GLUED &&
             pinfo->isCaptured() &&
             _tension_release >= 0 &&
             -contact_pressure >= _tension_release &&
             pinfo->_locked_this_step < 2)
    {
      pinfo->release();
      pinfo->_contact_force.zero();
    }

    if (_formulation == CF_AUGMENTED_LAGRANGE && pinfo->isCaptured())
      pinfo->_lagrange_multiplier -= getPenalty(*pinfo) * distance;
  }
}

bool
MechanicalContactConstraint::shouldApply()
{
  bool in_contact = false;

  std::map<dof_id_type, PenetrationInfo *>::iterator found =
    _penetration_locator._penetration_info.find(_current_node->id());
  if ( found != _penetration_locator._penetration_info.end() )
  {
    PenetrationInfo * pinfo = found->second;
    if ( pinfo != NULL && pinfo->isCaptured() )
    {
      in_contact = true;

      // This does the contact force once per constraint, rather than once per quad point and for
      // both master and slave cases.
      computeContactForce(pinfo);
    }
  }

  return in_contact;
}

void
MechanicalContactConstraint::computeContactForce(PenetrationInfo * pinfo)
{
  const Node * node = pinfo->_node;

  RealVectorValue res_vec;
  // Build up residual vector
  for (unsigned int i=0; i<_mesh_dimension; ++i)
  {
    dof_id_type dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);
  }
  RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
  const Real penalty = getPenalty(*pinfo);
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
          pinfo->_contact_force = (pinfo->_normal * (pinfo->_normal *
                                  ( pen_force + pinfo->_lagrange_multiplier * pinfo->_normal)));
          break;
        default:
          mooseError("Invalid contact formulation");
          break;
      }
      pinfo->_mech_status=PenetrationInfo::MS_SLIPPING;
      break;
    case CM_COULOMB:
      switch (_formulation)
      {
        case CF_KINEMATIC:
          pinfo->_contact_force =  -res_vec;
          break;
        case CF_PENALTY:
        {
          distance_vec = pinfo->_incremental_slip + (pinfo->_normal * (_mesh.node(node->id()) - pinfo->_closest_point)) * pinfo->_normal;
          pen_force = penalty * distance_vec;

          // Frictional capacity
          // const Real capacity( _friction_coefficient * (pen_force * pinfo->_normal < 0 ? -pen_force * pinfo->_normal : 0) );
          const Real capacity( _friction_coefficient * (res_vec * pinfo->_normal > 0 ? res_vec * pinfo->_normal : 0) );

          // Elastic predictor
          pinfo->_contact_force = pen_force + (pinfo->_contact_force_old - pinfo->_normal*(pinfo->_normal*pinfo->_contact_force_old));
          RealVectorValue contact_force_normal( (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal );
          RealVectorValue contact_force_tangential( pinfo->_contact_force - contact_force_normal );

          // Tangential magnitude of elastic predictor
          const Real tan_mag( contact_force_tangential.norm() );

          if ( tan_mag > capacity )
          {
            pinfo->_contact_force = contact_force_normal + capacity * contact_force_tangential / tan_mag;
            pinfo->_mech_status=PenetrationInfo::MS_SLIPPING;
          }
          else
            pinfo->_mech_status=PenetrationInfo::MS_STICKING;
          break;
        }
        case CF_AUGMENTED_LAGRANGE:
          pinfo->_contact_force = pen_force +
                                  pinfo->_lagrange_multiplier*distance_vec/distance_vec.norm();
          break;
        default:
          mooseError("Invalid contact formulation");
          break;
      }
      break;
    case CM_GLUED:
      switch (_formulation)
      {
        case CF_KINEMATIC:
          pinfo->_contact_force =  -res_vec;
          break;
        case CF_PENALTY:
          pinfo->_contact_force = pen_force;
          break;
        case CF_AUGMENTED_LAGRANGE:
          pinfo->_contact_force = pen_force +
                                  pinfo->_lagrange_multiplier*distance_vec/distance_vec.norm();
          break;
        default:
          mooseError("Invalid contact formulation");
          break;
      }
      pinfo->_mech_status=PenetrationInfo::MS_STICKING;
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
        const Real penalty = getPenalty(*pinfo);
        RealVectorValue pen_force(penalty * distance_vec);

        if (_model == CM_FRICTIONLESS)
          resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;

        else if (_model == CM_GLUED || _model == CM_COULOMB)
          resid += pen_force(_component);

      }
      return _test_slave[_i][_qp] * resid;
    case Moose::Master:
      return _test_master[_i][_qp] * -resid;
  }

  return 0;
}

Real
MechanicalContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  const Real penalty = getPenalty(*pinfo);

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
              for (unsigned int i=0; i<_mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars(i), 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return -pinfo->_normal(_component) * (pinfo->_normal*jac_vec) + (_phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp]) * pinfo->_normal(_component) * pinfo->_normal(_component);
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              double curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
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
              for (unsigned int i=0; i<_mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars(i), 0);
                jac_vec(i) = (*_jacobian)(dof_number, curr_master_node->dof_number(0, _vars(_component), 0));
              }
              return -pinfo->_normal(_component)*(pinfo->_normal*jac_vec) - (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) * pinfo->_normal(_component) * pinfo->_normal(_component);
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              Node * curr_master_node = _current_master->get_node(_j);
              double curr_jac = (*_jacobian)( _current_node->dof_number(0, _vars(_component), 0), curr_master_node->dof_number(0, _vars(_component), 0));
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
              for (unsigned int i=0; i<_mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars(i), 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return pinfo->_normal(_component)*(pinfo->_normal*jac_vec) * _test_master[_i][_qp];
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_test_master[_i][_qp] * penalty * _phi_slave[_j][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
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
              return 0;
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
              return 0;
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp];
            default:
              mooseError("Invalid contact formulation");
          }
        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0;
}

Real
MechanicalContactConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                      unsigned int jvar)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  const Real penalty = getPenalty(*pinfo);

  unsigned int coupled_component;
  double normal_component_in_coupled_var_dir = 1.0;
  if (getCoupledVarComponent(jvar,coupled_component))
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
              for (unsigned int i=0; i<_mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars(i), 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return -pinfo->_normal(_component) * (pinfo->_normal*jac_vec) + (_phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp]) * pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _phi_slave[_j][_qp] * penalty * _test_slave[_i][_qp] * pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
        {
          double curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
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
              for (unsigned int i=0; i<_mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars(i), 0);
                jac_vec(i) = (*_jacobian)(dof_number, curr_master_node->dof_number(0, _vars(_component), 0));
              }
              return -pinfo->_normal(_component)*(pinfo->_normal*jac_vec) - (_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp]) * pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_phi_master[_j][_qp] * penalty * _test_slave[_i][_qp] * pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
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
              for (unsigned int i=0; i<_mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars(i), 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]);
              }
              return pinfo->_normal(_component)*(pinfo->_normal*jac_vec) * _test_master[_i][_qp];
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_test_master[_i][_qp] * penalty * _phi_slave[_j][_qp] * pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
          switch (_formulation)
          {
            case CF_KINEMATIC:
            {
              double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
              return slave_jac * _test_master[_i][_qp];
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return 0;
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
              return 0;
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _test_master[_i][_qp] * penalty * _phi_master[_j][_qp] * pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
          return 0;
        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0;
}

Real
MechanicalContactConstraint::nodalArea(PenetrationInfo & pinfo)
{
  const Node * node = pinfo._node;

  dof_id_type dof = node->dof_number(_aux_system.number(), _nodal_area_var->number(), 0);

  Real area = (*_aux_solution)( dof );
  if (area == 0)
  {
    if (_t_step > 1)
    {
      mooseError("Zero nodal area found");
    }
    else
    {
      area = 1; // Avoid divide by zero during initialization
    }
  }
  return area;
}

Real
MechanicalContactConstraint::getPenalty(PenetrationInfo & pinfo)
{
  Real penalty = _penalty;
  if (_normalize_penalty)
  {
    penalty *= nodalArea(pinfo);
  }
  return penalty;
}

void
MechanicalContactConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), _var.number());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j=0; _j<_connected_dof_indices.size(); _j++)
      _Kee(_i,_j) += computeQpJacobian(Moose::SlaveSlave);

  if (_master_slave_jacobian)
  {
    DenseMatrix<Number> & Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());
    if (Ken.m() && Ken.n())
      for (_i=0; _i<_test_slave.size(); _i++)
        for (_j=0; _j<_phi_master.size(); _j++)
          Ken(_i,_j) += computeQpJacobian(Moose::SlaveMaster);

    _Kne.resize(_test_master.size(), _connected_dof_indices.size());
    for (_i=0; _i<_test_master.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j=0; _j<_connected_dof_indices.size(); _j++)
        _Kne(_i,_j) += computeQpJacobian(Moose::MasterSlave);
  }

  if (Knn.m() && Knn.n())
    for (_i=0; _i<_test_master.size(); _i++)
      for (_j=0; _j<_phi_master.size(); _j++)
        Knn(_i,_j) += computeQpJacobian(Moose::MasterMaster);
}

void
MechanicalContactConstraint::computeOffDiagJacobian(unsigned int jvar)
{
  getConnectedDofIndices(jvar);

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());

  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), jvar);

  for (_i=0; _i<_test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j=0; _j<_connected_dof_indices.size(); _j++)
      _Kee(_i,_j) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  if (_master_slave_jacobian)
  {
    DenseMatrix<Number> & Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);
    for (_i=0; _i<_test_slave.size(); _i++)
      for (_j=0; _j<_phi_master.size(); _j++)
        Ken(_i,_j) += computeQpOffDiagJacobian(Moose::SlaveMaster, jvar);

    _Kne.resize(_test_master.size(), _connected_dof_indices.size());
    if (_Kne.m() && _Kne.n())
      for (_i=0; _i<_test_master.size(); _i++)
        // Loop over the connected dof indices so we can get all the jacobian contributions
        for (_j=0; _j<_connected_dof_indices.size(); _j++)
          _Kne(_i,_j) += computeQpOffDiagJacobian(Moose::MasterSlave, jvar);
  }

  for (_i=0; _i<_test_master.size(); _i++)
    for (_j=0; _j<_phi_master.size(); _j++)
      Knn(_i,_j) += computeQpOffDiagJacobian(Moose::MasterMaster, jvar);
}

void
MechanicalContactConstraint::getConnectedDofIndices(unsigned int var_num)
{
  unsigned int component;
  if (getCoupledVarComponent(var_num,component) || _non_displacement_vars_jacobian)
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
  //dof_id_type current_node_var_dof_index = _sys.getVariable(0, _vars(component)).nodalDofIndex();
  dof_id_type current_node_var_dof_index = _sys.getVariable(0, var_num).nodalDofIndex();
  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to the dof associated with this node
  // and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j=0; j<_connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == current_node_var_dof_index)
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }
}

bool
MechanicalContactConstraint::getCoupledVarComponent(unsigned int var_num,
                                                    unsigned int &component)
{
  component = std::numeric_limits<unsigned int>::max();
  bool coupled_var_is_disp_var = false;
  for (unsigned int i=0; i<LIBMESH_DIM; ++i)
  {
    if (var_num == _vars(i))
    {
      coupled_var_is_disp_var = true;
      component = i;
      break;
    }
  }
  return coupled_var_is_disp_var;
}

