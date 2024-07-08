//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MOOSEVariableToNEML2.h"
#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", MOOSEVariableToNEML2);
registerMooseObject("SolidMechanicsApp", MOOSEOldVariableToNEML2);

#ifndef NEML2_ENABLED
#define MOOSEMVariableToNEML2Stub(name)                                                            \
  NEML2ObjectStubImplementationOpen(name, MOOSEToNEML2);                                           \
  NEML2ObjectStubParam(std::vector<VariableName>, "moose_variable");                               \
  NEML2ObjectStubParam(std::string, "neml2_variable");                                             \
  NEML2ObjectStubImplementationClose(name, MOOSEToNEML2)
MOOSEMVariableToNEML2Stub(MOOSEVariableToNEML2);
MOOSEMVariableToNEML2Stub(MOOSEOldVariableToNEML2);
#else

template <unsigned int state>
InputParameters
MOOSEVariableToNEML2Templ<state>::validParams()
{
  auto params = MOOSEToNEML2::validParams();
  params.addClassDescription("Gather a MOOSE variable for insertion into the specified input of a "
                             "NEML2 model.");

  params.addRequiredCoupledVar("moose_variable", "MOOSE variable to read from");
  return params;
}

template <>
MOOSEVariableToNEML2Templ<0>::MOOSEVariableToNEML2Templ(const InputParameters & params)
  : MOOSEToNEML2(params), _moose_variable(coupledValue("moose_variable"))
{
  if (!_neml2_variable.start_with("forces") && !_neml2_variable.start_with("state"))
    paramError("neml2_variable",
               "neml2_variable should be defined on the forces or the state sub-axis, got ",
               _neml2_variable.slice(0, 1),
               " instead");
}

template <>
MOOSEVariableToNEML2Templ<1>::MOOSEVariableToNEML2Templ(const InputParameters & params)
  : MOOSEToNEML2(params), _moose_variable(coupledValueOld("moose_variable"))
{
  if (!_neml2_variable.start_with("old_forces") && !_neml2_variable.start_with("old_state"))
    paramError("neml2_variable",
               "neml2_variable should be defined on the old_forces or the old_state sub-axis, got ",
               _neml2_variable.slice(0, 1),
               " instead");
}

template <unsigned int state>
torch::Tensor
MOOSEVariableToNEML2Templ<state>::convertQpMOOSEData() const
{
  return NEML2Utils::toNEML2<Real>(_moose_variable[_qp]);
}

template class MOOSEVariableToNEML2Templ<0>;
template class MOOSEVariableToNEML2Templ<1>;

#endif
