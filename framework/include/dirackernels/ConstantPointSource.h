#ifndef CONSTANTPOINTSOURCE_H_
#define CONSTANTPOINTSOURCE_H_

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class ConstantPointSource;

template<>
InputParameters validParams<ConstantPointSource>();

class ConstantPointSource : public DiracKernel
{
public:
  ConstantPointSource(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  Real _value;
  std::vector<Real> _point_param;
  Point _p;
};
 
#endif //CONSTANTPOINTSOURCE_H_
