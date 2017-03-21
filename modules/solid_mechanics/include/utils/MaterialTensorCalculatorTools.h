/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATERIALTENSORCALCULATORTOOLS_H
#define MATERIALTENSORCALCULATORTOOLS_H

#include "libmesh/vector_value.h"
#include "MooseTypes.h"
#include "SymmTensor.h"

// class SymmTensor;

namespace MaterialTensorCalculatorTools
{
/*
 * Extracts the value of the tensor component at the specified indices
 */
Real component(const SymmTensor & symm_tensor, unsigned int index);

/*
 * Extracts the value of the tensor component at the specified indices and
 * updates the unit direction vector.
 */
Real component(const SymmTensor & symm_tensor, unsigned int index, RealVectorValue & direction);

/*
 * The von Mises Stress is calculated in non-principle stress space
 * \sigma_{vm} = \sqrt \frac{1}{2} \left( (\sigma_x - \sigma_y)^2 + (\sigma_y - \sigma_z)^2 +
 * (\sigma_z - \sigma_z)^2 + 6 * \left( \tau_{xy}^2 + \tau_{yz}^2 + \tau_{zx}^2 \right) \right)
 * This scalar quanitity is often used to determine the onset of plasticity by
 * comparing the von Mises stress to the yield stress in J2 plasticity models.
 */
Real vonMisesStress(const SymmTensor & symm_stress);

/*
 * The equivalent plastic strain is calculated as
 * \epsilon_{eqv}^{pl} = \sqrt{\frac{2}{3}\epsilon_{ij}^{pl} \epsilon_{ij}^{pl}}
 * Users must take care to pass in the plastic inelastic_strain only.
 */
Real equivalentPlasticStrain(const SymmTensor & symm_strain);

/*
 * The hydrostatic scalar of a tensor is computed as the sum of the diagonal
 * terms divided by 3.
 */
Real hydrostatic(const SymmTensor & symm_tensor);

/*
 * The volumentric strain is the change in volume over the original volume. In
 * this method the squared and cubic terms are included so that the calculation
 * is valid for both small and finite strains.
 */
Real volumetricStrain(const SymmTensor & symm_strain);

/*
* The first invariant of a tensor is the sum of the diagonal component; defined
* in L. Malvern, Introduction to the Mechanics of a Continuous Mediam (1969) pg 89.
*/
Real firstInvariant(const SymmTensor & symm_tensor);

/*
 * The second invariant is calculated using the formula from K. Hjelmstad,
 * Fundamentals of Structural Mechanics (1997) pg. 24.
 * Note that the Hjelmstad version of the second invariant is the negative of
 * the second invariant given in L. Malvern, Introduction to the Mechanics of a
 * Continuous Medium (1969) pg 89.
*/
Real secondInvariant(const SymmTensor & symm_tensor);

/*
 * The third invariant of a rank 2 tensor is the determinate of the tensor; defined
 * in L. Malvern, Introduction to the Mechanics of a Continuous Mediam (1969) pg 89.
 */
Real thirdInvariant(const SymmTensor & symm_tensor);

/*
 * The max Principle method returns the largest principle value for a symmetric
 * tensor, using the calcEigenValues method.
 */
Real maxPrinciple(const SymmTensor & symm_tensor, RealVectorValue & direction);

/*
 * The mid Principle method calculates the second largest principle value for a
 * tensor.  This method is valid only for 3D problems and will return an error
 * if called in 2D problems.
 */
Real midPrinciple(const SymmTensor & symm_tensor, RealVectorValue & direction);

/*
 * The min Principle stress returns the smallest principle value from a symmetric
 * tensor.
 */
Real minPrinciple(const SymmTensor & symm_tensor, RealVectorValue & direction);

/*
 * This method is called by the *Principle methods to calculate the eigenvalues
 * of a symmetric tensor and return the desired value based on vector position.
 */
Real calcPrincipleValues(const SymmTensor & symm_tensor,
                         unsigned int index,
                         RealVectorValue & direction);

/*
 * The axial stress is the scalar component of the stress tensor in an user-defined
 * direction; the axis direction is specified by inputing two points.
 * axial-stress = axis^T_i * \sigma_{ij} * axis_j
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
Real axialStress(const SymmTensor & symm_stress,
                 const Point & point1,
                 const Point & point2,
                 RealVectorValue & direction);

/*
 * The hoop stress is calculated as
 * hoop-stress = z^T_i * \sigma_{ij} * z_j
 * where z is defined as the cross of the user-defined (via input points) axis
 * of rotation and the normal from the current position to the axis of rotation.
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The reference corresponding to the stress (pass in _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated.
 */
Real hoopStress(const SymmTensor & symm_stress,
                const Point & point1,
                const Point & point2,
                const Point & curr_point,
                RealVectorValue & direction);

/* The radial stress is calculated as
 * radial_stress = normal^T_i * \sigma_{ij} * normal_j
 * where normal is the position vector of the current point that is normal to
 * the user-defined axis of rotation; the axis direction is specified with two points.
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The reference corresponding to the stress (pass in _q_point[_qp])
 * @param direction The direction vector in which the scalar stress value is calculated.
*/
Real radialStress(const SymmTensor & symm_stress,
                  const Point & point1,
                  const Point & point2,
                  const Point & curr_point,
                  RealVectorValue & direction);

/*
 * This method is a helper method for the hoopStress and radialStress methods to
 * calculate the unit position vector which is normal to the user supplied axis
 * of rotation; the rotation axis direction is specified by inputing two points.
 * @param point1 The starting point of the rotation axis for a cylinderical system
 * @param point2 The end point of the rotation axis
 * @param curr_point The reference corresponding to the stress (pass in _q_point[_qp])
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
Real directionValueTensor(const SymmTensor & symm_tensor, const RealVectorValue & input_direction);

/*
 * Triaxiality is the ratio of the hydrostatic stress to the von Mises stress.
 */
Real triaxialityStress(const SymmTensor & symm_stress);
}

#endif // MATERIALTENSORCALCULATORTOOLS_H
