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


MooseVariableBase::MooseVariableBase(unsigned int var_num, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind) :
    _var_num(var_num),
    _var_kind(var_kind),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _variable(sys.system().variable(_var_num)),
    _assembly(assembly),
    _dof_map(sys.dofMap()),
    _scaling_factor(1.0),
    _is_nl(var_kind == Moose::VAR_NONLINEAR)
{
}

MooseVariableBase::~MooseVariableBase()
{
}

unsigned int
MooseVariableBase::index() const
{
  return _var_num;
}

unsigned int
MooseVariableBase::number() const
{
  return _var_num;
}

const std::string &
MooseVariableBase::name() const
{
  return _sys.system().variable(_var_num).name();
}

Moose::VarKindType
MooseVariableBase::kind() const
{
  return _var_kind;
}

void
MooseVariableBase::scalingFactor(Real factor)
{
  _scaling_factor = factor;
}

Real
MooseVariableBase::scalingFactor() const
{
  return _scaling_factor;
}

unsigned int
MooseVariableBase::order() const
{
  return static_cast<unsigned int>(_sys.system().variable_type(_var_num).order);
}

