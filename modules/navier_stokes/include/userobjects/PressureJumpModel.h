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
 * Interface for pressure-jump models evaluated by PorousRhieChowMassFlux.
 *
 * Pressure-jump models do not execute independently. The Rhie-Chow object evaluates them with its
 * current face mass flux and manages the relaxed jump state.
 */
class PressureJumpModel : public GeneralUserObject, public NonADFunctorInterface
{
public:
  /// Return the input parameters shared by pressure-jump models.
  static InputParameters validParams();

  /**
   * Construct a pressure-jump model for the configured boundaries.
   * @param params The input parameters for the model
   */
  PressureJumpModel(const InputParameters & params);

  /**
   * Return whether this model applies to a face.
   * @param fi The face to query
   */
  bool appliesTo(const FaceInfo & fi) const;

  /**
   * Return whether the FaceInfo element is the oriented owner of the pressure jump.
   * @param fi The face to query
   */
  bool elemIsOwner(const FaceInfo & fi) const;

  /**
   * Compute pressure on the non-owner side minus pressure on the owner side.
   * @param fi The face carrying the pressure jump
   * @param face_mass_flux The signed mass flux through the face
   */
  virtual Real computePressureJump(const FaceInfo & fi, Real face_mass_flux) const = 0;

  /**
   * Return whether pressure reconstruction should use independent values from each face side.
   * @param fi The face carrying the pressure jump
   */
  virtual bool useOneSidedReconstruction(const FaceInfo & fi) const = 0;

  /// Boundary IDs handled by this model.
  const std::unordered_set<BoundaryID> & boundaryIDs() const { return _boundary_ids; }

protected:
  ///@{ No-op execution hooks because the Rhie-Chow object evaluates this model directly.
  void initialize() final {}
  void execute() final {}
  void finalize() final {}
  ///@}

  /// Mesh used to resolve boundary names and determine pressure-jump orientation.
  MooseMesh & _moose_mesh;

private:
  /// Boundary IDs on which this pressure-jump model applies.
  std::unordered_set<BoundaryID> _boundary_ids;
};
