/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddV.h"

InputParameters &
AddV(InputParameters & parameters, const std::string & var_name)
{
  unsigned int op_num = parameters.get<unsigned int>("op_num");

  //Create variable names
  std::vector<VariableName> v;
  v.resize(op_num);

  if (op_num > 0)
  {
    if (!parameters.isParamValid("var_name_base"))
      mooseError("Must specify the var_name_base parameter if op_num > 0.");

    std::string var_name_base = parameters.get<std::string>("var_name_base");

    for (unsigned int op = 0; op < op_num; op++)
    {
      std::string coupled_var_name = var_name_base;
      std::stringstream out;
      out << op;
      coupled_var_name.append(out.str());
      v[op] = coupled_var_name;
    }

    parameters.remove(var_name);
    parameters.set<std::vector<VariableName> >(var_name) = v;
  }

  return parameters;
}
