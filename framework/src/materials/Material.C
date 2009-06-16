#include "Material.h"
#include <iostream>

void
Material::materialReinit()
{
  std::map<std::string, std::vector<Real> >::iterator it = _real_props.begin();
  std::map<std::string, std::vector<Real> >::iterator it_end = _real_props.end();

  for(;it!=it_end;++it)
    it->second.resize(_static_qrule[_tid]->n_points(),1);

  std::map<std::string, std::vector<RealGradient> >::iterator grad_it = _gradient_props.begin();
  std::map<std::string, std::vector<RealGradient> >::iterator grad_it_end = _gradient_props.end();

  for(;grad_it!=grad_it_end;++grad_it)
    grad_it->second.resize(_static_qrule[_tid]->n_points());

  std::map<std::string, std::vector<RealTensorValue> >::iterator tensor_it = _tensor_props.begin();
  std::map<std::string, std::vector<RealTensorValue> >::iterator tensor_it_end = _tensor_props.end();

  for(;tensor_it!=tensor_it_end;++tensor_it)
    tensor_it->second.resize(_static_qrule[_tid]->n_points());

  computeProperties();
}

/**
 * Updates the old (first) material properties to the current/new material properies (second)
 */
void
Material::updateDataState()
{
  if (_qp_prev.size() != _qp_curr.size()) throw std::out_of_range("_qp_prev != _qp_curr");

  std::map<unsigned int, std::vector<QpData *> >::iterator i_prev = _qp_prev.begin();
  std::map<unsigned int, std::vector<QpData *> >::iterator i_curr = _qp_curr.begin(); 
  std::vector<QpData *>::iterator j_prev, j_curr;

  while (i_prev != _qp_prev.end())
    for (j_prev = i_prev->second.begin(), j_curr = i_prev->second.begin(); 
          j_prev != i_prev->second.end(); 
          ++j_prev, ++j_curr)
      *j_prev = *j_curr;
}

std::vector<QpData *> &
Material::getData(QP_Data_Type qp_data_type)
{
  std::map<unsigned int, std::vector<QpData *> > *locMap;
  std::map<unsigned int, std::vector<QpData *> >::iterator i_map;
  unsigned int elemId = _current_elem->id();

  switch (qp_data_type)
  {
    case CURR:
      locMap = &_qp_curr;
      break;
    case PREV:
      locMap = &_qp_prev;
      break;
  }
  
  i_map = locMap->find(elemId);
  if (i_map != locMap->end())
    return i_map->second;
  else
  {
    // If the vector doesn't already exist create it
    std::vector<QpData *> *v;
    v = new std::vector<QpData *>(_qrule->n_points());
    std::vector<QpData *>::iterator i_vec = v->begin();

    for (;i_vec != v->end();++i_vec)       
      *i_vec = createData();
    
    (*locMap)[elemId] = *v;
    
    return *v;
  }
}
