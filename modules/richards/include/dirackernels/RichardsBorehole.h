#ifndef RICHARDSBOREHOLE_H
#define RICHARDSBOREHOLE_H

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class RichardsBorehole;

template<>
InputParameters validParams<RichardsBorehole>();

class RichardsBorehole : public DiracKernel
{
public:
  RichardsBorehole(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

protected:
  unsigned int _this_var_num;
  MaterialProperty<std::vector<unsigned int> > &_p_var_nums;

  Real _well_constant_production;
  Real _well_constant_injection;
  Real _p_bot;
  RealVectorValue _unit_weight;

  MaterialProperty<std::vector<Real> > &_viscosity;

  MaterialProperty<std::vector<std::vector<Real> > > &_dseff;

  MaterialProperty<std::vector<Real> > &_rel_perm;
  MaterialProperty<std::vector<Real> > &_drel_perm;

  MaterialProperty<std::vector<Real> > &_density;
  MaterialProperty<std::vector<Real> > &_ddensity;

  PostprocessorValue & _reporter;
  std::string _point_file;

  std::vector<Real> _xs;
  std::vector<Real> _ys;
  std::vector<Real> _zs;
  Point _bottom_point;

  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec);
};

#endif //RICHARDSBOREHOLE_H
