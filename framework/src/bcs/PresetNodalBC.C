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

#include "PresetNodalBC.h"

template<>
InputParameters validParams<PresetNodalBC>()
{
  InputParameters p = validParams<NodalBC>();
  return p;
}


PresetNodalBC::PresetNodalBC(const std::string & name, InputParameters parameters) :
  NodalBC(name, parameters)
{

}

void
PresetNodalBC::computeValue(NumericVector<Number> & current_solution)
{
  dof_id_type & dof_idx = _var.nodalDofIndex();
  _qp = 0;
  current_solution.set(dof_idx, computeQpValue());
}

Real
PresetNodalBC::computeQpResidual()
{
  return _u[_qp] - computeQpValue();
}
