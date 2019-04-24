//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxiliarySystem.h"
#include "PeridynamicsMesh.h"
#include "MooseVariableFEBase.h"
#include "MooseRandom.h"
#include "RandomizeCriticalValuePD.h"

registerMooseObject("PeridynamicsApp", RandomizeCriticalValuePD);

template <>
InputParameters
validParams<RandomizeCriticalValuePD>()
{
  InputParameters params = validParams<GeneralUserObjectBasePD>();
  params.addClassDescription(
      "Class for generating randomized value for elemental critical AuxVariable");

  params.addRequiredParam<std::string>("critical_variable", "Name of critical AuxVariable");
  params.addRequiredParam<Real>("mean",
                                "Reference value for bond critical stretch value to be randomized");
  params.addParam<Real>(
      "standard_deviation", 0.0, "Standard deviation for the normal distribution");
  params.addParam<unsigned int>("seed", 0, "Seed value for the random number generator");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  return params;
}

RandomizeCriticalValuePD::RandomizeCriticalValuePD(const InputParameters & parameters)
  : GeneralUserObjectBasePD(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _aux_sln(_aux.solution()),
    _mean(getParam<Real>("mean")),
    _standard_deviation(getParam<Real>("standard_deviation")),
    _critical_var(&_subproblem.getVariable(_tid, getParam<std::string>("critical_variable")))
{
  MooseRandom::seed(getParam<unsigned int>("seed"));
}

void
RandomizeCriticalValuePD::execute()
{
  for (unsigned int i = 0; i < _pdmesh.nElem(); ++i)
  {
    Elem * elem_i = _mesh.elemPtr(i);
    dof_id_type elem_i_dof = elem_i->dof_number(_aux.number(), _critical_var->number(), 0);
    Real rand_num = MooseRandom::randNormal(_mean, _standard_deviation);
    if (elem_i->processor_id() == processor_id())
      _aux_sln.set(elem_i_dof, std::abs(rand_num));
  }
}

void
RandomizeCriticalValuePD::finalize()
{
  _aux_sln.close();
}
