#include "FiniteStrainWeakPlaneTensile.h"
#include <math.h> // for M_PI
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<FiniteStrainWeakPlaneTensile>()
{
  InputParameters params = validParams<FiniteStrainPlasticBase>();
  params.addRequiredRangeCheckedParam<Real>("tension_cutoff", "tension_cutoff>=0", "Weak plane Tension cutoff");
  params.addRequiredParam<RealVectorValue>("wpt_normal_vector", "The normal vector to the weak plane");
  params.addParam<bool>("wpt_normal_rotates", true, "The normal vector to the weak plane rotates with the large deformations");
  params.addClassDescription("Non-associative weak-plane tensile plasticity with no hardening");

  return params;
}

FiniteStrainWeakPlaneTensile::FiniteStrainWeakPlaneTensile(const std::string & name,
                                                         InputParameters parameters) :
    FiniteStrainPlasticBase(name, parameters),
    _tension_cutoff(getParam<Real>("tension_cutoff")),
    _input_n(getParam<RealVectorValue>("wpt_normal_vector")),
    _normal_rotates(getParam<bool>("wpt_normal_rotates")),

    _n(declareProperty<RealVectorValue>("weak_plane_normal")),
    _n_old(declarePropertyOld<RealVectorValue>("weak_plane_normal")),
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
}


void
FiniteStrainWeakPlaneTensile::yieldFunction(const RankTwoTensor &stress, const std::vector<Real> & /*intnl*/, std::vector<Real> & f)
{
  f.assign(1, stress(2,2) - _tension_cutoff);
}

void
FiniteStrainWeakPlaneTensile::dyieldFunction_dstress(const RankTwoTensor & /*stress*/, const std::vector<Real> & /*intnl*/, std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.assign(1, RankTwoTensor());
  df_dstress[0](2, 2) = 1.0;
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
