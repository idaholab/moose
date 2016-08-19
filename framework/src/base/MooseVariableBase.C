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

#include "MooseVariableBase.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "SystemBase.h"

// libMesh includes
#include "libmesh/variable.h"
#include "libmesh/dof_map.h"


MooseVariableBase::MooseVariableBase(const std::string & name, unsigned int var_num, const FEType & fe_type, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind, unsigned int count) :
    _name(name),
    _var_num(var_num),
    _fe_type(fe_type),
    _var_kind(var_kind),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _variable(sys.system().variable(_var_num)),
    _assembly(assembly),
    _dof_map(sys.dofMap()),
    _elem(_assembly.elem()),
    _scaling_factor(1.0),
    _count(count)
{
}

MooseVariableBase::~MooseVariableBase()
{
}

const std::string &
MooseVariableBase::name() const
{
  return _name;
}

const std::vector<std::string>
MooseVariableBase::names() const
{
  if (count() == 1)
    return std::vector<std::string>{_name};

  // For ArrayVariables
  std::vector<std::string> the_names(count());

  for (auto i = decltype(count())(0); i < count(); i++)
    the_names[i] = _name + "_" + std::to_string(i);

  return the_names;
}


Order
MooseVariableBase::order() const
{
  return _fe_type.order;
}
