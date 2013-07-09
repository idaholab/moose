/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SOLUTIONAUX_H
#define SOLUTIONAUX_H

#include "AuxKernel.h"
#include "SolutionUserObject.h"

// Forward decleartion
class SolutionAux;

template<>
InputParameters validParams<SolutionAux>();

/** AuxKernal for reading a solution from file
 * Creates a function that extracts values from a solution read from a file,
 * via a SolutionUserObject. It is possible to scale and add a constant to the
 * solution read.
 */
class SolutionAux : public AuxKernel
{
public:

  /** Constructor
   * @param name The name of the SolutionAux kernel
   * @param parameter The input parameters for the kernel
   */
  SolutionAux(const std::string & name, InputParameters parameters);

  /** Empy destructor
   */
  virtual ~SolutionAux();


protected:

  /** Compute the value for the kernel
   * Computes a value for a node or element depending on the type of kernel,
   * it also uses the 'direct' flag to extract values based on the dof if the
   * flag is set to true.
   * @ return The desired value of the solution for the current node or element
   */
  virtual Real computeValue();

  /// Reference to the SolutionUserObject storing the solution
  const SolutionUserObject & _solution_object;

  /// The variable name of interest
  std::string _var_name;

  /// Flag for directly grabing the data based on the dof
  const bool _direct;

  /// Multiplier for the solution, the a of ax+b
  const Real _scale_factor;

  /// Additional factor added to the solution, the b of ax+b
  const Real _add_factor;
};

#endif //SOLUTIONAUX_H
