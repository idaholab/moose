#include "FiniteStrainWeakPlaneTensile.h"
#include <math.h> // for M_PI
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<FiniteStrainWeakPlaneTensile>()
{
  InputParameters params = validParams<FiniteStrainPlasticBase>();
  params.addRequiredRangeCheckedParam<Real>("wpt_tensile_strength", "wpt_tensile_strength>=0", "Weak plane tensile strength");
  params.addParam<Real>("wpt_tensile_strength_residual", "Tenile strength at infinite hardening.  If not given, this defaults to wpt_tensile_strength, ie, perfect plasticity");
  params.addRangeCheckedParam<Real>("wpt_tensile_strength_rate", 0, "wpt_tensile_strength_rate>=0", "Tensile strength = wpt_tensile_strenght_residual + (wpt_tensile_strength - wpt_tensile_strength_residual)*exp(-wpt_tensile_rate*plasticstrain).  Set to zero for perfect plasticity");
  params.addRequiredParam<RealVectorValue>("wpt_normal_vector", "The normal vector to the weak plane");
  params.addParam<bool>("wpt_normal_rotates", true, "The normal vector to the weak plane rotates with the large deformations");
  params.addClassDescription("Non-associative weak-plane tensile plasticity with no hardening");

  return params;
}

FiniteStrainWeakPlaneTensile::FiniteStrainWeakPlaneTensile(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainPlasticBase(name, parameters),
    _tension_cutoff(getParam<Real>("wpt_tensile_strength")),
    _tension_cutoff_residual(parameters.isParamValid("wpt_tensile_strength_residual") ? getParam<Real>("wpt_tensile_strength_residual") : _tension_cutoff),
    _tension_cutoff_rate(getParam<Real>("wpt_tensile_strength_rate")),
    _input_n(getParam<RealVectorValue>("wpt_normal_vector")),
    _normal_rotates(getParam<bool>("wpt_normal_rotates")),

    _n(declareProperty<RealVectorValue>("weak_plane_normal")),
    _n_old(declarePropertyOld<RealVectorValue>("weak_plane_normal")),
    _wpt_internal(declareProperty<Real>("weak_plane_tensile_internal")),
    _wpt_internal_old(declarePropertyOld<Real>("weak_plane_tensile_internal")),
    _yf(declareProperty<Real>("weak_plane_tensile_yield_function"))
{
   if (_input_n.size() == 0)
     mooseError("Weak-plane normal vector must not have zero length");
   else
     _input_n /= _input_n.size();
}

void FiniteStrainWeakPlaneTensile::initQpStatefulProperties()
{
  _n[_qp] = _input_n;
  _n_old[_qp] = _input_n;
  _wpt_internal[_qp] = 0;
  _wpt_internal_old[_qp] = 0;
  _yf[_qp] = 0.0;
  FiniteStrainPlasticBase::initQpStatefulProperties();
}

void
FiniteStrainWeakPlaneTensile::preReturnMap()
{
  // this is a rotation matrix that will rotate _n to the "z" axis
  RealTensorValue rot = RotationMatrix::rotVecToZ(_n[_qp]);

  // rotate the tensors to this frame
  _elasticity_tensor[_qp].rotate(rot);
  _stress_old[_qp].rotate(rot);
  _plastic_strain_old[_qp].rotate(rot);
  _strain_increment[_qp].rotate(rot);
}

void
FiniteStrainWeakPlaneTensile::postReturnMap()
{
  // this is a rotation matrix that will rotate "z" axis back to _n
  RealTensorValue rot = RotationMatrix::rotVecToZ(_n[_qp]).transpose();

  // rotate the tensors back to original frame where _n is correctly oriented
  _elasticity_tensor[_qp].rotate(rot);
  _stress_old[_qp].rotate(rot);
  _plastic_strain_old[_qp].rotate(rot);
  _strain_increment[_qp].rotate(rot);
  _stress[_qp].rotate(rot);
  _plastic_strain[_qp].rotate(rot);

  //Rotate n by _rotation_increment, if necessary
  if (_normal_rotates)
  {
    for (unsigned int i = 0 ; i < LIBMESH_DIM ; ++i)
    {
      _n[_qp](i) = 0;
      for (unsigned int j = 0 ; j < 3 ; ++j)
        _n[_qp](i) += _rotation_increment[_qp](i, j)*_n_old[_qp](j);
    }
  }

  // Record the value of the yield function
  _yf[_qp] = _f[_qp][0];

  // Record the value of the internal parameter
  _wpt_internal[_qp] = _intnl[_qp][0];
}



unsigned int
FiniteStrainWeakPlaneTensile::numberOfInternalParameters()
{
  return 1;
}

void
FiniteStrainWeakPlaneTensile::yieldFunction(const RankTwoTensor &stress, const std::vector<Real> & intnl, std::vector<Real> & f)
{
  f.assign(1, stress(2,2) - tensile_strength(intnl[0]));
}

void
FiniteStrainWeakPlaneTensile::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.assign(1, RankTwoTensor());
  df_dstress[0](2, 2) = 1.0;
}

void
FiniteStrainWeakPlaneTensile::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & df_dintnl)
{
  df_dintnl.resize(1);
  df_dintnl[0].assign(1, - dtensile_strength(intnl[0]));
}

void
FiniteStrainWeakPlaneTensile::flowPotential(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<RankTwoTensor> & r)
{
  r.assign(1, RankTwoTensor());
  r[0](2, 2) = 1.0;
}

void
FiniteStrainWeakPlaneTensile::dflowPotential_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<RankFourTensor> & dr_dstress)
{
  dr_dstress.assign(1, RankFourTensor());
}

void
FiniteStrainWeakPlaneTensile::dflowPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<RankTwoTensor> > & dr_dintnl)
{
  dr_dintnl.resize(1);
  dr_dintnl[0].assign(1, RankTwoTensor());
}

void
FiniteStrainWeakPlaneTensile::hardPotential(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<Real> > & h)
{
  h.resize(1);
  h[0].assign(1, -1.0);
}

void
FiniteStrainWeakPlaneTensile::dhardPotential_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<RankTwoTensor> > & dh_dstress)
{
  dh_dstress.resize(1);
  dh_dstress[0].assign(1, RankTwoTensor());
}

void
FiniteStrainWeakPlaneTensile::dhardPotential_dintnl(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<std::vector<std::vector<Real> > > & dh_dintnl)
{
  dh_dintnl.resize(1);
  dh_dintnl[0].resize(1);
  dh_dintnl[0][0].assign(1, 0.0);
}


Real
FiniteStrainWeakPlaneTensile::tensile_strength(const Real internal_param)
{
  return _tension_cutoff_residual + (_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}

Real
FiniteStrainWeakPlaneTensile::dtensile_strength(const Real internal_param)
{
  return -_tension_cutoff_rate*(_tension_cutoff - _tension_cutoff_residual)*std::exp(-_tension_cutoff_rate*internal_param);
}
