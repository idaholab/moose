#ifndef CONVECTIVEFLUXBC_H
#define CONVECTIVEFLUXBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class ConvectiveFluxBC;

template<>
InputParameters valid_params<ConvectiveFluxBC>();

class ConvectiveFluxBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ConvectiveFluxBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);
  
    

  virtual ~ConvectiveFluxBC() {}


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
