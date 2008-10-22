#include "BoundaryCondition.h"

#ifndef NEUMANNBC_H
#define NEUMANNBC_H

//Forward Declarations
class NeumannBC;

template<>
Parameters valid_params<NeumannBC>();

/**
 * Implements a simple constant Neumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class NeumannBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NeumannBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
    _value(_parameters.get<Real>("value"))
  {}
    
  virtual ~NeumannBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return -_phi_face[_i][_qp]*_value;
  }

private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
};

#endif //NEUMANNBC_H
