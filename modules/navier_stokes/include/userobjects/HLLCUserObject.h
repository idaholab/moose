//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideUserObject.h"
#include "MooseMesh.h"

using side_type = std::pair<const Elem *, unsigned int>;

// Forward Declarations
class HLLCUserObject;
class SinglePhaseFluidProperties;

class HLLCUserObject : public InternalSideUserObject
{
public:
  static InputParameters validParams();

  HLLCUserObject(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual void threadJoin(const UserObject & y) override;

  /// accessor for the wave speed
  std::vector<ADReal> waveSpeed(const Elem * elem, unsigned int side) const;

  /// Query whether this processor has data for the provided element and side
  bool hasData(const Elem * elem, unsigned int side) const;

protected:
  /// helper function for returning the FaceInfo object for an elem/side pair
  const FaceInfo & faceInfoHelper(const Elem * elem, unsigned int side) const;

  /// quadrature point dummy
  unsigned int _qp = 0;

  /// fluid properties
  const SinglePhaseFluidProperties & _fluid;

  /// FV face info from MooseMesh
  const std::vector<const FaceInfo *> & _face_info;

  /// face info lookup allows searching for face info entry from elem/side pair
  std::map<side_type, unsigned int> _side_to_face_info;

  /// data structure storing the wave speeds SL, SM, SR
  std::map<side_type, std::vector<ADReal>> _wave_speed;

  ///@{ material properties computed by VarMat that Riemann solver needs
  const ADMaterialProperty<RealVectorValue> & _vel_elem;
  const ADMaterialProperty<RealVectorValue> & _vel_neighbor;
  const ADMaterialProperty<Real> & _speed_elem;
  const ADMaterialProperty<Real> & _speed_neighbor;
  const ADMaterialProperty<Real> & _pressure_elem;
  const ADMaterialProperty<Real> & _pressure_neighbor;
  const ADMaterialProperty<Real> & _rho_elem;
  const ADMaterialProperty<Real> & _rho_neighbor;
  const ADMaterialProperty<Real> & _specific_internal_energy_elem;
  const ADMaterialProperty<Real> & _specific_internal_energy_neighbor;
  const MaterialProperty<Real> * const _eps_elem;
  const MaterialProperty<Real> * const _eps_neighbor;
  ///@}
};
