//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "FEProblemBase.h"
#include "MappingBase.h"

class MappingInterface
{
public:
  static InputParameters validParams();

  MappingInterface(const MooseObject * moose_object);

  MappingBase & getMapping(const std::string & name) const;

  MappingBase & getMappingByName(const UserObjectName & name) const;

  ///@}
private:
  /// Parameters of the object with this interface
  const InputParameters & _smi_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _smi_feproblem;

  /// Thread ID
  const THREAD_ID _smi_tid;
};
