#ifndef RICHARDSMASS_H
#define RICHARDSMASS_H

#include "ElementIntegralVariablePostprocessor.h"

//Forward Declarations
class RichardsMass;

template<>
InputParameters validParams<RichardsMass>();

/**
 * This postprocessor computes the fluid mass by integrating the density over the volume
 *
 */
class RichardsMass: public ElementIntegralVariablePostprocessor
{
public:
  RichardsMass(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  unsigned int _this_var_num;
  MaterialProperty<std::vector<unsigned int> > &_p_var_nums;

  MaterialProperty<Real> &_porosity;
  MaterialProperty<std::vector<Real> > &_sat;
  MaterialProperty<std::vector<Real> > &_density;
};

#endif
