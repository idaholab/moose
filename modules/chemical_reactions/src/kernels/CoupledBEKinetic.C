/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledBEKinetic.h"
#include "Material.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<CoupledBEKinetic>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::vector<Real> >("weight", "The weight of kinetic species concentration");
  params.addCoupledVar("v", "List of kinetic species being coupled by concentration");
  return params;
}

CoupledBEKinetic::CoupledBEKinetic(const std::string & name, InputParameters parameters)
  :Kernel(name,parameters),
   _porosity(getMaterialProperty<Real>("porosity")),
   _weight(getParam<std::vector<Real> >("weight"))
{
  int n = coupledComponents("v");
//    _vars.resize(n);
    _vals.resize(n);
    _vals_old.resize(n);

  for (unsigned int i=0; i<_vals.size(); ++i)
  {
    //    _vars[i] = coupled("v", i);
    _vals[i] = &coupledValue("v", i);
    _vals_old[i] = &coupledValueOld("v", i);
  }
}

Real CoupledBEKinetic::computeQpResidual()
{
  Real assemble_conc = 0.0;

  for (unsigned int i=0; i<_vals.size(); ++i)
    assemble_conc +=  _weight[i]*((*_vals[i])[_qp]-(*_vals_old[i])[_qp])/_dt;

  return _porosity[_qp]*_test[_i][_qp]*assemble_conc;
}

Real CoupledBEKinetic::computeQpJacobian()
{
  return 0.0;
}

/*
Real CoupledBEKinetic::computeQpOffDiagJacobian(unsigned int jvar)
  {
    if (jvar == _v_var1)
      return _sto_v1*_test[_i][_qp]*_phi[_j][_qp]/_dt;
  }
*/
