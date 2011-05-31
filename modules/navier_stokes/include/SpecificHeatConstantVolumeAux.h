#ifndef SPECIFICHEATCONSTANTVOLUMEAUX_H
#define SPECIFICHEATCONSTANTVOLUMEAUX_H

#include "AuxKernel.h"

//Forward Declarations
class SpecificHeatConstantVolumeAux;

template<>
InputParameters validParams<SpecificHeatConstantVolumeAux>();

/** 
 * Nodal auxiliary variable for the specific heat, c_v...  Specific heat 
 * is a material property so why do this?  At least two reasons:
 * 1.) We would like to be able to compute Temperature as an AuxVariable,
 *     AuxVariables cannot access material properties because material
 *     properties currently don't make sense at nodes.
 * 2.) We need to impose Dirichlet thermal boundary conditions on the 
 *     temperature, which translates to a Dirichlet condition on 
 *     the temperature, which in turn involves c_v.   Material properties
 *     are not available to nodal boundary conditions for the same 
 *     reason as above.
 * The work-around is to treat the specific heat as an AuxVariable itself,
 * and couple it to all the other kernels and boundary conditions which
 * require its value.  
 *
 * If the specific heat is eventually treated as a function of temperature,
 * which is the case for some high-temperature gasses which are still ideal
 * gasses but not calorically perfect gasses, we can also potentially use this
 * kernel to set its value at different points in the mesh.
 */
class SpecificHeatConstantVolumeAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  SpecificHeatConstantVolumeAux(const std::string & name, InputParameters parameters);

  virtual ~SpecificHeatConstantVolumeAux() {}
  
protected:
  virtual Real computeValue();

  // Data members
  Real _specific_heat;
};

#endif // SPECIFICHEATCONSTANTVOLUMEAUX_H
