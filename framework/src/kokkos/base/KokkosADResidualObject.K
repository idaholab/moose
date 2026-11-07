//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// KokkosKernels must see its headers before the MOOSE Kokkos macros, as in KokkosArray.K, because
// unity builds compile this file ahead of it
#include "KokkosBlas.hpp"

#include "KokkosADResidualObject.h"

#include "FEProblemBase.h"
#include "MooseVariableBase.h"
#include "SystemBase.h"

namespace Moose::Kokkos
{

ADAccessorRegistration
ADSlotSetup::add(const Variable & var, const bool gradient)
{
  _requests.push_back({var, gradient});

  // The seed of a time derivative is the derivative of the time derivative with respect to the
  // solution, which the time integrator owns
  const Real * seed_source = nullptr;
  if (var.dot())
    seed_source = &const_cast<Variable &>(var).mooseVar()->sys().duDotDu(var.var());

  return {static_cast<unsigned int>(_requests.size() - 1), seed_source};
}

unsigned int
ADSlotSetup::addProperty(const std::unordered_map<SubdomainID, std::vector<ADSlotBlock>> & inputs)
{
  _property_requests.push_back({&inputs});
  return _property_requests.size() - 1;
}

bool
ADSlotSetup::seeded(const Variable & ivar, const unsigned int sys, const unsigned int var) const
{
  const auto & residual_var = *const_cast<Variable &>(ivar).mooseVar();
  const auto & problem = residual_var.sys().feProblem();
  const auto residual_sys = residual_var.sys().number();

  return sys == residual_sys && problem.areCoupled(ivar.var(), var, residual_sys);
}

bool
ADSlotSetup::seeded(const Variable * ivar, const Variable & var, const unsigned int comp) const
{
  // Derivatives exist with respect to the current state of nonlinear variables
  if (!var.coupled() || var.old())
    return false;

  const auto * moose_var = const_cast<Variable &>(var).mooseVar(comp);
  if (moose_var->kind() != Moose::VarKindType::VAR_SOLVER)
    return false;

  if (!ivar)
    return true;

  // A residual object differentiates with respect to the variables of the residual system that the
  // coupling matrix couples to the residual variable
  const auto & residual_var = *const_cast<Variable *>(ivar)->mooseVar();
  const auto & problem = residual_var.sys().feProblem();
  const auto residual_sys = residual_var.sys().number();

  if (var.sys(comp) != residual_sys)
    return false;

  return problem.areCoupled(ivar->var(), var.var(comp), residual_sys);
}

void
ADSlotSetup::assign(const MooseObject & object, const Variable & ivar, const unsigned int max_slots)
{
  assign(object, ivar, max_slots, Variable());
}

void
ADSlotSetup::assign(const MooseObject & object,
                    const Variable & ivar,
                    const unsigned int max_slots,
                    const Variable & displacements)
{
  assignSlots(object, &ivar, max_slots, displacements);
}

void
ADSlotSetup::assign(const MooseObject & object,
                    const unsigned int max_slots,
                    const Variable & displacements,
                    const Mesh & mesh)
{
  assignSlots(object, nullptr, max_slots, displacements, &mesh);
}

void
ADSlotSetup::assign(const MooseObject & object,
                    const Variable & ivar,
                    const unsigned int max_slots,
                    const Variable & displacements,
                    const Mesh & mesh)
{
  assignSlots(object, &ivar, max_slots, displacements, &mesh);
}

void
ADSlotSetup::assignSlots(const MooseObject & object,
                         const Variable * ivar,
                         const unsigned int max_slots,
                         const Variable & displacements,
                         const Mesh * mesh)
{
  _table = ADSlotTable();

  for (const auto r : index_range(_requests))
  {
    const auto & request = _requests[r];
    const auto & var = request.var;

    _table.setAccessorFirst(r,
                            seeded(ivar, var) ? _table.add(var.sys(), var.var(), request.gradient)
                                              : libMesh::invalid_uint);
  }

  // Inputs of the AD material properties that the object reads, other than displacements, share
  // the slot blocks of the accessors reading the same quantity. Displacement inputs map to the
  // displacement slots, which the object carries when it is displaced or when a property it reads
  // depends on the displacements, provided that a displacement component is seeded.
  std::vector<std::pair<unsigned int, unsigned int>> displacement_components;

  if (displacements.coupled())
    for (const auto c : make_range(displacements.components()))
      displacement_components.emplace_back(displacements.sys(c), displacements.var(c));

  for (const auto & request : _property_requests)
    for (const auto & [_, inputs] : *request.inputs)
      for (const auto & input : inputs)
      {
        if (input.displacement_component != libMesh::invalid_uint)
        {
          if (displacement_components.empty())
            displacement_components.resize(AD_DISPLACEMENT_SLOTS /
                                               (AD_VALUE_SLOTS + AD_GRADIENT_SLOTS),
                                           {libMesh::invalid_uint, libMesh::invalid_uint});

          auto & component = displacement_components[input.displacement_component];
          if (component.first == libMesh::invalid_uint)
            component = {input.sys, input.var};
          else if (component != std::make_pair(input.sys, input.var))
            object.mooseError("An AD material property read by this object depends on a "
                              "displacement variable that differs from the displacement of the "
                              "same component of another property or of the object.");
          continue;
        }

        if (!ivar || seeded(*ivar, input.sys, input.var))
          _table.add(input.sys, input.var, input.gradient);
      }

  if (_table.numSlots() > max_slots)
    object.mooseError("The pointwise AD computation of this object reads ",
                      _table.numSlots(),
                      " derivative slots, more than its declared max_ad_slots of ",
                      max_slots,
                      ". Increase max_ad_slots in the class to the number of value and gradient "
                      "slots its computation depends on.");

  // Trailing unused components of a property dependence are dropped
  while (!displacement_components.empty() &&
         displacement_components.back().first == libMesh::invalid_uint)
    displacement_components.pop_back();

  if (!displacement_components.empty())
  {
    std::vector<bool> seeded_components(displacement_components.size());
    for (const auto c : index_range(displacement_components))
    {
      const auto [sys, var] = displacement_components[c];

      if (sys == libMesh::invalid_uint)
        seeded_components[c] = false;
      else if (displacements.coupled())
        seeded_components[c] = seeded(ivar, displacements, c);
      else
        seeded_components[c] = !ivar || seeded(*ivar, sys, var);
    }

    // Unseeded displacements carry no derivatives. Without displacement slots the object
    // dispatches the narrower instantiation, whose plain geometry already includes the
    // displacements.
    if (std::find(seeded_components.begin(), seeded_components.end(), true) !=
        seeded_components.end())
    {
      // The displacement slots follow the max_ad_slots of the accessors, so that their position is
      // the same for every object of a class
      _table.reserve(max_slots);
      _table.addDisplacements(displacement_components, seeded_components);
    }
  }

  if (mesh)
    assignProperties(ivar, *mesh);

  _table.finalize();
}

void
ADSlotSetup::assignProperties(const Variable * ivar, const Mesh & mesh)
{
  const auto num_subdomains = mesh.getNumSubdomains();

  for (const auto p : index_range(_property_requests))
  {
    const auto & request = _property_requests[p];

    // A subdomain where the property has no inputs keeps an empty map
    for (const auto sub : make_range(num_subdomains))
      _table.setPropertySlots(p, num_subdomains, sub, {});

    for (const auto & [subdomain, inputs] : *request.inputs)
    {
      unsigned int width = 0;
      for (const auto & input : inputs)
        width =
            std::max(width, input.first + (input.gradient ? AD_GRADIENT_SLOTS : AD_VALUE_SLOTS));

      std::vector<unsigned int> slots(width, libMesh::invalid_uint);

      for (const auto & input : inputs)
      {
        unsigned int first = libMesh::invalid_uint;

        if (input.displacement_component != libMesh::invalid_uint)
        {
          const auto c = input.displacement_component;
          first =
              input.gradient ? _table.displacementGradientSlot(c) : _table.displacementValueSlot(c);
        }
        else if (!ivar || seeded(*ivar, input.sys, input.var))
          for (const auto & block : _table.hostBlocks())
            if (block.displacement_component == libMesh::invalid_uint && block.sys == input.sys &&
                block.var == input.var && block.gradient == input.gradient)
              first = block.first;

        if (first == libMesh::invalid_uint)
          continue;

        for (const auto k : make_range(input.gradient ? AD_GRADIENT_SLOTS : AD_VALUE_SLOTS))
          slots[input.first + k] = first + k;
      }

      _table.setPropertySlots(p, num_subdomains, mesh.getContiguousSubdomainID(subdomain), slots);
    }
  }
}

} // namespace Moose::Kokkos
