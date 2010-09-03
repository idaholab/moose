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

#include "MatchedValueBC.h"

template<>
InputParameters validParams<MatchedValueBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredCoupledVar("v", "");
  params.set<bool>("_integrated") = false;
  return params;
}

MatchedValueBC::MatchedValueBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
    _v_face(coupledValue("v"))
  {}

Real
MatchedValueBC::computeQpResidual()
  {
    return _u[_qp]-_v_face[_qp];
  }
