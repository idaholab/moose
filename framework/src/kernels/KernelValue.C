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

#include "KernelValue.h"
#include "Moose.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<KernelValue>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


KernelValue::KernelValue(const std::string & name, MooseSystem & moose_system, InputParameters parameters):
  Kernel(name, moose_system, parameters)
{
}

KernelValue::~KernelValue()
{
}

void
KernelValue::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","KernelValue");
  
  DenseSubVector<Number> & var_Re = *_dof_data._var_Res[_var_num];

  _value.resize(_qrule->n_points(), 0);
  precalculateResidual();
  for (_i=0; _i<_phi.size(); _i++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*_value[_qp]*_test[_i][_qp];
  
//  Moose::perf_log.pop("computeResidual()","KernelValue");
}

Real
KernelValue::computeQpResidual()
{
  return 0;
}
