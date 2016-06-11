/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATERIALTENSORONLINE_H
#define MATERIALTENSORONLINE_H

#include "ElementUserObject.h"
#include "MaterialTensorCalculator.h"

class MaterialTensorOnLine;
class SymmTensor;

template<>
InputParameters validParams<MaterialTensorOnLine>();

class MaterialTensorOnLine : public ElementUserObject
{
public:
  MaterialTensorOnLine(const InputParameters & parameters);

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

  MaterialTensorCalculator _material_tensor_calculator;
  const MaterialProperty<SymmTensor> & _tensor;

  const Point _lp1;
  const Point _lp2;
  int _line_id;

  std::string _file_name;
  std::ofstream _output_file;
  bool _stream_open;

private:
  const VariableValue & _elem_line_id;
};

#endif

