#ifndef MATERIALTENSORONLINE_H
#define MATERIALTENSORONLINE_H

#include "ElementUserObject.h"
#include "MaterialTensorAux.h"

class MaterialTensorOnLine;
class SymmTensor;


template<>
InputParameters validParams<MaterialTensorOnLine>();

class MaterialTensorOnLine : public ElementUserObject
{


public:
  MaterialTensorOnLine(const std::string & name, InputParameters parameters);

  ~MaterialTensorOnLine(); // the destructor closes the output file

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & u );
  virtual void finalize();

protected:

//  std::map<unsigned int, Real> _dist;
//  std::map<unsigned int, Real> _value;
  std::map<std::pair<unsigned int,unsigned int>,Real> _dist;
  std::map<std::pair<unsigned int,unsigned int>,Real> _value;

  MaterialProperty<SymmTensor> & _tensor;
  const int _index;
  MooseEnum _quantity_moose_enum;
  MTA_ENUM _quantity;

  const Point _p1;
  const Point _p2;
  int _line_id;

  std::string _file_name;
  std::ofstream _output_file;
  bool _stream_open;


private:
  VariableValue & _elem_line_id;



};

#endif




