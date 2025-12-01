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
#include "NSFVUtils.h"
#include "FaceInfo.h"
#include "PetscVectorReader.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearSystem.h"

#include <cmath>
#include <limits>

registerMooseObject("NavierStokesApp", PorousRhieChowMassFlux);

InputParameters
PorousRhieChowMassFlux::validParams()
{
  auto params = RhieChowMassFlux::validParams();
  params.addClassDescription(
      "Rhie-Chow mass flux provider that carries porosity information and optional interface "
      "pressure jumps for porous media SIMPLE solves.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity functor.");
  MooseEnum eps_interp("harmonic arithmetic geometric", "harmonic");
  params.addParam<MooseEnum>("porosity_face_interpolation",
                             eps_interp,
                             "Interpolation used to project porosity-dependent quantities to faces "
                             "(harmonic, arithmetic, or geometric).");
  params.addParam<bool>("detect_porosity_jump_interfaces",
                        true,
                        "Automatically enable interface logic where porosity changes across a "
                        "face between neighboring blocks.");
  params.addParam<std::vector<BoundaryName>>(
      "porosity_jump_sidesets",
      {},
      "Sidesets that should be treated as porous interfaces with manual pressure jumps.");
  params.addParam<std::vector<Real>>("interface_pressure_jumps",
                                     {},
                                     "Constant pressure drops assigned to porosity jump sidesets.");
  params.addParam<std::vector<Real>>(
      "interface_form_factors",
      {},
      "Form factors applied at porosity interfaces to emulate irreversible head losses.");
  params.addParam<std::vector<Real>>(
      "interface_bernoulli_jumps",
      {},
      "Additional scaling applied to the Bernoulli jump term on the supplied sidesets.");
  return params;
}

PorousRhieChowMassFlux::PorousRhieChowMassFlux(const InputParameters & params)
  : RhieChowMassFlux(params),
    _eps(getFunctor<Real>(NS::porosity)),
    _eps_face_interp_method(getParam<MooseEnum>("porosity_face_interpolation")),
    _detect_block_interfaces(getParam<bool>("detect_porosity_jump_interfaces"))
{
  const auto sidesets = getParam<std::vector<BoundaryName>>("porosity_jump_sidesets");
  if (!sidesets.empty())
  {
    const auto ids = _moose_mesh.getBoundaryIDs(sidesets);
    const auto & constant_drops = getParam<std::vector<Real>>("interface_pressure_jumps");
    const auto & form_factors = getParam<std::vector<Real>>("interface_form_factors");
    const auto & bernoulli_factors = getParam<std::vector<Real>>("interface_bernoulli_jumps");

    if (!constant_drops.empty() && constant_drops.size() != ids.size())
      paramError("interface_pressure_jumps",
                 "Size mismatch with supplied 'porosity_jump_sidesets'.");
    if (!form_factors.empty() && form_factors.size() != ids.size())
      paramError(
          "interface_form_factors", "Size mismatch with supplied 'porosity_jump_sidesets'.");
    if (!bernoulli_factors.empty() && bernoulli_factors.size() != ids.size())
      paramError(
          "interface_bernoulli_jumps", "Size mismatch with supplied 'porosity_jump_sidesets'.");

    for (const auto i : index_range(ids))
    {
      JumpOptions opts;
      if (!constant_drops.empty())
        opts.constant = constant_drops[i];
      if (!form_factors.empty())
        opts.form_factor = form_factors[i];
      if (!bernoulli_factors.empty())
        opts.bernoulli = bernoulli_factors[i];
      _interface_jump_data.emplace(ids[i], opts);
    }
  }
}

bool
PorousRhieChowMassFlux::isManualInterfaceFace(const FaceInfo & fi) const
{
  if (_interface_jump_data.empty())
    return false;

  for (const auto & bd : fi.boundaryIDs())
    if (_interface_jump_data.count(bd))
      return true;
  return false;
}

bool
PorousRhieChowMassFlux::treatAsInterface(const FaceInfo & fi, const Moose::StateArg & time) const
{
  if (isManualInterfaceFace(fi))
    return true;

  if (!_detect_block_interfaces)
    return false;

  if (!fi.neighborPtr())
    return false;

  const auto [is_jump_face, eps_elem, eps_neighbor] = NS::isPorosityJumpFace(_eps, fi, time);
  libmesh_ignore(eps_elem);
  libmesh_ignore(eps_neighbor);
  return is_jump_face;
}

Real
PorousRhieChowMassFlux::evaluatePorosity(const Moose::FaceArg & face,
                                         const Moose::StateArg & time) const
{
  return _eps(face, time);
}

Real
PorousRhieChowMassFlux::evaluatePorosity(const Moose::ElemArg & elem_arg,
                                         const Moose::StateArg & time) const
{
  return _eps(elem_arg, time);
}

Real
PorousRhieChowMassFlux::interpolateFacePorosity(const FaceInfo & fi,
                                                const Moose::StateArg & /*time*/,
                                                Real elem_eps,
                                                Real neighbor_eps) const
{
  if (!fi.neighborPtr())
    return elem_eps;

  if (_eps_face_interp_method == "harmonic")
  {
    if (elem_eps <= std::numeric_limits<Real>::epsilon() ||
        neighbor_eps <= std::numeric_limits<Real>::epsilon())
      return 0.0;
    return 2.0 * elem_eps * neighbor_eps / (elem_eps + neighbor_eps);
  }
  else if (_eps_face_interp_method == "geometric")
  {
    return std::sqrt(std::max(elem_eps, 0.0) * std::max(neighbor_eps, 0.0));
  }
  else
  {
    return 0.5 * (elem_eps + neighbor_eps);
  }
}

void
PorousRhieChowMassFlux::initFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  for (const auto * fi : flowFaceInfo())
  {
    RealVectorValue density_times_velocity;

    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      const auto elem_arg = makeElemArg(fi->elemPtr());
      const auto neighbor_arg = makeElemArg(fi->neighborPtr());

      const Real elem_mass_density = _rho(elem_arg, time_arg) * evaluatePorosity(elem_arg, time_arg);
      const Real neighbor_mass_density =
          _rho(neighbor_arg, time_arg) * evaluatePorosity(neighbor_arg, time_arg);

      for (const auto dim_i : index_range(_vel))
        interpolate(InterpMethod::Average,
                    density_times_velocity(dim_i),
                    _vel[dim_i]->getElemValue(elem_info, time_arg) * elem_mass_density,
                    _vel[dim_i]->getElemValue(neighbor_info, time_arg) * neighbor_mass_density,
                    *fi,
                    true);
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

      const Real face_mass_density =
          _rho(boundary_face, time_arg) * evaluatePorosity(boundary_face, time_arg);

      for (const auto dim_i : index_range(_vel))
        density_times_velocity(dim_i) =
            boundary_normal_multiplier * face_mass_density *
            raw_value((*_vel[dim_i])(boundary_face, time_arg));
    }

    _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }
}

void
PorousRhieChowMassFlux::populateCouplingFunctors(
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv)
{
  using namespace Moose::FV;
  const auto time_arg = Moose::currentState();

  std::vector<PetscVectorReader> hbya_reader;
  for (const auto dim_i : index_range(raw_hbya))
    hbya_reader.emplace_back(*raw_hbya[dim_i]);

  std::vector<PetscVectorReader> ainv_reader;
  for (const auto dim_i : index_range(raw_Ainv))
    ainv_reader.emplace_back(*raw_Ainv[dim_i]);

  for (const auto * fi : flowFaceInfo())
  {
    Real face_mass_density = 0;
    RealVectorValue face_hbya;
    auto & Ainv = _Ainv[fi->id()];

    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];
      const auto neighbor_dof = neighbor_info.dofIndices()[_global_momentum_system_numbers[0]][0];

      const auto elem_arg = makeElemArg(fi->elemPtr());
      const auto neighbor_arg = makeElemArg(fi->neighborPtr());

      const Real elem_mass_density = _rho(elem_arg, time_arg) * evaluatePorosity(elem_arg, time_arg);
      const Real neighbor_mass_density =
          _rho(neighbor_arg, time_arg) * evaluatePorosity(neighbor_arg, time_arg);

      interpolate(Moose::FV::InterpMethod::Average,
                  face_mass_density,
                  elem_mass_density,
                  neighbor_mass_density,
                  *fi,
                  true);

      for (const auto dim_i : index_range(raw_hbya))
      {
        interpolate(Moose::FV::InterpMethod::Average,
                    face_hbya(dim_i),
                    hbya_reader[dim_i](elem_dof),
                    hbya_reader[dim_i](neighbor_dof),
                    *fi,
                    true);

        interpolate(Moose::FV::InterpMethod::Average,
                    Ainv(dim_i),
                    elem_mass_density * ainv_reader[dim_i](elem_dof),
                    neighbor_mass_density * ainv_reader[dim_i](neighbor_dof),
                    *fi,
                    true);
      }
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

      const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];
      const auto elem_arg = makeElemArg(elem_info.elem());

      const Real elem_mass_density = _rho(elem_arg, time_arg) * evaluatePorosity(elem_arg, time_arg);

      if (_vel[0]->isDirichletBoundaryFace(*fi))
      {
        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
        const Real boundary_mass_density =
            _rho(boundary_face, time_arg) * evaluatePorosity(boundary_face, time_arg);

        for (const auto dim_i : make_range(_dim))
        {
          face_hbya(dim_i) = -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, time_arg));

          if (!_body_force_kernel_names.empty())
            for (const auto & force_kernel : _body_force_kernels[dim_i])
            {
              force_kernel->setCurrentElemInfo(&elem_info);
              face_hbya(dim_i) -= force_kernel->computeRightHandSideContribution() *
                                  ainv_reader[dim_i](elem_dof) / elem_info.volume();
            }
          face_hbya(dim_i) *= boundary_normal_multiplier;
        }

        face_mass_density = boundary_mass_density;
      }
      else
      {
        face_mass_density = elem_mass_density;
        for (const auto dim_i : make_range(_dim))
          face_hbya(dim_i) = boundary_normal_multiplier * hbya_reader[dim_i](elem_dof);
      }

      for (const auto dim_i : index_range(raw_Ainv))
        Ainv(dim_i) = elem_mass_density * ainv_reader[dim_i](elem_dof);
    }

    _HbyA_flux[fi->id()] = face_hbya * fi->normal() * face_mass_density;
  }
}

Real
PorousRhieChowMassFlux::getVolumetricFaceFlux(const FaceInfo & fi) const
{
  const Moose::FaceArg face_arg{&fi,
                                Moose::FV::LimiterType::CentralDifference,
                                true,
                                /*correct_skewness*/ false,
                                &fi.elem(),
                                nullptr};
  const Real face_rho = _rho(face_arg, Moose::currentState());
  return libmesh_map_find(_face_mass_flux, fi.id()) / face_rho;
}

void
PorousRhieChowMassFlux::computeFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();
  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  for (const auto * fi : flowFaceInfo())
  {
    _p_diffusion_kernel->setupFaceData(fi);
    _p_diffusion_kernel->setCurrentFaceArea(1.0);

    Real p_grad_flux = 0.0;

    if (_p->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      const auto elem_dof = elem_info.dofIndices()[_global_pressure_system_number][0];
      const auto neighbor_dof = neighbor_info.dofIndices()[_global_pressure_system_number][0];

      Real p_elem_value = p_reader(elem_dof);
      Real p_neighbor_value = p_reader(neighbor_dof);

      bool downwind_is_elem = false;
      const auto [has_jump, jump_value] = computePressureJump(*fi, time_arg, downwind_is_elem);
      if (has_jump)
      {
        if (downwind_is_elem)
          p_elem_value -= jump_value;
        else
          p_neighbor_value -= jump_value;
      }

      const auto elem_matrix_contribution = _p_diffusion_kernel->computeElemMatrixContribution();
      const auto neighbor_matrix_contribution =
          _p_diffusion_kernel->computeNeighborMatrixContribution();
      const auto elem_rhs_contribution =
          _p_diffusion_kernel->computeElemRightHandSideContribution();

      p_grad_flux = (p_neighbor_value * neighbor_matrix_contribution +
                     p_elem_value * elem_matrix_contribution) -
                    elem_rhs_contribution;
    }
    else if (auto * bc_pointer = _p->getBoundaryCondition(*fi->boundaryIDs().begin()))
    {
      mooseAssert(fi->boundaryIDs().size() == 1, "We should only have one boundary on each face.");

      bc_pointer->setupFaceData(
          fi, fi->faceType(std::make_pair(_p->number(), _global_pressure_system_number)));

      const ElemInfo & elem_info =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
      const auto p_elem_value = _p->getElemValue(elem_info, time_arg);
      const auto matrix_contribution =
          _p_diffusion_kernel->computeBoundaryMatrixContribution(*bc_pointer);
      const auto rhs_contribution =
          _p_diffusion_kernel->computeBoundaryRHSContribution(*bc_pointer);

      p_grad_flux = (p_elem_value * matrix_contribution - rhs_contribution);
    }

    _face_mass_flux[fi->id()] = -_HbyA_flux[fi->id()] + p_grad_flux;
  }
}

std::pair<bool, Real>
PorousRhieChowMassFlux::computePressureJump(const FaceInfo & fi,
                                            const Moose::StateArg & time,
                                            bool & downwind_is_elem) const
{
  downwind_is_elem = false;

  if (!fi.neighborPtr())
    return {false, 0.0};

  if (!treatAsInterface(fi, time))
    return {false, 0.0};

  JumpOptions opts;
  if (isManualInterfaceFace(fi))
    for (const auto & bd : fi.boundaryIDs())
    {
      const auto it = _interface_jump_data.find(bd);
      if (it != _interface_jump_data.end())
      {
        opts.constant += it->second.constant;
        opts.form_factor += it->second.form_factor;
        opts.bernoulli += it->second.bernoulli;
      }
    }

  const auto elem_arg = makeElemArg(fi.elemPtr());
  const auto neighbor_arg = makeElemArg(fi.neighborPtr());
  const Real eps_elem = evaluatePorosity(elem_arg, time);
  const Real eps_neighbor = evaluatePorosity(neighbor_arg, time);
  const Real rho_elem = _rho(elem_arg, time);
  const Real rho_neighbor = _rho(neighbor_arg, time);

  RealVectorValue vel_elem;
  RealVectorValue vel_neighbor;
  for (const auto dim_i : index_range(_vel))
  {
    vel_elem(dim_i) = _vel[dim_i]->getElemValue(*fi.elemInfo(), time);
    vel_neighbor(dim_i) = _vel[dim_i]->getElemValue(*fi.neighborInfo(), time);
  }

  const Real v_dot_n_elem = vel_elem * fi.normal();
  const Real v_dot_n_neighbor = vel_neighbor * fi.normal();

  bool elem_is_upwind = true;
  if (std::abs(v_dot_n_elem) > 1e-12)
    elem_is_upwind = v_dot_n_elem >= 0.0;
  else if (std::abs(v_dot_n_neighbor) > 1e-12)
    elem_is_upwind = v_dot_n_neighbor <= 0.0;

  downwind_is_elem = !elem_is_upwind;

  const Real v_up = std::abs(elem_is_upwind ? v_dot_n_elem : v_dot_n_neighbor);
  const Real v_down = std::abs(elem_is_upwind ? v_dot_n_neighbor : v_dot_n_elem);
  const Real rho_up = elem_is_upwind ? rho_elem : rho_neighbor;
  const Real rho_down = elem_is_upwind ? rho_neighbor : rho_elem;
  const Real eps_up = elem_is_upwind ? eps_elem : eps_neighbor;
  const Real eps_down = elem_is_upwind ? eps_neighbor : eps_elem;

  Real delta_p = 0.5 * rho_down * v_down * v_down - 0.5 * rho_up * v_up * v_up;

  if (opts.constant != 0.0)
    delta_p += opts.constant;

  if (opts.form_factor != 0.0)
  {
    if (eps_down + 1e-12 < eps_up)
      delta_p += 0.5 * opts.form_factor * rho_down * v_down * v_down;
    else if (eps_up + 1e-12 < eps_down)
      delta_p -= 0.5 * opts.form_factor * rho_up * v_up * v_up;
    else
      delta_p +=
          0.5 * opts.form_factor * (rho_down * v_down * v_down - rho_up * v_up * v_up);
  }

  if (opts.bernoulli != 0.0)
    delta_p += 0.5 * opts.bernoulli * (rho_down * v_down * v_down - rho_up * v_up * v_up);

  if (std::abs(delta_p) < std::numeric_limits<Real>::epsilon())
    return {false, 0.0};

  return {true, delta_p};
}
