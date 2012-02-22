#include "ContactMaster.h"
#include "SystemBase.h"

// libmesh includes
#include "sparse_matrix.h"
#include "string_to_enum.h"

template<>
InputParameters validParams<ContactMaster>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<unsigned int>("boundary", "The master boundary");
  params.addRequiredParam<unsigned int>("slave", "The slave boundary");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<std::string>("order", "FIRST", "The finite element order");
  return params;
}

ContactMaster::ContactMaster(const std::string & name, InputParameters parameters) :
  DiracKernel(name, parameters),
  _component(getParam<unsigned int>("component")),
  _model(contactModel(getParam<std::string>("model"))),
  _penetration_locator(getPenetrationLocator(getParam<unsigned int>("boundary"), getParam<unsigned int>("slave"), Utility::string_to_enum<Order>(getParam<std::string>("order")))),
  _penalty(getParam<Real>("penalty")),
   _updateContactSet(true),
  _residual_copy(_sys.residualGhosted()),
  _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
  _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
  _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
  _vars(_x_var, _y_var, _z_var)
{
  if (_model == CM_GLUED)
  {
    _penetration_locator.setUpdate(false);
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
    updateContactSet();
    _updateContactSet = false;
  }
}

void
ContactMaster::updateContactSet()
{
  std::map<unsigned int, bool> & has_penetrated = _penetration_locator._has_penetrated;
  std::map<unsigned int, unsigned> & unlocked_this_step = _penetration_locator._unlocked_this_step;
  std::map<unsigned int, unsigned> & locked_this_step = _penetration_locator._locked_this_step;

  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();

  for (; it!=end; ++it)
  {
    PenetrationLocator::PenetrationInfo * pinfo = it->second;

    if (!pinfo)
    {
      continue;
    }

    if (_model == CM_EXPERIMENTAL)
    {
      const Node * node = pinfo->_node;

      // Build up residual vector
      RealVectorValue res_vec;
      for(unsigned int i=0; i<_dim; ++i)
      {
        int dof_number = node->dof_number(0, _vars(i), 0);
        res_vec(i) = _residual_copy(dof_number);
      }

      Real resid(0);
      switch(_model)
      {
      case CM_FRICTIONLESS:
      case CM_EXPERIMENTAL:

        resid = pinfo->_normal * res_vec;
        break;

      case CM_GLUED:
      case CM_TIED:

        resid = pinfo->_normal * res_vec;
        break;

      default:
        mooseError("Invalid or unavailable contact model");
      }

      unsigned int slave_node_num = it->first;

      // std::cout << locked_this_step[slave_node_num] << " " << pinfo->_distance << std::endl;

      if (has_penetrated[slave_node_num] && resid < 0 && locked_this_step[slave_node_num] < 3)
      {
        std::cout << "Releasing node " << node->id() << " " << resid << std::endl;
        has_penetrated[slave_node_num] = false;
        ++unlocked_this_step[slave_node_num];
      }
      else if (pinfo->_distance > 0)
      {
        if (!has_penetrated[slave_node_num])
        {
          std::cout << "Capturing node " << node->id() << " " << pinfo->_distance << " " << unlocked_this_step[slave_node_num] <<  std::endl;
          ++locked_this_step[slave_node_num];
        }
        has_penetrated[slave_node_num] = true;
      }
    }
    else
    {
      if (pinfo->_distance > 0)
      {
        unsigned int slave_node_num = it->first;
      has_penetrated.insert(std::make_pair<unsigned int, bool>(slave_node_num, true));
      }
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
//   const RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
//   const RealVectorValue pen_force(_penalty * distance_vec);
  switch(_model)
  {
  case CM_FRICTIONLESS:
  case CM_EXPERIMENTAL:

    resid = pinfo->_normal(_component) * (pinfo->_normal * res_vec);
    break;

  case CM_GLUED:
  case CM_TIED:
    resid = res_vec(_component);
//     resid -= pen_force(_component);
    break;

  default:
    mooseError("Invalid or unavailable contact model");
  }

  return _test[_i][_qp] * resid;
}

Real
ContactMaster::computeQpJacobian()
{

//   return _test[_i][_qp] * _penalty * _phi[_j][_qp];

  return 0;
/*
  if(_i != _j)
    return 0;

  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
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
