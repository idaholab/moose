#ifndef MATCHEDVALUEBC_H
#define MATCHEDVALUEBC_H

#include "BoundaryCondition.h"

//Forward Declarations
class MatchedValueBC;

template<>
InputParameters validParams<MatchedValueBC>();

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class MatchedValueBC : public BoundaryCondition
{
public:
  MatchedValueBC(std::string name, MooseSystem & moose_system, InputParameters parameters);
    
  virtual ~MatchedValueBC() {}

protected:
  virtual Real computeQpResidual();

private:
  MooseArray<Real> & _v_face;
};

#endif //MATCHEDVALUEBC_H
