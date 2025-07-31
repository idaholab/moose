//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "FunctionPeriodicBoundary.h"
#include "FEProblem.h"
#include "Function.h"
#include "MooseMesh.h"

// A mutex we can acquire to prevent simultaneous ParsedFunction
// evaluation on multiple threads.  ParsedFunction evaluation is
// currently not thread-safe.
Threads::spin_mutex parsed_function_mutex;

FunctionPeriodicBoundary::FunctionPeriodicBoundary(FEProblemBase & feproblem,
                                                   const std::vector<std::string> & fn_names,
                                                   const std::vector<std::string> & inv_fn_names)
  : libMesh::PeriodicBoundaryBase(),
    _dim(fn_names.size()),
    _tr(getFunctions(feproblem, fn_names)),
    _inv_tr(getFunctions(feproblem, inv_fn_names))
{
  mooseAssert(fn_names.size() == inv_fn_names.size(), "Size mismatch");

  // Make certain the the dimensions agree
  if (_dim != feproblem.mesh().dimension())
    mooseError("Transform function has to have the same dimension as the problem being solved.");
}

FunctionPeriodicBoundary::FunctionPeriodicBoundary(const FunctionPeriodicBoundary & o,
                                                   TransformationType t /* = FORWARD */)
  : libMesh::PeriodicBoundaryBase(o),
    _dim(o._dim),
    _tr(t == INVERSE ? o._inv_tr : o._tr),
    _inv_tr(t == INVERSE ? o._tr : o._inv_tr)
{
  if (t == INVERSE)
    std::swap(myboundary, pairedboundary);
}

Point
FunctionPeriodicBoundary::get_corresponding_pos(const Point & pt) const
{
  // Force thread-safe evaluation of what could be ParsedFunctions.
  Threads::spin_mutex::scoped_lock lock(parsed_function_mutex);

  Point p;
  for (const auto i : make_range(_dim))
  {
    mooseAssert(_tr[i], "Function not provided");
    p(i) = _tr[i]->value(0.0, pt);
  }
  return p;
}

std::unique_ptr<libMesh::PeriodicBoundaryBase>
FunctionPeriodicBoundary::clone(TransformationType t) const
{
  return std::make_unique<FunctionPeriodicBoundary>(*this, t);
}

std::array<const Function *, 3>
FunctionPeriodicBoundary::getFunctions(FEProblemBase & problem,
                                       const std::vector<std::string> & names)
{
  std::array<const Function *, 3> functions;
  for (const auto i : index_range(functions))
    if (names.size() > i)
    {
      functions[i] = &problem.getFunction(names[i]);
      const_cast<Function *>(functions[i])->initialSetup();
    }
  return functions;
}
