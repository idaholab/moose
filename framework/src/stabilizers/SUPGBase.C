#include "SUPGBase.h"

#include "quadrature_gauss.h"

template<>
InputParameters validParams<SUPGBase>()
{
  InputParameters params = validParams<Stabilizer>();
  return params;
}

SUPGBase::SUPGBase(const std::string & name, InputParameters parameters) :
    Stabilizer(name, parameters)
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
