//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

class ConsoleStream;
namespace libMesh
{
class MeshBase;
}

namespace MeshBaseDiagnosticsUtils
{
void checkNonConformalMesh(const std::unique_ptr<libMesh::MeshBase> & mesh,
                           const ConsoleStream & console,
                           const unsigned int num_outputs,
                           const Real conformality_tol,
                           unsigned int & num_nonconformal_nodes);

bool checkFirstOrderEdgeOverlap(const Elem & edge1,
                                const Elem & edge2,
                                Point & intersection_point,
                                const Real intersection_tol);
}
