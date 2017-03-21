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

#include "NodalNormalBC.h"

template <>
InputParameters
validParams<NodalNormalBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addCoupledVar("nx", "x-component of the normal");
  params.addCoupledVar("ny", "y-component of the normal");
  params.addCoupledVar("nz", "z-component of the normal");

  params.set<std::vector<VariableName>>("nx") = {"nodal_normal_x"};
  params.set<std::vector<VariableName>>("ny") = {"nodal_normal_y"};
  params.set<std::vector<VariableName>>("nz") = {"nodal_normal_z"};

  return params;
}

NodalNormalBC::NodalNormalBC(const InputParameters & parameters)
  : NodalBC(parameters), _nx(coupledValue("nx")), _ny(coupledValue("ny")), _nz(coupledValue("nz"))
{
}

void
NodalNormalBC::computeResidual(NumericVector<Number> & residual)
{
  _qp = 0;
  _normal = Point(_nx[_qp], _ny[_qp], _nz[_qp]);
  NodalBC::computeResidual(residual);
}
