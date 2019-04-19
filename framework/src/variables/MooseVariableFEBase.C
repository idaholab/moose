//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFE.h"

MooseVariableFEBase::MooseVariableFEBase(unsigned int var_num,
                                         const FEType & fe_type,
                                         SystemBase & sys,
                                         Moose::VarKindType var_kind,
                                         THREAD_ID tid,
                                         unsigned int count)
  : MooseVariableBase(var_num, fe_type, sys, var_kind, tid, count)
{
}
