#include "BoundaryCondition.h"

#ifndef NEUMANNBC_H
#define NEUMANNBC_H

class NeumannBC : public BoundaryCondition
{
public:
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
  Real _value;
};

#endif //NEUMANNBC_H
