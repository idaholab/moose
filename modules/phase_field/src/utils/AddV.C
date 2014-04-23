#include "AddV.h"

InputParameters &
AddV(InputParameters & parameters, const std::string & var_name)
{
  unsigned int crys_num = parameters.get<unsigned int>("crys_num");
  std::string var_name_base = parameters.get<std::string>("var_name_base");

  //Create variable names
  std::vector<VariableName> v;
  v.resize(crys_num);

  if (crys_num > 0)
  {
    for (unsigned int crys = 0; crys < crys_num; crys++)
    {
      std::string coupled_var_name = var_name_base;
      std::stringstream out;
      out << crys;
      coupled_var_name.append(out.str());
      v[crys] = coupled_var_name;
    }

    parameters.remove(var_name);
    parameters.set<std::vector<VariableName> >(var_name) = v;
  }

  return parameters;
}
