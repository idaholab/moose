//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUVector.h"
#include "GPUSystem.h"

// #define GPU_ASSEMBLY

void
GPUVector::MPIBuffer::create(const GPUArray<GPUArray<libMesh::dof_id_type>> & list)
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

  offset.copy();

  buffer.createDevice(offset.last());
}

void
GPUVector::MPIBuffer::destroy()
{
  list.destroy();
  count.destroy();
  offset.destroy();
  buffer.destroy();
}

void
GPUVector::create(libMesh::NumericVector<PetscScalar> & vector,
                  const GPUSystem & system,
                  bool assemble)
{
  auto petsc_vector = dynamic_cast<libMesh::PetscVector<PetscScalar> *>(&vector);

  mooseAssert(petsc_vector, "GPUVector error: provided vector is not a PetscVector.");

  PetscScalar * array;
  PetscMemType mtype;
  LibmeshPetscCallQ(VecGhostGetLocalForm(petsc_vector->vec(), &_local_vector));
  LibmeshPetscCallQ(
      VecGetArrayAndMemType(_local_vector ? _local_vector : petsc_vector->vec(), &array, &mtype));

  auto is_host = mtype == PETSC_MEMTYPE_HOST;
  auto is_ghosted = vector.type() == libMesh::ParallelType::GHOSTED;
  bool realloc =
      !_is_alloc || _assemble != assemble || _is_host != is_host || _is_ghosted != is_ghosted;

  _vector = petsc_vector->vec();
  _array = array;
  _system = &system;
  _assemble = assemble;
  _is_host = is_host;
  _is_ghosted = is_ghosted;

  if (realloc)
  {
    _comm = nullptr;
    _send.destroy();
    _recv.destroy();
    _local.destroy();
    _ghost.destroy();
  }

  if (assemble)
  {
    if (realloc)
    {
#ifdef GPU_ASSEMBLY
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
GPUVector::copy(GPUMemcpyKind dir)
{
  if (_assemble)
    mooseError("GPUVector error: copy() should not be called for an assembled vector.");

  if (_is_alloc && _is_host)
    _local.copy(dir);
}

void
GPUVector::destroy()
{
  if (!_is_alloc)
    return;

  _local_vector = PETSC_NULLPTR;
  _vector = PETSC_NULLPTR;
  _array = PETSC_NULLPTR;
  _system = nullptr;
  _comm = nullptr;

  _send.destroy();
  _recv.destroy();
  _local.destroy();
  _ghost.destroy();

  _assemble = false;
  _is_ghosted = false;
  _is_host = false;
  _is_alloc = false;
}

void
GPUVector::restore()
{
  LibmeshPetscCallQ(
      VecRestoreArrayAndMemType(_local_vector ? _local_vector : _vector, PETSC_NULLPTR));
  LibmeshPetscCallQ(VecGhostRestoreLocalForm(_vector, &_local_vector));
}

void
GPUVector::close()
{
  if (!_assemble)
    mooseError("GPUVector error: close() should not be called for a non-assembled vector.");

  if (!_is_alloc)
    return;

#ifdef GPU_ASSEMBLY

  // Pack data into send buffer

  for (_current_proc = 0; _current_proc < _comm->size(); ++_current_proc)
  {
    Kokkos::RangePolicy<PackBuffer, Kokkos::IndexType<PetscCount>> policy(
        0, _send.count[_current_proc]);
    Kokkos::parallel_for(policy, *this);
  }

  Kokkos::fence();

  // Perform MPI communications

  MPI_Alltoallv(_send.buffer.device_data(),
                _send.count.data(),
                _send.offset.data(),
                MPIU_SCALAR,
                _recv.buffer.device_data(),
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

#else

  _ghost.copy(GPUMemcpyKind::DEVICE_TO_HOST);

  LibmeshPetscCallQ(
      VecSetValues(_vector,
                   _system->getNumGhostDofs(),
                   reinterpret_cast<const PetscInt *>(_system->getDofMap().get_send_list().data()),
                   _ghost.data(),
                   ADD_VALUES));

#endif

  // Copy to the host vector if needed

  if (_is_host)
    _local.copy(GPUMemcpyKind::DEVICE_TO_HOST);

  // Restore the PETSc vector

  restore();
}

KOKKOS_FUNCTION void
GPUVector::operator()(PackBuffer, const PetscCount tid) const
{
  _send.buffer[_send.offset[_current_proc] + tid] = _ghost(_send.list[_current_proc][tid]);
}

KOKKOS_FUNCTION void
GPUVector::operator()(UnpackBuffer, const PetscCount tid) const
{
  _local[_recv.list[_current_proc][tid]] += _recv.buffer[_recv.offset[_current_proc] + tid];
}
