#ifndef MATCHEDVALUEBC_H
#define MATCHEDVALUEBC_H

#include "NodalBC.h"

//Forward Declarations
class MatchedValueBC;

template<>
InputParameters validParams<MatchedValueBC>();

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class MatchedValueBC : public NodalBC
{
public:
  MatchedValueBC(const std::string & name, InputParameters parameters);
    
  virtual ~MatchedValueBC() {}

protected:
  virtual Real computeQpResidual();

  VariableValue & _v_face;
};

#endif //MATCHEDVALUEBC_H
