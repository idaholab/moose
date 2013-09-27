#include "ACBulk.h"

#ifndef ACGRGRPOLY_H
#define ACGRGRPOLY_H

//Forward Declarations
class ACGrGrPoly;

template<>
InputParameters validParams<ACGrGrPoly>();

/**This kernel calculates the residual for grain growth.  It calculates the residual of the ith order parameter, and the values of all other order parameters are coupled variables and are stored in vals
**/
class ACGrGrPoly : public ACBulk
{
public:

  ACGrGrPoly(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /**
   * Coupled things come through as std::vector _refernces_.
   *
   * Since this is a reference it MUST be set in the Initialization List of the
   * constructor!
   */
  
  std::vector<VariableValue *> _vals;
  std::vector<unsigned int> _vals_var;

  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _gamma;

  bool _has_bnds;
  VariableValue & _bnds;
  
  unsigned int _ncrys;

//  Real _gamma;
  
};
#endif //ACGRGRPOLY_H
