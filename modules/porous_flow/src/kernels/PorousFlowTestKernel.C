/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowTestKernel.h"

template<>
InputParameters validParams<PorousFlowTestKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Test kernel");
  return params;
}

PorousFlowTestKernel::PorousFlowTestKernel(const InputParameters & parameters) :
  DerivativeMaterialInterface<Kernel>(parameters),
  _relative_permeability(getMaterialProperty<Real>("PorousFlow_relative_permeability0"))
{
  // Assume there are three variables (pressure, saturation and mass_fraction)
  // Create a vector of dummy variable names
  // Note: these are the dummy names used in the definitiion of the derivatives
  std::vector<VariableName> varnames(3);
  varnames[0] = "pressure_varname";
  varnames[1] = "saturation_varname";
  varnames[2] = "mass_fraction_varname";

  _drelative_permeability_dvar.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
  {
    _drelative_permeability_dvar[i] = & getMaterialPropertyDerivative<Real>("PorousFlow_relative_permeability0", varnames[i]);
  }
}

Real PorousFlowTestKernel::computeQpResidual()
{
  // Print out the relative perm and derivatives for checking
  _console << "_qp " << _qp << ", relperm = " << _relative_permeability[_qp] << std::endl;
  _console << "_qp " << _qp << ", d(relperm)/d(pressure) " << (*_drelative_permeability_dvar[0])[_qp] << std::endl;
  _console << "_qp " << _qp << ", d(relperm)/d(saturation) " << (*_drelative_permeability_dvar[1])[_qp] << std::endl;
  _console << "_qp " << _qp << ", d(relperm)/d(mass_fraction) " << (*_drelative_permeability_dvar[2])[_qp] << std::endl;

  return 0.0;
}
