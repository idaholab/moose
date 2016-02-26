/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "MaterialTensorCalculator.h"

template<>
InputParameters validParams<MaterialTensorCalculator>()
{
  InputParameters params = emptyInputParameters();
  MooseEnum quantities("VonMises=1 PlasticStrainMag Hydrostatic Direction Hoop Radial Axial MaxPrincipal MedPrincipal MinPrincipal FirstInvariant SecondInvariant ThirdInvariant TriAxiality VolumetricStrain");

  params.addParam<int>("index", -1, "The index into the tensor, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  params.addParam<MooseEnum>("quantity", quantities, "A scalar quantity to compute: " + quantities.getRawNames());

  params.addParam<RealVectorValue>("point1", RealVectorValue(0, 0, 0), "Start point for axis used to calculate some material tensor quantities");
  params.addParam<RealVectorValue>("point2", RealVectorValue(0, 1, 0), "End point for axis used to calculate some material tensor quantities");
  params.addParam<RealVectorValue>("direction", RealVectorValue(1, 0, 0), "Direction vector");
  return params;
}

MaterialTensorCalculator::MaterialTensorCalculator(const InputParameters & parameters):
  _index(parameters.get<int>("index")),
  _quantity_moose_enum(parameters.get<MooseEnum>("quantity")),
  _p1(parameters.get<RealVectorValue>("point1")),
  _p2(parameters.get<RealVectorValue>("point2")),
  _direction(parameters.get<RealVectorValue>("direction")/parameters.get<RealVectorValue>("direction").norm())
{
  const std::string & name = parameters.get<std::string>("_object_name");

  if (_quantity_moose_enum.isValid())
  {
    if ( _index > 0 )
      mooseError("Cannot define an index and a quantity in " + name);
    else
      _quantity = QUANTITY_ENUM(int(_quantity_moose_enum));
  }
  else
  {
    if ( _index < 0 )
      mooseError("Neither an index nor a quantity listed for " + name);
    else
      _quantity = COMPONENT; // default
  }

  if (_index > -1 && _index > 5)
  {
    mooseError("The material tensor index must be >= 0 and <= 5 OR < 0 (off).");
  }
}

Real
MaterialTensorCalculator::getTensorQuantity(const SymmTensor & tensor,
                                            const Point * curr_point,
                                            RealVectorValue &direction)
{
  direction.zero();
  Real value(0);
  if (_quantity == COMPONENT)
  {
    value = tensor.component(_index);
    if (_index < 3)
    {
      direction(_index) = 1.0;
    }
    else if (_index == 3)//xy
    {
      direction(0) = std::sqrt(0.5);
      direction(1) = direction(0);
    }
    else if (_index == 4)//yz
    {
      direction(1) = std::sqrt(0.5);
      direction(2) = direction(1);
    }
    else if (_index == 5)//zx
    {
      direction(0) = std::sqrt(0.5);
      direction(2) = direction(0);
    }
  }
  else if ( _quantity == VONMISES )
  {
    value = std::sqrt(0.5*(
                           std::pow(tensor.xx() - tensor.yy(), 2) +
                           std::pow(tensor.yy() - tensor.zz(), 2) +
                           std::pow(tensor.zz() - tensor.xx(), 2) + 6 * (
                           std::pow(tensor.xy(), 2) +
                           std::pow(tensor.yz(), 2) +
                           std::pow(tensor.zx(), 2))));
  }
  else if ( _quantity == PLASTICSTRAINMAG )
  {
    value = std::sqrt(2.0/3.0 * tensor.doubleContraction(tensor));
  }
  else if ( _quantity == HYDROSTATIC )
  {
    value = tensor.trace()/3.0;
  }
  else if ( _quantity == DIRECTION )
  {
    const Real p0 = _direction(0);
    const Real p1 = _direction(1);
    const Real p2 = _direction(2);
    value = (p0*tensor(0,0)+p1*tensor(1,0)+p2*tensor(2,0))*p0 +
            (p0*tensor(0,1)+p1*tensor(1,1)+p2*tensor(2,1))*p1 +
            (p0*tensor(0,2)+p1*tensor(1,2)+p2*tensor(2,2))*p2;
    direction = _direction;
  }
  else if ( _quantity == HOOP )
  {
    // This is the location of the stress.  A vector from the cylindrical axis to this point will define the x' axis.
    Point p0( *curr_point );

    // The vector _p1 + t*(_p2-_p1) defines the cylindrical axis.  The point along this
    // axis closest to p0 is found by the following for t:
    const Point p2p1( _p2 - _p1 );
    const Point p1p0( _p1 - p0 );
    const Real t( -(p1p0*p2p1)/p2p1.norm_sq() );
    // The nearest point on the cylindrical axis to p0 is p.
    const Point p( _p1 + t * p2p1 );
    Point xp( p0 - p );
    xp /= xp.norm();
    Point yp( p2p1/p2p1.norm() );
    Point zp( xp.cross( yp ));
    //
    // The following works but does more than we need
    //
//    // Rotation matrix R
//    ColumnMajorMatrix R(3,3);
//    // Fill with direction cosines
//    R(0,0) = xp(0);
//    R(1,0) = xp(1);
//    R(2,0) = xp(2);
//    R(0,1) = yp(0);
//    R(1,1) = yp(1);
//    R(2,1) = yp(2);
//    R(0,2) = zp(0);
//    R(1,2) = zp(1);
//    R(2,2) = zp(2);
//    // Rotate
//    ColumnMajorMatrix tensor( _tensor[_qp].columnMajorMatrix() );
//    ColumnMajorMatrix tensorp( R.transpose() * ( tensor * R ));
//    // Hoop stress is the zz stress
//    value = tensorp(2,2);
    //
    // Instead, tensorp(2,2) = R^T(2,:)*tensor*R(:,2)
    //
    const Real zp0( zp(0) );
    const Real zp1( zp(1) );
    const Real zp2( zp(2) );
    value = (zp0*tensor(0,0)+zp1*tensor(1,0)+zp2*tensor(2,0))*zp0 +
            (zp0*tensor(0,1)+zp1*tensor(1,1)+zp2*tensor(2,1))*zp1 +
            (zp0*tensor(0,2)+zp1*tensor(1,2)+zp2*tensor(2,2))*zp2;
    direction = zp;
  }
  else if ( _quantity == RADIAL )
  {
    // This is the location of the stress.  A vector from the cylindrical axis to this point will define the x' axis
    // which is the radial direction in which we want the stress.
    Point p0( *curr_point );

    // The vector _p1 + t*(_p2-_p1) defines the cylindrical axis.  The point along this
    // axis closest to p0 is found by the following for t:
    const Point p2p1( _p2 - _p1 );
    const Point p1p0( _p1 - p0 );
    const Real t( -(p1p0*p2p1)/p2p1.norm_sq() );
    // The nearest point on the cylindrical axis to p0 is p.
    const Point p( _p1 + t * p2p1 );
    Point xp( p0 - p );
    xp /= xp.norm();
    const Real xp0( xp(0) );
    const Real xp1( xp(1) );
    const Real xp2( xp(2) );
    value = (xp0*tensor(0,0)+xp1*tensor(1,0)+xp2*tensor(2,0))*xp0 +
            (xp0*tensor(0,1)+xp1*tensor(1,1)+xp2*tensor(2,1))*xp1 +
            (xp0*tensor(0,2)+xp1*tensor(1,2)+xp2*tensor(2,2))*xp2;
    direction = xp;
  }
  else if ( _quantity == AXIAL )
  {
    // The vector p2p1=(_p2-_p1) defines the axis, which is the direction in which we want the stress.
    Point p2p1( _p2 - _p1 );
    p2p1 /= p2p1.norm();

    const Real axis0( p2p1(0) );
    const Real axis1( p2p1(1) );
    const Real axis2( p2p1(2) );
    value = (axis0*tensor(0,0)+axis1*tensor(1,0)+axis2*tensor(2,0))*axis0 +
            (axis0*tensor(0,1)+axis1*tensor(1,1)+axis2*tensor(2,1))*axis1 +
            (axis0*tensor(0,2)+axis1*tensor(1,2)+axis2*tensor(2,2))*axis2;
    direction = p2p1;
  }
  else if ( _quantity == MAXPRINCIPAL )
  {
    value = principalValue( tensor, 0, direction );
  }
  else if ( _quantity == MEDPRINCIPAL )
  {
    value = principalValue( tensor, 1, direction );
  }
  else if ( _quantity == MINPRINCIPAL )
  {
    value = principalValue( tensor, 2, direction );
  }
  else if ( _quantity == FIRSTINVARIANT )
  {
    value = tensor.trace();
  }
  else if ( _quantity == SECONDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy() +
      tensor.yy()*tensor.zz() +
      tensor.zz()*tensor.xx() -
      tensor.xy()*tensor.xy() -
      tensor.yz()*tensor.yz() -
      tensor.zx()*tensor.zx();
  }
  else if ( _quantity == THIRDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy()*tensor.zz() -
      tensor.xx()*tensor.yz()*tensor.yz() +
      tensor.xy()*tensor.zx()*tensor.yz() -
      tensor.xy()*tensor.xy()*tensor.zz() +
      tensor.zx()*tensor.xy()*tensor.yz() -
      tensor.zx()*tensor.zx()*tensor.yy();
  }
  else if ( _quantity == TRIAXIALITY )
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
  else if ( _quantity == VOLUMETRICSTRAIN )
  {
    value =
      tensor.trace() +
      tensor.xx()*tensor.yy() +
      tensor.yy()*tensor.zz() +
      tensor.zz()*tensor.xx() +
      tensor.xx()*tensor.yy()*tensor.zz();
  }
  else
  {
    mooseError("Unknown quantity in MaterialTensorAux: " + _quantity_moose_enum.operator std::string());
  }
  return value;
}

Real
MaterialTensorCalculator::principalValue( const SymmTensor & tensor, unsigned int index, RealVectorValue &direction )
{
  ColumnMajorMatrix eval(3,1);
  ColumnMajorMatrix evec(3,3);
  tensor.columnMajorMatrix().eigen(eval, evec);
  // Eigen computes low to high.  We want high first.
  int i = -index + 2;
  direction(0) = evec(0,i);
  direction(1) = evec(1,i);
  direction(2) = evec(2,i);
  return eval(i);
}
