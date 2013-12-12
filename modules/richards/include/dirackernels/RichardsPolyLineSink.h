#ifndef RICHARDSPOLYLINESINK_H
#define RICHARDSPOLYLINESINK_H

#include "DiracKernel.h"
#include "LinearInterpolation.h"

//Forward Declarations
class RichardsPolyLineSink;

template<>
InputParameters validParams<RichardsPolyLineSink>();

class RichardsPolyLineSink : public DiracKernel
{
public:
  RichardsPolyLineSink(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();


protected:
  PostprocessorValue & _reporter;
  LinearInterpolation _sink_func;
  std::string _point_file;

  std::vector<Real> _xs;
  std::vector<Real> _ys;
  std::vector<Real> _zs;

  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec);
};

#endif //RICHARDSPOLYLINESINK_H
