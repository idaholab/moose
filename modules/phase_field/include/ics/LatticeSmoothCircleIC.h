#ifndef LATTICESMOOTHCIRCLEIC_H
#define LATTICESMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "MultiSmoothCircleIC.h"

// System includes
#include <string>

// Forward Declarations
class LatticeSmoothCircleIC;

template<>
InputParameters validParams<LatticeSmoothCircleIC>();

/**
 * LatticeSmoothcircleIC creates a lattice of smoothcircles as an initial condition.
 * They are either directly on the lattice or randomly perturbed from the lattice.
 **/
class LatticeSmoothCircleIC : public MultiSmoothCircleIC
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  LatticeSmoothCircleIC(const std::string & name,
                         InputParameters parameters);

  virtual void initialSetup();

protected:
  Real _Rnd_variation;
  std::vector<unsigned int> _circles_per_side;
};

#endif //LATTICESMOOTHCIRCLEIC_H
