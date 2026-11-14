//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics.h"

#include "MooseUtils.h"
#include "NS.h"
#include "ConservativeDiffuseInterfaceRhieChowMassFlux.h"
#include "TheWarehouse.h"

registerWCNSFVFlowPhysicsBaseTasks("NavierStokesApp",
                                   WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics);
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_interpolation_method_physics");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_gradient_method");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_linear_fv_kernel");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_linear_fv_bc");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics,
                    "add_functor_material");

InputParameters
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::validParams()
{
  InputParameters params = WCNSLinearFVFlowPhysics::validParams();

  params.addClassDescription("Define a linear-FV segregated diffuse-interface flow solve using "
                             "velocity components as the primary momentum unknowns.");
  params.set<std::vector<std::string>>("velocity_variable") =
      std::vector<std::string>(NS::velocity_vector, NS::velocity_vector + 3);

  // The unknown is the reduced pressure p_rgh. The hydrostatic part rho g.(x - x_ref) is not a
  // pressure variable at all here: the Rhie-Chow object carries it as a face force built from the
  // face-normal density gradient. The wall and outlet pressure conditions below are written for
  // p_rgh, so the constructor refuses the static-pressure form.
  params.set<bool>("solve_for_dynamic_pressure") = true;

  // The mass flux, the volume-fraction corrector transported with is used by momentum advection,
  // instead of forming density times volumetric flux, so that the momentum and volume-fraction
  // equations move the same mass across the interface.
  params.transferParam<MooseFunctorName>(
      ConservativeDiffuseInterfaceRhieChowMassFlux::validParams(), "vof_rho_phi_functor");
  params.addParam<MooseFunctorName>(
      "volume_fraction_functor",
      "",
      "Volume-fraction functor used to generate diffuse-interface geometry functors.");
  // The normal is published under this physics' prefix, so the volume-fraction physics has to be
  // told that name through its interface_normal_functor parameter
  params.addParam<bool>(
      "create_geometry_functors",
      true,
      "Whether to automatically add the diffuse-interface geometry functor material that produces "
      "the face-oriented interface unit normal.");
  // Added to |grad(alpha)|, in 1/length, before dividing by it. An interface gradient is of order
  // one over a few cells, a small regularization that ensures the normal is well defined everywhere
  params.addParam<Real>(
      "geometry_delta_n",
      1e-8,
      "Regularization used in the geometry material when constructing interface unit normals.");
  params.addParam<MooseFunctorName>(
      "surface_tension_coefficient",
      "",
      "Surface-tension coefficient functor. Leave empty to disable capillary forcing.");
  params.addParamNamesToGroup("create_geometry_functors volume_fraction_functor geometry_delta_n "
                              "surface_tension_coefficient",
                              "Diffuse Interface Geometry");
  // ConservativeDiffuseInterfaceRhieChowMassFlux supports only the standard projection
  params.suppressParameter<MooseEnum>("pressure_projection_method");

  return params;
}

WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::
    WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics(const InputParameters & parameters)
  : WCNSLinearFVFlowPhysics(parameters),
    _create_geometry_functors(getParam<bool>("create_geometry_functors"))
{
  if (!_solve_for_dynamic_pressure)
    paramError("solve_for_dynamic_pressure",
               "Conservative diffuse-interface flow physics requires solve_for_dynamic_pressure = "
               "true.");
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addOutletBC()
{
  unsigned int num_pressure_outlets = 0;
  for (const auto & [bdy, momentum_outlet_type] : _momentum_outlet_types)
    if (momentum_outlet_type == "fixed-pressure" ||
        momentum_outlet_type == "fixed-pressure-zero-gradient")
      num_pressure_outlets++;

  if (num_pressure_outlets != _pressure_functors.size())
    paramError("pressure_functors",
               "Size (" + std::to_string(_pressure_functors.size()) +
                   ") is not the same as the number of pressure outlet boundaries in "
                   "'fixed-pressure/fixed-pressure-zero-gradient' (size " +
                   std::to_string(num_pressure_outlets) + ")");

  const std::string u_names[3] = {"u", "v", "w"};
  for (const auto & [outlet_bdy, momentum_outlet_type] : _momentum_outlet_types)
  {
    // Both pressure outlet types get the same backflow-aware conditions: the total-pressure
    // condition below already distinguishes outflow from backflow face by face
    const bool pressure_outlet = momentum_outlet_type == "fixed-pressure" ||
                                 momentum_outlet_type == "fixed-pressure-zero-gradient";

    if (momentum_outlet_type == "zero-gradient" || pressure_outlet)
    {
      // A pressure outlet lets the flow reverse: on backflow the velocity BC fixes the tangential
      // velocity and extrapolates the normal one. Every backflow switch reads the flux frozen for
      // this outer iteration, so that the boundary value cannot depend on the pressure equation
      // that evaluates it.
      const std::string bc_type = pressure_outlet ? "LinearFVPressureInletOutletVelocityBC"
                                                  : "LinearFVAdvectionDiffusionOutflowBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
      params.set<bool>("use_two_term_expansion") = getParam<bool>("momentum_two_term_bc_expansion");
      if (pressure_outlet)
      {
        for (unsigned int d = 0; d < dimension(); ++d)
          params.set<SolverVariableName>(u_names[d]) = _velocity_names[d];
        params.set<MooseFunctorName>("face_flux") = "lagged_corrected_face_phi";
      }

      for (const auto d : make_range(dimension()))
      {
        params.set<LinearVariableName>("variable") = _velocity_names[d];
        if (pressure_outlet)
          params.set<MooseEnum>("momentum_component") = MooseEnum("x=0 y=1 z=2", NS::directions[d]);
        getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + outlet_bdy, params);
      }
    }

    if (pressure_outlet)
    {
      // The reduced pressure holds the total pressure less its hydrostatic part, and on backflow
      // less the incoming dynamic pressure
      const std::string bc_type = "LinearFVPrghTotalPressureBC";
      InputParameters params = getFactory().getValidParams(bc_type);
      params.set<LinearVariableName>("variable") = _pressure_name;
      params.set<MooseFunctorName>("functor") = libmesh_map_find(_pressure_functors, outlet_bdy);
      params.set<std::vector<BoundaryName>>("boundary") = {outlet_bdy};
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
      params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
      for (unsigned int d = 0; d < dimension(); ++d)
        params.set<SolverVariableName>(u_names[d]) = _velocity_names[d];
      params.set<MooseFunctorName>("face_flux") = "lagged_corrected_face_phi";
      getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + outlet_bdy, params);
    }
  }
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addWallsBC()
{
  const std::string u_names[3] = {"u", "v", "w"};
  for (const auto & [boundary_name, momentum_wall_type] : _momentum_wall_types)
  {
    if (momentum_wall_type == "noslip")
    {
      {
        const std::string bc_type = "LinearFVAdvectionDiffusionFunctorDirichletBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};

        for (const auto d : make_range(dimension()))
        {
          params.set<LinearVariableName>("variable") = _velocity_names[d];
          if (_momentum_wall_functors.count(boundary_name) == 0)
            params.set<MooseFunctorName>("functor") = "0";
          else
            params.set<MooseFunctorName>("functor") = _momentum_wall_functors[boundary_name][d];

          getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
        }
      }
      {
        // A wall carries the pressure gradient that makes the predictor flux match the prescribed
        // wall flux; the Rhie-Chow object computes and publishes that gradient after each
        // predictor, from the wall velocity and the predictor flux it holds. With the gradient
        // supplied, the condition reads Ainv to turn it back into a face flux and leaves HbyA_flux
        // unread; the latter is still a required parameter of the condition.
        const std::string bc_type = "LinearFVPressureFluxBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        params.set<LinearVariableName>("variable") = _pressure_name;
        params.set<MooseFunctorName>("constrained_pressure_normal_gradient") =
            "pressure_boundary_normal_gradient";
        params.set<MooseFunctorName>("HbyA_flux") = predictorFaceFluxName();
        params.set<MooseFunctorName>("Ainv") = pressureDiffusionCoefficientName();
        getProblem().addLinearFVBC(bc_type, _pressure_name + "_wall_flux_" + boundary_name, params);
      }
    }
    else if (momentum_wall_type == "symmetry")
    {
      {
        const std::string bc_type = "LinearFVVelocitySymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        for (unsigned int d = 0; d < dimension(); ++d)
          params.set<SolverVariableName>(u_names[d]) = _velocity_names[d];

        for (const auto d : make_range(dimension()))
        {
          params.set<LinearVariableName>("variable") = _velocity_names[d];
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          getProblem().addLinearFVBC(bc_type, _velocity_names[d] + "_" + boundary_name, params);
        }
      }
      {
        const std::string bc_type = "LinearFVPressureSymmetryBC";
        InputParameters params = getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
        params.set<LinearVariableName>("variable") = _pressure_name;
        params.set<MooseFunctorName>("HbyA_flux") = predictorFaceFluxName();
        getProblem().addLinearFVBC(bc_type, _pressure_name + "_" + boundary_name, params);
      }
    }
    else
      mooseError("Unsupported wall boundary condition type: " + std::string(momentum_wall_type));
  }

  if (getParam<bool>("pressure_two_term_bc_expansion"))
    paramWarning("pressure_two_term_bc_expansion",
                 "Ignored on diffuse-interface wall boundaries, which carry an explicit "
                 "zero-normal-flux pressure boundary condition instead.");
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addRhieChowUserObjects()
{
  mooseAssert(dimension(), "0-dimension not supported");

  // The flow coupling is owned by this physics: a diffuse-interface Rhie-Chow object on these
  // blocks is reused, a stock one on overlapping blocks is an input error
  const auto this_block_ids =
      getSubdomainIDs(std::set<SubdomainName>(_blocks.begin(), _blocks.end()));
  std::vector<UserObject *> objs;
  getProblem()
      .theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryInto(objs);
  for (const auto & obj : objs)
    if (const auto rc_obj = dynamic_cast<RhieChowMassFlux *>(obj))
    {
      const auto & rc_block_ids = rc_obj->blockIDs();
      const bool overlapping = rc_block_ids.empty() || this_block_ids.empty() ||
                               rc_block_ids.count(Moose::ANY_BLOCK_ID) ||
                               this_block_ids.count(Moose::ANY_BLOCK_ID) ||
                               MooseUtils::setsIntersect(rc_block_ids, this_block_ids);
      if (!overlapping)
        continue;

      if (dynamic_cast<ConservativeDiffuseInterfaceRhieChowMassFlux *>(obj))
        return;

      mooseError("Diffuse-interface flow physics '",
                 name(),
                 "' requires a ConservativeDiffuseInterfaceRhieChowMassFlux on blocks ",
                 Moose::stringify(_blocks),
                 ", but found existing RhieChowMassFlux '",
                 rc_obj->name(),
                 "' on overlapping blocks. Remove the stock Rhie-Chow object or use the "
                 "diffuse-interface flow physics as the owner of the segregated flow coupling.");
    }

  const auto object_type = "ConservativeDiffuseInterfaceRhieChowMassFlux";
  const std::string u_names[3] = {"u", "v", "w"};
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  for (unsigned int d = 0; d < dimension(); ++d)
    params.set<VariableName>(u_names[d]) = _velocity_names[d];

  params.set<VariableName>("pressure") = _pressure_name;
  // The pressure diffusion kernel of addPressureCorrectionKernels, which the object evaluates
  // face by face to recover the pressure flux the solved pressure implies
  params.set<std::string>("p_diffusion_kernel") = prefix() + "p_diffusion";
  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseEnum>("pressure_diffusion_interpolation") =
      getParam<MooseEnum>("pressure_diffusion_interpolation");
  // Gravity and the reference point define the hydrostatic head the reduced pressure leaves out;
  // without gravity the object's face force is zero and p_rgh is the pressure
  if (parameters().isParamValid("gravity"))
    params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<Point>("reference_pressure_point") = getParam<Point>("reference_pressure_point");
  params.set<MooseFunctorName>("vof_rho_phi_functor") =
      getParam<MooseFunctorName>("vof_rho_phi_functor");

  if (!getParam<MooseFunctorName>("surface_tension_coefficient").empty())
  {
    if (getParam<MooseFunctorName>("volume_fraction_functor").empty())
      paramError("volume_fraction_functor",
                 "Surface tension requires the volume-fraction functor used by the least-squares "
                 "CSF curvature operator.");

    params.set<MooseFunctorName>("surface_tension_force_density_functor") =
        prefix() + "surface_tension_force_density";
  }

  addUserObject(object_type, rhieChowUOName(), params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addFunctorMaterials()
{
  WCNSLinearFVFlowPhysics::addFunctorMaterials();

  // The surface tension material forms its own least-squares gradient of the volume fraction, so
  // it does not depend on the geometry material and is created either way.
  addSurfaceTensionFunctorMaterial();

  if (!_create_geometry_functors)
    return;
  if (getParam<MooseFunctorName>("volume_fraction_functor").empty())
  {
    paramWarning("volume_fraction_functor",
                 "No 'volume_fraction_functor' was supplied, so the diffuse-interface geometry "
                 "functor material will not be created automatically.");
    return;
  }

  // The face-oriented interface unit normal of the volume-fraction compression flux. Its name is
  // what the volume-fraction physics expects in interface_normal_functor.
  const std::string class_name = "ConservativeDiffuseInterfaceGeometryFunctorMaterial";
  auto params = getFactory().getValidParams(class_name);
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>("volume_fraction_functor") =
      getParam<MooseFunctorName>("volume_fraction_functor");
  params.set<Real>("delta_n") = getParam<Real>("geometry_delta_n");
  params.set<MooseFunctorName>("interface_unit_normal_name") =
      prefix() + "interface_unit_normal_face";
  getProblem().addFunctorMaterial(class_name, prefix() + "diffuse_interface_geometry", params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::addSurfaceTensionFunctorMaterial()
{
  const auto surface_tension_coefficient =
      getParam<MooseFunctorName>("surface_tension_coefficient");
  if (surface_tension_coefficient.empty())
    return;

  const std::string class_name = "ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial";
  auto params = getFactory().getValidParams(class_name);
  assignBlocks(params, _blocks);

  params.set<MooseFunctorName>("volume_fraction_functor") =
      getParam<MooseFunctorName>("volume_fraction_functor");
  params.set<MooseFunctorName>("surface_tension_coefficient") = surface_tension_coefficient;
  params.set<MooseFunctorName>("interface_curvature_name") = prefix() + "interface_curvature";
  params.set<MooseFunctorName>("surface_tension_force_density_name") =
      prefix() + "surface_tension_force_density";

  getProblem().addFunctorMaterial(
      class_name, prefix() + "diffuse_interface_surface_tension", params);
}

unsigned short
WCNSLinearFVConservativeDiffuseInterfaceFlowPhysics::getNumberAlgebraicGhostingLayersNeeded() const
{
  // The interface normal is a cell gradient of the volume fraction, so a face on a process
  // boundary reaches one cell past the ghosted neighbour: two layers. The capillary force adds the
  // curvature, which is the divergence of that normal, so it needs the normal one cell further out
  // and the volume fraction one cell beyond that: three layers.
  const bool surface_tension = !getParam<MooseFunctorName>("surface_tension_coefficient").empty();
  return surface_tension ? 3 : 2;
}
