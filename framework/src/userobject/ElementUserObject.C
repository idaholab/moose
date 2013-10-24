/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ElementUserObject.h"
#include "MooseVariable.h"
#include "SubProblem.h"

template<>
InputParameters validParams<ElementUserObject>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<BlockRestrictable>();
  params += validParams<RandomInterface>();

  return params;
}

ElementUserObject::ElementUserObject(const std::string & name, InputParameters parameters) :
    UserObject(name, parameters),
    BlockRestrictable(name, parameters),
    MaterialPropertyInterface(parameters),
    UserObjectInterface(parameters),
    Coupleable(parameters, false),
    MooseVariableDependencyInterface(),
    TransientInterface(parameters, name, "element_user_objects"),
    PostprocessorInterface(parameters),
    RandomInterface(name, parameters, _fe_problem, _tid, false),
    ZeroInterface(parameters),
    _mesh(_subproblem.mesh()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation())
{
  // Keep track of which variables are coupled so we know what we depend on
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for(unsigned int i=0; i<coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}
