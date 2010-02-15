#include "Stabilizer.h"

Stabilizer::Stabilizer(std::string name,
                       InputParameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as)
  :Kernel(name, parameters, var_name, false, coupled_to, coupled_as)
{}

  
