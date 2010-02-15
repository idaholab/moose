#ifndef CONVECTIONDIFFUSIONSUPG_H
#define CONVECTIONDIFFUSIONSUPG_H

#include "Moose.h"
#include "Stabilizer.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

//Forward Declarations
class ConvectionDiffusionSUPG;

template<>
InputParameters validParams<ConvectionDiffusionSUPG>();

/**
 * 
 */
class ConvectionDiffusionSUPG : public Stabilizer
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   * @param coupled_to The real names of the variables this ConvectionDiffusionSUPG is coupled to.
   * @param coupled_as The name this ConvectionDiffusionSUPG is going to use to refer to the coupled_to variables as.
   */
  ConvectionDiffusionSUPG(std::string name,
             InputParameters parameters,
             std::string var_name,
             std::vector<std::string> coupled_to,
             std::vector<std::string> coupled_as);

  /**
   * Compute the test functions.
   */
  virtual void computeTestFunctions();

protected:
  /**
   * Hyperbolic Cotangent
   */
  Real coth(Real x);

  /**
   * Diffusion coefficient.
   */
  Real _coef;

  /**
   * Reaction coefficient.
   */
  Real _rcoef;
  
  /**
   * A velocity vector that supports a dot product.
   */
  RealVectorValue _velocity;

  /**
   * Class variables to hold the components of velocity coming from the input parameters.
   */
  Real _x;
  Real _y;
  Real _z;

  /**
   * Velocity Magnitude
   */
  Real _vel_mag;
};

#endif //CONVECTIONDIFFUSIONSUPG_H
