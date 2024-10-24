//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
                                                   std::vector<std::string> fn_names)
  : _dim(fn_names.size()),
    _tr_x(&feproblem.getFunction(fn_names[0])),
    _tr_y(_dim > 1 ? &feproblem.getFunction(fn_names[1]) : NULL),
    _tr_z(_dim > 2 ? &feproblem.getFunction(fn_names[2]) : NULL)
{

  // Make certain the the dimensions agree
  if (_dim != feproblem.mesh().dimension())
    mooseError("Transform function has to have the same dimension as the problem being solved.");

  // Initialize the functions (i.e., call thier initialSetup methods)
  init();
}

FunctionPeriodicBoundary::FunctionPeriodicBoundary(const FunctionPeriodicBoundary & o)
  : libMesh::PeriodicBoundaryBase(o), _dim(o._dim), _tr_x(o._tr_x), _tr_y(o._tr_y), _tr_z(o._tr_z)
{
  // Initialize the functions (i.e., call thier initialSetup methods)
  init();
}

Point
FunctionPeriodicBoundary::get_corresponding_pos(const Point & pt) const
{
  // Force thread-safe evaluation of what could be ParsedFunctions.
  Threads::spin_mutex::scoped_lock lock(parsed_function_mutex);

  Real t = 0.;
  Point p;
  switch (_dim)
  {
    case 1:
      return Point(_tr_x->value(t, pt));

    case 2:
      mooseAssert(_tr_y, "Must provide a function to map y in 2D.");
      return Point(_tr_x->value(t, pt), _tr_y->value(t, pt));

    case 3:
      mooseAssert(_tr_y, "Must provide a function to map y in 2D.");
      mooseAssert(_tr_z, "Must provide a function to map z in 3D.");
      return Point(_tr_x->value(t, pt), _tr_y->value(t, pt), _tr_z->value(t, pt));

    default:
      mooseError("Unsupported dimension");
      break;
  }

  return pt;
}

std::unique_ptr<libMesh::PeriodicBoundaryBase>
FunctionPeriodicBoundary::clone(TransformationType t) const
{
  if (t == INVERSE)
    mooseError("No way to automatically clone() an inverse FunctionPeriodicBoundary object");

  return std::make_unique<FunctionPeriodicBoundary>(*this);
}

void
FunctionPeriodicBoundary::init()
{
  switch (_dim)
  {
    case 1:
      const_cast<Function *>(_tr_x)->initialSetup();
      break;
    case 2:
      const_cast<Function *>(_tr_x)->initialSetup();
      const_cast<Function *>(_tr_y)->initialSetup();
      break;
    case 3:
      const_cast<Function *>(_tr_x)->initialSetup();
      const_cast<Function *>(_tr_y)->initialSetup();
      const_cast<Function *>(_tr_z)->initialSetup();
      break;
    default:
      mooseError("Unsupported dimension");
      break;
  }
}
