#include "ACFracBulkRateMaterial.h"

template<>
InputParameters validParams<ACFracBulkRateMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<FunctionName>("function", "", "Multilinear function describing property distribution");
  params.addParam<Real>("L0", 1.0, "The mobility parameter for order parameter");
  params.addParam<Real>("gc_tens", 1.0, "Energy release rate type parameter");
  params.addParam<std::string>("frac_prop_file_name", "", "Name of the file containing gc");
  return params;
}

ACFracBulkRateMaterial::ACFracBulkRateMaterial(const std::string & name,
                                             InputParameters parameters) :
    Material(name, parameters),
    _L(declareProperty<Real>("L")),
    _gc_prop_tens(declareProperty<Real>("gc_prop_tens")),
    _gc_prop_tens_old(declarePropertyOld<Real>("gc_prop_tens")),
    _has_function(getParam<FunctionName>("function") != ""),
    _function(_has_function ? &getFunction("function") : NULL),
    _L0(getParam<Real>("L0")),
    _gc_tens(getParam<Real>("gc_tens")),
    _frac_prop_file_name(getParam<std::string>("frac_prop_file_name"))
{
  if (_has_function && !_function)
    mooseError("Unable to find function in FractureACBulkMatRate.");
}

void ACFracBulkRateMaterial::initQpStatefulProperties()
{

  _gc_prop_tens[_qp] = _gc_tens;

  read_prop();

  func_prop();

}

void
ACFracBulkRateMaterial::computeQpProperties()
{
  _L[_qp]=_L0;
}

void
ACFracBulkRateMaterial::read_prop()
{
  std::ifstream fileprop;
  Real val;
  unsigned int elemno;


  if (_frac_prop_file_name.length()!=0)
  {
    fileprop.open(_frac_prop_file_name.c_str());

    if (!fileprop)
      mooseError("Can't open property file ");
    else
    {
      while (!fileprop.eof())
      {
        fileprop >> elemno;
        fileprop >> val;

        if (elemno - 1 == _current_elem->id())
        {
          _gc_prop_tens[_qp]=val;


          fileprop.close();
          return;
        }
      }

      fileprop.close();
    }
  }
}


void
ACFracBulkRateMaterial::func_prop()
{

  if (_has_function)
  {
    // Input file data structure consistent with multilinear function system
    // 1st set of grid data for dvoid, read with time flag 0.0
    // 2nd set of grid data for gc in tension, read with time flag 1.0
    // 3rd set of grid data for gc in compression, read with time flag 2.0
    // Refer to test plate_hole_gbcrack.i

    _gc_prop_tens[_qp] = _function->value(1.0, _q_point[_qp]);

  }


}
