//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDirichletValueFunction.h"

namespace Moose::Kokkos
{

DirichletValueFunction::DirichletValueFunction(const NodalBCBase & bc) : _bc(bc)
{
  this->_initialized = true;
}

std::unique_ptr<libMesh::FunctionBase<Number>>
DirichletValueFunction::clone() const
{
  return std::make_unique<DirichletValueFunction>(_bc);
}

Number
DirichletValueFunction::operator()(const libMesh::Point & p, const Real time)
{
  return _bc.hostValue(p, time);
}

void
DirichletValueFunction::operator()(const libMesh::Point & p,
                                   const Real time,
                                   DenseVector<Number> & output)
{
  output(0) = _bc.hostValue(p, time);
}

} // namespace Moose::Kokkos
