#include "SlaveConstraint.h"

// Moose includes
#include "SystemBase.h"

// libmesh includes
#include "plane.h"
#include "sparse_matrix.h"
#include "string_to_enum.h"

template<>
InputParameters validParams<SlaveConstraint>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<unsigned int>("boundary", "The slave boundary");
  params.addRequiredParam<unsigned int>("master", "The master boundary");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<std::string>("order", "FIRST", "The finite element order");
  return params;
}

SlaveConstraint::SlaveConstraint(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _component(getParam<unsigned int>("component")),
   _model(contactModel(getParam<std::string>("model"))),
   _penetration_locator(getPenetrationLocator(getParam<unsigned int>("master"), getParam<unsigned int>("boundary"), Utility::string_to_enum<Order>(getParam<std::string>("order")))),
   _penalty(getParam<Real>("penalty")),
   _residual_copy(_sys.residualGhosted()),
   _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
   _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
   _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
   _vars(_x_var, _y_var, _z_var)
{}

void
SlaveConstraint::addPoints()
{
  _point_to_info.clear();

  std::map<unsigned int, bool> & has_penetrated = _penetration_locator._has_penetrated;

  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();
  for(; it!=end; ++it)
  {
    PenetrationLocator::PenetrationInfo * pinfo = it->second;

    if (!pinfo)
    {
      continue;
    }

    unsigned int slave_node_num = it->first;

    const Node * node = pinfo->_node;

    std::map<unsigned int, bool>::iterator it( has_penetrated.find( slave_node_num ) );
    if(it != has_penetrated.end() && it->second == true && node->processor_id() == libMesh::processor_id())
    {
      // Find an element that is connected to this node that and that is also on this processor

      std::vector<unsigned int> & connected_elems = _mesh.nodeToElemMap()[slave_node_num];

      Elem * elem = NULL;

      for(unsigned int i=0; i<connected_elems.size() && !elem; ++i)
      {
        Elem * cur_elem = _mesh.elem(connected_elems[i]);
        if(cur_elem->processor_id() == libMesh::processor_id())
          elem = cur_elem;
      }

      mooseAssert(elem, "Couldn't find an element on this processor that is attached to the slave node!");

      addPoint(elem, *node);
      _point_to_info[*node] = pinfo;
    }
  }
}

Real
SlaveConstraint::computeQpResidual()
{
  PenetrationLocator::PenetrationInfo * pinfo = _point_to_info[_current_point];
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
  Real resid(0);
  switch(_model)
  {
  case CM_FRICTIONLESS:
  case CM_EXPERIMENTAL:

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

//  std::cout<<node->id()<<":: "<<constraint_mag<<std::endl;
  return _test[_i][_qp] * resid;
}

Real
SlaveConstraint::computeQpJacobian()
{
  PenetrationLocator::PenetrationInfo * pinfo = _point_to_info[_current_point];
  const Node * node = pinfo->_node;
//   long int dof_number = node->dof_number(0, _var.number(), 0);


//  RealVectorValue jac_vec;

  // Build up jac vector
//  for(unsigned int i=0; i<_dim; i++)
//  {
//    unsigned int dof_row = _dof_data._var_dof_indices[_var_num][_i];
//    unsigned int dof_col = _dof_data._var_dof_indices[_var_num][_j];

//    Real jac_value = _jacobian_copy(dof_row, dof_col);
//  }

//    Real jac_mag = pinfo->_normal(_component) * jac_value;
/*
   return _test[_i][_qp] * (
     (1e8*-_phi[_j][_qp])
     -_jacobian_copy(dof_number, dof_number)
     );
   */

  RealVectorValue normal(pinfo->_normal);

  Real term(0);

  if ( CM_FRICTIONLESS == _model ||
       CM_EXPERIMENTAL == _model )
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

  return _test[_i][_qp] * term * _phi[_j][_qp];
}
