#include "BoundaryCondition.h"

#ifndef MATCHEDVALUEBC_H
#define MATCHEDVALUEBC_H

//Forward Declarations
class MatchedValueBC;

template<>
Parameters valid_params<MatchedValueBC>();

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class MatchedValueBC : public BoundaryCondition
{
public:
  MatchedValueBC(Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(parameters, var_name, false, boundary_id, coupled_to, coupled_as),
    _v_face(coupledValFace("v"))
  {}
    
  virtual ~MatchedValueBC(){}

protected:
  virtual Real computeQpResidual()
  {
    return _u_face[_qp]-_v_face[_qp];
  }

private:
  std::vector<Real> & _v_face;
};

#endif //MATCHEDVALUEBC_H
