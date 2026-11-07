//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestPetscObjectTypes.h"

#include "SystemBase.h"
#include "AuxiliarySystem.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

registerMooseObject("MooseTestApp", TestPetscObjectTypes);

InputParameters
TestPetscObjectTypes::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Errors if the PETSc vectors or matrices of a system do not have the "
                             "expected PETSc types.");
  params.addParam<std::string>(
      "system", "nl0", "The system to check. The auxiliary system checks under 'aux0'.");
  params.addRequiredParam<std::string>(
      "vec_type", "The expected PETSc type root of the non-ghosted vectors, e.g. 'standard'");
  params.addRequiredParam<std::string>(
      "ghost_vec_type", "The expected PETSc type root of the ghosted vectors, e.g. 'standard'");
  params.addParam<std::string>("mat_type",
                               "The expected PETSc type root of the matrices, e.g. 'aij'");
  return params;
}

TestPetscObjectTypes::TestPetscObjectTypes(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _test_sys(getParam<std::string>("system") == _fe_problem.getAuxiliarySystem().name()
                  ? static_cast<const SystemBase &>(_fe_problem.getAuxiliarySystem())
                  : static_cast<const SystemBase &>(_fe_problem.getSolverSystem(
                        _fe_problem.solverSysNum(getParam<std::string>("system")))))
{
}

namespace
{
/// Whether a PETSc object's type is \p root with or without its "seq" or "mpi" variant prefix
bool
hasTypeRoot(PetscObject obj, const std::string & root)
{
  // The PETSc name of the default host vector type is "standard", with variants "seq" and "mpi"
  const std::vector<std::string> names =
      root == "standard" ? std::vector<std::string>{"seq", "mpi", "standard"}
                         : std::vector<std::string>{root, "seq" + root, "mpi" + root};
  for (const auto & name : names)
  {
    PetscBool match;
    LibmeshPetscCallA(PETSC_COMM_SELF, PetscObjectTypeCompare(obj, name.c_str(), &match));
    if (match)
      return true;
  }
  return false;
}

std::string
typeName(PetscObject obj)
{
  const char * type;
  LibmeshPetscCallA(PETSC_COMM_SELF, PetscObjectGetType(obj, &type));
  return type ? type : "";
}
}

void
TestPetscObjectTypes::execute()
{
  const auto & lm_sys = _test_sys.system();

  auto check_vec = [this](const std::string & vec_name, const NumericVector<Number> & vec)
  {
    const auto & expected = getParam<std::string>(
        vec.type() == libMesh::GHOSTED && n_processors() > 1 ? "ghost_vec_type" : "vec_type");
    const auto obj = (PetscObject)cast_ref<const PetscVector<Number> &>(vec).vec();
    if (!hasTypeRoot(obj, expected))
      mooseError("Vector '",
                 vec_name,
                 "' has PETSc type '",
                 typeName(obj),
                 "', expected '",
                 expected,
                 "'");
  };

  check_vec("solution", *lm_sys.solution);
  check_vec("current_local_solution", *lm_sys.current_local_solution);
  for (const auto & [name, vec] : as_range(lm_sys.vectors_begin(), lm_sys.vectors_end()))
    check_vec(name, *vec);

  if (!isParamValid("mat_type"))
    return;
  const auto & expected = getParam<std::string>("mat_type");
  for (const auto & [name, mat] : as_range(lm_sys.matrices_begin(), lm_sys.matrices_end()))
    if (const auto * const petsc_mat = dynamic_cast<const PetscMatrix<Number> *>(mat.get()))
    {
      const auto obj = (PetscObject) const_cast<PetscMatrix<Number> *>(petsc_mat)->mat();
      if (!hasTypeRoot(obj, expected))
        mooseError(
            "Matrix '", name, "' has PETSc type '", typeName(obj), "', expected '", expected, "'");
    }
}
