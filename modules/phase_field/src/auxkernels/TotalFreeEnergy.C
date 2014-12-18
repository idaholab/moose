#include "TotalFreeEnergy.h"

template<>
InputParameters validParams<TotalFreeEnergy>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<std::string>("f_name", "F"," Base name of the free energy function used to name the material properties");
  params.addRequiredCoupledVar("interfacial_vars", "Variable names that contribute to interfacial energy");
  params.addRequiredParam< std::vector<std::string> >("kappa_names", "Name of the kappa parameters corresponding to each variable name in interfacial_vars. The order of the variable names in interfacial_vars needs to be the same as their kappas in this vector");

  return params;
}

TotalFreeEnergy::TotalFreeEnergy(const std::string & name,
                                                         InputParameters parameters) :
    AuxKernel(name, parameters),
    _nvars(coupledComponents("interfacial_vars")),
    _F(getMaterialProperty<Real>(getParam<std::string>("f_name")) ),
    _kappa_names(getParam<std::vector<std::string> >("kappa_names"))
{
  //Error check to ensure size of interfacial_vars is the same as kappa_names
  if (_nvars != _kappa_names.size())
    mooseError("Size of interfacial_vars is not equal to the size of kappa_names in TotalFreeEnergy");

  _grad_vars.resize(_nvars);
  _kappas.resize(_nvars);

  //Assign values of material and variable vectors
  for (unsigned int i = 0; i < _nvars; ++i)
  {
    _kappas[i] = &getMaterialProperty<Real>(_kappa_names[i]);
    _grad_vars[i] = &coupledGradient("interfacial_vars", i);
  }
}

Real
TotalFreeEnergy::computeValue()
{
  //Include bulk energy contribution
  Real total_energy = _F[_qp];

  // Calculate interfacial energy of each variable

  for (unsigned int i = 0; i < _nvars; ++i)
  {
    Real abs_grad_var = (*_grad_vars[i])[_qp].size();
    total_energy += (*_kappas[i])[_qp]/2.0*abs_grad_var*abs_grad_var;
  }

  return total_energy;
}
