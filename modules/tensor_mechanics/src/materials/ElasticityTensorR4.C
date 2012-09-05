#include "ElasticityTensorR4.h"
#include "MaterialProperty.h"

 void
 ElasticityTensorR4::fillFromInputVector(const std::vector<Real> input, bool all)
 {
   if((all == true && input.size() != 21) || (all == false && input.size() != 9 ))
    mooseError("Please check the number of entries in the stiffness input vector.");
  
  zero();
  
  if(all == true)
  {
    _vals[0][0][0][0] = input[0]; //C1111
    _vals[0][0][1][1] = input[1]; //C1122
    _vals[0][0][2][2] = input[2]; //C1133
    _vals[0][0][1][2] = input[3]; //C1123
    _vals[0][0][0][2] = input[4]; //C1113
    _vals[0][0][0][1] = input[5]; //C1112

    _vals[1][1][1][1] = input[6]; //C2222
    _vals[1][1][2][2] = input[7]; //C2233
    _vals[1][1][1][2] = input[8]; //C2223
    _vals[0][2][1][1] = input[9]; //C2213  //flipped for filling purposes
    _vals[0][1][1][1] = input[10]; //C2212 //flipped for filling purposes

    _vals[2][2][2][2] = input[11]; //C3333
    _vals[1][2][2][2] = input[12]; //C3323 //flipped for filling purposes
    _vals[0][2][2][2] = input[13]; //C3313 //flipped for filling purposes
    _vals[0][1][2][2] = input[14]; //C3312 //flipped for filling purposes
    
    _vals[1][2][1][2] = input[15]; //C2323
    _vals[0][2][1][2] = input[16]; //C2313 //flipped for filling purposes
    _vals[0][1][1][2] = input[17]; //C2312 //flipped for filling purposes
     
    _vals[0][2][0][2] = input[18]; //C1313
    _vals[0][1][0][2] = input[19]; //C1312 //flipped for filling purposes
    
    _vals[0][1][0][1] = input[20]; //C1212
  }
  else
  {
    _vals[0][0][0][0] = input[0];   //C1111
    _vals[0][0][1][1] = input[1];   //C1122
    _vals[0][0][2][2] = input[2];   //C1133
    _vals[1][1][1][1] = input[3];   //C2222
    _vals[1][1][2][2] = input[4];   //C2233
    _vals[2][2][2][2] = input[5];  //C3333
    _vals[1][2][1][2] = input[6];  //C2323
    _vals[0][2][0][2] = input[7];  //C1313
    _vals[0][1][0][1] = input[8];  //C1212
  }
  
  //fill in from symmetry relations
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][l][k] =
            _vals[j][i][k][l] = 
            _vals[j][i][l][k] =
            _vals[k][l][i][j] =
            _vals[l][k][j][i] = 
            _vals[k][l][j][i] =
            _vals[l][k][i][j] = _vals[i][j][k][l];
 }

  
Real
ElasticityTensorR4::stiffness( const unsigned int i, const unsigned int j, const RealGradient & test, const RealGradient & phi)
{
  RealGradient b(0);
  
  unsigned int A[N], B[N], C[N], D[N];

  //these are actual crystal indices
  if (i == 0)
  {
    A[0] = 1; A[1] = 2; A[2] = 1;
    B[0] = 1; B[1] = 3; B[2] = 2;
  }
  if (i == 1)
  {
    A[0] = 1; A[1] = 2; A[2] = 1;
    B[0] = 1; B[1] = 3; B[2] = 2;
  }
  if (i == 2)
  {
    A[0] = 1; A[1] = 2; A[2] = 1;
    B[0] = 1; B[1] = 3; B[2] = 2;
  }
  if (j == 0)
  {
    C[0] = 1; C[1] = 2; C[2] = 1;
    D[0] = 1; D[1] = 3; D[2] = 2;
  }
  if (j == 1)
  {
    C[0] = 1; C[1] = 2; C[2] = 1;
    D[0] = 1; D[1] = 3; D[2] = 2;
  }
  if (j == 2)
  {
    C[0] = 1; C[1] = 2; C[2] = 1;
    D[0] = 1; D[1] = 3; D[2] = 2;
  }

  for(unsigned int k(0); k<N; k++)
  {
    for(unsigned int l(0); l<N; l++)
    {
      // array indices - thus subtracting 1
      unsigned int m, n, o, p;
      m = A[k] - 1;
      n = B[k] - 1;
      o = C[l] - 1;
      p = D[l] - 1;
      
      b(k) = b(k) + _vals[m][n][o][p]*phi(l);
    }
  }
  return test*b;
}
