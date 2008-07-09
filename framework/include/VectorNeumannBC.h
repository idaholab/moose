#include "BoundaryCondition.h"

//libMesh includes
#include "vector_value.h"

#ifndef VECTORNEUMANNBC_H
#define VECTORNEUMANNBC_H

//Forward Declarations
class VectorNeumannBC;

template<>
Parameters valid_params<VectorNeumannBC>()
{
  Parameters params;
  params.set<Real>("value0")=0.0;
  params.set<Real>("value1")=0.0;
  params.set<Real>("value2")=0.0;
  return params;
}

/**
 * Implements a simple constant VectorNeumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class VectorNeumannBC : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  VectorNeumannBC(Parameters parameters, EquationSystems * es, std::string var_name, unsigned int boundary_id)
    :BoundaryCondition(parameters, es, var_name, true, boundary_id)
  {
    _value(0)=_parameters.get<Real>("value0");
    _value(1)=_parameters.get<Real>("value1");
    _value(2)=_parameters.get<Real>("value2");
  }

  virtual ~VectorNeumannBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return -_JxW_face[_qp]*_phi_face[_i][_qp]*(_value*_normals_face[_qp]);
  }

private:
  /**
   * Vector to dot with the normal.
   */
  VectorValue<Real> _value;
};

#endif //NEUMANNBC_H
