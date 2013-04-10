#include "SlaveConstraint.h"
#include "FrictionalContactProblem.h"

// Moose includes
#include "SystemBase.h"

// libmesh includes
#include "libmesh/plane.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<SlaveConstraint>()
{
  MooseEnum orders("CONSTANT, FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<BoundaryName>("boundary", "The slave boundary");
  params.addRequiredParam<BoundaryName>("master", "The master boundary");
  params.addRequiredParam<BoundaryName>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<std::string>("formulation", "default", "The contact formulation");
  return params;
}

SlaveConstraint::SlaveConstraint(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _component(getParam<unsigned int>("component")),
   _model(contactModel(getParam<std::string>("model"))),
   _formulation(contactFormulation(getParam<std::string>("formulation"))),
   _penetration_locator(getPenetrationLocator(getParam<BoundaryName>("master"), getParam<BoundaryName>("boundary"), Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),
   _penalty(getParam<Real>("penalty")),
   _friction_coefficient(getParam<Real>("friction_coefficient")),
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
  if (parameters.isParamValid("normal_smoothing_distance"))
  {
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));
  }
}

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
  std::map<unsigned int, Real> & lagrange_multiplier = _penetration_locator._lagrange_multiplier;

  PenetrationLocator::PenetrationInfo * pinfo = _point_to_info[_current_point];
  const Node * node = pinfo->_node;

  Real resid(0);
  RealVectorValue res_vec;
  // Build up residual vector
  for(unsigned int i=0; i<_dim; ++i)
  {
    int dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);
  }

  RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
  RealVectorValue pen_force(_penalty * distance_vec);
  RealVectorValue tan_residual(0,0,0);
  RealVectorValue unity(1.0, 1.0, 1.0);
  if (_model == CM_FRICTIONLESS || _model == CM_EXPERIMENTAL)
  {

    if (_formulation == CF_DEFAULT)
    {
      resid = pinfo->_normal(_component) * (pinfo->_normal * ( pen_force - res_vec ));
    }
    else if (_formulation == CF_PENALTY)
    {
      resid = pinfo->_normal(_component) * (pinfo->_normal * ( pen_force ));
    }
    else if (_formulation == CF_AUGMENTED_LAGRANGE)
    {
      resid = pinfo->_normal(_component) * (pinfo->_normal *
          //( pen_force + (lagrange_multiplier[node->id()]/distance_vec.size())*distance_vec));
          ( pen_force + (lagrange_multiplier[node->id()] * pinfo->_normal)));
    }
    else
    {
      mooseError("Invalid contact formulation");
    }

    pinfo->_contact_force(_component) = resid;

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

    resid = pinfo->_contact_force(_component);

  }
  else if (_model == CM_GLUED || _model == CM_TIED)
  {

    if(_formulation == CF_DEFAULT)
    {
      resid = pen_force(_component) - res_vec(_component);
    }
    else if (_formulation == CF_PENALTY)
    {
      resid = pen_force(_component);
    }
    else if (_formulation == CF_AUGMENTED_LAGRANGE)
    {
      resid = lagrange_multiplier[node->id()]*distance_vec(_component)/distance_vec.size() + pen_force(_component);
    }
    else
    {
      mooseError("Invalid contact formulation");
    }

    pinfo->_contact_force(_component) = resid;

  }
  else
  {
    mooseError("Invalid or unavailable contact model");
  }

  return _test[_i][_qp] * resid;
}

Real
SlaveConstraint::computeQpJacobian()
{

  // TODO: for the default formulation,
  //   we should subtract off the existing Jacobian weighted by the effect of the normal


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
    else if ( _dim == 2 )
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
            CM_TIED == _model ||
            CM_COULOMB == _model )
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
