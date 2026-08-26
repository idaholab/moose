//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{

/**
 Virtual base class for representing discrete symmetry transforms between equivalent vertices in a
 mesh
 */
class DiscreteSymmetry
{
public:
  DiscreteSymmetry() = default;
  /**
   * Apply a symmetry transform from one boundary coordinate to another coordinate equivalent
   * through symmetry constraints
   */
  virtual void ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out) = 0;
};

class TranslationalSymmetry : public DiscreteSymmetry
{
public:
  TranslationalSymmetry(const mfem::Vector & lattice_vector)
    : DiscreteSymmetry(), _lattice_vector(lattice_vector){};

  virtual void ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out) override;

private:
  const mfem::Vector & _lattice_vector;
};

class RotationalSymmetry : public DiscreteSymmetry
{
public:
  RotationalSymmetry(const unsigned int rotational_symmetry_order)
    : DiscreteSymmetry(),
      _rotational_symmetry_order(rotational_symmetry_order),
      _rotation_angle(2 * pi / rotational_symmetry_order){};

  virtual void ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out) override;

private:
  const unsigned int _rotational_symmetry_order;
  const mfem::real_t _rotation_angle; // radians
};

}

#endif
