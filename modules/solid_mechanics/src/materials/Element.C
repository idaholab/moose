#include "Element.h"



#include "Nonlinear3D.h"

#include "Problem.h"


namespace Elk
{
namespace SolidMechanics
{


Element::Element( const std::string & name,
                  InputParameters parameters ) :
  Material(name+"_Element", parameters)
{
}

////////////////////////////////////////////////////////////////////////

Element::~Element()
{
}

////////////////////////////////////////////////////////////////////////

void
Element::rotateSymmetricTensor( const ColumnMajorMatrix & R,
                                const SymmTensor & T,
                                SymmTensor & result )
{

  //     R           T         Rt
  //  00 01 02   00 01 02   00 10 20
  //  10 11 12 * 10 11 12 * 01 11 21
  //  20 21 22   20 21 22   02 12 22
  //
  const Real T00 = R(0,0)*T.xx() + R(0,1)*T.xy() + R(0,2)*T.zx();
  const Real T01 = R(0,0)*T.xy() + R(0,1)*T.yy() + R(0,2)*T.yz();
  const Real T02 = R(0,0)*T.zx() + R(0,1)*T.yz() + R(0,2)*T.zz();

  const Real T10 = R(1,0)*T.xx() + R(1,1)*T.xy() + R(1,2)*T.zx();
  const Real T11 = R(1,0)*T.xy() + R(1,1)*T.yy() + R(1,2)*T.yz();
  const Real T12 = R(1,0)*T.zx() + R(1,1)*T.yz() + R(1,2)*T.zz();

  const Real T20 = R(2,0)*T.xx() + R(2,1)*T.xy() + R(2,2)*T.zx();
  const Real T21 = R(2,0)*T.xy() + R(2,1)*T.yy() + R(2,2)*T.yz();
  const Real T22 = R(2,0)*T.zx() + R(2,1)*T.yz() + R(2,2)*T.zz();

  result.xx( T00 * R(0,0) + T01 * R(0,1) + T02 * R(0,2) );
  result.yy( T10 * R(1,0) + T11 * R(1,1) + T12 * R(1,2) );
  result.zz( T20 * R(2,0) + T21 * R(2,1) + T22 * R(2,2) );
  result.xy( T00 * R(1,0) + T01 * R(1,1) + T02 * R(1,2) );
  result.yz( T10 * R(2,0) + T11 * R(2,1) + T12 * R(2,2) );
  result.zx( T00 * R(2,0) + T01 * R(2,1) + T02 * R(2,2) );

}

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////


} // namespace solid_mechanics
} // namespace elk
