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

#include "SUPGBase.h"

#include "quadrature_gauss.h"

template<>
InputParameters validParams<SUPGBase>()
{
  InputParameters params = validParams<Stabilizer>();
  return params;
}

SUPGBase::SUPGBase(const std::string & name,
                   MooseSystem & moose_system,
                   InputParameters parameters)
  :Stabilizer(name, moose_system, parameters)
{}

void
SUPGBase::computeTestFunctions()
{
  unsigned int num_q_points = _qrule->n_points();
  unsigned int num_shape = _phi.size();

  _tau.resize(num_q_points);
  _velocity.resize(num_q_points);

  computeTausAndVelocities();

  for(_i=0; _i<num_shape; _i++)
    for(_qp=0; _qp<num_q_points; _qp++)
      _test[_i][_qp] += _tau[_qp]*(_velocity[_qp]*_grad_test[_i][_qp]);
}
