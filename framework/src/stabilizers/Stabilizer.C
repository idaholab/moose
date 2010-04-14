#include "Stabilizer.h"

#include <vector>

template<>
InputParameters validParams<Stabilizer>()
{
  InputParameters params;
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  params.addParam<std::vector<std::string> >("coupled_to", "The list of variable names this Stabilizer is coupled to.");
  params.addParam<std::vector<std::string> >("coupled_as", "The list of variable names as referenced inside of this Stabilizer which correspond with the coupled_as names");
  return params;
}


Stabilizer::Stabilizer(std::string name,
                       MooseSystem & moose_system,
                       InputParameters parameters)
  :Kernel(name,
          parameters,
          parameters.have_parameter<std::string>("var_name") ? parameters.get<std::string>("var_name") : "",
          false,
          parameters.have_parameter<std::vector<std::string> >("coupled_to") ? parameters.get<std::vector<std::string> >("coupled_to") : std::vector<std::string>(0),
          parameters.have_parameter<std::vector<std::string> >("coupled_as") ? parameters.get<std::vector<std::string> >("coupled_as") : std::vector<std::string>(0))
{}

  
