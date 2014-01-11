#ifndef RICHARDSBOREHOLE_H
#define RICHARDSBOREHOLE_H

// Moose Includes
#include "DiracKernel.h"
#include "RichardsSumQuantity.h"
#include "RichardsPorepressureNames.h"

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
  bool _debug_things;

  const RichardsPorepressureNames & _pp_name_UO;
  unsigned int _pvar;

  Real _well_constant_production;
  Real _well_constant_injection;
  Real _p_bot;
  RealVectorValue _unit_weight;

  MaterialProperty<std::vector<Real> > &_viscosity;

  MaterialProperty<RealTensorValue> & _permeability;

  MaterialProperty<std::vector<std::vector<Real> > > &_dseff;

  MaterialProperty<std::vector<Real> > &_rel_perm;
  MaterialProperty<std::vector<Real> > &_drel_perm;

  MaterialProperty<std::vector<Real> > &_density;
  MaterialProperty<std::vector<Real> > &_ddensity;
  
  RichardsSumQuantity & _total_outflow_mass;
  std::string _point_file;

  std::vector<Real> _xs;
  std::vector<Real> _ys;
  std::vector<Real> _zs;
  Point _bottom_point;

  std::vector<Real> _half_seg_len;

  std::vector<RealTensorValue> _rot_matrix;

  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec);
  RealTensorValue rotVecToZ(RealVectorValue v2);
};

#endif //RICHARDSBOREHOLE_H
