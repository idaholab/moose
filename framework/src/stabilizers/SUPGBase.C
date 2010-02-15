#include "SUPGBase.h"

template<>
InputParameters validParams<SUPGBase>()
{
  InputParameters params;
  return params;
}

SUPGBase::SUPGBase(std::string name,
                       InputParameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as)
  :Stabilizer(name, parameters, var_name, coupled_to, coupled_as)
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
      _test[_i][_qp] += _tau[_qp]*(_velocity[_qp]*_dphi[_i][_qp]);
}
