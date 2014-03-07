#include "DesorptionFromMatrix.h"

#include <iostream>


template<>
InputParameters validParams<DesorptionFromMatrix>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("pressure_var", "Variable representing the porepressure of the fluid");
  params.addClassDescription("Mass flow rate from the matrix.  Add this to TimeDerivative kernel to get complete DE");
  return params;
}

DesorptionFromMatrix::DesorptionFromMatrix(const std::string & name,
                                           InputParameters parameters) :
    Kernel(name,parameters),
    _pressure_var(coupled("pressure_var")),
    _desorption_time_const(getMaterialProperty<Real>("desorption_time_const")),
    _adsorption_time_const(getMaterialProperty<Real>("adsorption_time_const")),
    _equilib_conc(getMaterialProperty<Real>("desorption_equilib_conc")),
    _equilib_conc_prime(getMaterialProperty<Real>("desorption_equilib_conc_prime"))
{}


Real
DesorptionFromMatrix::computeQpResidual()
{
  if (_u[_qp] > _equilib_conc[_qp])
  {
    if (_desorption_time_const[_qp] > 0)
      return _test[_i][_qp]*(_u[_qp] - _equilib_conc[_qp])/_desorption_time_const[_qp];
    return 0.0;
  }
  if (_adsorption_time_const[_qp] > 0)
    return _test[_i][_qp]*(_u[_qp] - _equilib_conc[_qp])/_adsorption_time_const[_qp];
  return 0.0;
}

Real
DesorptionFromMatrix::computeQpJacobian()
{
  if (_u[_qp] > _equilib_conc[_qp])
  {
    if (_desorption_time_const[_qp] > 0)
      return _test[_i][_qp]*_phi[_j][_qp]/_desorption_time_const[_qp];
    return 0.0;
  }
  if (_adsorption_time_const[_qp] > 0)
    return _test[_i][_qp]*_phi[_j][_qp]/_adsorption_time_const[_qp];
  return 0.0;
}

Real
DesorptionFromMatrix::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _pressure_var)
    return 0.0;
  if (_u[_qp] > _equilib_conc[_qp])
  {
    if (_desorption_time_const[_qp] > 0)
      return -_test[_i][_qp]*_equilib_conc_prime[_qp]*_phi[_j][_qp]/_desorption_time_const[_qp];
    return 0.0;
  }
  if (_adsorption_time_const[_qp] > 0)
    return -_test[_i][_qp]*_equilib_conc_prime[_qp]*_phi[_j][_qp]/_adsorption_time_const[_qp];
  return 0.0;
}
