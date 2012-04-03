#include "MaterialTensorAux.h"

#include "SymmTensor.h"

template<>
InputParameters validParams<MaterialTensorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addParam<int>("index", -1, "The index into the tensor, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  params.addParam<std::string>("quantity", "", "A scalar quantity to compute: VonMises, Hydrostatic, Hoop, FirstInvariant, SecondInvariant, ThirdInvariant, TriAxiality.");
  std::vector<Real> fred(3,0.0);
  params.addParam<std::vector<Real> >("point1", fred, "Point one for defining an axis");
  fred[1] = 1;
  params.addParam<std::vector<Real> >("point2", fred, "Point two for defining an axis");
  return params;
}

MaterialTensorAux::MaterialTensorAux( const std::string & name,
                                      InputParameters parameters )
  :AuxKernel( name, parameters ),
   _tensor( getMaterialProperty<SymmTensor>( getParam<std::string>("tensor") ) ),
   _index( getParam<int>("index") ),
   _quantity_string( getParam<std::string>("quantity") ),
   _quantity( MTA_COMPONENT ),
   _p1( getParam<std::vector<Real> >("point1")[0], getParam<std::vector<Real> >("point1")[1], getParam<std::vector<Real> >("point1")[2] ),
   _p2( getParam<std::vector<Real> >("point2")[0], getParam<std::vector<Real> >("point2")[1], getParam<std::vector<Real> >("point2")[2] )
{
  std::transform(_quantity_string.begin(), _quantity_string.end(),
                 _quantity_string.begin(), ::tolower);
  if ( _quantity_string == "" && _index < 0 )
  {
    mooseError("Neither an index nor a quantity listed for " + _name);
  }
  else if ( _quantity_string == "vonmises" )
  {
    _quantity = MTA_VONMISES;
  }
  else if ( _quantity_string == "plasticstrainmag" )
  {
    _quantity = MTA_PLASTICSTRAINMAG;
  }
  else if ( _quantity_string == "hydrostatic" )
  {
    _quantity = MTA_HYDROSTATIC;
  }
  else if ( _quantity_string == "hoop" )
  {
    _quantity = MTA_HOOP;
  }
  else if ( _quantity_string == "firstinvariant" )
  {
    _quantity = MTA_FIRSTINVARIANT;
  }
  else if ( _quantity_string == "secondinvariant" )
  {
    _quantity = MTA_SECONDINVARIANT;
  }
  else if ( _quantity_string == "thirdinvariant" )
  {
    _quantity = MTA_THIRDINVARIANT;
  }
  else if ( _quantity_string == "triaxiality" )
  {
    _quantity = MTA_TRIAXIALITY;
  }
  else if ( _quantity_string != "" )
  {
    mooseError("Unrecognized quantity in " + _name);
  }

  if (_index > 0 && _quantity != MTA_COMPONENT)
  {
    mooseError("Cannot define an index and a quantity in " + _name);
  }

  if (_index > -1 && _index > 5)
  {
    mooseError("MaterialTensorAux requires the index to be >= 0 and <= 5 OR < 0 (off).");
  }
}

Real
MaterialTensorAux::computeValue()
{
  Real value(0);
  const SymmTensor & tensor( _tensor[_qp] );
  if (_quantity == MTA_COMPONENT)
  {
    value = tensor.component(_index);
  }
  else if ( _quantity == MTA_VONMISES )
  {
    value = std::sqrt(0.5*(
                           std::pow(tensor.xx() - tensor.yy(), 2) +
                           std::pow(tensor.yy() - tensor.zz(), 2) +
                           std::pow(tensor.zz() - tensor.xx(), 2) + 6 * (
                           std::pow(tensor.xy(), 2) +
                           std::pow(tensor.yz(), 2) +
                           std::pow(tensor.zx(), 2))));
  }
  else if ( _quantity == MTA_PLASTICSTRAINMAG )
  {
    value = std::sqrt(2.0/3.0 * tensor.doubleContraction(tensor));
  }
  else if ( _quantity == MTA_HYDROSTATIC )
  {
    value = tensor.trace()/3.0;
  }
  else if ( _quantity == MTA_HOOP )
  {
    if (LIBMESH_DIM == 2)
    {
      value = tensor.zz();
    }
    else
    {
      // This is the location of the stress.  A vector from the cylindrical axis to this point will define the x' axis.
      Point p0( _q_point[_qp] );

      // The vector _p1 + t*(_p2-_p1) defines the cylindrical axis.  The point along this
      // axis closest to p0 is found by the following for t:
      const Point p2p1( _p2 - _p1 );
      const Point p2p0( _p2 - p0 );
      const Point p1p0( _p1 - p0 );
      const Real t( -(p1p0*p2p1)/p2p1.size_sq() );
      // The nearest point on the cylindrical axis to p0 is p.
      const Point p( _p1 + t * p2p1 );
      Point xp( p0 - p );
      xp /= xp.size();
      Point yp( p2p1/p2p1.size() );
      Point zp( xp.cross( yp ));
      //
      // The following works but does more than we need
      //
//      // Rotation matrix R
//      ColumnMajorMatrix R(3,3);
//      // Fill with direction cosines
//      R(0,0) = xp(0);
//      R(1,0) = xp(1);
//      R(2,0) = xp(2);
//      R(0,1) = yp(0);
//      R(1,1) = yp(1);
//      R(2,1) = yp(2);
//      R(0,2) = zp(0);
//      R(1,2) = zp(1);
//      R(2,2) = zp(2);
//      // Rotate
//      ColumnMajorMatrix tensor( _tensor[_qp].columnMajorMatrix() );
//      ColumnMajorMatrix tensorp( R.transpose() * ( tensor * R ));
//      // Hoop stress is the zz stress
//      value = tensorp(2,2);
      //
      // Instead, tensorp(2,2) = R^T(2,:)*tensor*R(:,2)
      //
      const Real zp0( zp(0) );
      const Real zp1( zp(1) );
      const Real zp2( zp(2) );
      value = (zp0*tensor(0,0)+zp1*tensor(1,0)+zp2*tensor(2,0))*zp0 +
              (zp0*tensor(0,1)+zp1*tensor(1,1)+zp2*tensor(2,1))*zp1 +
              (zp0*tensor(0,2)+zp1*tensor(1,2)+zp2*tensor(2,2))*zp2;
    }
  }
  else if ( _quantity == MTA_FIRSTINVARIANT )
  {
    value = tensor.trace();
  }
  else if ( _quantity == MTA_SECONDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy() +
      tensor.yy()*tensor.zz() +
      tensor.zz()*tensor.xx() -
      tensor.xy()*tensor.xy() -
      tensor.yz()*tensor.yz() -
      tensor.zx()*tensor.zx();
  }
  else if ( _quantity == MTA_THIRDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy()*tensor.zz() -
      tensor.xx()*tensor.yz()*tensor.yz() +
      tensor.xy()*tensor.zx()*tensor.yz() -
      tensor.xy()*tensor.xy()*tensor.zz() +
      tensor.zx()*tensor.xy()*tensor.yz() -
      tensor.zx()*tensor.zx()*tensor.yy();
  }
  else if ( _quantity == MTA_TRIAXIALITY )
  {
    Real hydrostatic = tensor.trace()/3.0;
    Real von_mises = std::sqrt(0.5*(
                                 std::pow(tensor.xx() - tensor.yy(), 2) +
                                 std::pow(tensor.yy() - tensor.zz(), 2) +
                                 std::pow(tensor.zz() - tensor.xx(), 2) + 6 * (
                                   std::pow(tensor.xy(), 2) +
                                   std::pow(tensor.yz(), 2) +
                                   std::pow(tensor.zx(), 2))));

    value = std::abs(hydrostatic / von_mises);
  }
  else
  {
    mooseError("Internal logic error from " + name());
  }
  return value;
}


