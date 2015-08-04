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

template<>
InputParameters validParams<PoroMechanicsAction>()
{
  InputParameters params = validParams<TensorMechanicsAction>();
  params.addRequiredParam<NonlinearVariableName>("porepressure", "The porepressure variable");
  return params;
}

PoroMechanicsAction::PoroMechanicsAction(const InputParameters & params) :
    TensorMechanicsAction(params)
{
}

void
PoroMechanicsAction::act()
{
  TensorMechanicsAction::act();

  //Prepare displacements and set value for dim
  unsigned int dim = 1;
  std::vector<VariableName> vars;
  vars.push_back(getParam<NonlinearVariableName>("disp_x"));
  if (isParamValid("disp_y"))
  {
    ++dim;
    vars.push_back(getParam<NonlinearVariableName>("disp_y"));
    if (isParamValid("disp_z"))
    {
      ++dim;
      vars.push_back(getParam<NonlinearVariableName>("disp_z"));
    }
  }


  // all the kernels added below have porepressure as a coupled variable
  // add this to the kernel's params
  std::string type("PoroMechanicsCoupling");
  InputParameters params = _factory.getValidParams(type);
  VariableName pp_var(getParam<NonlinearVariableName>("porepressure"));
  params.addCoupledVar("porepressure", "");
  params.set<std::vector<VariableName> >("porepressure") = std::vector<VariableName>(1, pp_var);


  // now add the kernels
  std::string short_name = "PoroMechanics";
  for (unsigned int i = 0; i < dim; ++i)
  {
    std::stringstream name;
    name << short_name;
    name << i;

    params.set<unsigned int>("component") = i;
    params.set<NonlinearVariableName>("variable") = vars[i];

    _problem->addKernel(type, name.str(), params);
  }
}

