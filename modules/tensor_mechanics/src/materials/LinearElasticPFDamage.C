#include "LinearElasticPFDamage.h"


#include <cmath>

extern "C" void dsyev_ ( ... );

template<>
InputParameters validParams<LinearElasticPFDamage>()
{
  InputParameters params = validParams<TensorMechanicsMaterial>();
  params.addRequiredCoupledVar("c","Concentration");
  params.addParam<FunctionName>("function", "", "Bilinear function describing property distribution");
  params.addParam<Real>("kvoid",1e-6,"Stiffness of cracked matrix");
  params.addParam<bool>("numerical_jac",true,"Flag to evaluate numerical jacobian: Used form formulation 0");
  params.addParam<std::string>("void_prop_file_name","", "Name of the file containing d values");
  params.addParam<unsigned int>("formulation",0,"Formulation type");//As implemented - 0 (Miehe et. al.), 1(Damage in compression)
  return params;
}

LinearElasticPFDamage::LinearElasticPFDamage(const std::string & name,
                                             InputParameters parameters) :
    TensorMechanicsMaterial(name, parameters),
    _delasticity_tensor_dc(declareProperty<ElasticityTensorR4>("delasticity_tensor_dc")),
    _d2elasticity_tensor_dc2(declareProperty<ElasticityTensorR4>("d2elasticity_tensor_dc2")),
    _dstress_dc(declareProperty<RankTwoTensor>("dstress_dc")),
    _c(coupledValue("c")),
    _G0_pos(declareProperty<Real>("G0_pos")),
    _dG0_pos_dstrain(declareProperty<RankTwoTensor>("dG0_pos_dstrain")),
    _d_void(declareProperty<Real>("d_void")),
    _d_void_old(declarePropertyOld<Real>("d_void")),
    _has_function(getParam<FunctionName>("function") != ""),
    _function( _has_function ? &getFunction("function") : NULL ),
    _kvoid(getParam<Real>("kvoid")),
    _num_jac(getParam<bool>("numerical_jac")),
    _void_prop_file_name(getParam<std::string>("void_prop_file_name")),
    _form_type(getParam<unsigned int>("formulation")),
    _c_gauss(declareProperty<Real>("c_gauss"))
{
  if (_has_function && !_function)
    mooseError ("Unable to find function in LinearElasticPFDamage.");
}

void LinearElasticPFDamage::initQpStatefulProperties()
{

  _d_void[_qp]=1.0;

  read_prop();

  func_prop();

}

void LinearElasticPFDamage::computeQpStrain()
{
  RankTwoTensor grad_tensor(_grad_disp_x[_qp],_grad_disp_y[_qp], _grad_disp_z[_qp]);
  _elastic_strain[_qp] = (grad_tensor + grad_tensor.transpose())/2.0;
}

void LinearElasticPFDamage::computeQpStress()
{
  _stress[_qp].zero();
  update_var(_elastic_strain[_qp]);
}

void LinearElasticPFDamage::computeQpElasticityTensor()
{
}

void
LinearElasticPFDamage::read_prop()
{
  std::ifstream fileprop;
  Real xmin,xmax,ymin,ymax;
  int istart=0;

  Real xgp,ygp;

  xgp=_q_point[_qp](0);
  ygp=_q_point[_qp](1);

  _d_void[_qp]=1.0;

  if (_void_prop_file_name.length()!=0)
  {
    fileprop.open(_void_prop_file_name.c_str());

    if (!fileprop)
      mooseError("Can't open property file ");
    else
    {
      while (!fileprop.eof())
      {
        fileprop >> xmin;
        fileprop >> xmax;
        fileprop >> ymin;
        fileprop >> ymax;

        if (istart==0)
        {
          if ((xgp < xmin || xgp > xmax) && (ygp < ymin || ygp > ymax) )
          {
            _d_void[_qp]=1.0;
            fileprop.close();
            return;
          }

          istart=1;
        }
        else
        {
          if ((xgp > xmin && xgp < xmax) && (ygp > ymin && ygp < ymax) )
          {
            _d_void[_qp]=1e-8;
            fileprop.close();
            return;
          }
        }
      }

      fileprop.close();
    }
  }
}

void
LinearElasticPFDamage::func_prop()
{

  if (_has_function)
  {
    // Input file data structure consistent with multilinear function system
    // 1st set of grid data for dvoid, read with time flag 0.0
    // 2nd set of grid data for gc in tension, read with time flag 1.0
    // 3rd set of grid data for gc in compression, read with time flag 2.0
    // Refer to test plate_hole_gbcrack.i
    Real t = 0;
    _d_void[_qp] = _function->value(t, _q_point[_qp]) + 1e-8;
  }

}


void LinearElasticPFDamage::update_var(RankTwoTensor &strain)
{

  _c_gauss[_qp] = _c[_qp];

  if (_form_type == 0) //Miehe et. al. (only for Isotropic Elasticity)
  {
    RankTwoTensor etens1, etens2, etens3;
    Real cmat[3][3], w[3], work[20];
    int info;
    int nd=3;
    int lwork=20;
    RankTwoTensor stress0pos,stress0neg,stress0;
    Real evec1[3],evec2[3],evec3[3];

    Real lambda=_Cijkl(0,0,1,1)*_d_void[_qp];
    Real mu=_Cijkl(0,1,0,1)*_d_void[_qp];


    _elasticity_tensor[_qp]=_Cijkl;
    _elasticity_tensor[_qp]*=_d_void[_qp];


    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        cmat[i][j] = strain(i,j);


    dsyev_("V","U",&nd,cmat,&nd,&w,&work,&lwork,&info);

    for (int i=0;i <3; i++)
    {
      evec1[i]=cmat[0][i];
      evec2[i]=cmat[1][i];
      evec3[i]=cmat[2][i];
    }


    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
      {
        etens1(i,j)=evec1[i]*evec1[j];
        etens2(i,j)=evec2[i]*evec2[j];
        etens3(i,j)=evec3[i]*evec3[j];
      }

    Real etr=w[0]+w[1]+w[2];
    Real etrpos=(std::abs(etr)+etr)/2.0;
    Real etrneg=(std::abs(etr)-etr)/2.0;

    stress0pos =   etens1 * (lambda * etrpos + 2.0 * mu * (std::abs(w[0])+w[0])/2.0)
                 + etens2 * (lambda * etrpos + 2.0 * mu * (std::abs(w[1])+w[1])/2.0)
                 + etens3 * (lambda * etrpos + 2.0 * mu * (std::abs(w[2])+w[2])/2.0);
    stress0neg =   etens1 * (lambda * etrneg + 2.0 * mu * (std::abs(w[0])-w[0])/2.0)
                 + etens2 * (lambda * etrneg + 2.0 * mu * (std::abs(w[1])-w[1])/2.0)
                 + etens3 * (lambda * etrneg + 2.0 * mu * (std::abs(w[2])-w[2])/2.0);

    Real c=_c[_qp];


    Real xfac = std::pow(1.0-c,2.0)+_kvoid;

    _stress[_qp]=stress0pos*xfac-stress0neg;

    Real e1pos = (std::abs(w[0])+w[0])/2.0;
    Real e2pos = (std::abs(w[1])+w[1])/2.0;
    Real e3pos = (std::abs(w[2])+w[2])/2.0;

    _G0_pos[_qp] = lambda * std::pow(etrpos,2.0)/2.0 + mu*(std::pow(e1pos,2.0) + std::pow(e2pos,2.0) + std::pow(e3pos,2.0));

    _dG0_pos_dstrain[_qp] = stress0pos;

    _dstress_dc[_qp]=-stress0pos*(2*(1.0-c));

    if ( _num_jac )
      calc_num_jac(strain);
    else
      _Jacobian_mult[_qp] = _elasticity_tensor[_qp];

  }
  else if (_form_type == 1)//Damage in Compression
  {
    _elasticity_tensor[_qp]=_Cijkl;
    _elasticity_tensor[_qp]*=_d_void[_qp];

    Real c = _c[_qp];

    RankTwoTensor stress0=_elasticity_tensor[_qp]*strain;

    Real xfac = std::pow(1.0-c,2.0) + _kvoid;
    _stress[_qp] = stress0 * xfac;


    _G0_pos[_qp] = 0.5 * stress0.doubleContraction(strain);

    _dG0_pos_dstrain[_qp] = stress0 * 0.5;

    _elasticity_tensor[_qp] *= xfac;
    _Jacobian_mult[_qp] = _elasticity_tensor[_qp];

    _dstress_dc[_qp] = -stress0*(2*(1.0-c));

  }
  else
    mooseError("Provide appropriate formulation type ");
}

void LinearElasticPFDamage::calc_num_jac(RankTwoTensor &strain0)
{
  if (_form_type == 0)
  {
    for (unsigned int ke = 0; ke < 3; ++ke)
      for (unsigned int le = ke; le < 3; ++le)
      {
        RankTwoTensor strain=strain0;
        Real pert;

        if (std::abs(strain(ke,le)) > 1e-4)
          pert = 1e-2 * strain(ke,le);
        else
        {
          Real signp = 1.0;
          if (strain(ke,le) < 0.0)
            signp=-1.0;

          pert = 1e-4 * signp;
        }

        strain(ke,le) = strain(ke,le) + pert;
        strain(le,ke) = strain(ke,le);

        RankTwoTensor etens1, etens2, etens3;
        Real cmat[3][3], w[3], work[20];
        int info;
        int nd=3;
        int lwork=20;
        RankTwoTensor stress0pos, stress0neg, stress0;
        Real evec1[3], evec2[3], evec3[3];

        Real lambda = _Cijkl(0,0,1,1) * _d_void[_qp];
        Real mu = _Cijkl(0,1,0,1)  *_d_void[_qp];

        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int j = 0; j < 3; ++j)
            cmat[i][j] = strain(i,j);

        dsyev_("V","U",&nd,cmat,&nd,&w,&work,&lwork,&info);

        for (unsigned int i = 0; i < 3; ++i)
        {
          evec1[i] = cmat[0][i];
          evec2[i] = cmat[1][i];
          evec3[i] = cmat[2][i];
        }

        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int j = 0; j < 3; ++j)
          {
            etens1(i,j) = evec1[i] * evec1[j];
            etens2(i,j) = evec2[i] * evec2[j];
            etens3(i,j) = evec3[i] * evec3[j];
          }

        Real etr=w[0]+w[1]+w[2];
        Real etrpos = (std::abs(etr) + etr)/2.0;
        Real etrneg = (std::abs(etr) - etr)/2.0;

        stress0pos =   etens1 * (lambda * etrpos + 2.0*mu * (std::abs(w[0])+w[0])/2.0)
                     + etens2 * (lambda * etrpos + 2.0*mu * (std::abs(w[1])+w[1])/2.0)
                     + etens3 * (lambda * etrpos + 2.0*mu * (std::abs(w[2])+w[2])/2.0);
        stress0neg =   etens1 * (lambda * etrneg + 2.0*mu * (std::abs(w[0])-w[0])/2.0)
                     + etens2 * (lambda * etrneg + 2.0*mu * (std::abs(w[1])-w[1])/2.0)
                     + etens3 * (lambda * etrneg + 2.0*mu * (std::abs(w[2])-w[2])/2.0);

        Real c = _c[_qp];

        Real xfac = std::pow(1.0-c, 2.0) + _kvoid;

        RankTwoTensor stress;

        stress = stress0pos * xfac - stress0neg;
        stress0 = _stress[_qp];

        for (unsigned int is = 0; is < 3; ++is)
          for (unsigned int js = 0; js < 3; ++js)
          {
            _Jacobian_mult[_qp](is,js,ke,le) = (stress(is,js) - stress0(is,js)) / (strain(ke,le) - strain0(ke,le));

            if (is != js && ke != le)
              _Jacobian_mult[_qp](is,js,ke,le) = _Jacobian_mult[_qp](is,js,ke,le)/2.0;

            _Jacobian_mult[_qp](is,js,le,ke) = _Jacobian_mult[_qp](is,js,ke,le);
          }
      }
  }
}
