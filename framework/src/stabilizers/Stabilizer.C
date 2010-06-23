#include "Stabilizer.h"
#include "MooseSystem.h"
#include "ElementData.h"

#include <vector>

template<>
InputParameters validParams<Stabilizer>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  return params;
}


Stabilizer::Stabilizer(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  PDEBase(name, moose_system, parameters, moose_system._element_data),
  _element_data(moose_system._element_data),
  _test((_element_data._test[_tid])[_var_num]),
  _grad_test(*(_element_data._grad_phi[_tid])[_fe_type])
{
}

Stabilizer::~Stabilizer()
{
}

Real
Stabilizer::computeQpResidual()
{
  return 0;
}
