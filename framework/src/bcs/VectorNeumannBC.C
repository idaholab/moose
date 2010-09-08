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

#include "VectorNeumannBC.h"

template<>
InputParameters validParams<VectorNeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value0", 0.0, "x component of the vector this BC should act in.");
  params.addParam<Real>("value1", 0.0, "y component of the vector this BC should act in.");
  params.addParam<Real>("value2", 0.0, "z component of the vector this BC should act in.");
  return params;
}

VectorNeumannBC::VectorNeumannBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters)
  {
    _value(0)=getParam<Real>("value0");
    _value(1)=getParam<Real>("value1");
    _value(2)=getParam<Real>("value2");
  }


Real
VectorNeumannBC::computeQpResidual()
  {
    return -_phi[_i][_qp]*(_value*_normals[_qp]);
  }

