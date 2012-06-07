#include "SolidMechanicsAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"

template<>
InputParameters validParams<SolidMechanicsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("disp_x", "", "The x displacement");
  params.addParam<std::string>("disp_y", "", "The y displacement");
  params.addParam<std::string>("disp_z", "", "The z displacement");
  params.addParam<std::string>("disp_r", "", "The r displacement");
  params.addParam<std::string>("temp", "", "The temperature");
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

SolidMechanicsAction::SolidMechanicsAction(const std::string & name, InputParameters params) :
  Action(name, params),
  _disp_x(getParam<std::string>("disp_x")),
  _disp_y(getParam<std::string>("disp_y")),
  _disp_z(getParam<std::string>("disp_z")),
  _disp_r(getParam<std::string>("disp_r")),
  _temp(getParam<std::string>("temp"))
{
}

void
SolidMechanicsAction::act()
{
  // Determine whether RZ
  bool rz(false);
  unsigned int dim(1);
  std::vector<std::string> keys;
  std::vector<std::string> vars;
  std::string type("StressDivergence");
  if (_problem->coordSystem() == Moose::COORD_RZ)
  {
    rz = true;
    dim = 2;
    keys.push_back("disp_r");
    keys.push_back("disp_z");
    vars.push_back(_disp_r);
    vars.push_back(_disp_z);
    type = "StressDivergenceRZ";
  }

  if (!rz && _disp_x == "")
  {
    mooseError("disp_x must be specified");
  }

  if (!rz)
  {
    keys.push_back("disp_x");
    vars.push_back(_disp_x);
    if ( _disp_y != "" )
    {
      ++dim;
      keys.push_back("disp_y");
      vars.push_back(_disp_y);
      if ( _disp_z != "" )
      {
        ++dim;
        keys.push_back("disp_z");
        vars.push_back(_disp_z);
      }
    }
  }

  unsigned int num_coupled(dim);
  if (_temp != "")
  {
    ++num_coupled;
    keys.push_back("temp");
    vars.push_back(_temp);
  }

  // Create divergence objects
  std::string short_name(_name);
  // Chop off "SolidMechanics/"
  short_name.erase(0, 15);

  InputParameters params = Factory::instance()->getValidParams(type);
  for (unsigned j(0); j < num_coupled; ++j)
  {
    params.addCoupledVar(keys[j], "");
    params.set<std::vector<std::string> >(keys[j]) = std::vector<std::string>(1, vars[j]);
  }

  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
  params.set<std::string>("appended_property_name") = getParam<std::string>("appended_property_name");

  for (unsigned int i(0); i < dim; ++i)
  {
    std::stringstream name;
    name << "Kernels/";
    name << short_name;
    name << i;

    params.set<unsigned int>("component") = i;
    params.set<std::string>("variable") = vars[i];

    _problem->addKernel(type, name.str(), params);
  }

}
