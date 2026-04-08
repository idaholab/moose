//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "DataIO.h"
#include "MFEMProblemData.h"

namespace
{
void
storeGridFunction(std::ostream & stream, mfem::ParGridFunction & gridfunction, void * context)
{
  const auto size = gridfunction.Size();
  dataStore(stream, size, context);

  const auto * values = gridfunction.HostRead();
  for (int i = 0; i < size; ++i)
  {
    auto value = values[i];
    dataStore(stream, value, context);
  }
}

void
loadGridFunction(std::istream & stream, mfem::ParGridFunction & gridfunction, void * context)
{
  int size = 0;
  dataLoad(stream, size, context);
  mooseAssert(size == gridfunction.Size(),
              "MFEM restartable GridFunction size mismatch during restore.");

  auto * values = gridfunction.HostWrite();
  for (int i = 0; i < size; ++i)
    dataLoad(stream, values[i], context);
}
}

template <>
void
dataStore(std::ostream & stream, Moose::MFEM::SolutionState & /*state*/, void * context)
{
  auto * const data = static_cast<MFEMProblemData *>(context);
  mooseAssert(data, "Missing MFEMProblemData context for solution restart storage.");

  auto num_gridfunctions = data->gridfunctions.size();
  dataStore(stream, num_gridfunctions, context);
  for (const auto & [name, gridfunction] : data->gridfunctions)
  {
    auto stored_name = name;
    dataStore(stream, stored_name, context);
    storeGridFunction(stream, *gridfunction, context);
  }

  auto num_complex_gridfunctions = data->cmplx_gridfunctions.size();
  dataStore(stream, num_complex_gridfunctions, context);
  for (const auto & [name, gridfunction] : data->cmplx_gridfunctions)
  {
    auto stored_name = name;
    dataStore(stream, stored_name, context);
    storeGridFunction(stream, gridfunction->real(), context);
    storeGridFunction(stream, gridfunction->imag(), context);
  }
}

template <>
void
dataLoad(std::istream & stream, Moose::MFEM::SolutionState & /*state*/, void * context)
{
  auto * const data = static_cast<MFEMProblemData *>(context);
  mooseAssert(data, "Missing MFEMProblemData context for solution restart load.");

  int num_gridfunctions = 0;
  dataLoad(stream, num_gridfunctions, context);
  for (int i = 0; i < num_gridfunctions; ++i)
  {
    std::string name;
    dataLoad(stream, name, context);
    auto & gf = data->gridfunctions.GetRef(name);
    loadGridFunction(stream, gf, context);
  }

  int num_complex_gridfunctions = 0;
  dataLoad(stream, num_complex_gridfunctions, context);
  for (int i = 0; i < num_complex_gridfunctions; ++i)
  {
    std::string name;
    dataLoad(stream, name, context);
    auto & gf = data->cmplx_gridfunctions.GetRef(name);
    loadGridFunction(stream, gf.real(), context);
    loadGridFunction(stream, gf.imag(), context);
  }
}

#endif
