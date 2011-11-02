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
#include "MultiDContactConstraint.h"

#include "SystemBase.h"
#include "PenetrationLocator.h"

// libMesh includes
#include "string_to_enum.h"

template<>
InputParameters validParams<MultiDContactConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<bool>("jacobian_update", false, "Whether or not to update the 'in contact' list every jacobian evaluation (by default it will happen once per timestep");

  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");

  // TODO: Reenable this
//  params.addParam<std::string>("order", "FIRST", "The finite element order");

  return params;
}

MultiDContactConstraint::MultiDContactConstraint(const std::string & name, InputParameters parameters) :
    NodeFaceConstraint(name, parameters),
    _residual_copy(_sys.residualGhosted()),
    _jacobian_update(getParam<bool>("jacobian_update")),
    _component(getParam<unsigned int>("component")),
    _model(contactModel(getParam<std::string>("model"))),
    _penalty(getParam<Real>("penalty")),
    _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
    _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
    _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
    _vars(_x_var, _y_var, _z_var)
{
  _overwrite_slave_residual = false;
}

void
MultiDContactConstraint::timestepSetup()
{
  updateContactSet();
}

void
MultiDContactConstraint::jacobianSetup()
{
  updateContactSet();
}

void
MultiDContactConstraint::updateContactSet()
{
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

    if (pinfo->_distance > 0)
    {
      unsigned int slave_node_num = it->first;
      has_penetrated[slave_node_num] = true;
    }
  }
}

bool
MultiDContactConstraint::shouldApply()
{
  return _penetration_locator._has_penetrated[_current_node->id()];
}

Real
MultiDContactConstraint::computeQpSlaveValue()
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  std::cerr<<std::endl
           <<"Popping out node: "<<_current_node->id()<<std::endl
           <<"Closest Point x: "<<pinfo->_closest_point(_component)<<std::endl
           <<"Current Node x: "<<(*_current_node)(_component)<<std::endl
           <<"Current Value: "<<_u_slave[_qp]<<std::endl<<std::endl;
  
  return pinfo->_closest_point(_component) - ((*_current_node)(_component) - _u_slave[_qp]);
}

Real
MultiDContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  const Node * node = pinfo->_node;

  RealVectorValue res_vec;
  // Build up residual vector
  for(unsigned int i=0; i<_dim; ++i)
  {
    int dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);
  }

  const RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
  const RealVectorValue pen_force(_penalty * distance_vec);
  Real resid = 0;

  switch(type)
  {
  case Moose::Slave:
    switch(_model)
    {
    case CM_FRICTIONLESS:
      
      resid = pinfo->_normal(_component) * (pinfo->_normal * ( pen_force - res_vec ));
      break;

    case CM_GLUED:
    case CM_TIED:

      resid = pen_force(_component)
        - res_vec(_component)
        ;

      break;
      
    default:
      mooseError("Invalid or unavailable contact model");
    }
    return _test_slave[_i][_qp] * resid;
  case Moose::Master:
    switch(_model)
    {
    case CM_FRICTIONLESS:

      resid = pinfo->_normal(_component) * (pinfo->_normal * res_vec);
      break;

    case CM_GLUED:
    case CM_TIED:
      resid = res_vec(_component);
      break;

    default:
      mooseError("Invalid or unavailable contact model");
    }
    return _test_master[_i][_qp] * resid;
  }
  return 0;
}

Real
MultiDContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  const Node * node = pinfo->_node;

  RealVectorValue normal(pinfo->_normal);
  
  Real term = 0;
  
  double slave_jac = 0;
  switch(type)
  {
  case Moose::SlaveSlave:

    if ( CM_FRICTIONLESS == _model )
    {
      const Real nnTDiag = normal(_component) * normal(_component);
      term =  _penalty * nnTDiag;

      const RealGradient & A1( pinfo->_dxyzdxi [0] );
      RealGradient A2;
      RealGradient d2;
      if ( _dim == 3 )
      {
        A2 = pinfo->_dxyzdeta[0];
        d2 = pinfo->_d2xyzdxideta[0];
      }
      else
      {
        A2.zero();
        d2.zero();
      }

      const RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
      const Real ATA11( A1 * A1 );
      const Real ATA12( A1 * A2 );
      const Real ATA22( A2 * A2 );
      const Real D11( -ATA11 );
      const Real D12( -ATA12 + d2 * distance_vec );
      const Real D22( -ATA22 );

      Real invD11(0);
      Real invD12(0);
      Real invD22(0);
      if ( _dim == 3)
      {
        const Real detD( D11*D22 - D12*D12 );
        invD11 =  D22/detD;
        invD12 = -D12/detD;
        invD22 =  D11/detD;
      }
      else
      {
        invD11 = 1 / D11;
      }

      const Real AinvD11( A1(0)*invD11 + A2(0)*invD12 );
      const Real AinvD12( A1(0)*invD12 + A2(0)*invD22 );
      const Real AinvD21( A1(1)*invD11 + A2(1)*invD12 );
      const Real AinvD22( A1(1)*invD12 + A2(1)*invD22 );
      const Real AinvD31( A1(2)*invD11 + A2(2)*invD12 );
      const Real AinvD32( A1(2)*invD12 + A2(2)*invD22 );

      const Real AinvDAT11( AinvD11*A1(0) + AinvD12*A2(0) );
//     const Real AinvDAT12( AinvD11*A1(1) + AinvD12*A2(1) );
//     const Real AinvDAT13( AinvD11*A1(2) + AinvD12*A2(2) );
//     const Real AinvDAT21( AinvD21*A1(0) + AinvD22*A2(0) );
      const Real AinvDAT22( AinvD21*A1(1) + AinvD22*A2(1) );
//     const Real AinvDAT23( AinvD21*A1(2) + AinvD22*A2(2) );
//     const Real AinvDAT31( AinvD31*A1(0) + AinvD32*A2(0) );
//     const Real AinvDAT32( AinvD31*A1(1) + AinvD32*A2(1) );
      const Real AinvDAT33( AinvD31*A1(2) + AinvD32*A2(2) );

      if ( _component == 0 )
      {
        term += _penalty * ( 1 - nnTDiag + AinvDAT11 );
      }
      else if ( _component == 1 )
      {
        term += _penalty * ( 1 - nnTDiag + AinvDAT22 );
      }
      else
      {
        term += _penalty * ( 1 - nnTDiag + AinvDAT33 );
      }
    }
    else if ( CM_GLUED == _model ||
              CM_TIED == _model )
    {
      normal.zero();
      normal(_component) = 1;
      term = _penalty;
    }
    else
    {
      mooseError("Invalid or unavailable contact model");
    }

    return _test_slave[_i][_qp] * term * _phi_slave[_j][_qp];
    break;
  case Moose::SlaveMaster:
    return 0;
    return -_phi_master[_j][_qp]*_test_slave[_i][_qp];
  case Moose::MasterSlave:
    return 0;
    slave_jac = (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]);
    return slave_jac*_test_master[_i][_qp];
  case Moose::MasterMaster:
    return 0;
  }
  return 0;
}
