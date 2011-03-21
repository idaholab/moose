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

#include "DashpotBC.h"

template<>
InputParameters validParams<DashpotBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<unsigned int>("component", "The displacement component corresponding the variable this BC acts on.");
  params.addRequiredCoupledVar("disp_x", "Displacement in the x direction");
  params.addCoupledVar("disp_y", "Displacement in the y direction");
  params.addCoupledVar("disp_z", "Displacement in the z direction");

  params.addParam<Real>("coefficient", 1.0, "The viscocity coefficient");
  
  return params;
}

DashpotBC::DashpotBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _component(getParam<unsigned int>("component")),
    _coefficient(getParam<Real>("coefficient")),
    _disp_x_var(coupled("disp_x")),
    _disp_y_var(isCoupled("disp_y") ? coupled("disp_y") : 0),
    _disp_z_var(isCoupled("disp_z") ? coupled("disp_z") : 0),

    _disp_x_dot(coupledValue("disp_x")),
    _disp_y_dot(isCoupled("disp_y") ? coupledDot("disp_y") : _zero),
    _disp_z_dot(isCoupled("disp_z") ? coupledDot("disp_z") : _zero)
{}

Real
DashpotBC::computeQpResidual()
{
  RealVectorValue velocity(_disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]);
  
  return _test[_i][_qp]*_coefficient*_normals[_qp]*velocity;
}

Real
DashpotBC::computeQpJacobian()
{
  RealVectorValue velocity;
  velocity(_component) = _phi[_j][_qp]/_dt;
   
  return _test[_i][_qp]*_coefficient*_normals[_qp]*velocity;
}

Real
DashpotBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  RealVectorValue velocity;
  unsigned int component = 0;
  
  if(jvar == _disp_x_var)
    component = 0;
  else if(jvar == _disp_y_var)
    component = 1;
  else if(jvar == _disp_z_var)
    component = 2;
  
  velocity(component) = _phi[_j][_qp]/_dt;
  
  return -_test[_i][_qp]*_normals[_qp]*velocity;
}


