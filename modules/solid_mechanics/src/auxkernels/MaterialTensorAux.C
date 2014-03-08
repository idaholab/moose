#include "MaterialTensorAux.h"

#include "SymmTensor.h"

template<>
InputParameters validParams<MaterialTensorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  addMaterialTensorParams(params);
  return params;
}

void addMaterialTensorParams(InputParameters& params)
{
  MooseEnum quantities("VonMises=1, PlasticStrainMag, Hydrostatic, Hoop, Radial, Axial, MaxPrincipal, MedPrincipal, MinPrincipal, FirstInvariant, SecondInvariant, ThirdInvariant, TriAxiality, VolumetricStrain");

  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addParam<int>("index", -1, "The index into the tensor, from 0 to 5 (xx, yy, zz, xy, yz, zx).");
  params.addParam<MooseEnum>("quantity", quantities, "A scalar quantity to compute: " + quantities.getRawNames());

  params.addParam<RealVectorValue>("point1", RealVectorValue(0, 0, 0), "Point one for defining an axis");
  params.addParam<RealVectorValue>("point2", RealVectorValue(0, 1, 0), "Point two for defining an axis");
}

MaterialTensorAux::MaterialTensorAux( const std::string & name, InputParameters parameters ) :
    AuxKernel( name, parameters ),
    _tensor( getMaterialProperty<SymmTensor>( getParam<std::string>("tensor") ) ),
    _index( getParam<int>("index") ),
    _quantity_moose_enum( getParam<MooseEnum>("quantity") ),
    _p1( getParam<RealVectorValue>("point1") ),
    _p2( getParam<RealVectorValue>("point2") )
{
  checkMaterialTensorParams(_quantity, _quantity_moose_enum, _index, _name);

}

void
MaterialTensorAux::checkMaterialTensorParams(MTA_ENUM &quantity,
                                             const MooseEnum &quantity_moose_enum,
                                             const int index,
                                             const std::string &name)
{
  if (quantity_moose_enum.isValid())
  {
    if ( index > 0 )
      mooseError("Cannot define an index and a quantity in " + name);
    else
      quantity = MTA_ENUM(int(quantity_moose_enum));
  }
  else
  {
    if ( index < 0 )
      mooseError("Neither an index nor a quantity listed for " + name);
    else
      quantity = MTA_COMPONENT; // default
  }

  if (index > -1 && index > 5)
  {
    mooseError("MaterialTensorAux requires the index to be >= 0 and <= 5 OR < 0 (off).");
  }
}

Real
MaterialTensorAux::computeValue()
{
  Real value = getTensorQuantity(_tensor[_qp],
                                 _quantity,
                                 _quantity_moose_enum,
                                 _index,
                                 &_q_point[_qp],
                                 &_p1,
                                 &_p2);
  return value;
}

Real
MaterialTensorAux::getTensorQuantity(const SymmTensor & tensor,
                                     const MTA_ENUM quantity,
                                     const MooseEnum & quantity_moose_enum,
                                     const int index,
                                     const Point * curr_point,
                                     const Point * p1,
                                     const Point * p2)
{
  Real value(0);
  if (quantity == MTA_COMPONENT)
  {
    value = tensor.component(index);
  }
  else if ( quantity == MTA_VONMISES )
  {
    value = std::sqrt(0.5*(
                           std::pow(tensor.xx() - tensor.yy(), 2) +
                           std::pow(tensor.yy() - tensor.zz(), 2) +
                           std::pow(tensor.zz() - tensor.xx(), 2) + 6 * (
                           std::pow(tensor.xy(), 2) +
                           std::pow(tensor.yz(), 2) +
                           std::pow(tensor.zx(), 2))));
  }
  else if ( quantity == MTA_PLASTICSTRAINMAG )
  {
    value = std::sqrt(2.0/3.0 * tensor.doubleContraction(tensor));
  }
  else if ( quantity == MTA_HYDROSTATIC )
  {
    value = tensor.trace()/3.0;
  }
  else if ( quantity == MTA_HOOP )
  {
    // This is the location of the stress.  A vector from the cylindrical axis to this point will define the x' axis.
    Point p0( *curr_point );

    // The vector p1 + t*(p2-p1) defines the cylindrical axis.  The point along this
    // axis closest to p0 is found by the following for t:
    const Point p2p1( *p2 - *p1 );
    const Point p2p0( *p2 - p0 );
    const Point p1p0( *p1 - p0 );
    const Real t( -(p1p0*p2p1)/p2p1.size_sq() );
    // The nearest point on the cylindrical axis to p0 is p.
    const Point p( *p1 + t * p2p1 );
    Point xp( p0 - p );
    xp /= xp.size();
    Point yp( p2p1/p2p1.size() );
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
  }
  else if ( quantity == MTA_RADIAL )
  {
    // This is the location of the stress.  A vector from the cylindrical axis to this point will define the x' axis
    // which is the radial direction in which we want the stress.
    Point p0( *curr_point );

    // The vector p1 + t*(p2-p1) defines the cylindrical axis.  The point along this
    // axis closest to p0 is found by the following for t:
    const Point p2p1( *p2 - *p1 );
    const Point p2p0( *p2 - p0 );
    const Point p1p0( *p1 - p0 );
    const Real t( -(p1p0*p2p1)/p2p1.size_sq() );
    // The nearest point on the cylindrical axis to p0 is p.
    const Point p( *p1 + t * p2p1 );
    Point xp( p0 - p );
    xp /= xp.size();
    const Real xp0( xp(0) );
    const Real xp1( xp(1) );
    const Real xp2( xp(2) );
    value = (xp0*tensor(0,0)+xp1*tensor(1,0)+xp2*tensor(2,0))*xp0 +
            (xp0*tensor(0,1)+xp1*tensor(1,1)+xp2*tensor(2,1))*xp1 +
            (xp0*tensor(0,2)+xp1*tensor(1,2)+xp2*tensor(2,2))*xp2;
  }
  else if ( quantity == MTA_AXIAL )
  {
    // The vector p2p1=(p2-p1) defines the axis, which is the direction in which we want the stress.
    Point p2p1( *p2 - *p1 );
    p2p1 /= p2p1.size();

    const Real axis0( p2p1(0) );
    const Real axis1( p2p1(1) );
    const Real axis2( p2p1(2) );
    value = (axis0*tensor(0,0)+axis1*tensor(1,0)+axis2*tensor(2,0))*axis0 +
            (axis0*tensor(0,1)+axis1*tensor(1,1)+axis2*tensor(2,1))*axis1 +
            (axis0*tensor(0,2)+axis1*tensor(1,2)+axis2*tensor(2,2))*axis2;
  }
  else if ( quantity == MTA_MAXPRINCIPAL )
  {
    value = principalValue( tensor, 0 );
  }
  else if ( quantity == MTA_MEDPRINCIPAL )
  {
    value = principalValue( tensor, 1 );
  }
  else if ( quantity == MTA_MINPRINCIPAL )
  {
    value = principalValue( tensor, 2 );
  }
  else if ( quantity == MTA_FIRSTINVARIANT )
  {
    value = tensor.trace();
  }
  else if ( quantity == MTA_SECONDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy() +
      tensor.yy()*tensor.zz() +
      tensor.zz()*tensor.xx() -
      tensor.xy()*tensor.xy() -
      tensor.yz()*tensor.yz() -
      tensor.zx()*tensor.zx();
  }
  else if ( quantity == MTA_THIRDINVARIANT )
  {
    value =
      tensor.xx()*tensor.yy()*tensor.zz() -
      tensor.xx()*tensor.yz()*tensor.yz() +
      tensor.xy()*tensor.zx()*tensor.yz() -
      tensor.xy()*tensor.xy()*tensor.zz() +
      tensor.zx()*tensor.xy()*tensor.yz() -
      tensor.zx()*tensor.zx()*tensor.yy();
  }
  else if ( quantity == MTA_TRIAXIALITY )
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
  else if ( quantity == MTA_VOLUMETRICSTRAIN )
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
    mooseError("Unknown quantity in MaterialTensorAux: " + quantity_moose_enum.operator std::string());
  }
  return value;
}

Real
MaterialTensorAux::principalValue( const SymmTensor & tensor, unsigned int index )
{
  ColumnMajorMatrix eval(3,1);
  ColumnMajorMatrix evec(3,3);
  tensor.columnMajorMatrix().eigen(eval, evec);
  // Eigen computes low to high.  We want high first.
  int i = -index + 2;
  return eval(i);
}
