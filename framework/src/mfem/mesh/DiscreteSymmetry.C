//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "DiscreteSymmetry.h"
#include "MooseError.h"

namespace Moose::MFEM
{

void
TranslationalSymmetry::ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out)
{
  mooseAssert((coord_in.Size() == _lattice_vector.Size()),
              "Size of lattice vector doesn't match the space dimension");
  add(coord_in, _lattice_vector, coord_out);
}

RotationalSymmetry::RotationalSymmetry(const unsigned int rotational_symmetry_order,
                                       const mfem::Vector & rotation_axis)
  : DiscreteSymmetry(),
    _rotation_angle(2 * M_PI / rotational_symmetry_order),
    _rotation_axis(rotation_axis),
    _2d_rotation_matrix(Build2DRotationMatrix(_rotation_angle)),
    _3d_rotation_matrix(Build3DRotationMatrix(_rotation_axis, _rotation_angle))
{
}

mfem::DenseMatrix
RotationalSymmetry::Build2DRotationMatrix(const mfem::real_t & rotation_angle)
{
  mfem::DenseMatrix rotation_matrix(2);

  const mfem::real_t cos_a = cos(rotation_angle);
  const mfem::real_t sin_a = sin(rotation_angle);

  rotation_matrix(0, 0) = cos_a;
  rotation_matrix(0, 1) = -sin_a;
  rotation_matrix(1, 0) = sin_a;
  rotation_matrix(1, 1) = cos_a;

  return rotation_matrix;
}

mfem::DenseMatrix
RotationalSymmetry::Build3DRotationMatrix(const mfem::Vector & rotation_axis,
                                          const mfem::real_t & rotation_angle)
{
  mfem::DenseMatrix rotation_matrix(3);

  mfem::Vector unit_rot_axis = rotation_axis;
  unit_rot_axis /= rotation_axis.Norml2();

  const mfem::real_t cos_a = cos(rotation_angle);
  const mfem::real_t sin_a = sin(rotation_angle);

  const mfem::real_t ux = unit_rot_axis(0);
  const mfem::real_t uxx = unit_rot_axis(0) * unit_rot_axis(0);
  const mfem::real_t uxy = unit_rot_axis(0) * unit_rot_axis(1);
  const mfem::real_t uxz = unit_rot_axis(0) * unit_rot_axis(2);

  const mfem::real_t uy = unit_rot_axis(1);
  const mfem::real_t uyy = unit_rot_axis(1) * unit_rot_axis(1);
  const mfem::real_t uyz = unit_rot_axis(1) * unit_rot_axis(2);

  const mfem::real_t uz = unit_rot_axis(2);
  const mfem::real_t uzz = unit_rot_axis(2) * unit_rot_axis(2);

  rotation_matrix(0, 0) = uxx * (1 - cos_a) + cos_a;
  rotation_matrix(0, 1) = uxy * (1 - cos_a) - uz * sin_a;
  rotation_matrix(0, 2) = uxz * (1 - cos_a) + uy * sin_a;

  rotation_matrix(1, 0) = uxy * (1 - cos_a) + uz * sin_a;
  rotation_matrix(1, 1) = uyy * (1 - cos_a) + cos_a;
  rotation_matrix(1, 2) = uyz * (1 - cos_a) - ux * sin_a;

  rotation_matrix(2, 0) = uxz * (1 - cos_a) - uy * sin_a;
  rotation_matrix(2, 1) = uyz * (1 - cos_a) + ux * sin_a;
  rotation_matrix(2, 2) = uzz * (1 - cos_a) + cos_a;

  return rotation_matrix;
}

void
RotationalSymmetry::ApplyTransform(const mfem::Vector & coord_in, mfem::Vector & coord_out)
{
  if (coord_in.Size() == 2)
    _2d_rotation_matrix.Mult(coord_in, coord_out);
  else if (coord_in.Size() == 3)
    _3d_rotation_matrix.Mult(coord_in, coord_out);
  else
    mooseError("Provided coordinate has an incompatible spatial dimension with available rotation "
               "matrices");
}

}

#endif
