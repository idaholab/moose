//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANKTWOSCALARTOOLS_H
#define RANKTWOSCALARTOOLS_H

// MOOSE includes
#include "MooseTypes.h"

// Forward declarations
class MooseEnum;
class RankTwoTensor;

namespace libMesh
{
class Point;
}

namespace RankTwoScalarTools
{
/*
 * Return the scalar_type MooseEnum
 */
MooseEnum scalarOptions();

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
Real getQuantity(const RankTwoTensor & tensor,
                 const MooseEnum scalar_type,
                 const Point & point1,
                 const Point & point2,
                 const Point & curr_point,
                 Point & direction);

/*
 * Extracts the value of the tensor component at the specified indices
 */
Real component(const RankTwoTensor & r2tensor, unsigned int i, unsigned int j);

/*
 * Extracts the value of the tensor component at the specified indices and
 * updates the unit direction vector.
 */
Real component(const RankTwoTensor & r2tensor, unsigned int i, unsigned int j, Point & direction);

/*
 * The von Mises Stress is calculated in the deviatoric stress space:
 * \sigma_{vm} = \sqrt{\frac{3}{2}S_{ij}S_{ij}}
 * This scalar quanitity is often used to determine the onset of plasticity by
 * comparing the von Mises stress to the yield stress in J2 plasticity models.
 */
Real vonMisesStress(const RankTwoTensor & tensor);

/*
 * The equivalent plastic strain is calculated as
 * \epsilon_{eqv}^{pl} = \sqrt{\frac{2}{3}\epsilon_{ij}^{pl} \epsilon_{ij}^{pl}}
 * Users must take care to pass in the plastic inelastic_strain only.
 */
Real equivalentPlasticStrain(const RankTwoTensor & strain);

/*
 * The effective strain is calculated as
 * \epsilon_{eff} = \sqrt{\frac{2}{3}\epsilon_{ij} \epsilon_{ij}}
 */
Real effectiveStrain(const RankTwoTensor & strain);

/*
 * The hydrostatic scalar of a tensor is computed as the sum of the diagonal
 * terms divided by 3.
 */
Real hydrostatic(const RankTwoTensor & r2tensor);

/*
 * Computes the L2 normal of a rank two tensor
 */
Real L2norm(const RankTwoTensor & r2tensor);

/*
 * The volumentric strain is the change in volume over the original volume. In
 * this method the squared and cubic terms are included so that the calculation
 * is valid for both small and finite strains.
 */
Real volumetricStrain(const RankTwoTensor & strain);

/*
* The first invariant of a tensor is the sum of the diagonal component; defined
* in L. Malvern, Introduction to the Mechanics of a Continuous Mediam (1969) pg 89.
*/
Real firstInvariant(const RankTwoTensor & r2tensor);

/*
 * The second invariant is calculated using the formula from K. Hjelmstad,
 * Fundamentals of Structural Mechanics (1997) pg. 24.
 * Note that the Hjelmstad version of the second invariant is the negative of
 * the second invariant given in L. Malvern, Introduction to the Mechanics of a
 * Continuous Medium (1969) pg 89.
*/
Real secondInvariant(const RankTwoTensor & r2tensor);

/*
 * The third invariant of a rank 2 tensor is the determinate of the tensor; defined
 * in L. Malvern, Introduction to the Mechanics of a Continuous Mediam (1969) pg 89.
 */
Real thirdInvariant(const RankTwoTensor & r2tensor);

/*
 * The max Principal method returns the largest principal value for a symmetric
 * tensor, using the calcEigenValues method.
 * param r2tensor RankTwoTensor from which to extract the principal value
 * param direction Direction corresponding to the principal value
 */
Real maxPrincipal(const RankTwoTensor & r2tensor, Point & direction);

/*
 * The mid Principal method calculates the second largest principal value for a
 * tensor.
 * param r2tensor RankTwoTensor from which to extract the principal value
 * param direction Direction corresponding to the principal value
 */
Real midPrincipal(const RankTwoTensor & r2tensor, Point & direction);

/*
 * The min Principal stress returns the smallest principal value from a symmetric
 * tensor.
 * param r2tensor RankTwoTensor from which to extract the principal value
 * param direction Direction corresponding to the principal value
 */
Real minPrincipal(const RankTwoTensor & r2tensor, Point & direction);

/*
 * This method is called by the *Principal methods to calculate the eigenvalues
 * and eigenvectors of a symmetric tensor and return the desired value based on
 * vector position.
 * param r2tensor The RankTwoTensor from which to extract eigenvalues/vectors
 * param index The index of the principal value
 * param direction The eigenvector corresponding to the computed eigenvalue
 */
Real
calcEigenValuesEigenVectors(const RankTwoTensor & r2tensor, unsigned int index, Point & eigenvec);

/*
 * The axial stress is the scalar component of the stress tensor in an user-defined
 * direction; the axis direction is specified by inputing two points.
 * axial-stress = axis^T_i * \sigma_{ij} * axis_j
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
Real axialStress(const RankTwoTensor & stress,
                 const Point & point1,
                 const Point & point2,
                 Point & direction);

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
Real hoopStress(const RankTwoTensor & stress,
                const Point & point1,
                const Point & point2,
                const Point & curr_point,
                Point & direction);

/* The radial stress is calculated as
 * radial_stress = normal^T_i * \sigma_{ij} * normal_j
 * where normal is the position vector of the current point that is normal to
 * the user-defined axis of rotation; the axis direction is specified with two points.
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The point corresponding to the stress (pass in & _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated.
*/
Real radialStress(const RankTwoTensor & stress,
                  const Point & point1,
                  const Point & point2,
                  const Point & curr_point,
                  Point & direction);

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
 * This method calculates the scalar value of the supplied rank-2 tensor in the
 * direction specified by the user.
 */
Real directionValueTensor(const RankTwoTensor & r2tensor, Point & direction);

/*
 * Triaxiality is the ratio of the hydrostatic stress to the von Mises stress.
 */
Real triaxialityStress(const RankTwoTensor & stress);
}

#endif // RANKTWOSCALARTOOLS_H
