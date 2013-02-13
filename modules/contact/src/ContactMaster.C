#include "ContactMaster.h"
#include "FrictionalContactProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<ContactMaster>()
{
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<BoundaryName>("boundary", "The master boundary");
  params.addRequiredParam<BoundaryName>("slave", "The slave boundary");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<Real>("tension_release", 0.0, "Tension release threshold.  A node in contact will not be released if its tensile load is below this value.  Must be positive.");

  params.addParam<std::string>("formulation", "default", "The contact formulation");
  return params;
}



ContactMaster::ContactMaster(const std::string & name, InputParameters parameters) :
  DiracKernel(name, parameters),
  _component(getParam<unsigned int>("component")),
  _model(contactModel(getParam<std::string>("model"))),
  _formulation(contactFormulation(getParam<std::string>("formulation"))),
  _penetration_locator(getPenetrationLocator(getParam<BoundaryName>("boundary"), getParam<BoundaryName>("slave"), Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),
  _penalty(getParam<Real>("penalty")),
  _friction_coefficient(getParam<Real>("friction_coefficient")),
  _tension_release(getParam<Real>("tension_release")),
  _updateContactSet(true),
  _time_last_called(-std::numeric_limits<Real>::max()),
  _residual_copy(_sys.residualGhosted()),
  _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
  _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
  _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
  _vars(_x_var, _y_var, _z_var)
{
  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  if (_model == CM_GLUED)
  {
    _penetration_locator.setUpdate(false);
  }
  if (_tension_release < 0)
  {
    mooseError("The parameter 'tension_release' must be non-negative");
  }
  if (_friction_coefficient < 0)
  {
    mooseError("The friction coefficient must be nonnegative");
  }
}

void
ContactMaster::jacobianSetup()
{
  if (_component == 0)
  {
    if (_updateContactSet)
    {
      updateContactSet();
    }
    _updateContactSet = true;
  }
}

void
ContactMaster::timestepSetup()
{
  if (_component == 0)
  {
    _penetration_locator._unlocked_this_step.clear();
    _penetration_locator._locked_this_step.clear();
    _penetration_locator.setStartingContactPoint();
    updateContactSet();
    _updateContactSet = false;
    if (_t > _time_last_called)
    {
      _penetration_locator.saveContactForce();
    }
    _time_last_called = _t;
  }
}

void
ContactMaster::updateContactSet()
{
  std::map<unsigned int, bool> & has_penetrated = _penetration_locator._has_penetrated;
  std::map<unsigned int, unsigned> & unlocked_this_step = _penetration_locator._unlocked_this_step;
  std::map<unsigned int, unsigned> & locked_this_step = _penetration_locator._locked_this_step;
  std::map<unsigned int, Real> & lagrange_multiplier = _penetration_locator._lagrange_multiplier;

  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();

  for (; it!=end; ++it)
  {
    PenetrationLocator::PenetrationInfo * pinfo = it->second;

    if (!pinfo)
    {
      continue;
    }
    const unsigned int slave_node_num = it->first;

    if (_model == CM_EXPERIMENTAL)
    {
      const Node * node = pinfo->_node;

      Real resid( pinfo->_normal * -pinfo->_contact_force );

      // std::cout << locked_this_step[slave_node_num] << " " << pinfo->_distance << std::endl;
      const Real distance( pinfo->_normal * (pinfo->_closest_point - _mesh.node(node->id())));

      if (has_penetrated[slave_node_num] && resid < -_tension_release && locked_this_step[slave_node_num] < 2)
      {
        std::cout << "Releasing node " << node->id() << " " << resid << " < " << -_tension_release << std::endl;
        has_penetrated[slave_node_num] = false;
        pinfo->_contact_force.zero();
        ++unlocked_this_step[slave_node_num];
      }
      else if (distance > 0)
      {
        if (!has_penetrated[slave_node_num])
        {
          std::cout << "Capturing node " << node->id() << " " << distance << " " << unlocked_this_step[slave_node_num] <<  std::endl;
          ++locked_this_step[slave_node_num];
        }
        has_penetrated[slave_node_num] = true;
      }
    }
    else
    {
      if (pinfo->_distance >= 0)
      {
        //unsigned int slave_node_num = it->first;
        //has_penetrated.insert(std::make_pair<unsigned int, bool>(slave_node_num, true));
        has_penetrated[slave_node_num] = true;
      }
    }
    if (_formulation == CF_AUGMENTED_LAGRANGE && has_penetrated[slave_node_num])
    {
      const RealVectorValue distance_vec(_mesh.node(slave_node_num) - pinfo->_closest_point);
      lagrange_multiplier[slave_node_num] += _penalty * pinfo->_normal * distance_vec;
    }
  }
}

void
ContactMaster::addPoints()
{
  _point_to_info.clear();

  std::map<unsigned int, bool> & has_penetrated = _penetration_locator._has_penetrated;

  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();
  for (; it!=end; ++it)
  {
    PenetrationLocator::PenetrationInfo * pinfo = it->second;

    if (!pinfo)
    {
      continue;
    }

    unsigned int slave_node_num = it->first;


    std::map<unsigned int, bool>::iterator it( has_penetrated.find( slave_node_num ) );
    if ( it != has_penetrated.end() && it->second == true )
    {
      addPoint(pinfo->_elem, pinfo->_closest_point);
      _point_to_info[pinfo->_closest_point] = pinfo;
    }
  }
}

Real
ContactMaster::computeQpResidual()
{
  std::map<unsigned int, Real> & lagrange_multiplier = _penetration_locator._lagrange_multiplier;

  PenetrationLocator::PenetrationInfo * pinfo = _point_to_info[_current_point];
  const Node * node = pinfo->_node;
//  std::cout<<node->id()<<std::endl;
//  long int dof_number = node->dof_number(0, _var_num, 0);
//  std::cout<<dof_number<<std::endl;
//  std::cout<<_residual_copy(dof_number)<<std::endl;

//  std::cout<<node->id()<<": "<<_residual_copy(dof_number)<<std::endl;
  Real resid(0);
  RealVectorValue res_vec;
  // Build up residual vector
  for(unsigned int i=0; i<_dim; ++i)
  {
    long int dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);
  }
  RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
  RealVectorValue pen_force(_penalty * distance_vec);
  RealVectorValue tan_residual(0,0,0);
//  Real tan_residual_mag(0);
  RealVectorValue unity(1.0, 1.0, 1.0);
  if (_model == CM_FRICTIONLESS ||
      _model == CM_EXPERIMENTAL)
  {

    switch (_formulation)
    {
    case CF_DEFAULT:
      resid = pinfo->_normal(_component) * (pinfo->_normal * res_vec);
      break;
    case CF_PENALTY:
      resid = -pinfo->_normal(_component) * (pinfo->_normal * pen_force);
      break;
    case CF_AUGMENTED_LAGRANGE:
      resid = -(pinfo->_normal(_component) * (pinfo->_normal *
          //( pen_force + (lagrange_multiplier[node->id()]/distance_vec.size())*distance_vec)));
          ( pen_force + lagrange_multiplier[node->id()] * pinfo->_normal)));
      break;
    default:
      mooseError("Invalid contact formulation");
      break;
    }
    pinfo->_contact_force(_component) = -resid;
  }
  else if (_model == CM_COULOMB)
  {

    if (_formulation == CF_PENALTY)
    {
      distance_vec = pinfo->_incremental_slip + (pinfo->_normal * (_mesh.node(node->id()) - pinfo->_closest_point)) * pinfo->_normal;
      pen_force = _penalty * distance_vec;

      resid = pinfo->_normal * pen_force;
    }
    else
    {
      mooseError("Invalid contact formulation");
    }

    // Frictional capacity
    // const Real capacity( _friction_coefficient * (pen_force * pinfo->_normal < 0 ? -resid : 0) );
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
    }

    // Change the sign since master force is equal and opposite slave force
    resid = -pinfo->_contact_force(_component);

  }
  else if (_model == CM_GLUED ||
           _model == CM_TIED)
  {
    switch(_formulation)
    {
    case CF_DEFAULT:
      resid = res_vec(_component);
      break;
    case CF_PENALTY:
      resid = -pen_force(_component);
      break;
    case CF_AUGMENTED_LAGRANGE:
      resid = -(pen_force(_component) +
          lagrange_multiplier[node->id()]*distance_vec(_component)/distance_vec.size());
      break;
    default:
      mooseError("Invalid contact formulation");
      break;
    }
    pinfo->_contact_force(_component) = -resid;
  }
  else
  {
    mooseError("Invalid or unavailable contact model");
  }

  return _test[_i][_qp] * resid;
}

Real
ContactMaster::computeQpJacobian()
{

  PenetrationLocator::PenetrationInfo * pinfo = _point_to_info[_current_point];

  switch(_model)
  {
  case CM_FRICTIONLESS:
  case CM_EXPERIMENTAL:
    switch (_formulation)
    {
    case CF_DEFAULT:
      return 0;
      break;
    case CF_PENALTY:
    case CF_AUGMENTED_LAGRANGE:
      return _test[_i][_qp] * _penalty * _phi[_j][_qp] * pinfo->_normal(_component) * pinfo->_normal(_component);
      break;
    default:
      mooseError("Invalid contact formulation");
      break;
    }
    break;
  case CM_GLUED:
  case CM_COULOMB:
  case CM_TIED:
    switch (_formulation)
    {
    case CF_DEFAULT:
      return 0;
      break;
    case CF_PENALTY:
    case CF_AUGMENTED_LAGRANGE:
      return _test[_i][_qp] * _penalty * _phi[_j][_qp];
      break;
    default:
      mooseError("Invalid contact formulation");
      break;
    }
    break;
  default:
    mooseError("Invalid or unavailable contact model");
    break;
  }

  return 0;
/*
  if(_i != _j)
    return 0;

  Node * node = pinfo->_node;

  RealVectorValue jac_vec;

  // Build up jac vector
  for(unsigned int i=0; i<_dim; i++)
  {
    long int dof_number = node->dof_number(0, _vars(i), 0);
    jac_vec(i) = _jacobian_copy(dof_number, dof_number);
  }

  Real jac_mag = pinfo->_normal * jac_vec;

  return _test[_i][_qp]*pinfo->_normal(_component)*jac_mag;
*/
}

ContactModel
contactModel(const std::string & the_name)
{
  ContactModel model(CM_INVALID);
  std::string name(the_name);
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  if ("frictionless" == name)
  {
    model = CM_FRICTIONLESS;
  }
  else if ("glued" == name)
  {
    model = CM_GLUED;
  }
  else if ("coulomb" == name)
  {
    model = CM_COULOMB;
  }
  else if ("tied" == name)
  {
    model = CM_TIED;
  }
  else if ("experimental" == name)
  {
    model = CM_EXPERIMENTAL;
  }
  else
  {
    std::string err("Invalid contact model found: ");
    err += name;
    mooseError( err );
  }
  return model;
}

ContactFormulation
contactFormulation(const std::string & the_name)
{
  ContactFormulation formulation(CF_INVALID);
  std::string name(the_name);
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  if ("default" == name)
  {
    formulation = CF_DEFAULT;
  }
  else if ("penalty" == name)
  {
    formulation = CF_PENALTY;
  }
  else if ("augmented_lagrange" == name)
  {
    formulation = CF_AUGMENTED_LAGRANGE;
  }
  if (formulation == CF_INVALID)
  {
    std::string err("Invalid formulation found: ");
    err += name;
    mooseError( err );
  }
  return formulation;
}
