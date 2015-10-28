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

#include "TimeNodalKernel.h"

template<>
InputParameters validParams<TimeNodalKernel>()
{
  InputParameters params = validParams<NodalKernel>();
  return params;
}

TimeNodalKernel::TimeNodalKernel(const InputParameters & parameters) :
    NodalKernel(parameters)
{
}

void
TimeNodalKernel::computeResidual()
{
  if (_var.isNodalDefined())
  {
    dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    Real res = computeQpResidual();
    _assembly.cacheResidualContribution(dof_idx, res, Moose::KT_TIME);

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i=0; i<_save_in.size(); i++)
        _save_in[i]->sys().solution().add(_save_in[i]->nodalDofIndex(), res);
    }
  }
}
