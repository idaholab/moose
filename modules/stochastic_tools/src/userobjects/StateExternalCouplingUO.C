/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateExternalCouplingUO.h"

template <>
InputParameters
validParams<StateExternalCouplingUO>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<std::vector<std::string>>("eval_names", "Lookup names for the item values");
  params.addParam<std::vector<Real>>("init_values", "item inital values - optional");
  return params;
}

StateExternalCouplingUO::StateExternalCouplingUO(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _eval_names(parameters.get<std::vector<std::string>>("eval_names")),
    _values(NULL),
    _no_dflt_values()
{
  _values = &_no_dflt_values;
  if (parameters.isParamValid("init_values")) //_values == NULL) //todo see if initial values were passed in
  {
    const std::vector<Real> & init_values = parameters.get<std::vector<Real>>("init_values");
    for (unsigned long i = 0; i < _eval_names.size(); ++i)
    {
      if (init_values.size() > i)
      {
        _values->push_back(init_values[i]);
      }
      else
      {
        _values->push_back(0.0);
      }
    }
  }
  {
    for (unsigned long i = 0; i < _eval_names.size(); ++i)
    {
      _values->push_back(0.0);
    }
  }
  //TODO if not enough values for the names add them.
}

void
StateExternalCouplingUO::execute()
{
  //for testing they dynamic setting of a coupled variable.
  if ((_t_step % 40) == 0)
  {
    unsigned int idx = (unsigned int)(_t_step / 40) - 1;
    if (idx < _values->size())
    {
      (*_values)[idx] = 1.0;
    }
  }
}
