//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NeuralNetworkUserObject.h"
#include "MooseEnum.h"
#include "pugixml.h"

registerMooseObject("StochasticToolsApp", NeuralNetworkUserObject);

template <>
InputParameters
validParams<NeuralNetworkUserObject>()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Reconstructs a neural network from a file and evaluates it");
  params.addRequiredParam<FileName>("weights_file", "Name of the file with the neuron weights");
  return params;
}

NeuralNetworkUserObject::NeuralNetworkUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _weightsFile(getParam<FileName>("weights_file")),
    _layerActivationFunctionEnum(MultiMooseEnum("SIGMOID SOFTSIGN TANH LINEAR LOGSIGMOID" ) )
{
  setXMLWeights();

}
void NeuralNetworkUserObject::setXMLWeights()
{
  std::ifstream ifile;
  std::string file_dump;
  pugi::xml_document doc;

  ifile.open(_weightsFile);
  if (!ifile)
    {
      paramError("Cannot read neural net file");
    }
  pugi::xml_parse_result result = doc.load(ifile);
  if (!result)
    {
      paramError("Cannot parse as XML file");
    }

pugi::xml_node root = doc.document_element();
std::string activation_layers;

std::size_t idx = 0;

 for (pugi::xml_node layer = root.child("LAYER"); layer; layer = layer.next_sibling("LAYER") )
  {
    auto neuron_type = layer.child("TYPE").text().as_string();
    _layerActivationFunctionEnum.push_back(neuron_type);

    switch (_layerActivationFunctionEnum.get(idx))
    {
      case 0: //SIGMOID
        break;
      case 1: //SOFTSIGN
        break;

      case 2: //TANH
        break;

      case 3: //LINEAR
        {
          size_t m = layer.child("M").text().as_uint();
          size_t n = layer.child("N").text().as_uint();

          //Assigning neural net size data
          if(idx==0)
            {
              _d_in = n;
              _h = m;
            }

          _d_out = m;
          _n = 1+idx;
          std::cout << _d_in << _h << _d_out << _n << "\n";

          DenseMatrix<Real> linear_weight(m,n);
          DenseVector<Real> linear_bias(m);
          std::istringstream W (layer.child("WEIGHTS").text().as_string() );
          std::istringstream B (layer.child("BIAS").text().as_string() );

          std::size_t S =0;
          size_t j; size_t i =0;
          for(std::string s; W >> s; )
            {
              j = S%n;
              i = (S - j)/n;
              linear_weight(i,j) = std::stod(s);
              ++S;
            }

          S =0;
          for(std::string s; B >> s; )
            {
              i = S%m;
              linear_bias(i) = std::stod(s);
              ++S;
            }
          _weights.push_back(linear_weight);
          _bias.push_back(linear_bias);
          break;
        }
      case 4: //LOGSIGMOID
        break;
    }
    idx++;
    }

}


Real
NeuralNetworkUserObject::evaluate(DenseVector<Real> & input, std::size_t op_id ) const
{
  if (input.size()!= _d_in)
    {
      mooseError("Input vector size does not match input size for neural network");
    }

  DenseVector<Real> feed_forward(_h);
  DenseVector<Real> temp(_h);
  DenseVector<Real> output(_d_out);

  // Apply input layer weights
  for (std::size_t i =0; i < _h; ++i)
  {
    feed_forward(i) = 0;
    for (std::size_t j = 0; j < _d_in; ++j)
      feed_forward(i)+=input(j)*_weights[0](i,j);
    feed_forward(i)+=_bias[0](i);

  }

  std::size_t idx = 1; //tracks the index of weights to read
  for (std::size_t n = 1; n < _n-1; ++n)
  {

    switch (_layerActivationFunctionEnum.get(n))
      {
        case 0: //SIGMOID
          for (std::size_t i = 0; i < _h; ++i)
            feed_forward(i) = 1 / (1 + std::exp(-1 * feed_forward(i) ));
          break;
        case 1: //SOFTSIGN
          break;

        case 2: //TANH
          for (std::size_t i = 0; i < _h; ++i)
            feed_forward(i) = std::tanh(feed_forward(i) );
          break;

        case 3: //LINEAR
            {
              for (std::size_t i =0; i < _h; ++i)
                {
                temp(i) = 0;
                for (std::size_t j = 0; j < _h; ++j)
                  temp(i) += feed_forward(j)*_weights[idx](i,j);
                temp(i)+= _bias[idx](i);
                }
              feed_forward = temp;
              idx+=1;
              break;
            }
        case 4: //LOGSIGMOID
          for (std::size_t i =0; i < _h; ++i)
            feed_forward(i) = std::log(1 / (1 + std::exp(-1 * feed_forward(i) )) ) ;
          break;
      }
  }

  //Apply final output layer
  auto n = _weights.size() - 1;
  for (std::size_t i =0; i < _d_out; ++i)
  {
    output(i) = 0;
    for (std::size_t j = 0; j < _h; ++j)
      output(i)+=feed_forward(j)*_weights[n](i,j);
    output(i)+=_bias[n](i);
  }

  return (output( op_id) );
  }
