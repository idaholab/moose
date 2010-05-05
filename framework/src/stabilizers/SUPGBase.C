#include "SUPGBase.h"

template<>
InputParameters validParams<SUPGBase>()
{
  InputParameters params = validParams<Stabilizer>();
  return params;
}

SUPGBase::SUPGBase(std::string name,
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
      _test[_i][_qp] += _tau[_qp]*(_velocity[_qp]*_dtest[_i][_qp]);
}
