#ifndef LATTICESMOOTHCIRCLEIC_H
#define LATTICESMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "SmoothCircleBaseIC.h"

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
class LatticeSmoothCircleIC : public SmoothCircleBaseIC
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

  virtual void computeCircleRadii();

  virtual void computeCircleCenters();

protected:
  Real _lattice_variation;
  std::vector<unsigned int> _circles_per_side;
  unsigned int _numbub;
  Real _Lx;
  Real _Ly;
  Real _Lz;

  Real _radius;
  Real _radius_variation;
};

#endif //LATTICESMOOTHCIRCLEIC_H
