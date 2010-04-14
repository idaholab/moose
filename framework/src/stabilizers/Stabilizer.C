#include "Stabilizer.h"

#include <vector>

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

  
