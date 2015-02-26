#ifndef LINALG_H
#define LINALG_H

/*
 * =====================================================================================
 *
 *       Filename:  linalg.cpp
 *
 *    Description:  Templated generic linear algebra functions
 *
 *        Version:  1.0
 *        Created:  09/03/2014 04:26:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joshua Hernandez (jah), endopol@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  lanczos(const double[NDIM][NDIM], float[NDIM], int)
 *  Description:  Computes the largest eigenvalue and corresponding eigenvectors of a
 *                  symmetric positive definite matrix
 * =====================================================================================
 */


template <typename T>
float norm2(const T v[NDIM]){
    double sum = 0;
    for(int i=0; i<NDIM; i++)
        sum += v[i]*v[i];
    return sum;
}

template <typename T1, typename T2>
float gauss(const T1 x[NDIM], T2 sigma){    
    return exp(-norm2(x)/(2*sigma*sigma));
}

template <typename T1, typename T2>
double lanczos(const T1 A[NDIM][NDIM], T2 b[NDIM], int niter){
    double b_n, eig=0;

    for(int n=0; n<niter; n++){

        b_n = sqrt(norm2(b));

        double temp[NDIM];
        for(int i = 0; i<NDIM; i++){
            temp[i] = 0;

            for(int j=0; j<NDIM; j++)
                temp[i] += A[i][j]*b[j];
        }
        
        eig = temp[0]/b[0];

        for(int i=0; i<NDIM; i++)
            b[i] = temp[i]/b_n;
    }

    return eig;
}

template <typename T1, typename T2>
double biggest_eigval(T1 mat[NDIM][NDIM], T2 v[NDIM]){

    double eig, total = 0;
    for(int n=0; n<NDIM; n++){
        float v1[NDIM] = {1, 0, 0};
        eig = lanczos(mat, v1, 10);

        if(eig<=0){            
            v[0] = v1[0];
            v[1] = v1[1];
            v[2] = v1[2];
            return eig + total;
        }

        total += eig;
        for(int i=0; i<NDIM; i++)
            mat[i][i] -= eig;
    }

    v[0] = 0;    v[1] = 0;    v[2] = 0; // error state

    return 0;
}

template <typename T1, typename T2>
float dot(const T1 v1[NDIM], const T2 v2[NDIM]){
    float sum = 0;
    for(int i=0; i<NDIM; i++)
        sum += v1[i]*v2[i];
    return sum;
}

template <typename T1, typename T2>
void scale(T1 v[NDIM], T2 a){
    for(int i=0; i<NDIM; i++)
        v[i] = v[i]*a;
}

template <typename T>
void print(const T v[NDIM]){
    for(int i=0; i<NDIM; i++)
        cout << v[i] << " ";
}

template <typename T1, typename T2>
void copyTo(const T1 v1[NDIM], T2 v2[NDIM]){
    for(int i=0; i<NDIM; i++)
        v2[i] = v1[i];
}

template <typename T1, typename T2>
void addTo(const T1 v1[NDIM], T2 v2[NDIM]){
    for(int i=0; i<NDIM; i++)
        v2[i]+=v1[i];
}

template <typename T1, typename T2, typename T3>
void sub(const T1 v1[NDIM], const T2 v2[NDIM], T3 v3[NDIM]){
    for(int i=0; i<NDIM; i++)
        v3[i] = v1[i] - v2[i];
}

template <typename T1, typename T2, typename T3>
void sum(const T1 v1[NDIM], const T2 v2[NDIM], T3 v3[NDIM]){
    for(int i=0; i<NDIM; i++)
        v3[i] = v1[i] + v2[i];
}

template <typename T1, typename T2>
float conjugate(const T1 b[NDIM], const T2 A[NDIM][NDIM]){
    float sum = 0;
    for(int i=0; i<NDIM; i++){
        float partsum = 0;
        for(int j=0; j<NDIM; j++){
            //cout << setw(7) << A[i][j] << " ";
            partsum += A[i][j]*b[j];
        }
        //cout << "\t" << b[i] << endl;
        sum += b[i]*partsum;
    }
    //cout << endl;

    //cout << sqrt(abs(sum)) << endl << endl;


    return abs(sum);
}

template <typename T1, typename T2>
double gain(T1 mat[NDIM][NDIM], T2 v[NDIM]){
    double total = 0;
    for(int i=0; i<NDIM; i++){
        double row_sum = 0;
        for(int j=0; j<NDIM; j++){
            row_sum += mat[i][j]*v[j];
        }
        //total += row_sum*v[i];
        total += row_sum*row_sum;
    }
    return sqrt(total);
}

template <typename T> 
bool isZero(T* v, int numel){
    for(int i=0; i<numel; i++)
        if(v[i]!=0)
            return false;
        
    return true;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  l2Dist(const vector<int>&, const vector<int>&)
 *  Description:  Compute the l2 distance between two vectors
 * =====================================================================================
 */
template <typename T1, typename T2>
float l2Dist(const T1 p1[NDIM], const T2 p2[NDIM]){
    float dist2 = 0;

    for(int i=0; i<NDIM; i++){
        float diff_i = p2[i] - p1[i];
        dist2 += diff_i*diff_i;
    }

    return sqrt(dist2);
}

#endif // LINALG_H
