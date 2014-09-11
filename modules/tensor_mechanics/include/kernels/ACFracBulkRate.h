#ifndef ACFRACBULKRATE_H
#define ACFRACBULKRATE_H
/*
Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311
Equation 63
*/
#include "KernelValue.h"
#include "RankTwoTensor.h"

//Forward Declarations
class ACFracBulkRate;

template<>
InputParameters validParams<ACFracBulkRate>();


class ACFracBulkRate : public KernelValue
{
public:

  ACFracBulkRate(const std::string & name, InputParameters parameters);

protected:

  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeDFDOP(PFFunctionType type);
  std::string _mob_name;
  MaterialProperty<Real> & _L;
  MaterialProperty<Real> & _gc_prop_tens;
  //Critical energy release rate for fracture due to positive or hydrostatic component of stress to energy
  //Read from input file, slow

  /**
   * Coupled things come through as std::vector _refernces_.
   *
   * Since this is a reference it MUST be set in the Initialization List of the
   * constructor!
   */

  MaterialProperty<Real> & _G0_pos;//Contribution from positive or hydrostatic component of stress to energy
  MaterialProperty<RankTwoTensor> & _dG0_pos_dstrain;
  VariableValue & _betaval;//Auxiliary variable
  unsigned int _beta_var;

  const bool _xdisp_coupled;
  const bool _ydisp_coupled;
  const bool _zdisp_coupled;

  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;

  Real _l;//Characteristic length, controls crack thickness
  Real _visco;//Viscosity (10^-3 provides rate independent behavior, slow convergence)

 private:

};
#endif //ACFRACBULKMIEHE_H
