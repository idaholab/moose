/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MaterialTensorOnLine.h"
#include "SymmTensor.h"
#include "FEProblem.h"

#include <cmath>
#include <algorithm>
#include <set>

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<MaterialTensorOnLine>()
{
  InputParameters params = validParams<ElementUserObject>();
  params += validParams<MaterialTensorCalculator>();

  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addParam<RealVectorValue>("line_point1", RealVectorValue(0, 0, 0), "Start point of line along which material data is output");
  params.addParam<RealVectorValue>("line_point2", RealVectorValue(0, 1, 0), "End point of line along which material data is output");
  params.addCoupledVar("element_line_id","Element line ID: if not zero, output stress at integration points");
  params.addRequiredParam<std::string>("filename","Output file name");
  params.addParam<int>("line_id",1,"ID of the line of elements to output stresses on");
  params.set<MultiMooseEnum>("execute_on") = "timestep_end";

  return params;
}

MaterialTensorOnLine :: MaterialTensorOnLine(const InputParameters & parameters) :
  ElementUserObject(parameters),
  _material_tensor_calculator( parameters),
  _tensor( getMaterialProperty<SymmTensor>( getParam<std::string>("tensor") ) ),
  _lp1( getParam<RealVectorValue>("line_point1") ),
  _lp2( getParam<RealVectorValue>("line_point2") ),
  _line_id( getParam<int>("line_id") ),
  _file_name( getParam<std::string>("filename") ),
  _stream_open(false),
  _elem_line_id(coupledValue("element_line_id"))
{

  if (!_stream_open && processor_id() == 0)
  {
    _output_file.open(_file_name.c_str(), std::ios::trunc | std::ios::out);
    _stream_open = true;
  }

}

MaterialTensorOnLine::~MaterialTensorOnLine()
{
  if (_stream_open && processor_id() == 0)
  {
    _output_file.close();
    _stream_open = false;
  }
}


void
MaterialTensorOnLine::initialize()
{
  _dist.clear();
  _value.clear();
}

void
MaterialTensorOnLine::execute()
{
  unsigned int qp(0); // all integration points have the same _elem_line_id, just use qp=0
  int id = _elem_line_id[qp] + .5;


  if (id == _line_id)
  {

    const Point line_vec(_lp2-_lp1);
    const Real length(line_vec.norm());
    const Point line_unit_vec(line_vec/length);

    for ( qp = 0; qp < _qrule->n_points(); ++qp )
    {
      const Point qp_pos(_q_point[qp]);

      const Point line1_qp_vec(qp_pos-_lp1);
      const Real proj(line1_qp_vec*line_unit_vec);
      const Point proj_vec(proj*line_unit_vec);

      const Point dist_vec(line1_qp_vec-proj_vec);
      const Real distance(dist_vec.norm());

      const SymmTensor & tensor( _tensor[qp] );
      RealVectorValue direction;

//      _dist[_current_elem->id()] = distance;
//      _value[_current_elem->id()] = tensor.component(_index);
      _dist[std::make_pair(_current_elem->id(),qp)] = distance;
//      _value[std::make_pair(_current_elem->id(),qp)] = tensor.component(_index);
      _value[std::make_pair(_current_elem->id(),qp)] = _material_tensor_calculator.getTensorQuantity(tensor,_q_point[qp],direction);

    }
  }

}


void
MaterialTensorOnLine::threadJoin(const UserObject & u )
{
  const MaterialTensorOnLine & sp = dynamic_cast<const MaterialTensorOnLine &>(u);

  for ( std::map<std::pair<unsigned int, unsigned int>,Real>::const_iterator it = sp._dist.begin();
        it != sp._dist.end();
        ++it )
    _dist[it->first] = it->second;

  for ( std::map<std::pair<unsigned int, unsigned int>,Real>::const_iterator it = sp._value.begin();
        it != sp._value.end();
        ++it )
    _value[it->first] = it->second;
}

void
MaterialTensorOnLine::finalize()
{

    _communicator.set_union(_dist);
    _communicator.set_union(_value);

   if (processor_id() == 0)
   {
     std::vector<std::pair<Real, Real> > table;
//     std::map<unsigned int, Real>::iterator id(_dist.begin());
//     std::map<unsigned int, Real>::iterator is(_value.begin());
     std::map<std::pair<unsigned int, unsigned int>, Real>::iterator id(_dist.begin());
     std::map<std::pair<unsigned int, unsigned int>, Real>::iterator is(_value.begin());

     while (id != _dist.end() && is != _value.end())
     {
       table.push_back(std::make_pair(id->second,is->second));
       id++;
       is++;
     }

     std::sort(table.begin(),table.end());

     _output_file << "time " << _fe_problem.time() << std::endl;
     for (std::vector<std::pair<Real, Real> >::iterator it = table.begin();
          it != table.end();
          ++it)
     {
       _output_file << it->first << "  " << it->second << std::endl;
     }
   }

}

