//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "INSFVVelocityVariable.h"
#include "NS.h"

/**
 * Computes wall y+ based on wall functions.
 */
class RANSYPlusAux : public AuxKernel
{
public:
  static InputParameters validParams();

  virtual void initialSetup() override;

  RANSYPlusAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// the dimension of the simulation
  const unsigned int _dim;

  /// x-velocity
  const Moose::Functor<Real> & _u_var;
  /// y-velocity
  const Moose::Functor<Real> * _v_var;
  /// z-velocity
  const Moose::Functor<Real> * _w_var;

  /// Turbulent kinetic energy
  const Moose::Functor<Real> * _k;

  /// Density
  const Moose::Functor<Real> & _rho;

  /// Dynamic viscosity
  const Moose::Functor<Real> & _mu;

  /// Wall boundary names
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Method used for wall treatment
  NS::WallTreatmentEnum _wall_treatment;

  /// C_mu constant
  const Real _C_mu;

  ///@{
  /// Maps for wall treatement
  std::unordered_set<const Elem *> _wall_bounded;
  std::map<const Elem *, std::vector<Real>> _dist;
  std::map<const Elem *, std::vector<const FaceInfo *>> _face_infos;
  ///@}
};
