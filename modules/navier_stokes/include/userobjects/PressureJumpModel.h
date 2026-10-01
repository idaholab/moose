//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralUserObject.h"
#include "NonADFunctorInterface.h"

#include <unordered_set>

class FaceInfo;

/**
 * Base class for pressure jump models called by PorousRhieChowMassFlux.
 */
class PressureJumpModel : public GeneralUserObject, public NonADFunctorInterface
{
public:
  static InputParameters validParams();
  PressureJumpModel(const InputParameters & params);

  /// Whether this model applies to the face.
  bool appliesTo(const FaceInfo & fi) const;

  /// Return whether the FaceInfo element is the owner of the pressure jump.
  bool elemIsOwner(const FaceInfo & fi) const;

  /// Compute p_non_owner - p_owner for the supplied face mass flux.
  virtual Real computePressureJump(const FaceInfo & fi, Real face_mass_flux) const = 0;

  /// Whether pressure reconstruction should be one-sided on the supplied face.
  virtual bool useOneSidedReconstruction(const FaceInfo & fi) const = 0;

  /// Boundary IDs handled by this model.
  const std::unordered_set<BoundaryID> & boundaryIDs() const { return _boundary_ids; }

protected:
  void initialize() final {}
  void execute() final {}
  void finalize() final {}

  MooseMesh & _moose_mesh;

private:
  std::unordered_set<BoundaryID> _boundary_ids;
};
