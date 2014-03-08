#include "DesorptionToPorespace.h"

#include <iostream>


template<>
InputParameters validParams<DesorptionToPorespace>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("conc_var", "Variable representing the concentration (kg/m^3) of fluid in the matrix that will be desorped to porespace");
  params.addClassDescription("Add this to the variable's equation so fluid contained in the matrix with concentration conc_var will desorb to the variable's porespace mass");
  return params;
}

DesorptionToPorespace::DesorptionToPorespace(const std::string & name,
                                             InputParameters parameters) :
    Kernel(name,parameters),
    _conc_val(&coupledValue("conc_var")),
    _conc_var(coupled("conc_var")),

    _desorption_time_const(getMaterialProperty<Real>("desorption_time_const")),
    _adsorption_time_const(getMaterialProperty<Real>("adsorption_time_const")),
    _equilib_conc(getMaterialProperty<Real>("desorption_equilib_conc")),
    _equilib_conc_prime(getMaterialProperty<Real>("desorption_equilib_conc_prime"))
{
}


Real
DesorptionToPorespace::computeQpResidual()
{
  if ((*_conc_val)[_qp] > _equilib_conc[_qp])
  {
    if (_desorption_time_const[_qp] > 0)
      return -_test[_i][_qp]*((*_conc_val)[_qp] - _equilib_conc[_qp])/_desorption_time_const[_qp];
    return 0.0;
  }
  if (_adsorption_time_const[_qp] > 0)
    return -_test[_i][_qp]*((*_conc_val)[_qp] - _equilib_conc[_qp])/_adsorption_time_const[_qp];
  return 0.0;

}

Real
DesorptionToPorespace::computeQpJacobian()
{
  if ((*_conc_val)[_qp] > _equilib_conc[_qp])
  {
    if (_desorption_time_const[_qp] > 0)
      return _test[_i][_qp]*_equilib_conc_prime[_qp]*_phi[_j][_qp]/_desorption_time_const[_qp];
    return 0.0;
  }
  if (_adsorption_time_const[_qp] > 0)
    return _test[_i][_qp]*_equilib_conc_prime[_qp]*_phi[_j][_qp]/_adsorption_time_const[_qp];
  return 0.0;

}

Real
DesorptionToPorespace::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _conc_var)
    return 0.0;
  if ((*_conc_val)[_qp] > _equilib_conc[_qp])
  {
    if (_desorption_time_const[_qp] > 0)
      return -_test[_i][_qp]*_phi[_j][_qp]/_desorption_time_const[_qp];
    return 0.0;
  }
  if (_adsorption_time_const[_qp] > 0)
    return -_test[_i][_qp]*_phi[_j][_qp]/_adsorption_time_const[_qp];
  return 0.0;
}
