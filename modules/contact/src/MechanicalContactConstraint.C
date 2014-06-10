/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "MechanicalContactConstraint.h"

#include "SystemBase.h"
#include "PenetrationLocator.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<MechanicalContactConstraint>()
{
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST");

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
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<Real>("tension_release", 0.0, "Tension release threshold.  A node in contact will not be released if its tensile load is below this value.  Must be positive.");

  params.addParam<std::string>("formulation", "default", "The contact formulation");
  return params;
}

MechanicalContactConstraint::MechanicalContactConstraint(const std::string & name, InputParameters parameters) :
  NodeFaceConstraint(name, parameters),
  _component(getParam<unsigned int>("component")),
  _model(contactModel(getParam<std::string>("model"))),
  _formulation(contactFormulation(getParam<std::string>("formulation"))),
  _penalty(getParam<Real>("penalty")),
  _friction_coefficient(getParam<Real>("friction_coefficient")),
  _tension_release(getParam<Real>("tension_release")),
  _update_contact_set(true),
  _time_last_called(-std::numeric_limits<Real>::max()),
  _residual_copy(_sys.residualGhosted()),
  _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
  _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
  _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
  _mesh_dimension(_mesh.dimension()),
  _vars(_x_var, _y_var, _z_var),
  _nodal_area_var(getVar("nodal_area", 0)),
  _aux_system( _nodal_area_var->sys() ),
  _aux_solution( _aux_system.currentSolution() )
{
  _overwrite_slave_residual = false;

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(parameters.get<std::string>("normal_smoothing_method"));

  if (_model == CM_GLUED ||
      (_model == CM_COULOMB && _formulation == CF_DEFAULT))
    _penetration_locator.setUpdate(false);

  if (_tension_release < 0)
    mooseError("The parameter 'tension_release' must be non-negative");

  if (_friction_coefficient < 0)
    mooseError("The friction coefficient must be nonnegative");
}

void
MechanicalContactConstraint::timestepSetup()
{
  if (_component == 0)
  {
    _penetration_locator._unlocked_this_step.clear();
    _penetration_locator._locked_this_step.clear();
    bool beginning_of_step = false;
    if (_t > _time_last_called)
    {
      beginning_of_step = true;
      _penetration_locator.saveContactStateVars();
    }
    updateContactSet(beginning_of_step);
    _update_contact_set = false;
    _time_last_called = _t;
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
  std::set<unsigned int> & has_penetrated = _penetration_locator._has_penetrated;
  std::map<unsigned int, unsigned> & unlocked_this_step = _penetration_locator._unlocked_this_step;
  std::map<unsigned int, unsigned> & locked_this_step = _penetration_locator._locked_this_step;
  std::map<unsigned int, Real> & lagrange_multiplier = _penetration_locator._lagrange_multiplier;

  std::map<unsigned int, PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();

  for (; it!=end; ++it)
  {
    PenetrationInfo * pinfo = it->second;
    if (!pinfo)
      continue;

    const unsigned int slave_node_num = it->first;
    std::set<unsigned int>::iterator hpit = has_penetrated.find(slave_node_num);

    if (beginning_of_step)
    {
      if (hpit != has_penetrated.end())
        pinfo->_penetrated_at_beginning_of_step = true;
      else
        pinfo->_penetrated_at_beginning_of_step = false;

      pinfo->_starting_elem = it->second->_elem;
      pinfo->_starting_side_num = it->second->_side_num;
      pinfo->_starting_closest_point_ref = it->second->_closest_point_ref;
    }

    const Node * node = pinfo->_node;
    unsigned int dof = node->dof_number(_aux_system.number(), _nodal_area_var->number(), 0);
    Real area = (*_aux_solution)( dof );
    if (area == 0)
    {
      if (_t_step > 1)
        mooseError("Zero nodal area found");
      else
        area = 1; // Avoid divide by zero during initialization
    }

    if (_model == CM_EXPERIMENTAL ||
        (_model == CM_COULOMB && _formulation == CF_DEFAULT))
    {

      Real resid( -(pinfo->_normal * pinfo->_contact_force) / area );

      // Moose::out << locked_this_step[slave_node_num] << " " << pinfo->_distance << std::endl;
      const Real distance( pinfo->_normal * (pinfo->_closest_point - _mesh.node(node->id())));

      if (hpit != has_penetrated.end() && resid < -_tension_release && locked_this_step[slave_node_num] < 2)
      {
        //Moose::out << "Releasing node " << node->id() << " " << resid << " < " << -_tension_release << std::endl;
        has_penetrated.erase(hpit);
        pinfo->_contact_force.zero();
        pinfo->_mech_status=PenetrationInfo::MS_NO_CONTACT;
        ++unlocked_this_step[slave_node_num];
      }
      else if (distance > 0)
      {
        if (hpit == has_penetrated.end())
        {
          //Moose::out << "Capturing node " << node->id() << " " << distance << " " << unlocked_this_step[slave_node_num] <<  std::endl;
          ++locked_this_step[slave_node_num];
          has_penetrated.insert(slave_node_num);
        }
      }
    }
    else if (_formulation == CF_PENALTY)
    {
      if (pinfo->_distance >= 0)  //Penetrated
      {
        if (hpit == has_penetrated.end())
          has_penetrated.insert(slave_node_num);
      }
      else if ((pinfo->_contact_force * pinfo->_normal) / area < 0) //In contact and in compression
      {
        // Do nothing.
      }
      else
      {
        if (hpit != has_penetrated.end())
          has_penetrated.erase(hpit);
        pinfo->_contact_force.zero();
        pinfo->_mech_status=PenetrationInfo::MS_NO_CONTACT;
      }
    }
    else
    {
      if (pinfo->_distance >= 0)
      {
        if (hpit == has_penetrated.end())
          has_penetrated.insert(slave_node_num);
      }
    }

    if (_formulation == CF_AUGMENTED_LAGRANGE && hpit != has_penetrated.end())
    {
      const RealVectorValue distance_vec(_mesh.node(slave_node_num) - pinfo->_closest_point);
      lagrange_multiplier[slave_node_num] += _penalty * pinfo->_normal * distance_vec;
    }
  }
}

bool
MechanicalContactConstraint::shouldApply()
{
  //TODO:  We'll need to do something to call computeContactForce() for the nodes that are
  //       off-processor.  There are no methods that get called for all nodes (at least
  //       that I know of), so we can't do this correctly yet, but I'm leaving this here
  //       to remind us that we need to do this.

//  _point_to_info.clear();
//
//  std::set<unsigned int> & has_penetrated = _penetration_locator._has_penetrated;
//
//  std::map<unsigned int, PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
//  std::map<unsigned int, PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();
//
//  for (; it!=end; ++it)
//  {
//    PenetrationInfo * pinfo = it->second;
//
//    if (!pinfo)
//      continue;
//
//    unsigned int slave_node_num = it->first;
//
//    std::set<unsigned int>::iterator hpit = has_penetrated.find(slave_node_num);
//
//    if ( hpit != has_penetrated.end() )
//    {
//      addPoint(pinfo->_elem, pinfo->_closest_point);
//      _point_to_info[pinfo->_closest_point] = pinfo;
//      computeContactForce(pinfo);
//    }
//  }

  std::set<unsigned int>::iterator hpit = _penetration_locator._has_penetrated.find(_current_node->id());
  return (hpit != _penetration_locator._has_penetrated.end());
}

void
MechanicalContactConstraint::computeContactForce(PenetrationInfo * pinfo)
{
  std::map<unsigned int, Real> & lagrange_multiplier = _penetration_locator._lagrange_multiplier;
  const Node * node = pinfo->_node;

  RealVectorValue res_vec;
  // Build up residual vector
  for (unsigned int i=0; i<_mesh_dimension; ++i)
  {
    long int dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);
  }
  RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
  RealVectorValue pen_force(_penalty * distance_vec);
  RealVectorValue tan_residual(0,0,0);

  switch (_model)
  {
    case CM_FRICTIONLESS:
    case CM_EXPERIMENTAL:
      switch (_formulation)
      {
        case CF_DEFAULT:
          pinfo->_contact_force = -pinfo->_normal * (pinfo->_normal * res_vec);
          break;
        case CF_PENALTY:
          pinfo->_contact_force = pinfo->_normal * (pinfo->_normal * pen_force);
          break;
        case CF_AUGMENTED_LAGRANGE:
          pinfo->_contact_force = (pinfo->_normal * (pinfo->_normal *
                                  ( pen_force + lagrange_multiplier[node->id()] * pinfo->_normal)));
                                //( pen_force + (lagrange_multiplier[node->id()]/distance_vec.size())*distance_vec)));
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
        case CF_DEFAULT:
          pinfo->_contact_force =  -res_vec;
          break;
        case CF_PENALTY:
        {
          distance_vec = pinfo->_incremental_slip + (pinfo->_normal * (_mesh.node(node->id()) - pinfo->_closest_point)) * pinfo->_normal;
          pen_force = _penalty * distance_vec;

          // Frictional capacity
          // const Real capacity( _friction_coefficient * (pen_force * pinfo->_normal < 0 ? -pen_force * pinfo->_normal : 0) );
          const Real capacity( _friction_coefficient * (res_vec * pinfo->_normal > 0 ? res_vec * pinfo->_normal : 0) );

          // Elastic predictor
          pinfo->_contact_force = pen_force + (pinfo->_contact_force_old - pinfo->_normal*(pinfo->_normal*pinfo->_contact_force_old));
          RealVectorValue contact_force_normal( (pinfo->_contact_force*pinfo->_normal) * pinfo->_normal );
          RealVectorValue contact_force_tangential( pinfo->_contact_force - contact_force_normal );

          // Tangential magnitude of elastic predictor
          const Real tan_mag( contact_force_tangential.size() );

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
                                  lagrange_multiplier[node->id()]*distance_vec/distance_vec.size();
          break;
        default:
          mooseError("Invalid contact formulation");
          break;
      }
      break;
    case CM_GLUED:
    case CM_TIED:
      switch (_formulation)
      {
        case CF_DEFAULT:
          pinfo->_contact_force =  -res_vec;
          break;
        case CF_PENALTY:
          pinfo->_contact_force = pen_force;
          break;
        case CF_AUGMENTED_LAGRANGE:
          pinfo->_contact_force = pen_force +
                                  lagrange_multiplier[node->id()]*distance_vec/distance_vec.size();
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
  computeContactForce(pinfo);
  Real resid = pinfo->_contact_force(_component);

  switch (type)
  {
    case Moose::Slave:
      if (_formulation == CF_DEFAULT)
      {
        //Real distance = (*_current_node)(_component) - pinfo->_closest_point(_component);
        //Real pen_force = _penalty * distance;
        RealVectorValue distance_vec(*_current_node - pinfo->_closest_point);
        RealVectorValue pen_force(_penalty * distance_vec);

        if (_model == CM_FRICTIONLESS || _model == CM_EXPERIMENTAL)
          resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;

        else if (_model == CM_GLUED || _model == CM_TIED || _model == CM_COULOMB)
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

  switch (type)
  {
    case Moose::SlaveSlave:
      switch (_model)
      {
        case CM_FRICTIONLESS:
        case CM_EXPERIMENTAL:
          switch (_formulation)
          {
            case CF_DEFAULT:
            {
              double curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
              //TODO:  Need off-diagonal term/s
              return (-curr_jac + _phi_slave[_j][_qp] * _penalty * _test_slave[_i][_qp]) * pinfo->_normal(_component) * pinfo->_normal(_component);
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              //TODO:  Need off-diagonal terms
              return _phi_slave[_j][_qp] * _penalty * _test_slave[_i][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
        case CM_TIED:
          switch (_formulation)
          {
            case CF_DEFAULT:
            {
              double curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
              return -curr_jac + _phi_slave[_j][_qp] * _penalty * _test_slave[_i][_qp];
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _phi_slave[_j][_qp] * _penalty * _test_slave[_i][_qp];
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
        case CM_EXPERIMENTAL:
          switch (_formulation)
          {
            case CF_DEFAULT:
            {
              Node * curr_master_node = _current_master->get_node(_j);
              double curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), curr_master_node->dof_number(0, _vars(_component), 0));
              //TODO:  Need off-diagonal terms
              return (-curr_jac - _phi_master[_j][_qp] * _penalty * _test_slave[_i][_qp]) * pinfo->_normal(_component) * pinfo->_normal(_component);
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              //TODO:  Need off-diagonal terms
              return -_phi_master[_j][_qp] * _penalty * _test_slave[_i][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
        case CM_TIED:
          switch (_formulation)
          {
            case CF_DEFAULT:
            {
              Node * curr_master_node = _current_master->get_node(_j);
              double curr_jac = (*_jacobian)( _current_node->dof_number(0, _vars(_component), 0), curr_master_node->dof_number(0, _vars(_component), 0));
              return -curr_jac - _phi_master[_j][_qp] * _penalty * _test_slave[_i][_qp];
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_phi_master[_j][_qp] * _penalty * _test_slave[_i][_qp];
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
        case CM_EXPERIMENTAL:
          switch (_formulation)
          {
            case CF_DEFAULT:
            {
              //TODO:  Need off-diagonal terms
              double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
              //TODO: To get off-diagonal terms correct using an approach like this, we would need to assemble in the rows for
              //all displacement components times their components of the normal vector.
              return slave_jac * _test_master[_i][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              //TODO:  Need off-diagonal terms
              return -_test_master[_i][_qp] * _penalty * _phi_slave[_j][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
        case CM_TIED:
          switch (_formulation)
          {
            case CF_DEFAULT:
            {
              double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
              return slave_jac * _test_master[_i][_qp];
            }
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return -_test_master[_i][_qp] * _penalty * _phi_slave[_j][_qp];
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
        case CM_EXPERIMENTAL:
          switch (_formulation)
          {
            case CF_DEFAULT:
              return 0;
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              //TODO: Need off-diagonal terms
              return _test_master[_i][_qp] * _penalty * _phi_master[_j][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
            default:
              mooseError("Invalid contact formulation");
          }
        case CM_COULOMB:
        case CM_GLUED:
        case CM_TIED:
          switch (_formulation)
          {
            case CF_DEFAULT:
              return 0;
            case CF_PENALTY:
            case CF_AUGMENTED_LAGRANGE:
              return _test_master[_i][_qp] * _penalty * _phi_master[_j][_qp];
            default:
              mooseError("Invalid contact formulation");
          }
        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0;
}
