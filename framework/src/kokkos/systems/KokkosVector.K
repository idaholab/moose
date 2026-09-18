//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVector.h"
#include "KokkosDofSpace.h"

namespace Moose::Kokkos
{

void
Vector::create(libMesh::NumericVector<PetscScalar> & vector,
               const DofSpace & dof_space,
               bool assemble,
               bool read_only)
{
  auto petsc_vector = dynamic_cast<libMesh::PetscVector<PetscScalar> *>(&vector);

  mooseAssert(petsc_vector, "Kokkos vector error: provided vector is not a PetscVector.");

  if (read_only && assemble)
    mooseError("Kokkos vector error: an assembled vector cannot be read-only.");

  PetscScalar * array = PETSC_NULLPTR;
  const PetscScalar * read_array = PETSC_NULLPTR;
  PetscMemType mtype;
  LibmeshPetscCallQ(VecGhostGetLocalForm(petsc_vector->vec(), &_local_vector));

  const auto local_form = _local_vector ? _local_vector : petsc_vector->vec();

  // The read-only accessor leaves a write lock the caller holds on the vector intact, which is what
  // lets an operator application read its input where it already is
  if (read_only)
    LibmeshPetscCallQ(VecGetArrayReadAndMemType(local_form, &read_array, &mtype));
  else
    LibmeshPetscCallQ(VecGetArrayAndMemType(local_form, &array, &mtype));

  auto is_host = mtype == PETSC_MEMTYPE_HOST;
  auto is_ghosted = vector.type() == libMesh::ParallelType::GHOSTED;
  bool realloc = !_is_alloc || _assemble != assemble || _is_host != is_host ||
                 _is_ghosted != is_ghosted || _read_only != read_only;

#ifndef MOOSE_ENABLE_KOKKOS_GPU
  if (!is_host)
    mooseError("PETSc vectors must be on host when Kokkos device capabilities are disabled.");
#endif

  _global_vector = petsc_vector->vec();
  _array = array;
  _read_array = read_array;
  _dof_space = &dof_space;
  _assemble = assemble;
  _is_host = is_host;
  _is_ghosted = is_ghosted;
  _read_only = read_only;

  if (realloc)
  {
    _local.destroy();
    _ghost.destroy();
    _local_read.destroy();
  }

  if (assemble)
  {
    // Off-process contributions accumulate here and are reduced into their owner in close(). The
    // target PETSc vector's own memory type decides whether that reduction hands PETSc a device
    // or a host array, so both are kept: a plain (non-Kokkos-typed) vector's native COO
    // implementation only accepts a host array.
    if (realloc)
    {
      _ghost.create(_dof_space->getNumGhostDofs());
      _ghost.offset(_dof_space->getNumLocalDofs());
    }

    if (!_is_host)
    {
      if (realloc)
        _local.init(_dof_space->getNumLocalDofs());

      _local.aliasDevice(_array);
    }
    else
    {
      if (realloc)
        _local.createDevice(_dof_space->getNumLocalDofs());

      _local.aliasHost(_array);
    }
  }
  else
  {
    // A ghosted vector holds its ghost degrees of freedom alongside the local ones
    const auto size = _is_ghosted
                          ? _dof_space->getNumLocalDofs() + _dof_space->getNumGhostDofs()
                          : _dof_space->getNumLocalDofs();

    if (!_is_host)
    {
      if (_read_only)
      {
        if (realloc)
          _local_read.init(size);

        _local_read.aliasDevice(_read_array);
      }
      else
      {
        if (realloc)
          _local.init(size);

        _local.aliasDevice(_array);
      }
    }
    else
    {
      if (realloc)
        _local.createDevice(size);

      // A read-only vector's host array is const and so cannot back the owned device storage;
      // copyToDevice() copies its values in instead
      if (!_read_only)
        _local.aliasHost(_array);
    }
  }

  _is_alloc = true;
}

void
Vector::destroy()
{
  if (!_is_alloc)
    return;

  _global_vector = PETSC_NULLPTR;
  _local_vector = PETSC_NULLPTR;
  _array = PETSC_NULLPTR;
  _read_array = PETSC_NULLPTR;
  _dof_space = nullptr;

  _local.destroy();
  _ghost.destroy();
  _local_read.destroy();

  _assemble = false;
  _is_ghosted = false;
  _is_host = false;
  _read_only = false;
  _is_alloc = false;
}

void
Vector::copyToDevice()
{
  if (_is_alloc && _is_host)
  {
    // A read-only vector's host array is held const, so its values are copied into the owned device
    // storage rather than that storage aliasing them
    if (_read_only)
      _local.copyIn(_read_array, MemcpyType::HOST_TO_DEVICE, _local.size());
    else
      _local.copyToDevice();
  }

  if (_assemble)
    _ghost = 0;
}

void
Vector::copyToHost()
{
  if (_assemble)
    mooseError("Kokkos vector error: copyToHost() should not be called for an assembled vector.");

  if (_read_only)
    mooseError("Kokkos vector error: copyToHost() should not be called for a read-only vector.");

  if (_is_alloc && _is_host)
    _local.copyToHost();
}

void
Vector::restore()
{
  const auto local_form = _local_vector ? _local_vector : _global_vector;

  if (_read_only)
    LibmeshPetscCallQ(VecRestoreArrayReadAndMemType(local_form, &_read_array));
  else
    LibmeshPetscCallQ(VecRestoreArrayAndMemType(local_form, &_array));

  LibmeshPetscCallQ(VecGhostRestoreLocalForm(_global_vector, &_local_vector));
}

void
Vector::close()
{
  if (!_assemble)
    mooseError("Kokkos vector error: close() should not be called for a non-assembled vector.");

  if (!_is_alloc)
    return;

  // Copy to the host vector if needed

  if (_is_host)
    _local.copyToHost();

  // Restore the PETSc vector

  restore();

  // Every row this rank holds a contribution for but does not own is reduced into its owner
  // through PETSc's coordinate-format vector interface. The row indices are the same global
  // degrees of freedom a VecSetValues call would use, in the same order the entity-block and
  // quadrature-point Jacobian loops filled _ghost in.

  LibmeshPetscCallQ(VecSetPreallocationCOO(
      _global_vector,
      _dof_space->getNumGhostDofs(),
      reinterpret_cast<const PetscInt *>(_dof_space->getDofMap().get_send_list().data())));

  // A Kokkos/CUDA-typed vector's COO implementation detects and accepts a device array; a plain
  // vector's native implementation reads its values array directly from the host and does not, so
  // the device-resident contributions are brought back for it, as they were before this vector
  // used PETSc's COO interface
  if (_is_host)
  {
    _ghost.copyToHost();
    LibmeshPetscCallQ(VecSetValuesCOO(_global_vector, _ghost.hostData(), ADD_VALUES));
  }
  else
    LibmeshPetscCallQ(VecSetValuesCOO(_global_vector, _ghost.deviceData(), ADD_VALUES));
}

} // namespace Moose::Kokkos
