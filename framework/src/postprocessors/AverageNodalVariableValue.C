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

#include "AverageNodalVariableValue.h"
#include "MooseMesh.h"
#include "SubProblem.h"
// libMesh
#include "boundary_info.h"

template<>
InputParameters validParams<AverageNodalVariableValue>()
{
  InputParameters params = validParams<NodalPostprocessor>();
  return params;
}

AverageNodalVariableValue::AverageNodalVariableValue(const std::string & name, InputParameters parameters) :
    NodalPostprocessor(name, parameters),
    _avg(0),
    _n(0)
{
}

void
AverageNodalVariableValue::initialize()
{
  _avg = 0;
  _n = 0;
}

void
AverageNodalVariableValue::execute()
{
  _avg += _u[_qp];
  _n++;
}

Real
AverageNodalVariableValue::getValue()
{
  gatherSum(_avg);
  gatherSum(_n);

  return _avg / _n;
}

void
AverageNodalVariableValue::threadJoin(const UserObject & y)
{
  const AverageNodalVariableValue & pps = dynamic_cast<const AverageNodalVariableValue &>(y);
  _avg += pps._avg;
  _n += pps._n;
}
