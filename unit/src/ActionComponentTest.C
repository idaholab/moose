//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseAppTestUtils.h"
#include "MooseMain.h"
#include "MooseMesh.h"
#include "MooseUnitUtils.h"

#include <string>
#include <vector>

using MooseAppTestUtils::Args;

namespace
{
/// Builds and runs a MooseUnitApp from an input file in files/ActionComponentTest, with additional
/// command line arguments
void
runInput(const std::string & input, const std::vector<std::string> & cli_args = {})
{
  std::vector<std::string> args = {"-i", "files/ActionComponentTest/" + input};
  args.insert(args.end(), cli_args.begin(), cli_args.end());
  Args app_args(args);
  Moose::createMooseApp("MooseUnitApp", app_args.argc(), app_args.argv())->run();
}
}

TEST(ActionComponentTest, cylinderDimension)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(runInput("cylinder.i", {"ActionComponents/cyl/dimension=0"}),
                                 "0D cylinder not implemented");
}

TEST(ActionComponentTest, cylinderRadialDiscretization)
{
  // 1D cylinders do not have a radial discretization
  EXPECT_MOOSEERROR_MSG_CONTAINS(runInput("cylinder.i", {"ActionComponents/cyl/dimension=1"}),
                                 "Should not be provided for a 1D cylinder");
}

TEST(ActionComponentTest, cylinder3DDiscretization)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(runInput("cylinder.i", {"ActionComponents/cyl/dimension=3"}),
                                 "Should be provided in 3D");
}

TEST(ActionComponentTest, cylinderRZCoordinates)
{
  Args args({"-i", "files/ActionComponentTest/cylinder_rz.i"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  const auto & mesh = app->getExecutioner()->feProblem().mesh();

  // RZ is only applied to the cylinder, the other subdomains keep their coordinate system
  const auto cyl_id = mesh.getSubdomainID("cyl");
  EXPECT_EQ(mesh.getCoordSystem(cyl_id), Moose::COORD_RZ);
  EXPECT_EQ(mesh.getCoordSystem(mesh.getSubdomainID("cyl_xy")), Moose::COORD_XYZ);

  // The axis goes through the cylinder position, along its direction
  const auto & [origin, direction] = mesh.getGeneralAxisymmetricCoordAxis(cyl_id);
  EXPECT_EQ(origin, Point(5, 0, 0));
  EXPECT_EQ(direction, RealVectorValue(0, 1, 0));
}

TEST(ActionComponentTest, cylinderRZBlock)
{
  // The RZ axis is set per subdomain, so the cylinder must have a named subdomain
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("cylinder.i", {"ActionComponents/cyl/use_RZ_coordinates=true"}),
      "Should be specified to use RZ coordinates");
}

TEST(ActionComponentTest, cylinderRotationRZ)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(runInput("cylinder.i",
                                          {"ActionComponents/cyl/use_RZ_coordinates=true",
                                           "ActionComponents/cyl/block=cyl",
                                           "ActionComponents/cyl/rotation='0 0 90'"}),
                                 "Rotation + 2D RZ cylinder not implemented");
}

TEST(ActionComponentTest, rotationAndDirection)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(runInput("cylinder.i",
                                          {"ActionComponents/cyl/rotation='0 0 90'",
                                           "ActionComponents/cyl/direction='0 1 0'"}),
                                 "Parameter 'direction' should not be specified if parameter "
                                 "'rotation' is specified.");
}

TEST(ActionComponentTest, cavityDimension)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(runInput("cavity.i", {"ActionComponents/cavity/dimension=1"}),
                                 "Cavity not implemented for 0D and 1D");
}

TEST(ActionComponentTest, cavityBoundaryLayerWidth)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("cavity.i", {"ActionComponents/cavity/n_boundary_layers=2"}),
      "Must be passed if n_boundary_layers > 0");
}

TEST(ActionComponentTest, surfaceExtrusionRadialExtent)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("surface_extrusion.i", {"ActionComponents/extrusion/start_radial_extent=1"}),
      "Parameter 'start_radial_extent' should not be set if parameter 'end_radial_extent' is not "
      "specified.");
}

TEST(ActionComponentTest, surfaceExtrusionGrowthMethod)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("surface_extrusion.i", {"ActionComponents/extrusion/radial_growth_method=cubic"}),
      "Parameter 'radial_growth_method' should not be set if parameter 'end_radial_extent' is "
      "not specified.");
}

TEST(ActionComponentTest, surfaceExtrusionGrowthRates)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("surface_extrusion.i",
               {"ActionComponents/extrusion/end_radial_extent=2",
                "ActionComponents/extrusion/start_radial_growth_rate=0.5"}),
      "Parameter 'start_radial_growth_rate' should not be set by the user if parameter "
      "'radial_growth_method' has not been set to 'CUBIC'");
}

TEST(ActionComponentTest, surfaceExtrusion1DRadialExpansion)
{
  // Warnings are turned into exceptions in the unit test application
  EXPECT_THROW_MSG_CONTAINS(
      runInput("surface_extrusion_1D.i", {"ActionComponents/extrusion/end_radial_extent=2"}),
      std::runtime_error,
      "1D surface extrusion currently does not support radial expansion");
}

TEST(ActionComponentTest, junctionStitchingParameters)
{
  // Extrusion parameters are not used when stitching
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("junction.i", {"ActionComponents/junction/n_elem_normal=2"}),
      "Parameter 'n_elem_normal' should not be set by the user if parameter 'junction_method' "
      "has not been set to 'extrude_boundary'");
}

TEST(ActionComponentTest, junctionExtrusionNumberElements)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("junction.i", {"ActionComponents/junction/junction_method=extrude_boundary"}),
      "Should be specified if junction_method = 'extrude_boundary'");
}

TEST(ActionComponentTest, junctionStitchDifferentDimensions)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("junction.i",
               {"ActionComponents/cyl2/dimension=3", "ActionComponents/cyl2/n_sectors=2"}),
      "Stiching meshes of different dimensions is not implemented");
}

TEST(ActionComponentTest, meshGeneratorComponentSavedMesh)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("mesh_generator_component.i",
               {"ActionComponents/comp/mesh_generator_type=saved_mesh"}),
      "The name of the saved mesh must be provided if using a mesh generator that saves the mesh "
      "for the component to use");
}

TEST(ActionComponentTest, boundaryConditionInputSizes)
{
  // One list of boundaries per variable
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("component_with_physics.i",
               {"ActionComponents/cyl/fixed_value_bc_variables='u'",
                "ActionComponents/cyl/fixed_value_bc_boundaries='cyl_top_boundary; "
                "cyl_bottom_boundary'",
                "ActionComponents/cyl/fixed_value_bc_values='1; 2'"}),
      "Vector parameters 'fixed_value_bc_variables' (size 1) and 'fixed_value_bc_boundaries' "
      "(size 2) must be the same size");
  // One value per boundary
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("component_with_physics.i",
               {"ActionComponents/cyl/flux_bc_variables='u'",
                "ActionComponents/cyl/flux_bc_boundaries='cyl_top_boundary cyl_bottom_boundary'",
                "ActionComponents/cyl/flux_bc_values='1'"}),
      "Vector at index 0 of 2D vector parameter 'flux_bc_boundaries' is not the same size as its "
      "counterpart from 2D vector parameter 'flux_bc_values'");
}

TEST(ActionComponentTest, boundaryConditionRepeatedVariable)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("component_with_physics.i",
               {"ActionComponents/cyl/fixed_value_bc_variables='u u'",
                "ActionComponents/cyl/fixed_value_bc_boundaries='cyl_top_boundary; "
                "cyl_bottom_boundary'",
                "ActionComponents/cyl/fixed_value_bc_values='1; 2'"}),
      "Item 'u' specified in vector parameter 'fixed_value_bc_variables' is repeated, which is "
      "not allowed.");
}

TEST(ActionComponentTest, initialConditionRepeatedVariable)
{
  EXPECT_MOOSEERROR_MSG_CONTAINS(
      runInput("component_with_physics.i",
               {"ActionComponents/cyl/initial_condition_variables='u u'",
                "ActionComponents/cyl/initial_condition_values='1 2'"}),
      "Item 'u' specified in vector parameter 'initial_condition_variables' is repeated, which is "
      "not allowed.");
}
