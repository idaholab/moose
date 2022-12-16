//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PCNSFVKT.h"
#include <memory>
#include <unordered_map>
#include <utility>

namespace Moose
{
namespace FV
{
template <typename>
class Limiter;
}
}
class SinglePhaseFluidProperties;
class FaceInfo;

class PCNSFVKTDC : public PCNSFVKT
{
public:
  static InputParameters validParams();
  PCNSFVKTDC(const InputParameters & params);
  void timestepSetup() override;
  void residualSetup() override;
  void jacobianSetup() override;

protected:
  virtual ADReal computeQpResidual() override;
  Real getOldFlux(bool upwind) const;

  std::unique_ptr<Moose::FV::Limiter<ADReal>> _upwind_limiter;
  std::unordered_map<dof_id_type, Real> & _old_upwind_fluxes;
  /// Old high order fluxes
  std::unordered_map<dof_id_type, Real> & _old_ho_fluxes;
  std::unordered_map<dof_id_type, Real> & _current_upwind_fluxes;
  /// Current high order fluxes
  std::unordered_map<dof_id_type, Real> & _current_ho_fluxes;
  const Real _ho_implicit_fraction;
};
