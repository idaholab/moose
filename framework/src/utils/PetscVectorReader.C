//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PetscVectorReader.h"

PetscVectorReader::PetscVectorReader(PetscVector<Number> & vec)
  : _vec(vec), _raw_value(vec.get_array_read())
{
}

PetscVectorReader::PetscVectorReader(NumericVector<Number> & vec)
  : _vec(libMesh::cast_ref<PetscVector<Number> &>(vec)), _raw_value(_vec.get_array_read())
{
}

PetscVectorReader::~PetscVectorReader() { restore(); }

void
PetscVectorReader::restore()
{
  mooseAssert(readable(), "Array not retrieved");
  _vec.restore_array();
  _raw_value = nullptr;
}
