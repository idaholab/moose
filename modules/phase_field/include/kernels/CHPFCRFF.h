#ifndef CHPFCRFF_H
#define CHPFCRFF_H

#include "CHBulk.h"


//Forward Declarations
class CHPFCRFF;

template<>
InputParameters validParams<CHPFCRFF>();

/**
 *This kernel calculates the main portion of the cahn-hilliard residual for the RFF form of the phase field crystal model
 **/
class CHPFCRFF : public CHBulk
{
public:

  CHPFCRFF(const std::string & name, InputParameters parameters);
  
protected:

  virtual RealGradient computeGradDFDCons(PFFunctionType type, Real c, RealGradient grad_c);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:

  MooseEnum _log_approach;
  Real _tol;
  std::vector<unsigned int> _vals_var;
  std::vector<VariableGradient *> _grad_vals;
  unsigned int _num_L;
  
  
  
  
};
#endif //CHPFCRFF_H
