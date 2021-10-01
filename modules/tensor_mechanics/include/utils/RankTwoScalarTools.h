//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "libmesh/point.h"
#include "RankTwoTensor.h"
#include "MooseError.h"
#include "MooseUtils.h"

namespace RankTwoScalarTools
{
/*
 * Return the scalar_type MooseEnum
 */
MooseEnum scalarOptions();

MooseEnum invariantOptions();
MooseEnum cylindricalOptions();
MooseEnum sphericalOptions();

enum class InvariantType
{
  VonMisesStress,
  EffectiveStrain,
  Hydrostatic,
  L2norm,
  VolumetricStrain,
  FirstInvariant,
  SecondInvariant,
  ThirdInvariant,
  TriaxialityStress,
  MaxShear,
  StressIntensity,
  MaxPrincipal,
  MidPrincipal,
  MinPrincipal
};

enum class CylindricalComponent
{
  AxialStress,
  HoopStress,
  RadialStress
};

enum class SphericalComponent
{
  HoopStress,
  RadialStress
};

/*
 * Extracts the value of the tensor component at the specified indices
 */
template <typename T>
T
component(const RankTwoTensorTempl<T> & r2tensor, unsigned int i, unsigned int j)
{
  return r2tensor(i, j);
}

/*
 * Extracts the value of the tensor component at the specified indices and
 * updates the unit direction vector.
 */
template <typename T>
T
component(const RankTwoTensorTempl<T> & r2tensor, unsigned int i, unsigned int j, Point & direction)
{
  direction.zero();
  if (i == j)
    direction(i) = 1.0;
  else
  {
    direction(i) = std::sqrt(0.5);
    direction(j) = std::sqrt(0.5);
  }

  return r2tensor(i, j);
}

/*
 * The von Mises Stress is calculated in the deviatoric stress space:
 * \sigma_{vm} = \sqrt{\frac{3}{2}S_{ij}S_{ij}}
 * This scalar quanitity is often used to determine the onset of plasticity by
 * comparing the von Mises stress to the yield stress in J2 plasticity models.
 */
template <typename T>
T
vonMisesStress(const RankTwoTensorTempl<T> & stress)
{
  RankTwoTensorTempl<T> dev_stress = stress.deviatoric();

  return std::sqrt(1.5 * dev_stress.doubleContraction(dev_stress));
}

/*
 * The effective strain increment (or equivalent von Mises strain) is calculated as
 * \epsilon_{eff} = \sqrt{\frac{2}{3}\epsilon_{ij} \epsilon_{ij}}
 */
template <typename T>
T
effectiveStrain(const RankTwoTensorTempl<T> & strain)
{
  return std::sqrt(2.0 / 3.0 * strain.doubleContraction(strain));
}

/*
 * The hydrostatic scalar of a tensor is computed as the sum of the diagonal
 * terms divided by 3.
 */
template <typename T>
T
hydrostatic(const RankTwoTensorTempl<T> & r2tensor)
{
  return r2tensor.trace() / 3.0;
}

/*
 * Computes the L2 normal of a rank two tensor
 */
template <typename T>
T
L2norm(const RankTwoTensorTempl<T> & r2tensor)
{
  return r2tensor.L2norm();
}

/*
 * The volumentric strain is the change in volume over the original volume. In
 * this method the squared and cubic terms are included so that the calculation
 * is valid for both small and finite strains.
 * Since the strains are logarithmic strains, which are by definition log(L/L0),
 * exp(log_strain) = L/L0
 * The ratio of the volume change of a strained cube to the original volume
 * (delta V / V) is thus:
 * exp(log_strain_11) * exp(log_strain_22) * exp(log_strain_33) - 1
 *
 * Since eng_strain = exp(log_strain) - 1, the equivalent calculation using
 * engineering strains would be:
 * (1 + eng_strain_11) * (1 + eng_strain_22) + (1 + eng_strain_33) - 1
 * If strains are small, the resulting terms that involve squared and cubed
 * strains are negligible, resulting in the following approximate form:
 * strain_11 + strain_22 + strain_33
 * There is not currently an option to compute this small-strain form of the
 * volumetric strain, but at small strains, it is approximately equal to the
 * finite strain form that is computed.
 *
 * @param strain Total logarithmic strain
 * @return volumetric strain (delta V / V)
 */
template <typename T>
T
volumetricStrain(const RankTwoTensorTempl<T> & strain)
{
  return std::exp(strain(0, 0)) * std::exp(strain(1, 1)) * std::exp(strain(2, 2)) - 1.0;
}

/*
 * The first invariant of a tensor is the sum of the diagonal component; defined
 * in L. Malvern, Introduction to the Mechanics of a Continuous Mediam (1969) pg 89.
 */
template <typename T>
T
firstInvariant(const RankTwoTensorTempl<T> & r2tensor)
{
  return r2tensor.trace();
}

/*
 * The second invariant is calculated using the formula from K. Hjelmstad,
 * Fundamentals of Structural Mechanics (1997) pg. 24.
 * Note that the Hjelmstad version of the second invariant is the negative of
 * the second invariant given in L. Malvern, Introduction to the Mechanics of a
 * Continuous Medium (1969) pg 89.
 */
template <typename T>
T
secondInvariant(const RankTwoTensorTempl<T> & r2tensor)
{
  T val = 0.0;
  for (unsigned int i = 0; i < 2; ++i)
    for (unsigned int j = i + 1; j < 3; ++j)
    {
      val += r2tensor(i, i) * r2tensor(j, j);
      val -= (r2tensor(i, j) * r2tensor(i, j) + r2tensor(j, i) * r2tensor(j, i)) * 0.5;
    }

  return val;
}

/*
 * The third invariant of a rank 2 tensor is the determinate of the tensor; defined
 * in L. Malvern, Introduction to the Mechanics of a Continuous Mediam (1969) pg 89.
 */
template <typename T>
T
thirdInvariant(const RankTwoTensorTempl<T> & r2tensor)
{
  T val = 0.0;
  val = r2tensor(0, 0) * r2tensor(1, 1) * r2tensor(2, 2) -
        r2tensor(0, 0) * r2tensor(1, 2) * r2tensor(2, 1) +
        r2tensor(0, 1) * r2tensor(1, 2) * r2tensor(2, 0) -
        r2tensor(0, 1) * r2tensor(1, 0) * r2tensor(2, 2) +
        r2tensor(0, 2) * r2tensor(1, 0) * r2tensor(2, 1) -
        r2tensor(0, 2) * r2tensor(1, 1) * r2tensor(2, 0);

  return val;
}

/*
 * This method is called by the *Principal methods to calculate the eigenvalues
 * and eigenvectors of a symmetric tensor and return the desired value based on
 * vector position.
 * param r2tensor The RankTwoTensor from which to extract eigenvalues/vectors
 * param index The index of the principal value
 * param direction The eigenvector corresponding to the computed eigenvalue
 */
template <typename T>
T
calcEigenValuesEigenVectors(const RankTwoTensorTempl<T> & r2tensor,
                            unsigned int index,
                            Point & eigenvec)
{
  std::vector<T> eigenval(LIBMESH_DIM);
  RankTwoTensorTempl<T> eigvecs;
  r2tensor.symmetricEigenvaluesEigenvectors(eigenval, eigvecs);

  T val = eigenval[index];
  eigenvec = eigvecs.column(index);

  return val;
}

/*
 * The max Principal method returns the largest principal value for a symmetric
 * tensor, using the calcEigenValues method.
 * param r2tensor RankTwoTensor from which to extract the principal value
 * param direction Direction corresponding to the principal value
 */
template <typename T>
T
maxPrincipal(const RankTwoTensorTempl<T> & r2tensor, Point & direction)
{
  return calcEigenValuesEigenVectors(r2tensor, (LIBMESH_DIM - 1), direction);
}

/*
 * The mid Principal method calculates the second largest principal value for a
 * tensor.
 * param r2tensor RankTwoTensor from which to extract the principal value
 * param direction Direction corresponding to the principal value
 */
template <typename T>
T
midPrincipal(const RankTwoTensorTempl<T> & r2tensor, Point & direction)
{
  return calcEigenValuesEigenVectors(r2tensor, 1, direction);
}

/*
 * The min Principal stress returns the smallest principal value from a symmetric
 * tensor.
 * param r2tensor RankTwoTensor from which to extract the principal value
 * param direction Direction corresponding to the principal value
 */
template <typename T>
T
minPrincipal(const RankTwoTensorTempl<T> & r2tensor, Point & direction)
{
  return calcEigenValuesEigenVectors(r2tensor, 0, direction);
}

/*
 * The axial stress is the scalar component of the stress tensor in an user-defined
 * direction; the axis direction is specified by inputing two points.
 * axial-stress = axis^T_i * \sigma_{ij} * axis_j
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
template <typename T>
T
axialStress(const RankTwoTensorTempl<T> & stress,
            const Point & point1,
            const Point & point2,
            Point & direction)
{
  Point axis = point2 - point1;
  axis /= axis.norm();

  // Calculate the stress in the direction of the axis specifed by the user
  T axial_stress = axis * (stress * axis);

  direction = axis;

  return axial_stress;
}

/*
 * This method is a helper method for the hoopStress and radialStress methods to
 * calculate the unit position vector which is normal to the user supplied axis
 * of rotation; the rotation axis direction is specified by inputing two points.
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The point corresponding to the stress (pass in & _q_point[_qp])
 * @param normalPosition The vector from the current point that is normal to the rotation axis
 */
void normalPositionVector(const Point & point1,
                          const Point & point2,
                          const Point & curr_point,
                          Point & normalPosition);

/*
 * The hoop stress is calculated as
 * hoop-stress = z^T_i * \sigma_{ij} * z_j
 * where z is defined as the cross of the user-defined (via input points) axis
 * of rotation and the normal from the current position to the axis of rotation.
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The point corresponding to the stress (pass in & _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
template <typename T>
T
hoopStress(const RankTwoTensorTempl<T> & stress,
           const Point & point1,
           const Point & point2,
           const Point & curr_point,
           Point & direction)
{
  // Calculate the cross of the normal to the axis of rotation from the current
  // location and the axis of rotation
  Point xp;
  normalPositionVector(point1, point2, curr_point, xp);
  Point axis_rotation = point2 - point1;
  Point yp = axis_rotation / axis_rotation.norm();
  Point zp = xp.cross(yp);

  // Calculate the scalar value of the hoop stress
  T hoop_stress = zp * (stress * zp);

  direction = zp;

  return hoop_stress;
}

/* The radial stress is calculated as
 * radial_stress = normal^T_i * \sigma_{ij} * normal_j
 * where normal is the position vector of the current point that is normal to
 * the user-defined axis of rotation; the axis direction is specified with two points.
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The point corresponding to the stress (pass in & _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
template <typename T>
T
radialStress(const RankTwoTensorTempl<T> & stress,
             const Point & point1,
             const Point & point2,
             const Point & curr_point,
             Point & direction)
{
  Point radial_norm;
  normalPositionVector(point1, point2, curr_point, radial_norm);

  // Compute the scalar stress component in the direction of the normal vector from the
  // user-defined axis of rotation.
  T radial_stress = radial_norm * (stress * radial_norm);

  direction = radial_norm;

  return radial_stress;
}

/* The radial stress is calculated as
 * radial_stress = radial^T_i * \sigma_{ij} * radial_j
 * where normal is the position vector of the current point that is normal to
 * the user-defined axis of rotation;
 * @param center The center point of the spherical system
 * @param curr_point The point corresponding to the stress (pass in & _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
template <typename T>
T
radialStress(const RankTwoTensorTempl<T> & stress,
             const Point & center,
             const Point & curr_point,
             Point & direction)
{
  Point radial = curr_point - center;
  radial /= radial.norm();

  // Compute the scalar stress component in the radial direction
  T radial_stress = radial * (stress * radial);

  direction = radial;

  return radial_stress;
}

/* The hoop stress is calculated as
 * hoop_stress = tangential^T_i * \sigma_{ij} * tangential_j
 * where tangential is tangential direction at the current point in the spherical system;
 * @param center The center point of the spherical system
 * @param curr_point The point corresponding to the stress (pass in & _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
template <typename T>
T
hoopStress(const RankTwoTensorTempl<T> & stress,
           const Point & center,
           const Point & curr_point,
           Point & direction)
{
  Point radial = curr_point - center;
  Real r = radial.norm();
  radial /= r;

  // Given normal vector N=(n1,n2,n3) and current point C(c1,c2,c3), the tangential plane is then
  // defined as n1(x-c1) + n2(y-c2) + n3(z-c3)=0. Let us assume n1!=0, the arbitrary point P on this
  // plane can be taken as P(x,c2+r,c3+r) where r is the radius. The x can be solved as x =
  // -r(n2+n3)/n1 + c1. The tangential vector PC is given as P-C.

  Point tangential;

  if (!MooseUtils::absoluteFuzzyEqual(radial(0), 0.0))
  {
    Real x = curr_point(0) - (radial(1) + radial(2)) * r / radial(0);
    tangential = Point(x, curr_point(1) + r, curr_point(2) + r) - curr_point;
  }
  else if (!MooseUtils::absoluteFuzzyEqual(radial(1), 0.0))
  {
    Real y = curr_point(1) - (radial(0) + radial(2)) * r / radial(1);
    tangential = Point(curr_point(0) + r, y, curr_point(2) + r) - curr_point;
  }
  else if (!MooseUtils::absoluteFuzzyEqual(radial(1), 0.0))
  {
    Real z = curr_point(2) - (radial(0) + radial(1)) * r / radial(2);
    tangential = Point(curr_point(0) + r, curr_point(1) + r, z) - curr_point;
  }
  else
    mooseError("In Hoop stress calculation for spherical geometry, the current (quadracture) point "
               "is likely to be at the center. ");

  tangential /= tangential.norm();

  // Compute the scalar stress component in the hoop direction
  T hoop_stress = 0.0;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      hoop_stress += tangential(j) * stress(j, i) * tangential(i);

  direction = tangential;

  return hoop_stress;
}

/*
 * This method calculates the scalar value of the supplied rank-2 tensor in the
 * direction specified by the user.
 */
template <typename T>
T
directionValueTensor(const RankTwoTensorTempl<T> & r2tensor, const Point & direction)
{
  T tensor_value_in_direction = direction * (r2tensor * direction);

  return tensor_value_in_direction;
}

/*
 * Triaxiality is the ratio of the hydrostatic stress to the von Mises stress.
 */
template <typename T>
T
triaxialityStress(const RankTwoTensorTempl<T> & stress)
{
  return hydrostatic(stress) / vonMisesStress(stress);
}

/*
 * maxShear is the maximum shear stress defined as the maximum principal
 * stress minus the minimum principal stress.
 */
template <typename T>
T
maxShear(const RankTwoTensorTempl<T> & stress)
{
  Point dummy;
  return (maxPrincipal(stress, dummy) - minPrincipal(stress, dummy)) / 2.;
}
/*
 * stressIntensity is defined as two times the maximum shear stress.
 */
template <typename T>
T
stressIntensity(const RankTwoTensorTempl<T> & stress)
{
  return 2. * maxShear(stress);
}

/*
 * Return scalar quantity of a rank two tensor based on the user specified scalar_type
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The point corresponding to the stress (pass in & _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated
 * point1 and point2 are required only for the cases of axialStress, hoopStress and radialStress
 * curr_point is required only for the cases of hoopStress and radialStress
 * direction is required only for directionValueTensor
 * for all other cases, these parameters will take the default values
 */
template <typename T>
T
getQuantity(const RankTwoTensorTempl<T> & tensor,
            const MooseEnum & scalar_type,
            const Point & point1,
            const Point & point2,
            const Point & curr_point,
            Point & direction)
{
  switch (scalar_type)
  {
    case 0:
      return vonMisesStress(tensor);
    case 1:
      mooseError("To compute an effective inelastic strain use "
                 "RankTwoScalarTools::effectiveStrain()");
    case 2:
      return hydrostatic(tensor);
    case 3:
      return L2norm(tensor);
    case 4:
      return maxPrincipal(tensor, direction);
    case 5:
      return midPrincipal(tensor, direction);
    case 6:
      return minPrincipal(tensor, direction);
    case 7:
      return volumetricStrain(tensor);
    case 8:
      return firstInvariant(tensor);
    case 9:
      return secondInvariant(tensor);
    case 10:
      return thirdInvariant(tensor);
    case 11:
      return axialStress(tensor, point1, point2, direction);
    case 12:
      return hoopStress(tensor, point1, point2, curr_point, direction);
    case 13:
      return radialStress(tensor, point1, point2, curr_point, direction);
    case 14:
      return triaxialityStress(tensor);
    case 15:
      return directionValueTensor(tensor, direction);
    case 16:
      return maxShear(tensor);
    case 17:
      return stressIntensity(tensor);
    default:
      mooseError("RankTwoScalarTools Error: invalid scalar type");
  }
}

template <typename T>
T
getCylindricalComponent(const RankTwoTensorTempl<T> & tensor,
                        const CylindricalComponent & scalar_type,
                        const Point & point1,
                        const Point & point2,
                        const Point & curr_point,
                        Point & direction)
{
  switch (scalar_type)
  {
    case CylindricalComponent::AxialStress:
      return axialStress(tensor, point1, point2, direction);
    case CylindricalComponent::HoopStress:
      return hoopStress(tensor, point1, point2, curr_point, direction);
    case CylindricalComponent::RadialStress:
      return radialStress(tensor, point1, point2, curr_point, direction);
    default:
      mooseError("RankTwoCylindricalComponent Error: scalar type invalid");
  }
}

template <typename T>
T
getSphericalComponent(const RankTwoTensorTempl<T> & tensor,
                      const SphericalComponent & scalar_type,
                      const Point & center,
                      const Point & curr_point,
                      Point & direction)
{
  switch (scalar_type)
  {
    case SphericalComponent::HoopStress:
      return hoopStress(tensor, center, curr_point, direction);
    case SphericalComponent::RadialStress:
      return radialStress(tensor, center, curr_point, direction);
    default:
      mooseError("RankTwoSphericalComponent Error: scalar type invalid");
  }
}

template <typename T>
T
getPrincipalComponent(const RankTwoTensorTempl<T> & tensor,
                      const InvariantType & scalar_type,
                      Point & direction)
{
  switch (scalar_type)
  {
    case InvariantType::MaxPrincipal:
      return maxPrincipal(tensor, direction);
    case InvariantType::MidPrincipal:
      return midPrincipal(tensor, direction);
    case InvariantType::MinPrincipal:
      return minPrincipal(tensor, direction);
    default:
      mooseError("RankTwoInvariant Error: valid invariant");
  }
}

template <typename T>
T
getDirectionalComponent(const RankTwoTensorTempl<T> & tensor, const Point & direction)
{
  return directionValueTensor(tensor, direction);
}

template <typename T>
T
getInvariant(const RankTwoTensorTempl<T> & tensor, const InvariantType & scalar_type)
{
  switch (scalar_type)
  {
    case InvariantType::VonMisesStress:
      return vonMisesStress(tensor);
    case InvariantType::EffectiveStrain:
      mooseError("To compute an effective inelastic strain use "
                 "RankTwoScalarTools::effectiveStrain()");
    case InvariantType::Hydrostatic:
      return hydrostatic(tensor);
    case InvariantType::L2norm:
      return L2norm(tensor);
    case InvariantType::VolumetricStrain:
      return volumetricStrain(tensor);
    case InvariantType::FirstInvariant:
      return firstInvariant(tensor);
    case InvariantType::SecondInvariant:
      return secondInvariant(tensor);
    case InvariantType::ThirdInvariant:
      return thirdInvariant(tensor);
    case InvariantType::TriaxialityStress:
      return triaxialityStress(tensor);
    case InvariantType::MaxShear:
      return maxShear(tensor);
    case InvariantType::StressIntensity:
      return stressIntensity(tensor);
    default:
      mooseError("RankTwoCartesianComponent Error: Pass valid invariant - " +
                 invariantOptions().getRawNames());
  }
}
}
