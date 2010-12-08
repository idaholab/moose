#ifndef FLUXBC_H
#define FLUXBC_H

#include "NeumannBC.h"


//Forward Declarations
class FluxBC;

template<>
InputParameters validParams<FluxBC>();

/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class FluxBC : public NeumannBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  FluxBC(const std::string & name, InputParameters parameters);
  
virtual ~FluxBC() {}

protected:
  virtual Real computeQpResidual();
  
private:
  
  //Thermal conductivity
  MaterialProperty<Real> & _k;
};

#endif //NEUMANNBC_H
