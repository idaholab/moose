#include "BoundaryCondition.h"

#ifndef CONVECTIVEFLUXBC_H
#define CONVECTIVEFLUXBC_H

//Forward Declarations
class ConvectiveFluxBC;

template<>
Parameters valid_params<ConvectiveFluxBC>();

class ConvectiveFluxBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ConvectiveFluxBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
     _initial(_parameters.get<Real>("initial")),
     _final(_parameters.get<Real>("final")),
     _rate(_parameters.get<Real>("rate")),
     _duration(_parameters.get<Real>("duration"))
    {}

  virtual ~ConvectiveFluxBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  
private:
  /**
   * Ratio of u to du/dn
   */
  Real _initial;
  Real _final;
  Real _rate;
  Real _duration;
};

#endif //CONVECTIVEFLUXBC_H
