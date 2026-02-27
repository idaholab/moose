//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVector.h"
#include "KokkosSystem.h"

// #define DEVICE_ASSEMBLY

namespace Moose::Kokkos
{

#ifdef DEVICE_ASSEMBLY

void
Vector::DeviceAssembly::create(const Array<Array<libMesh::dof_id_type>> & list)
{
  this->list = list;

  count.createHost(list.size());
  offset.create(list.size() + 1);
  offset = 0;

  for (unsigned int i = 0; i < list.size(); ++i)
  {
    count[i] = list[i].size();
    offset[i + 1] = offset[i] + count[i];
  }

  offset.copyToDevice();

  buffer.createDevice(offset.last());
}

void
Vector::DeviceAssembly::destroy()
{
  list.destroy();
  count.destroy();
  offset.destroy();
  buffer.destroy();
}

KOKKOS_FUNCTION void
Vector::operator()(PackBuffer, const PetscCount tid) const
{
  _send.buffer[_send.offset[_current_proc] + tid] = _ghost(_send.list[_current_proc][tid]);
}

KOKKOS_FUNCTION void
Vector::operator()(UnpackBuffer, const PetscCount tid) const
{
  _local[_recv.list[_current_proc][tid]] += _recv.buffer[_recv.offset[_current_proc] + tid];
}

#endif

void
Vector::create(libMesh::NumericVector<PetscScalar> & vector, const System & system, bool assemble)
{
  auto petsc_vector = dynamic_cast<libMesh::PetscVector<PetscScalar> *>(&vector);

  mooseAssert(petsc_vector, "Kokkos vector error: provided vector is not a PetscVector.");

  PetscScalar * array;
  PetscMemType mtype;
  LibmeshPetscCallQ(VecGhostGetLocalForm(petsc_vector->vec(), &_local_vector));
  LibmeshPetscCallQ(
      VecGetArrayAndMemType(_local_vector ? _local_vector : petsc_vector->vec(), &array, &mtype));

  auto is_host = mtype == PETSC_MEMTYPE_HOST;
  auto is_ghosted = vector.type() == libMesh::ParallelType::GHOSTED;
  bool realloc =
      !_is_alloc || _assemble != assemble || _is_host != is_host || _is_ghosted != is_ghosted;

#ifndef MOOSE_ENABLE_KOKKOS_GPU
  if (!is_host)
    mooseError("PETSc vectors must be on host when Kokkos device capabilities are disabled.");
#endif

  _global_vector = petsc_vector->vec();
  _array = array;
  _system = &system;
  _assemble = assemble;
  _is_host = is_host;
  _is_ghosted = is_ghosted;

  if (realloc)
  {
    _comm = nullptr;

#ifdef DEVICE_ASSEMBLY
    _send.destroy();
    _recv.destroy();
#endif

    _local.destroy();
    _ghost.destroy();
  }

  if (assemble)
  {
    if (realloc)
    {
#ifdef DEVICE_ASSEMBLY
      _comm = &_system->getComm();
      _send.create(_system->getGhostCommList());
      _recv.create(_system->getLocalCommList());
      _ghost.createDevice(_system->getNumGhostDofs());
#else
      _ghost.create(_system->getNumGhostDofs());
#endif
      _ghost.offset(_system->getNumLocalDofs());
    }

    if (!_is_host)
    {
      if (realloc)
        _local.init(_system->getNumLocalDofs());

      _local.aliasDevice(_array);
    }
    else
    {
      if (realloc)
        _local.createDevice(_system->getNumLocalDofs());

      _local.aliasHost(_array);
    }
  }
  else
  {
    if (!_is_host)
    {
      if (realloc)
        _local.init(_is_ghosted ? _system->getNumLocalDofs() + _system->getNumGhostDofs()
                                : _system->getNumLocalDofs());

      _local.aliasDevice(_array);
    }
    else
    {
      if (realloc)
        _local.createDevice(_is_ghosted ? _system->getNumLocalDofs() + _system->getNumGhostDofs()
                                        : _system->getNumLocalDofs());

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
  _system = nullptr;
  _comm = nullptr;

#ifdef DEVICE_ASSEMBLY
  _send.destroy();
  _recv.destroy();
#endif

  _local.destroy();
  _ghost.destroy();

  _assemble = false;
  _is_ghosted = false;
  _is_host = false;
  _is_alloc = false;
}

void
Vector::copyToDevice()
{
  if (_is_alloc && _is_host)
    _local.copyToDevice();

  if (_assemble)
    _ghost = 0;
}

void
Vector::copyToHost()
{
  if (_assemble)
    mooseError("Kokkos vector error: copyToHost() should not be called for an assembled vector.");

  if (_is_alloc && _is_host)
    _local.copyToHost();
}

void
Vector::restore()
{
  LibmeshPetscCallQ(
      VecRestoreArrayAndMemType(_local_vector ? _local_vector : _global_vector, &_array));
  LibmeshPetscCallQ(VecGhostRestoreLocalForm(_global_vector, &_local_vector));
}

void
Vector::close()
{
  if (!_assemble)
    mooseError("Kokkos vector error: close() should not be called for a non-assembled vector.");

  if (!_is_alloc)
    return;

#ifdef DEVICE_ASSEMBLY

  // Pack data into send buffer

  for (_current_proc = 0; _current_proc < _comm->size(); ++_current_proc)
  {
    Kokkos::RangePolicy<PackBuffer, Kokkos::IndexType<PetscCount>> policy(
        0, _send.count[_current_proc]);
    Kokkos::parallel_for(policy, *this);
  }

  Kokkos::fence();

  // Perform MPI communications

  MPI_Alltoallv(_send.buffer.deviceData(),
                _send.count.data(),
                _send.offset.data(),
                MPIU_SCALAR,
                _recv.buffer.deviceData(),
                _recv.count.data(),
                _recv.offset.data(),
                MPIU_SCALAR,
                _comm->get());

  // Unpack data from receive buffer

  for (_current_proc = 0; _current_proc < _comm->size(); ++_current_proc)
  {
    Kokkos::RangePolicy<UnpackBuffer, Kokkos::IndexType<PetscCount>> policy(
        0, _recv.count[_current_proc]);
    Kokkos::parallel_for(policy, *this);
  }

  Kokkos::fence();

#endif

  // Copy to the host vector if needed

  if (_is_host)
    _local.copyToHost();

  // Restore the PETSc vector

  restore();

#ifndef DEVICE_ASSEMBLY

  _ghost.copyToHost();

  LibmeshPetscCallQ(
      VecSetValues(_global_vector,
                   _system->getNumGhostDofs(),
                   reinterpret_cast<const PetscInt *>(_system->getDofMap().get_send_list().data()),
                   _ghost.data(),
                   ADD_VALUES));

#endif
}

} // namespace Moose::Kokkos
