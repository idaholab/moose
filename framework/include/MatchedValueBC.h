#ifndef MATCHEDVALUEBC_H
#define MATCHEDVALUEBC_H

#include "BoundaryCondition.h"


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
  MatchedValueBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);
    
  virtual ~MatchedValueBC() {}

protected:
  virtual Real computeQpResidual();

private:
  std::vector<Real> & _v_face;
};

#endif //MATCHEDVALUEBC_H
