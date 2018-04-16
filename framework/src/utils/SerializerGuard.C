//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SerializerGuard.h"

#include "MooseUtils.h"

#include "libmesh/parallel.h"

SerializerGuard::SerializerGuard(const libMesh::Parallel::Communicator & comm, bool warn)
  : _comm(comm), _warn(warn)
{
  MooseUtils::serialBegin(_comm, _warn);
}

SerializerGuard::~SerializerGuard() { MooseUtils::serialEnd(_comm, _warn); }
