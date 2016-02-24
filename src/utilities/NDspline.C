#include "ND_Interpolation_Functions.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <limits>
#include "MDreader.h"


NDSpline::NDSpline(std::string filename, std::vector<double> alfa, std::vector<double> beta){
 // constructor for scattered data interpolation functions

 _alpha = alfa;
 _beta = beta;

 readOrderedNDarray(filename, _dimensions, _discretizations, _values);

 NDSpline_init(_discretizations, _values, _alpha, _beta);
}

NDSpline::NDSpline(std::string filename){
 // constructor for scattered data interpolation functions

 readOrderedNDarray(filename, _dimensions, _discretizations, _values);

 std::vector<double> alpha(_dimensions);
 std::vector<double> beta(_dimensions);

 for (int nDim=0; nDim<_dimensions; nDim++){
   alpha.at(nDim) = 0.0;
   beta.at(nDim) = 0.0;
 }

 NDSpline_init(_discretizations, _values, alpha, beta);
}

NDSpline::NDSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta){
        NDSpline_init(discretizations, values, alpha, beta);
}

void NDSpline::NDSpline_init(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta){
        _discretizations = discretizations;
        _values = values;
        _alpha = alpha;
        _beta = beta;

        bool check = (_alpha.size() == _beta.size()) && (_beta.size() == _discretizations.size());

        if (check == false)
                throw ("ND spline: Dimensions of alpha and beta do not agree with data dimensionality");

        _dimensions = _discretizations.size();

         std::cerr << "ND spline initialization"<<std::endl;

         for (int nDim=0; nDim<_dimensions; nDim++){
                 int length = _discretizations.at(nDim).size();
             _hj.push_back((_discretizations.at(nDim).at(length-1) - _discretizations.at(nDim).at(0))/(length-1));
             //std::cerr << "dim " << nDim << ": _hj: " << _hj.at(nDim) << std::endl;

             _min_disc.push_back(_discretizations.at(nDim).at(0));
             _max_disc.push_back(_discretizations.at(nDim).at(length-1));

             //std::cout << "nDim: " << nDim << " ; _min_disc.at(nDim).at(0): " <<  _min_disc.at(nDim) << " ; _max_disc.at(nDim): " << _max_disc.at(nDim) << std::endl;
         }

         _completed_init = true;

         if (_dimensions > 1)
          calculateCoefficients();
         else
          _spline_coefficients = getCoefficients(_values, _hj.at(0), _alpha.at(0), _beta.at(0));

            for (int i=0; i<_dimensions; i++){
              _cellPoint0.push_back(_discretizations.at(i).at(0));
              _cellDxs.push_back(_discretizations.at(i).at(_discretizations.at(i).size()-1)-_discretizations.at(i).at(0));
            }

         for (int i=0; i<_dimensions; i++){
             _lowerBound.push_back(_cellPoint0.at(i));
             _upperBound.push_back(_cellPoint0.at(i) + _cellDxs.at(i));
         }
         std::cout << "ND spline initialization completed " << std::endl;
}



NDSpline::NDSpline(){
    _completed_init = false;
    _dimensions = -1;
}

NDSpline::~NDSpline() {

}

double NDSpline::interpolateAt(std::vector<double> point_coordinate){
 double interpolated_value;

 bool outcome = checkBoundaries(point_coordinate);

 if (outcome==true){
  if (not _completed_init)
  {
    throw ("Error in interpolateAt: the class has not been completely initialized... you can not interpolate!!!!");
  }
  interpolated_value = spline_cartesian_interpolation(point_coordinate);
 }else{
  std::vector<double> distances (_values.size());
  std::vector<int> indexes (point_coordinate.size());
  std::vector<int> coordinates (point_coordinate.size());

  int minIndex = -1;
  double minDistance = std::numeric_limits<double>::max();

  for (int nDim=0; nDim<_dimensions; nDim++)
   indexes.at(nDim) = _discretizations.at(nDim).size();

  for (unsigned int i=0; i<_values.size(); i++){
   coordinates = from1DtoNDconverter(i, indexes);
   double distance = 0;

   for (int nDim=0; nDim<_dimensions; nDim++)
    distance = distance + pow((_discretizations.at(nDim).at(coordinates.at(nDim))-point_coordinate.at(nDim)),2);

   distances.at(i) = sqrt(distance);

   if (distances.at(i) < minDistance){
    minIndex = i;
    minDistance = distances.at(i);
   }
  }
  interpolated_value = _values.at(minIndex);
 }

 return interpolated_value;
}


double NDSpline::getGradientAt(std::vector<double> point_coordinate){
 // TO BE COMPLETED
 double gradient= -1;
 if (not _completed_init)
 {
   throw ("Error in getGradientAt: the class has not been completely initialized... you can not interpolate!!!!");
 }
 return gradient;
}

void NDSpline::fit(std::vector< std::vector<double> > coordinates, std::vector<double> values){
  throw ("Error in NDSpline: fit not yet implemented!!!");
  _completed_init = true;
}

double NDSpline::spline_cartesian_interpolation(std::vector<double> point_coordinate){
 double interpolated_value = 0;
 std::vector<int> coordinates (point_coordinate.size());
 std::vector<int> indexes (point_coordinate.size());

 int numberOfIterations = _spline_coefficients.size();

 for (int nDim=0; nDim<_dimensions; nDim++)
  indexes.at(nDim) = _discretizations.at(nDim).size() + 2;

 for (int i=0; i<numberOfIterations; i++){
  coordinates = from1DtoNDconverter(i, indexes);

  double product=1;
  for (int nDim=0; nDim<_dimensions; nDim++){
   //u_k(double x, std::vector<double> & discretizations, double k)
   product *= u_k(point_coordinate.at(nDim), _discretizations.at(nDim), coordinates.at(nDim)+1);
   //product *= u_k(point_coordinate.at(nDim), _discretizations.at(nDim).at(0), _hj.at(nDim), coordinates.at(nDim)+1);       //u_k(double x, double a, double h, double k)
  }

  interpolated_value += _spline_coefficients.at(i)*product;
 }
 return interpolated_value;
}

void NDSpline::calculateCoefficients(){
 std::vector<int> loop_locator (_dimensions);

 //std::cerr << "calculateCoefficients" << std::endl;
 std::vector<double> coeff = fillArrayCoefficient(_dimensions, _values, loop_locator);
 //std::cerr << " done calculateCoefficients" << std::endl;
 _spline_coefficients = coeff;
}


std::vector<double> NDSpline::fillArrayCoefficient(int n_dimensions, std::vector<double> & data, std::vector<int> & loop_locator){
 std::vector<std::vector<double> > tempCoefficients;
 std::vector<double> temp;
 std::vector<double> y;

 for(unsigned int n=0; n<_discretizations.at(n_dimensions-1).size(); n++){
  loop_locator.at(n_dimensions-1)=n;

  if (n_dimensions>2){
   int tempIndex=n_dimensions-1;
   temp = fillArrayCoefficient(tempIndex, data, loop_locator);

   tempCoefficients.push_back(temp);
   temp.clear();
  }
  else{  // n=1
   y = getValues(loop_locator); // get data

   tempCoefficients.push_back(getCoefficients(y, _hj.at(n_dimensions-1), _alpha.at(n_dimensions-1), _beta.at(n_dimensions-1)));
   y.clear();
  }
 }

 // Create tensor-product
 //std::cerr << "Create tensor-product" << std::endl;

 std::vector<std::vector<double> > finalCoefficients = tensorProductInterpolation(tempCoefficients, _hj.at(n_dimensions-1), _alpha.at(n_dimensions-1), _beta.at(n_dimensions-1));

 // Adjust  Data
 //std::cerr << "Adjust data" << std::endl;
 std::vector<double> coefficients = coefficientRestructuring(finalCoefficients);

 tempCoefficients.clear();
 finalCoefficients.clear();

 return coefficients;
}

std::vector<double> NDSpline::coefficientRestructuring(std::vector<std::vector<double> > matrix){
 std::vector<double> array;

 for (unsigned int i=0; i<matrix.size(); i++)
  array.insert(array.end(), matrix.at(i).begin(), matrix.at(i).end());

 return array;
}



std::vector<std::vector<double> > NDSpline::tensorProductInterpolation(std::vector<std::vector<double> > step1, double h, double alpha, double beta){

 std::vector<std::vector<double> > step1restructured = matrixRestructuring(step1);

 std::vector<std::vector<double> > step2;
 std::vector<double> temp;

 for (unsigned int n=0; n<step1restructured.size(); n++){
  temp = getCoefficients(step1restructured.at(n), h, alpha, beta);
  step2.push_back(temp);
  temp.clear();
 }

 std::vector<std::vector<double> > step2restructured = matrixRestructuring(step2);

 step2.clear();
 return step2restructured;
}


std::vector<std::vector<double> > NDSpline::matrixRestructuring(std::vector<std::vector<double> > step1){
 int rows = step1.size();
 int cols = step1.at(0).size();

 std::vector<std::vector<double> > step2 (cols, std::vector<double>(rows,0));

 for (int r=0; r<rows; r++)
  for (int c=0; c<cols; c++)
   step2.at(c).at(r) = step1.at(r).at(c);

 return step2;
}


std::vector<double> NDSpline::getValues(std::vector<int> & loop_locator){
 std::vector<double> values (_discretizations.at(0).size());

 for (unsigned int n=0; n<_discretizations.at(0).size(); n++){
  loop_locator.at(0) = n;
  int oneDcoordinate = fromNDto1Dconverter(loop_locator);
     values.at(n) = _values.at(oneDcoordinate);
 }

 return values;
}


void NDSpline::from2Dto1Drestructuring(std::vector<std::vector<double> > & twoDdata, std::vector<double> & oneDdata){
 // this function restructures a 2D vector into a 1D vector
 // example: 2D [[1,2],[3,4]] --> 1D restructuring [1,2,3,4]

 for (unsigned int i=0; i<twoDdata.size(); i++)
  oneDdata.insert(oneDdata.end(), twoDdata.at(i).begin(), twoDdata.at(i).end());
}




//double NDSpline::u_k(double x, double a, double h, double k){
// // defined in Christian Habermann, Fabian Kindermann, "Multidimensional Spline Interpolation: Theory and Applications", Computational Economics, Vol.30-2, pp 153-169 (2007) [http://link.springer.com/article/10.1007%2Fs10614-007-9092-4]
// return phi((x-a)/h - (k-2));
//}


double NDSpline::u_k(double x, std::vector<double> & discretizations, double k){
  // defined in Christian Habermann, Fabian Kindermann, "Multidimensional Spline Interpolation: Theory and Applications", Computational Economics, Vol.30-2, pp 153-169 (2007) [http://link.springer.com/article/10.1007%2Fs10614-007-9092-4]
  //double up   = discretizations[0];

  //double down = discretizations[discretizations.size()-1];
  int down=0;

  for(unsigned int n=0; n<discretizations.size(); n++){
    if (x>discretizations.at(n)){
      down = n;
      break;
    }
  }

  //up is never used
  //for(int n=discretizations.size(); n<0; n--)
  // if (x<discretizations[n])
  //  up = n;

  // Node re-scaling - linear type
  //double scaled_x = down + (x-discretizations[down])/(discretizations[up]-discretizations[down]);

  //double scaled_x = down + (x-discretizations[(int)down])/(discretizations[(int)down+1]-discretizations[(int)down]);

  double scaled_x = (double)down + (x-discretizations.at(down))/(discretizations.at(down+1)-discretizations.at(down));

  double a = 0.0;
  double h = 1.0;
  return phi((scaled_x-a)/h - (k-2));

  //return phi((x-discretizations[0])/h - (k-2));
}



void NDSpline::from1Dto2Drestructuring(std::vector<std::vector<double> > & twoDdata, std::vector<double> & oneDdata, int spacing){
    /**
     * This function restructures a 1D vector into a 2D vector
     * example: 1D [1,2,3,4,5,6] spacing=2 --> 2D restructuring [[1,2],[3,4],[5,6]]
     */

 if (oneDdata.size()%spacing == 0)
  for (unsigned int i=0; i<oneDdata.size()/spacing; i++){
   for (int j=0; j<spacing; j++)
    twoDdata[i][j] = oneDdata[spacing*i+j];
  }
 else
  throw ("Error in from1Dto2Drestructuring: spacing value not a multiplier for oneDdata");
}


//double NDSpline::u_k(double x, double a, double h, double k){
// // defined in Christian Habermann, Fabian Kindermann, "Multidimensional Spline Interpolation: Theory and Applications", Computational Economics, Vol.30-2, pp 153-169 (2007) [http://link.springer.com/article/10.1007%2Fs10614-007-9092-4]
// return phi((x-a)/h - (k-2));
//}


double NDSpline::phi(double t){
 // defined in Christian Habermann, Fabian Kindermann, "Multidimensional Spline Interpolation: Theory and Applications", Computational Economics, Vol.30-2, pp 153-169 (2007) [http://link.springer.com/article/10.1007%2Fs10614-007-9092-4]
 double phi_value=0.0;

 if ((fabs(t)<=2.0) & (fabs(t)>=1.0))
  phi_value = std::pow(2.0-fabs(t),3);

 if ((fabs(t))<1)
  phi_value = 4.0 - 6.0*std::pow(fabs(t),2) + 3.0*std::pow(fabs(t),3);

 return phi_value;
}


void NDSpline::tridag(std::vector<double> & a, std::vector<double> & b, std::vector<double> & c, std::vector<double> & r, std::vector<double> & u){
 //Source: "Numerical Recipies" pp 56
 int j, n=a.size();
 double bet;
 std::vector<double> gam(n);

 if (b.at(0)==0) throw ("Error 1 in tridag: b[0]==0");
 u.at(0) = r.at(0)/(bet=b.at(0));

 for (int j=1; j<n; j++){
   gam.at(j)=c.at(j-1)/bet;
   bet=b.at(j)-a.at(j)*gam.at(j);
   if (bet == 0) throw ("Error 1 in tridag: bet == 0");
   u.at(j)=(r.at(j)-a.at(j)*u.at(j-1))/bet;
 }

 for (j=n-2;j>=0;j--)
   u.at(j) -= gam.at(j+1)*u.at(j+1);

}


std::vector<double> NDSpline::getCoefficients(std::vector<double> & y, double h, double alpha, double beta){
    // alfa,beta = second derivative of the spline function in a and b of [a,b];
    // natural splines have alfa and beta = 0

 int n = y.size();

 std::vector<double> a (n-2);
 std::vector<double> b (n-2);
 std::vector<double> c (n-2);
 std::vector<double> r (n-2);

 std::vector<double> tempCoefficients (n-2);
 std::vector<double> coefficients (n+2);

 double c_2   = 1/6.0*(y.at(0)- alpha*h*h/6);
 double c_np2 = 1/6.0*(y.at(n-1)- beta*h*h/6);

 for (int i=0; i<(n-2); i++){
  a.at(i)=1;
  b.at(i)=4;
  c.at(i)=1;

  if (i==0)
   r.at(i) = y.at(1)-c_2;
  else if (i==(n-3))
   r.at(i) = y.at(n-2)-c_np2;
  else
   r.at(i)=y.at(i+1);
 }

 tridag(a,b,c,r,tempCoefficients);

 double c_1   = alpha*h*h/6.0 + 2.0*c_2   - tempCoefficients.at(0);
 double c_np3 =  beta*h*h/6.0 + 2.0*c_np2 - tempCoefficients.at(n-3);

 coefficients.at(0) = c_1;
 coefficients.at(1) = c_2;

 for(int i=2; i<(n); i++)
  coefficients.at(i) = tempCoefficients.at(i-2);

 coefficients.at(n) = c_np2;
 coefficients.at(n+1) = c_np3;

 return coefficients;
    }


int NDSpline::fromNDto1Dconverter(std::vector<int> coordinate){
 int coordinate1D = 0;
 int spacing;

 for (int i=0; i<_dimensions; i++){
  spacing=1;

  for (int j=0; j<i; j++)
   spacing *= _discretizations.at(j).size();
  coordinate1D += (coordinate.at(i))*spacing;
 }
 return (coordinate1D);
}

std::vector<int> NDSpline::from1DtoNDconverter(int oneDcoordinate, std::vector<int> indexes){
 int n_dimensions = indexes.size();
 std::vector<int> NDcoordinates (n_dimensions);
 std::vector<int> weights (n_dimensions);

 weights.at(0)=1;
 for (int nDim=1; nDim<n_dimensions; nDim++)
  weights.at(nDim)=weights.at(nDim-1)*indexes.at(nDim-1);

 for (int nDim=(n_dimensions-1); nDim>=0; nDim--){
  if (nDim>0){
   NDcoordinates.at(nDim) = oneDcoordinate/weights.at(nDim);
   oneDcoordinate -= NDcoordinates.at(nDim)*weights.at(nDim);
  }
  else{
   NDcoordinates.at(0) = oneDcoordinate;
  }
 }
 return NDcoordinates;
}


double NDSpline::retrieveCoefficient(std::vector<int> coefficient_coordinate){
 int oneDlocation = fromNDto1Dconverter(coefficient_coordinate);

 return _spline_coefficients.at(oneDlocation);
}

bool NDSpline::checkBoundaries(std::vector<double> point){
 bool outcome = true; // True: within boundaries ; False: outside boundaries

 for (int nDim=0; nDim<_dimensions; nDim++){
  if (point.at(nDim) > _max_disc.at(nDim))
   outcome = outcome && false;
  if (point.at(nDim) < _min_disc.at(nDim))
   outcome = outcome && false;
 }

 //std::cout<<"Outcome Boundaries: " << outcome << std::endl;

 return outcome;
}

double NDSpline::U_K(double x, std::vector<double> & discretizations, double k){

        int down=0;

        for(unsigned int n=0; n<discretizations.size(); n++)
                if (x>discretizations.at(n)){
                        down = n;
                        break;
                }

        double scaled_x = (double)down + (x-discretizations.at(down))/(discretizations.at(down+1)-discretizations.at(down));

        double a = 0.0;
        double h = 1.0;

        return PHI((scaled_x-a)/h - (k-2.0)) * (discretizations.at(down+1)-discretizations.at(down));

        //return PHI((scaled_x-a)/h - (k-2.0));

        //return PHI((x-discretizations.at(0))/(discretizations.at(down+1)-discretizations.at(down)) - (k-2.0)) * (discretizations.at(down+1)-discretizations.at(down));
}


double NDSpline::spline_cartesian_integration(std::vector<double> point_coordinate){
         double interpolated_value = 0.0;
         std::vector<int> coordinates (point_coordinate.size());
         std::vector<int> indexes (point_coordinate.size());

         int numberOfIterations = _spline_coefficients.size();

         for (int nDim=0; nDim<_dimensions; nDim++)
          indexes.at(nDim) = _discretizations.at(nDim).size() + 2;

         for (int i=0; i<numberOfIterations; i++){
            coordinates = from1DtoNDconverter(i, indexes);

            double product=1.0;
            for (int nDim=0; nDim<_dimensions; nDim++){
                product *= U_K(point_coordinate.at(nDim), _discretizations.at(nDim), coordinates.at(nDim)+1);
            }
            interpolated_value += _spline_coefficients.at(i)*product;
         }

         return interpolated_value;
}

double NDSpline::spline_cartesian_marginal_integration(double coordinate,int marginal_variable){
         double value = 0.0;
         std::vector<int> coordinates (_dimensions);
         std::vector<int> indexes (_dimensions);

         int numberOfIterations = _spline_coefficients.size();

         for (int nDim=0; nDim<_dimensions; nDim++)
          indexes.at(nDim) = _discretizations.at(nDim).size() + 2;

         for (int i=0; i<numberOfIterations; i++){
            coordinates = from1DtoNDconverter(i, indexes);

            double product=1.0;
            for (int nDim=0; nDim<_dimensions; nDim++){
                if (nDim == marginal_variable){
                        product *= U_K(coordinate, _discretizations.at(nDim), coordinates.at(nDim)+1);
                        }
                else{
                        double last_coord = _discretizations.at(nDim).at(_discretizations.at(nDim).size()-1);
                        product *= U_K(last_coord, _discretizations.at(nDim), coordinates.at(nDim)+1);
                }
            }
            value += _spline_coefficients.at(i)*product;
         }
         return value;
}

double NDSpline::spline_cartesian_inverse_marginal(double CDF,int marginal_variable, double precision){
  //  Newton–Raphson method used here

  if ((CDF<0.0) and (CDF>1.0))
    throw ("Error in spline_cartesian_inverse_marginal: CDF provided is out of boundaries [0.0,1.0]");

  double up = _discretizations.at(marginal_variable).at(_discretizations.at(marginal_variable).size()-1);
  double down = _discretizations.at(marginal_variable).at(0)+0.000001;
  CDF = spline_cartesian_marginal_integration(down,marginal_variable) + CDF*(spline_cartesian_marginal_integration(up,marginal_variable) - spline_cartesian_marginal_integration(down,marginal_variable));

  int mid_position = _discretizations.at(marginal_variable).size()/2;

  double epsilon = 1.0;
  double x_n   = _discretizations.at(marginal_variable).at(mid_position);
  double x_np1 = _discretizations.at(marginal_variable).at(mid_position+1);
  double derivative;

  do{
    if (x_np1>x_n)
      derivative = (spline_cartesian_marginal_integration(x_np1,marginal_variable) - spline_cartesian_marginal_integration(x_n,marginal_variable))/(x_np1 - x_n);
    else
      derivative = (spline_cartesian_marginal_integration(x_n,marginal_variable) - spline_cartesian_marginal_integration(x_np1,marginal_variable))/(x_n - x_np1);
    double next = x_n - (spline_cartesian_marginal_integration(x_n,marginal_variable) - CDF) / derivative;
    epsilon = std::abs(x_np1 - x_n);
    x_n = x_np1;
    x_np1 = next;
  }while(epsilon>precision);

  return x_np1;

  //return 3.0;
}

double NDSpline::integralSpline(std::vector<double> point_coordinate){
        return spline_cartesian_integration(point_coordinate);
}

/**
 * These functions are implemented in NDspline.C.
 * They implement the integral of the kernel functions of the ND-spline
 * which are needed to calculate CDF and marginal distributions.
 * Six functions are needed since the kernel function is piecewise with modulus operator.
 */

double NDSpline::val1(double t){
        return 0.0;
}
double NDSpline::val2(double t){
        double value = 8.0*t + 6.0*pow(t,2) + 2.0*pow(t,3) + 0.25*pow(t,4);
        return value;
}
double NDSpline::val3(double t){
        double value = 4.0*t - 2.0*pow(t,3) - 0.75*pow(t,4);
        return value;
}
double NDSpline::val4(double t){
        double value = 4.0*t - 2.0*pow(t,3) + 0.75*pow(t,4);
        return value;
}
double NDSpline::val5(double t){
        double value = 8.0*t - 6.0*pow(t,2) + 2.0*pow(t,3) - 0.25*pow(t,4);
        return value;
}
double NDSpline::val6(double t){
        return 0.0;
}

double NDSpline::PHI(double t){
        double PHI_value=-1;

        if (t<=-2.0)
                PHI_value = val1(t);
        else if ((t>-2.0) and (t<=-1.0))
                PHI_value = val2(t) - val2(-2.0);
        else if ((t>-1.0) and (t<=-0.0))
                PHI_value = val2(-1.0) - val2(-2.0) + val3(t) - val3(-1.0);
        else if ((t>0.0) and (t<=1.0))
                PHI_value = val2(-1.0) - val2(-2.0) + val3(0.0) - val3(-1.0) + val4(t)-val4(0.0);
        else if ((t>1.0) and (t<=2.0))
                PHI_value = val2(-1.0) - val2(-2.0) + val3(0.0) - val3(-1.0) + val4(1.0) - val4(0.0) + (val5(t)-val5(1.0));
        else if (t>2.0)
                PHI_value = val2(-1.0) - val2(-2.0) + val3(0.0) - val3(-1.0) + val4(1.0) - val4(0.0) + val5(2.0)-val5(1.0) + (val6(t)-val6(2.0));

        return PHI_value;
}

//void NDSpline::iterationStep(int nDim, std::vector<double> & coefficients, std::vector<double> & data){
// int numberOfCoefficients=1;
// for (int i=nDim; i<_dimensions; i++)
//  numberOfCoefficients *= _discretizations[i].size();
//
// std::vector<double> tempCoefficients;
// std::vector<std::vector<double> > twoDcoefficients(numberOfCoefficients, std::vector<double> (_hj[nDim]+3,0));
// std::vector<std::vector<double> > twoDdata;
//
// from1Dto2Drestructuring(twoDdata, _values, _hj[nDim]);
//
// for (int i=0; i<numberOfCoefficients; i++){
//  getCoefficients(tempCoefficients, twoDdata[i], _hj[nDim], _alpha[nDim], _beta[nDim]);
//  twoDcoefficients[i]=tempCoefficients;
// }
//
// from2Dto1Drestructuring(twoDcoefficients, coefficients);
//}



//void NDSpline::initializeCoefficientsVector(){
// int numberOfCoefficientsToStore = 1;
//
// for(int i=0; i<_dimensions; i++)
//  numberOfCoefficientsToStore *= _discretizations[i].size() + 3;
//
// _spline_coefficients = std::vector<double>(numberOfCoefficientsToStore);
//}

//std::vector<double> NDSpline::NDinverseFunctionGrid(double F, double g){
// generate RN in [0,1]
// int seed = time(0);
// boost::random::mt19937 rng;
// rng.seed(seed);
// boost::random::uniform_distribution<> dist(1, numberOfCells);
//
// int pickedCell = dist(rng)-1;
//  std::vector<double> coordinate(_dimensions);
//  for (int i=0; i<_dimensions; i++){
//    coordinate.at(i) = spline_cartesian_inverse_marginal(F, i, _tolerance);
//  }
//  return coordinate;
//}
