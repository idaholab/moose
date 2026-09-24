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
    _stash.destroy();
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
      _stash.create(_dof_space->getNumGhostDofs());
      _stash.offset(_dof_space->getNumLocalDofs());
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
  _stash.destroy();
  _local_read.destroy();

  _assemble = false;
  _is_ghosted = false;
  _is_host = false;
  _read_only = false;
  _is_alloc = false;

  // The next close() on this wrapper reduces over whatever layout it is then created around, so the
  // star forest built for the old one no longer applies
  _reduction.reset();
  _reduction_dof_space = nullptr;
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
    _stash = 0;
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

  // Every row this rank holds a contribution for but does not own is reduced into its owner through
  // a star forest over the vector's layout. Leaf i is _stash entry i, which the entity-block and
  // quadrature-point Jacobian loops filled against the send list's i'th global degree of freedom, so
  // the graph is that list read as the row each leaf belongs to.
  //
  // PETSc's coordinate-format interface reduces the same contributions and builds the same star
  // forest to do it, but its preallocation also builds a map from each local row to the entries
  // landing on it, so that repeated indices can be summed. Its cost is therefore proportional to the
  // vector's local size rather than to the number of contributions, and it is kept on the vector, so
  // an operator application handed whichever of its caller's work vectors is free pays that setup
  // once per vector. There is exactly one contribution per ghost degree of freedom here, so there is
  // nothing for that map to coalesce, and the forest alone is built once per layout instead.

  if (_dof_space != _reduction_dof_space)
  {
    PetscSF reduction;
    LibmeshPetscCallQ(PetscSFCreate(_dof_space->getDofMap().comm().get(), &reduction));

    PetscLayout layout;
    LibmeshPetscCallQ(VecGetLayout(_global_vector, &layout));
    LibmeshPetscCallQ(PetscSFSetGraphLayout(
        reduction,
        layout,
        _dof_space->getNumGhostDofs(),
        LIBMESH_PETSC_NULLPTR,
        PETSC_COPY_VALUES,
        reinterpret_cast<const PetscInt *>(_dof_space->getDofMap().get_send_list().data())));

    _reduction = std::shared_ptr<PetscSF>(new PetscSF(reduction),
                                                [](PetscSF * const sf)
                                                {
                                                  PetscSFDestroy(sf);
                                                  delete sf;
                                                });
    _reduction_dof_space = _dof_space;
  }

  // A star forest built once per degree-of-freedom layout, rather than PETSc's coordinate-format
  // vector interface. PETSc caches that interface's preallocation on the vector it was called for,
  // and an operator application is handed whichever of its caller's work vectors is free, so a
  // solve that cycles among several dozen of them builds a preallocation per vector where a forest
  // keyed on the layout is built once. There is also nothing here for such a preallocation to
  // coalesce, since this buffer holds exactly one contribution per off-process degree of freedom.
  //
  // The reduction reads and writes arrays rather than the vector, so it runs while the array is
  // still held, and a device-resident contribution is reduced where it already is
  const auto memtype = _is_host ? PETSC_MEMTYPE_HOST : PETSC_MEMTYPE_DEVICE;

  if (_is_host)
    _stash.copyToHost();

  const PetscScalar * const contributions = _is_host ? _stash.hostData() : _stash.deviceData();

  LibmeshPetscCallQ(PetscSFReduceWithMemTypeBegin(
      *_reduction, MPIU_SCALAR, memtype, contributions, memtype, _array, MPIU_SUM));
  LibmeshPetscCallQ(
      PetscSFReduceEnd(*_reduction, MPIU_SCALAR, contributions, _array, MPIU_SUM));

  // Restore the PETSc vector

  restore();
}

} // namespace Moose::Kokkos
