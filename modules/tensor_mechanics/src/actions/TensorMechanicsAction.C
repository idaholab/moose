#include "TensorMechanicsAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<TensorMechanicsAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<NonlinearVariableName>("disp_x", "The x displacement");
  params.addParam<NonlinearVariableName>("disp_y", "The y displacement");
  params.addParam<NonlinearVariableName>("disp_z", "The z displacement");
  params.addParam<NonlinearVariableName>("temp", "The temperature");
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addParam<bool>("use_displaced_mesh", false, "Whether to use displaced mesh in the kernels");

  return params;
}

TensorMechanicsAction::TensorMechanicsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
TensorMechanicsAction::act()
{
  unsigned int dim = 1;
  std::vector<std::string> keys;
  std::vector<VariableName> vars;
  std::string type("StressDivergenceTensors");

  //Prepare displacements and set value for dim
  keys.push_back("disp_x");
  vars.push_back(getParam<NonlinearVariableName>("disp_x"));

  if (isParamValid("disp_y"))
  {
    ++dim;
    keys.push_back("disp_y");
    vars.push_back(getParam<NonlinearVariableName>("disp_y"));
    if (isParamValid("disp_z"))
    {
      ++dim;
      keys.push_back("disp_z");
      vars.push_back(getParam<NonlinearVariableName>("disp_z"));
    }
  }

  //Add in the temperature
  unsigned int num_coupled(dim);
  if (isParamValid("temp"))
  {
    ++num_coupled;
    keys.push_back("temp");
    vars.push_back(getParam<NonlinearVariableName>("temp"));
  }

  InputParameters params = _factory.getValidParams(type);
  for (unsigned int j = 0; j < num_coupled; ++j)
  {
    params.addCoupledVar(keys[j], "");
    params.set<std::vector<VariableName> >(keys[j]) = std::vector<VariableName>(1, vars[j]);
  }

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
  params.set<std::string>("appended_property_name") = getParam<std::string>("appended_property_name");

  std::string short_name = "TensorMechanics";

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
