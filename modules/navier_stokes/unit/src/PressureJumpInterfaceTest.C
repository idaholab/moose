//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "gtest/gtest.h"

#include "PressureJumpInterface.h"

TEST(PressureJumpInterface, OrthogonalIsotropic)
{
  const RealVectorValue normal(1.0, 0.0, 0.0);
  const Point half_cell(0.5, 0.0, 0.0);
  const RealVectorValue diffusion(2.0, 2.0, 2.0);
  const RealVectorValue gradient(3.0, -4.0, 0.0);

  const auto data = NS::FV::pressureJumpInterfaceData(
      normal, half_cell, half_cell, diffusion, diffusion, gradient, gradient, 3.0, true);

  ASSERT_TRUE(data.valid);
  EXPECT_DOUBLE_EQ(data.elem_conductance, 12.0);
  EXPECT_DOUBLE_EQ(data.neighbor_conductance, 12.0);
  EXPECT_DOUBLE_EQ(data.transmissibility, 6.0);
  EXPECT_DOUBLE_EQ(data.elem_correction, 0.0);
  EXPECT_DOUBLE_EQ(data.neighbor_correction, 0.0);
  EXPECT_DOUBLE_EQ(data.correction, 0.0);
}

TEST(PressureJumpInterface, ConservesFluxAndPreservesJump)
{
  const RealVectorValue normal(0.8, 0.6, 0.0);
  const Point elem_to_face(0.5, 0.1, 0.0);
  const Point face_to_neighbor(0.35, 0.3, 0.0);
  const RealVectorValue elem_diffusion(2.0, 5.0, 1.0);
  const RealVectorValue neighbor_diffusion(7.0, 3.0, 1.0);
  const RealVectorValue elem_gradient(1.2, -0.4, 0.0);
  const RealVectorValue neighbor_gradient(-0.3, 2.1, 0.0);
  const Real elem_pressure = 11.0;
  const Real neighbor_pressure = 4.0;
  const Real jump = 2.5;

  const auto data = NS::FV::pressureJumpInterfaceData(normal,
                                                      elem_to_face,
                                                      face_to_neighbor,
                                                      elem_diffusion,
                                                      neighbor_diffusion,
                                                      elem_gradient,
                                                      neighbor_gradient,
                                                      1.7,
                                                      true);
  ASSERT_TRUE(data.valid);

  const Real flux = NS::FV::pressureJumpFlux(data, elem_pressure, neighbor_pressure, jump);
  const Real elem_face_pressure =
      elem_pressure - (flux + data.elem_correction) / data.elem_conductance;
  const Real neighbor_face_pressure =
      neighbor_pressure + (flux + data.neighbor_correction) / data.neighbor_conductance;
  const Real elem_half_flux =
      data.elem_conductance * (elem_pressure - elem_face_pressure) - data.elem_correction;
  const Real neighbor_half_flux =
      data.neighbor_conductance * (neighbor_face_pressure - neighbor_pressure) -
      data.neighbor_correction;

  EXPECT_NEAR(elem_face_pressure - neighbor_face_pressure, jump, 1e-13);
  EXPECT_NEAR(elem_half_flux, flux, 1e-13);
  EXPECT_NEAR(neighbor_half_flux, flux, 1e-13);
}

TEST(PressureJumpInterface, OrientationReversal)
{
  const RealVectorValue normal(0.8, 0.6, 0.0);
  const Point elem_to_face(0.5, 0.1, 0.0);
  const Point face_to_neighbor(0.35, 0.3, 0.0);
  const RealVectorValue elem_diffusion(2.0, 5.0, 1.0);
  const RealVectorValue neighbor_diffusion(7.0, 3.0, 1.0);
  const RealVectorValue elem_gradient(1.2, -0.4, 0.0);
  const RealVectorValue neighbor_gradient(-0.3, 2.1, 0.0);

  const auto forward = NS::FV::pressureJumpInterfaceData(normal,
                                                         elem_to_face,
                                                         face_to_neighbor,
                                                         elem_diffusion,
                                                         neighbor_diffusion,
                                                         elem_gradient,
                                                         neighbor_gradient,
                                                         1.7,
                                                         true);
  const auto reverse = NS::FV::pressureJumpInterfaceData(-normal,
                                                         -face_to_neighbor,
                                                         -elem_to_face,
                                                         neighbor_diffusion,
                                                         elem_diffusion,
                                                         neighbor_gradient,
                                                         elem_gradient,
                                                         1.7,
                                                         true);
  ASSERT_TRUE(forward.valid);
  ASSERT_TRUE(reverse.valid);

  const Real forward_flux = NS::FV::pressureJumpFlux(forward, 11.0, 4.0, 2.5);
  const Real reverse_flux = NS::FV::pressureJumpFlux(reverse, 4.0, 11.0, -2.5);

  EXPECT_NEAR(reverse.elem_conductance, forward.neighbor_conductance, 1e-13);
  EXPECT_NEAR(reverse.neighbor_conductance, forward.elem_conductance, 1e-13);
  EXPECT_NEAR(reverse.correction, -forward.correction, 1e-13);
  EXPECT_NEAR(reverse_flux, -forward_flux, 1e-13);
}

TEST(PressureJumpInterface, RejectsDegenerateHalfCell)
{
  const auto data = NS::FV::pressureJumpInterfaceData(RealVectorValue(1.0, 0.0, 0.0),
                                                      Point(0.0, 1.0, 0.0),
                                                      Point(0.5, 0.0, 0.0),
                                                      RealVectorValue(1.0, 1.0, 1.0),
                                                      RealVectorValue(1.0, 1.0, 1.0),
                                                      RealVectorValue(),
                                                      RealVectorValue(),
                                                      1.0,
                                                      true);
  EXPECT_FALSE(data.valid);
}
