/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PoroMechanicsAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "Conversion.h"

template <>
InputParameters
validParams<PoroMechanicsAction>()
{
  InputParameters params = validParams<TensorMechanicsAction>();
  params.addRequiredParam<NonlinearVariableName>("porepressure", "The porepressure variable");
  return params;
}

PoroMechanicsAction::PoroMechanicsAction(const InputParameters & params)
  : TensorMechanicsAction(params)
{
}

void
PoroMechanicsAction::act()
{
  TensorMechanicsAction::act();

  if (_current_task == "add_kernel")
  {
    // Prepare displacements and set value for dim
    std::vector<NonlinearVariableName> displacements =
        getParam<std::vector<NonlinearVariableName>>("displacements");
    unsigned int dim = displacements.size();

    // all the kernels added below have porepressure as a coupled variable
    // add this to the kernel's params
    std::string type("PoroMechanicsCoupling");
    InputParameters params = _factory.getValidParams(type);
    VariableName pp_var(getParam<NonlinearVariableName>("porepressure"));
    params.addCoupledVar("porepressure", "");
    params.set<std::vector<VariableName>>("porepressure") = {pp_var};

    // now add the kernels
    for (unsigned int i = 0; i < dim; ++i)
    {
      std::string kernel_name = "PoroMechanics" + Moose::stringify(i);

      params.set<unsigned int>("component") = i;
      params.set<NonlinearVariableName>("variable") = displacements[i];

      _problem->addKernel(type, kernel_name, params);
    }
  }
}
