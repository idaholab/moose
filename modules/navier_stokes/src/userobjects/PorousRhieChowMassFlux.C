//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousRhieChowMassFlux.h"

#include "MooseMesh.h"
#include "NS.h"
#include "FVPressureJumpGreenGaussGradient.h"
#include "FVReconstructedPressureGradient.h"
#include "LinearSystem.h"
#include "LinearFVPressureCorrectionDiffusion.h"
#include "LinearFVPressureCorrectionDiffusionJump.h"
#include "LinearPWCNSFVMomentumPressure.h"
#include "MooseLinearVariableFV.h"
#include "PressureJumpInterface.h"
#include "PressureJumpModel.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", PorousRhieChowMassFlux);

InputParameters
PorousRhieChowMassFlux::validParams()
{
  InputParameters params = RhieChowMassFlux::validParams();
  params.addClassDescription(
      "Rhie-Chow mass flux object specialized for porous flow/baffle cases.");
  params.set<MooseEnum>("pressure_diffusion_interpolation") = "harmonic";

  params.addParam<MooseFunctorName>(
      NS::porosity, "1", "Porosity functor (defaults to 1 for non-porous calculations).");
  params.addParam<std::vector<UserObjectName>>(
      "pressure_jump_models", {}, "Pressure jump models applied by this object.");
  params.addRangeCheckedParam<Real>("pressure_jump_relaxation",
                                    1.0,
                                    "0.0<pressure_jump_relaxation<=1.0",
                                    "Under-relaxation factor for pressure jump updates.");
  return params;
}

PorousRhieChowMassFlux::PorousRhieChowMassFlux(const InputParameters & params)
  : RhieChowMassFlux(params),
    _eps(getFunctor<Real>(NS::porosity)),
    _pressure_jump_relaxation(getParam<Real>("pressure_jump_relaxation")),
    _baffle_jump(
        declareRestartableData<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            "baffle_jump", _moose_mesh, blockIDs(), "baffle_jump"))
{
  std::unordered_set<BoundaryID> pressure_jump_boundary_ids;
  for (const auto & model_name : getParam<std::vector<UserObjectName>>("pressure_jump_models"))
  {
    const auto & model = getUserObjectByName<PressureJumpModel>(model_name);
    for (const auto boundary_id : model.boundaryIDs())
      if (!pressure_jump_boundary_ids.insert(boundary_id).second)
        paramError("pressure_jump_models",
                   "Pressure jump models must not act on overlapping boundaries.");
    _pressure_jump_models.push_back(&model);

    // A jump model evaluating different porosity or density than this object would act on a
    // flow that is not the one being solved.
    for (const auto & [model_param, rc_param] :
         {std::make_pair("porosity", NS::porosity), std::make_pair("density", NS::density)})
      if (model.parameters().have_parameter<MooseFunctorName>(model_param) &&
          model.getParam<MooseFunctorName>(model_param) != getParam<MooseFunctorName>(rc_param))
        paramError("pressure_jump_models",
                   "Pressure jump model '",
                   model.name(),
                   "' uses ",
                   model_param,
                   " '",
                   model.getParam<MooseFunctorName>(model_param),
                   "' but this object uses '",
                   getParam<MooseFunctorName>(rc_param),
                   "'.");
  }
}

void
PorousRhieChowMassFlux::linkMomentumPressureSystems(
    const std::vector<LinearSystem *> & momentum_systems,
    LinearSystem & pressure_system,
    const std::vector<unsigned int> & momentum_system_numbers)
{
  RhieChowMassFlux::linkMomentumPressureSystems(
      momentum_systems, pressure_system, momentum_system_numbers);

  // The porous momentum pressure kernels must weight the pressure gradient with the porosity
  // this object uses to form the face fluxes.
  for (const auto system_number : momentum_system_numbers)
  {
    std::vector<LinearFVElementalKernel *> kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribThread>(_tid)
        .template condition<AttribSystem>("LinearFVElementalKernel")
        .template condition<AttribSysNum>(system_number)
        .queryInto(kernels);

    for (const auto * const kernel : kernels)
      if (const auto * const pressure_kernel =
              dynamic_cast<const LinearPWCNSFVMomentumPressure *>(kernel);
          pressure_kernel && MooseUtils::setsIntersect(pressure_kernel->blockIDs(), blockIDs()) &&
          pressure_kernel->getParam<MooseFunctorName>(NS::porosity) !=
              getParam<MooseFunctorName>(NS::porosity))
        paramError(NS::porosity,
                   "Momentum pressure kernel '",
                   pressure_kernel->name(),
                   "' uses porosity '",
                   pressure_kernel->getParam<MooseFunctorName>(NS::porosity),
                   "' but this object uses '",
                   getParam<MooseFunctorName>(NS::porosity),
                   "'.");
  }

  if (hasPressureBaffles() &&
      !dynamic_cast<const LinearFVPressureCorrectionDiffusionJump *>(_p_diffusion_kernel))
    paramError("p_diffusion_kernel",
               "Pressure jump models require LinearFVPressureCorrectionDiffusionJump.");

  auto * const pressure_var =
      dynamic_cast<MooseLinearVariableFVReal *>(&_pressure_system->getVariable(0, _p->number()));
  mooseAssert(pressure_var, "The pressure variable type was checked by RhieChowMassFlux.");
  const auto & solution_gradient_reader = pressure_var->requestCellGradients();

  if (hasPressureBaffles())
  {
    auto * const jump_gradient_method = dynamic_cast<FVPressureJumpGreenGaussGradient *>(
        &_fe_problem.getFVGradientMethod(solution_gradient_reader.method().name(), _tid));
    if (!jump_gradient_method)
      mooseError("Pressure variable '",
                 pressure_var->name(),
                 "' must use FVPressureJumpGreenGaussGradient as its default gradient method "
                 "when PorousRhieChowMassFlux '",
                 name(),
                 "' has pressure baffles.");

    jump_gradient_method->linkFlowSystem(*this, solution_gradient_reader);

    if (usingReconstructedPressureGradientMethod())
    {
      if (&basePressureGradientField().method() != &solution_gradient_reader.method())
        mooseError("FVReconstructedPressureGradient '",
                   reconstructedGradientMethod().name(),
                   "' must use the pressure variable's FVPressureJumpGreenGaussGradient as its "
                   "base_gradient_method when PorousRhieChowMassFlux '",
                   name(),
                   "' has pressure baffles.");
    }
    else if (&pressureGradientField().method() != &solution_gradient_reader.method())
      mooseError("Momentum pressure kernels coupled to PorousRhieChowMassFlux '",
                 name(),
                 "' must use the pressure variable's FVPressureJumpGreenGaussGradient or an "
                 "FVReconstructedPressureGradient based on it when pressure baffles are present.");
  }

  setupPorousMeshInformation();
}

void
PorousRhieChowMassFlux::computeHbyA(bool verbose)
{
  // This runs before every pressure corrector, so PIMPLE relaxes the jump once per PISO corrector
  // and the effective relaxation per outer iteration grows with num_piso_iterations. Without a
  // reconstructed pressure gradient, all those updates use the same face flux.
  updateBaffleJumps();

  // Pressure jumps are external data for the variable's gradient method. Refresh that field
  // before pressure assembly even though the pressure solution itself has not changed yet.
  if (hasPressureBaffles())
    _pressure_system->updateFVGradient(basePressureGradientField());

  RhieChowMassFlux::computeHbyA(verbose);
}

void
PorousRhieChowMassFlux::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  _baffle_jump.clear();
  setupPorousMeshInformation();
}

void
PorousRhieChowMassFlux::setupPorousMeshInformation()
{
  const auto time_arg = Moose::currentState();
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
    if (hasBlocks(elem_info->subdomain_id()))
    {
      const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];
      const Real cell_volume = elem_info->volume() * elem_info->coordFactor();
      _cell_volumes->set(elem_dof, cell_volume * _eps(makeElemArg(elem_info->elem()), time_arg));
    }

  _cell_volumes->close();
}

void
PorousRhieChowMassFlux::initCouplingField()
{
  RhieChowMassFlux::initCouplingField();

  for (auto & fi : _fe_problem.mesh().faceInfo())
    _baffle_jump[fi->id()];
}

Real
PorousRhieChowMassFlux::getFaceSidePorosity(const FaceInfo & fi,
                                            bool elem_side,
                                            const Moose::StateArg & time) const
{
  const Elem * const elem = elem_side ? fi.elemPtr() : fi.neighborPtr();
  if (!elem)
    return 1.0;

  return _eps(makeElemArg(elem), time);
}

Real
PorousRhieChowMassFlux::getSignedBaffleJump(const FaceInfo & fi, bool elem_side) const
{
  const auto * const pressure_jump_model = getPressureJumpModel(fi);
  if (!pressure_jump_model)
    return 0.0;

  const Real J = _baffle_jump.evaluate(&fi);
  const bool elem_is_owner = pressure_jump_model->elemIsOwner(fi);
  // J is stored as (p_non_owner - p_owner), so the owner side sees -J.
  return (elem_side == elem_is_owner) ? -J : J;
}

void
PorousRhieChowMassFlux::updateBaffleJumps()
{
  if (_pressure_jump_models.empty())
    return;

  for (auto & fi : _flow_face_info)
  {
    const auto * const pressure_jump_model = getPressureJumpModel(*fi);
    if (!pressure_jump_model || !fi->neighborPtr())
      continue;
    if (!hasBlocks(fi->elemPtr()->subdomain_id()) || !hasBlocks(fi->neighborPtr()->subdomain_id()))
      continue;

    const Real new_jump =
        pressure_jump_model->computePressureJump(*fi, _face_mass_flux.evaluate(fi));
    _baffle_jump[fi->id()] = _pressure_jump_relaxation * new_jump +
                             (1.0 - _pressure_jump_relaxation) * _baffle_jump[fi->id()];
  }
}

bool
PorousRhieChowMassFlux::faceIsBaffle(const FaceInfo & fi) const
{
  return getPressureJumpModel(fi) != nullptr;
}

Real
PorousRhieChowMassFlux::cellPressureDiffusionCoefficient(const ElemInfo & elem_info,
                                                         const unsigned int component) const
{
  mooseAssert(component < _Ainv_raw.size(), "Invalid pressure-diffusion component.");

  const auto momentum_dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  const Real density = _rho(makeElemArg(elem_info.elem()), Moose::currentState());
  return density * (*_Ainv_raw[component])(momentum_dof);
}

bool
PorousRhieChowMassFlux::pressureDiffusionDataReady() const
{
  return _p_diffusion_kernel && _Ainv_raw.size() == dimension();
}

bool
PorousRhieChowMassFlux::pressureDiffusionUsesNonorthogonalCorrection() const
{
  mooseAssert(pressureDiffusionDataReady(), "Pressure-diffusion data must be ready first.");
  return _p_diffusion_kernel->useNonorthogonalCorrection();
}

NS::FV::PressureJumpInterfaceData
PorousRhieChowMassFlux::pressureJumpInterfaceData(const FaceInfo & fi,
                                                  const RealVectorValue & elem_gradient,
                                                  const RealVectorValue & neighbor_gradient,
                                                  const Real face_area) const
{
  NS::FV::PressureJumpInterfaceData data;
  if (!fi.neighborPtr() || !faceIsBaffle(fi))
    return data;

  mooseAssert(pressureDiffusionDataReady(),
              "Pressure-diffusion data must be ready before constructing a baffle face operator.");

  const auto & elem_info = *fi.elemInfo();
  const auto & neighbor_info = *fi.neighborInfo();
  RealVectorValue elem_diffusion;
  RealVectorValue neighbor_diffusion;
  for (const auto component : make_range(dimension()))
  {
    elem_diffusion(component) = cellPressureDiffusionCoefficient(elem_info, component);
    neighbor_diffusion(component) = cellPressureDiffusionCoefficient(neighbor_info, component);
  }

  data = NS::FV::pressureJumpInterfaceData(fi.normal(),
                                           fi.faceCentroid() - elem_info.centroid(),
                                           neighbor_info.centroid() - fi.faceCentroid(),
                                           elem_diffusion,
                                           neighbor_diffusion,
                                           elem_gradient,
                                           neighbor_gradient,
                                           face_area,
                                           pressureDiffusionUsesNonorthogonalCorrection(),
                                           getSignedBaffleJump(fi, /*elem_side=*/true));

  // A valid mesh gives positive projected half-cell distances, and a positive momentum diagonal
  // and density give positive conductances.
  mooseAssert(data.valid,
              "Cannot construct the pressure-jump operator on face " + std::to_string(fi.id()) +
                  ": the half-cell distances, normal pressure diffusion coefficients, face area, "
                  "and conductances must be finite and positive, and the corrections finite.");

  return data;
}

const PressureJumpModel *
PorousRhieChowMassFlux::getPressureJumpModel(const FaceInfo & fi) const
{
  for (const auto * const model : _pressure_jump_models)
    if (model->appliesTo(fi))
      return model;

  return nullptr;
}
