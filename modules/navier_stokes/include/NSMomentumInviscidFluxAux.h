#ifndef NSMOMENTUMINVISCIDFLUXAUX_H
#define NSMOMENTUMINVISCIDFLUXAUX_H

#include "AuxKernel.h"

//Forward Declarations
class NSMomentumInviscidFluxAux;

template<>
InputParameters validParams<NSMomentumInviscidFluxAux>();

/** 
 * Nodal aux variable for entries in the inviscid momentum flux vector.
 * This is to allow us to compute with nodal values of "F" which generally
 * works better for compressible flows, according to B. Kirk.  These entries
 * are, for i=1,..,n_dim
 *
 * rho * u_i * u_1 + delta_{i1} * P  ... (x_1-momentum equation)
 * rho * u_i * u_2 + delta_{i2} * P  ... (x_2-momentum equation)
 * rho * u_i * u_3 + delta_{i3} * P  ... (x_3-momentum equation)
 *
 * Nodal aux vars can only return a single value (not a vector) so you'll need one of these
 * in your input file for each direction.
 */
class NSMomentumInviscidFluxAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NSMomentumInviscidFluxAux(const std::string & name, InputParameters parameters);

  virtual ~NSMomentumInviscidFluxAux() {}
  
protected:
  virtual Real computeValue();
  
  // The momentum used must match the flux_vector_subscript used below.
  // So if you specify _flux_vector_subscript=0 in the input file, you must
  // also specify "pu" as the momentum variable to use.

  // So as not to be redundant with indexing, we'll just couple to all the momentums...
  VariableValue & _rhou; // solution variable
  VariableValue & _rhov; // solution variable
  VariableValue & _rhow; // solution variable

  VariableValue & _u_vel; // nodal aux
  VariableValue & _v_vel; // nodal aux
  VariableValue & _w_vel; // nodal aux

  VariableValue & _pressure; // nodal aux

  // Which F_i are we currently computing?  
  // 0, 1, 2 for x, y, and z, respectively.
  unsigned _flux_vector_subscript;

  // Which momentum equation are we currently computing for?
  // 0, 1, 2 for x, y, and z, respectively.
  unsigned _equation_index;
};

#endif // NSMOMENTUMINVISCIDFLUXAUX_H
