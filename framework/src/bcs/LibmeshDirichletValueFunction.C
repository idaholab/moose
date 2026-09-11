//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibmeshDirichletValueFunction.h"

#include "libmesh/dense_vector.h"

LibmeshDirichletValueFunction::LibmeshDirichletValueFunction(const LibmeshDirichletBCBase & bc)
  : _bc(bc)
{
  this->_initialized = true;
}

std::unique_ptr<libMesh::FunctionBase<Number>>
LibmeshDirichletValueFunction::clone() const
{
  return std::make_unique<LibmeshDirichletValueFunction>(_bc);
}

Number
LibmeshDirichletValueFunction::operator()(const libMesh::Point & p, const Real time)
{
  return _bc.value(p, time);
}

void
LibmeshDirichletValueFunction::operator()(const libMesh::Point & p,
                                          const Real time,
                                          DenseVector<Number> & output)
{
  for (const auto i : make_range(output.size()))
    output(i) = _bc.value(p, time);
}

Number
LibmeshDirichletValueFunction::component(unsigned int, const libMesh::Point & p, const Real time)
{
  return _bc.value(p, time);
}
