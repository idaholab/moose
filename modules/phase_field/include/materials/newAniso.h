/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#pragma once

#include "DerivativeFunctionMaterialBase.h"
#include "EulerAngleProvider.h"
#include "RotationMatrix.h"
#include "RotationTensor.h"

#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"

class AnisoGBEnergyMaterial;

template <>
InputParameters validParams<AnisoGBEnergyMaterial>();

class AnisoGBEnergyMaterial : public DerivativeFunctionMaterialBase
{
public:
  AnisoGBEnergyMaterial(const InputParameters & parameters);

protected:
  const unsigned int _op_num;

  virtual Real computeF();


  //////////
  //THIS WILL BE A VECTOR WHEN WE ARE FINISHED
  //////////
  virtual Real computeDF(unsigned int j_var);

  /**
   * This calculates the energy given the two orientations
   *
   * @param P the orientation matrix of  one grain
   * @param S the orientation matrix of the other grain
   */
  Real gB5DOF(RealTensorValue P, RealTensorValue S, std::string whichComponent = "None");

  /**
   * This function determines the distances as defined in Bulatov et. al from the
   * actual rotation to the different ideal rotations around <100>, <110> and <111>
   * this assumes the grain boundary normal is at [100] it discards distances that are >1
   *
   * @param P the orientation matrix for one grain
   * @param S the orientation matrix for the other grain
   * @param geom holds the different values related to the ideal rotation (distances, ksi, eta, phi)
   * as defined in Bulatov et. al
   * @param axes a vector of the rotation axes in a set (i.e. all of the <100> axes)
   * @param dirs orthogonal axes to axes
   */
  void distancesToSet(const RealTensorValue & P,
                      RealTensorValue & S,
                      std::vector<std::vector<Real>> & geom,
                      const std::vector<RealVectorValue> & axes,
                      const std::vector<RealVectorValue> & dirs,
                      std::string whichComponent);

  /**
   * sorts the geom vector by (distances, ksi, eta, phi) smallest to largest
   *
   * @params geom hold values related to the geometry of idealized rotations
   */
  void sortGeom(std::vector<std::vector<Real>> & geom);

  // simple quaternion typedef
  typedef Real Quaternion[4];

  /**
   * Converts a quaternion into a matrix
   *
   * @param q a quaternion
   * @param M the matrix formed by q
   */
  void quat2Mat(const Quaternion & q, RealTensorValue & M);

  /**
   * Converts a matrix into a quaternion
   *
   * @param M a matrix
   * @param q the quaternion formed by M
   */
  void mat2Quat(const RealTensorValue & M, Quaternion & q);

  /**
   * calculates the energy from the geom vectors. It calculates the energy for
   * different ideal rotations using the ksi, eta and phi from the geom vectors
   * and then uses a weighting function to calculate the energy at an arbitrary
   * grain boundary.
   *
   * @return the grain boundary energy
   */
  Real weightedMeanEnergy(std::string whichComponent);

  /// Calculate the dimensionless energy contribution to the boundary based on the nearby <100> rotations.
  void set100(std::string whichComponent);

  /// Calculate the dimensionless energy contribution to the boundary based on the nearby <110> rotations.
  void set110(std::string whichComponent);

  /// Calculate the dimensionless energy contribution to the boundary based on the nearby <111> rotations.
  void set111(std::string whichComponent);

  /**
   * Calculate the dimensionless energy contribution from the nearby <100> twists
   *
   * @param ksi the angle between the two grains or the twist angle
   */
  void twist100(std::vector<Real> ksi, std::string whichComponent); // Pass by value because we alter ksi internally.

  /// like twist100 but for <110> twists
  void twist110(std::vector<Real> ksi, std::string whichComponent);

  /// like twist100 but for <111> twists
  void twist111(std::vector<Real> ksi, std::string whichComponent);

  /**
   * Calculates the energy contributions for <100> asymmetric tilts
   *
   * @param eta the difference in the tilts of the two grains
   * @param ksi the angle between the grains
   */
  void aTGB100(const std::vector<Real> & eta, const std::vector<Real> & ksi, std::string whichComponent);

  /// like aTGB100 but for <110> aTGBs
  void aTGB110(const std::vector<Real> & eta, const std::vector<Real> & ksi, std::string whichComponent);

  /// like aTGB100 but for <111> aTGBs. note there isn't a STGB111
  void aTGB111(const std::vector<Real> & eta, const std::vector<Real> & ksi, std::string whichComponent);

  /**
   * Calculates the energy contribution from <100> symmetric tilts
   *
   * @param ksi the angle between the grains
   * @param en  the energies of <100> symmetric tilts
   */
  void sTGB100(const std::vector<Real> & ksi, std::vector<Real> & en, std::vector<Real> & Den, std::string whichComponent);

  // like sTGB100 but for <110> symmetric tilts
  void sTGB110(const std::vector<Real> & ksi, std::vector<Real> & en, std::vector<Real> & Den, std::string whichComponent);

  /**
   * Evalutes the Read-Shockley Wolfe function at theta
   *
   * @params theta where to evaluate the RSW function
   * @params thetaMin the starting angle of the interval
   * @params thetaMax the ending angle of the interval
   * @params a the shape factor
   * @return the value of the RSW function at theta
   */
  Real rSW(Real theta, Real thetaMin, Real thetaMax, Real a, std::string whichComponent = "None");

  /// isotropic grain boundary energy used as a fall-back
  Real _gb_energy_isotropic;

  /// order parameters
  std::vector<const VariableValue *> _v;

  /// gradient of the order parameters used to find the grain boundary normal
  std::vector<const VariableGradient *> _grad_v;

  /// orientations of the grains(orthogonal matrix)
  std::vector<RealTensorValue> _orientation_matrix;

  const EulerAngleProvider & _euler;

  /// material specific parameters for the energy function
  std::vector<Real> _par_vec;

  ///@{ holds the values for the idealized rotations
  std::vector<std::vector<Real>> _geom100;
  std::vector<std::vector<Real>> _geom110;
  std::vector<std::vector<Real>> _geom111;
  //@}

  std::vector<RealVectorValue> _axes100; // all of the <100> axes
  std::vector<RealVectorValue> _dirs100; // orthogonal to the <100> axes
  std::vector<RealVectorValue> _axes110;
  std::vector<RealVectorValue> _dirs110;
  std::vector<RealVectorValue> _axes111;
  std::vector<RealVectorValue> _dirs111;

  ///{@ 90 degree rotaions around the coordinate axes
  const RotationTensor _rot_X_p90;
  const RotationTensor _rot_Y_p90;
  const RotationTensor _rot_Z_p90;
  const RotationTensor _rot_Z_n90;
  ///@}

private:
  /// storage for symmetry variants in distancesToSet
  std::vector<RealTensorValue> _symmetry_variants;

  /// epsilon for fuzzy compares
  const Real _epsilon;

  ///@{ work vectors
  std::vector<Real> _e100, _e110, _e111;
  std::vector<Real> _s100, _s110, _s111;
  std::vector<Real> _ksi, _ksi_back, _en1, _en2;
  std::vector<Real> _Den1, _Den2;
  std::vector<Real> _entwist, _entilt;
  //@}
};
