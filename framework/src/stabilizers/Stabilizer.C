#include "Stabilizer.h"
#include "MooseSystem.h"
#include "ElementData.h"

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


Stabilizer::Stabilizer(std::string name, MooseSystem & moose_system, InputParameters parameters):
  _name(name),
  _moose_system(moose_system),
  _element_data(moose_system._element_data),
  _tid(Moose::current_thread_id),
  _parameters(parameters),
  _var_name(parameters.get<std::string>("variable")),
  _is_aux(_moose_system._aux_system->has_variable(_var_name)),
  _var_num(_is_aux ? _moose_system._aux_system->variable_number(_var_name) : _moose_system._system->variable_number(_var_name)),
  _fe_type(_is_aux ? _moose_system._aux_dof_map->variable_type(_var_num) : _moose_system._dof_map->variable_type(_var_num)),
  _current_elem(_element_data._current_elem[_tid]),
  _phi(*(_element_data._phi[_tid])[_fe_type]),
  _test((_element_data._test[_tid])[_var_num]),
  _dtest(*(_element_data._dphi[_tid])[_fe_type]),
  _qrule(_element_data._qrule[_tid])
{
}

Stabilizer::~Stabilizer()
{
}

void
Stabilizer::subdomainSetup()
{
}

unsigned int
Stabilizer::variable()
{
  return _var_num;
}

Real
Stabilizer::computeQpResidual()
{
  return 0;
}
