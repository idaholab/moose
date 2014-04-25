/*
 * dynamicArray.h
 *
 *      Author: MANDD
 */

#ifndef DYNAMICARRAY_H_
#define DYNAMICARRAY_H_

template <typename T>
T **AllocateDynamicArray( int nRows, int nCols)
{
      T **dynamicArray;

      dynamicArray = new T*[nRows];
      for( int i = 0 ; i < nRows ; i++ )
    	  dynamicArray[i] = new T [nCols];

      return dynamicArray;
}

template <typename T>
void FreeDynamicArray(T** dArray)
{
      delete [] *dArray;
      delete [] dArray;
}


#endif /* DYNAMICARRAY_H_ */
