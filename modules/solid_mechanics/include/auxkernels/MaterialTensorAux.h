#ifndef MATERIALTENSORAUX_H
#define MATERIALTENSORAUX_H

#include "AuxKernel.h"

class MaterialTensorAux;
class SymmTensor;

template<>
InputParameters validParams<MaterialTensorAux>();


class MaterialTensorAux : public AuxKernel
{
public:
  MaterialTensorAux( const std::string & name, InputParameters parameters );

  virtual ~MaterialTensorAux() {}

protected:
  enum MTA_ENUM
  {
    MTA_COMPONENT,
    MTA_VONMISES,
    MTA_PLASTICSTRAINMAG,
    MTA_HYDROSTATIC,
    MTA_HOOP,
    MTA_RADIAL,
    MTA_AXIAL,
    MTA_MAXPRINCIPAL,
    MTA_MEDPRINCIPAL,
    MTA_MINPRINCIPAL,
    MTA_FIRSTINVARIANT,
    MTA_SECONDINVARIANT,
    MTA_THIRDINVARIANT,
    MTA_TRIAXIALITY
  };


  virtual Real computeValueOld();
  virtual Real computeValue();
  static Real getTensorQuantity(const SymmTensor & tensor,
                                const MTA_ENUM quantity,
                                MooseEnum & quantity_moose_enum,
                                const int index,
                                const Point * curr_point,
                                const Point * p1,
                                const Point * p2);

  static Real principalValue( const SymmTensor & tensor, unsigned int index );

  MaterialProperty<SymmTensor> & _tensor;
  const int _index;
  MooseEnum _quantity_moose_enum;
  MTA_ENUM _quantity;

  const Point _p1;
  const Point _p2;

};

#endif // MATERIALTENSORAUX_H
