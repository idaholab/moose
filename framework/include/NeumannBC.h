#include "BoundaryCondition.h"

#ifndef NEUMANNBC_H
#define NEUMANNBC_H

//Forward Declarations
class NeumannBC;

template<>
Parameters valid_params<NeumannBC>()
{
  Parameters params;
  params.set<Real>("value")=0.0;
  return params;
}

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
  NeumannBC(Parameters parameters, EquationSystems * es, std::string var_name, unsigned int boundary_id)
    :BoundaryCondition(parameters, es, var_name, true, boundary_id),
     _value(_parameters.get<Real>("value"))
  {}

  /**
   * Standalone constructor.
   */
  NeumannBC(EquationSystems * es, std::string var_name, unsigned int boundary_id, Real value)
    :BoundaryCondition(es, var_name, true, boundary_id),
     _value(value)
  {}
    
  virtual ~NeumannBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return -_JxW_face[_qp]*_phi_face[_i][_qp]*_value;
  }

private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
};

#endif //NEUMANNBC_H
