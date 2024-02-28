//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "SubblockIndexProvider.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"
#include "GeneralizedPlaneStrainUOInterface.h"

class Function;

class GeneralizedPlaneStrainUserObject : public ElementUserObject,
                                         public GeneralizedPlaneStrainUOInterface
{
public:
  static InputParameters validParams();

  GeneralizedPlaneStrainUserObject(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject & uo) override;
  void finalize() override;
  virtual Real returnResidual(unsigned int scalar_var_id = 0) const override;
  virtual Real returnReferenceResidual(unsigned int scalar_var_id = 0) const override;
  virtual Real returnJacobian(unsigned int scalar_var_id = 0) const override;

protected:
  /// Base name of the material system
  const std::string _base_name;

  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// The stress tensor
  const MaterialProperty<RankTwoTensor> & _stress;

  /// A Userobject that carries the subblock ID for all elements
  const SubblockIndexProvider * _subblock_id_provider;

  /// Function defining applied out-of-plane pressure
  const Function * _out_of_plane_pressure_function;
  /// Material property defining applied out-of-plane pressure
  const MaterialProperty<Real> & _out_of_plane_pressure_material;
  /// Factor applied to out-of-plane pressure applied by function and material
  const Real _pressure_factor;

  /// The direction of the out-of-plane strain scalar variable
  unsigned int _scalar_out_of_plane_strain_direction;
  std::vector<Real> _residual;
  std::vector<Real> _reference_residual;
  std::vector<Real> _jacobian;
};
