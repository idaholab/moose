//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEANISOTROPICMULTIGRAINEIGENSTRAIN_H
#define COMPUTEANISOTROPICMULTIGRAINEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"
#include "RankTwoTensor.h"
#include "GrainTrackerInterface.h"

class ComputeAnisotropicMultiGrainEigenstrain;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;
class EulerAngleProvider;
class RotationTensor;
class GrainTrackerInterface;

template <>
InputParameters validParams<ComputeAnisotropicMultiGrainEigenstrain>();

/**
 * ComputeAnisotropicMultiGrainEigenstrain is a class for models that compute
 * eigenstrains of an anisotropic material in a multi grain structure.
 */
class ComputeAnisotropicMultiGrainEigenstrain
  : public DerivativeMaterialInterface<ComputeEigenstrainBase>
{
public:
  ComputeAnisotropicMultiGrainEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;
  /*
   * Compute the total strain in each direction relative to the stress-free
   * temperatures at thevcurrent temperature, as well as the current
   * instantaneous thermal expansion coefficients and the constant eigenstrain
   * coefficients
   * param total_strain      The current total strain
   *                         (\delta L / L)
   * param instantaneous_cte The current instantaneous coefficient of thermal
   *                         expansion (derivative of thermal_strain wrt
   *                         temperature
   */
  // virtual void computeThermalStrain(Real & total_strain, Real & instantaneous_cte) = 0;

  // const VariableValue & _temperature;
  const MaterialProperty<Real> & _temperature;
  // MaterialProperty<RankTwoTensor> & _deigenstrain_dT;

  /// Anisotropic stress free temperatures
  const VariableValue & _stress_free_temperature_xx;
  const VariableValue & _stress_free_temperature_yy;
  const VariableValue & _stress_free_temperature_zz;

  /// Anisotropic constant eigenstrain values
  const Real & _constant_eigenstrain_xx;
  const Real & _constant_eigenstrain_yy;
  const Real & _constant_eigenstrain_zz;

  /// Anisotropic thermal expansion coefficients
  const Real & _thermal_expansion_coeff_xx;
  const Real & _thermal_expansion_coeff_yy;
  const Real & _thermal_expansion_coeff_zz;

  /// Number of order parameters
  const unsigned int _op_num;

  /// Order parameters
  std::vector<const VariableValue *> _vals;

  /// object providing the Euler angles
  const EulerAngleProvider & _euler;

  /// Grain tracker used to get unique grain IDs
  const GrainTrackerInterface & _grain_tracker;

  // /// Base name of the material system /////////////////////////////////////////
  // const std::string _orientation_name;
  // ///Stores the current orientation
  // MaterialProperty<Real> & _orientation;

};

#endif // COMPUTEANISOTROPICMULTIGRAINEIGENSTRAIN_H
