/*****************************************************************************
File: cg-lib.h

Rendu Graphique Avancé RGA
Master Professionnel d'informatique
Christian Jacquemin, Université Paris 11

Copyright (C) 2004 University Paris 11 
This file is provided without support, instruction, or implied
warranty of any kind.  University Paris 11 makes no guarantee of its
fitness for a particular purpose and is not liable under any
circumstances for any damages or loss whatsoever arising from the use
or inability to use this file or items derived from it.
******************************************************************************/

#ifndef VC_LIB_H
#define VC_LIB_H

#include <math.h>

class vector3;
class matrix44;;

/////////////////////////////////////////////////////////
// POINTS AND VECTORS
/////////////////////////////////////////////////////////
class point3 {
 public:
  GLfloat   x;
  GLfloat   y;
  GLfloat   z;
  int IDSelectable;
  int estSelectionne;
  point3( void );
  point3( GLfloat X , GLfloat Y , GLfloat Z );
  void operator=(point3&);
  void operator+=(vector3&);
  void operator+(point3 &p);
  void update( GLfloat X , GLfloat Y , GLfloat Z );
  void middle( point3 &p1 , point3 &p2 );
  GLfloat distance( point3 &p1 );
  GLfloat distancexz( point3 &p1 );
  void linearCombination( point3 *points , 
			  float w1 , float w2 );
  void linearCombination( point3 *points , 
			  float w1 , float w2 , float w3 );
  void linearCombination( point3 *points , 
			  float w1 , float w2 , float w3 , float w4 );
  void rotationv( point3 &source , float alpha );
  void interpolate( point3 *p1 , point3 *p2 );
  void product( point3 &p , matrix44 &m );
  void operator *( matrix44 &modelViewMatrix );
};

class vector3 {
 public:
  GLfloat   x;
  GLfloat   y;
  GLfloat   z;
  vector3( void );
  vector3( GLfloat X , GLfloat Y , GLfloat Z );
  vector3( GLfloat v[3] );
  vector3( point3 &p1 , point3 &p2 );
  GLfloat norme( void );
  void normalize( void );
  void averageVectors( vector3 *vectors , int nbVectors );
  GLfloat prodScal( vector3 &v1 );
  GLfloat prodScal( point3 &p1 , point3 &p2 );
  GLfloat prodScalxz( vector3 &v1 );
  GLfloat prodScalxz( point3 &p1 , point3 &p2 );
  void prodVect( vector3 &v1 , vector3 &v2 );
  void averageVector( vector3 &v1 , vector3 &v2 );
  void averageNormalizedVector( vector3 &v1 , vector3 &v2 );
  void prodVect( point3 &p1 , point3 &p2 , point3 &p3 );
  void prodVect( point3 &p1 , point3 &p2 , vector3 &v );
  void rotationv( vector3 &source , float alpha );
  void product( vector3 &v , matrix44 &m );
  GLfloat cosinus( vector3 &v1 , vector3 &v2 );
  void operator=(vector3&);
  void operator+=(vector3&);
  void operator*(float f);
  int operator==(vector3&);
  void operator*=(double d);
  void operator/=(double d);
  void update( GLfloat X , GLfloat Y , GLfloat Z );
  void update( point3 &p1 , point3 &p2 );
  void interpolate( vector3 **tabVectors , 
		    float *weights1 , 
		    float *weights2 , 
		    int numberOfComponents ,
		    float coef );
  void interpolate( vector3 *p1 , vector3 *p2 );
};

GLboolean invert_matrix(const GLfloat * m, GLfloat * out);

class matrix44 {
 private:
  GLfloat   values[16];
  int DoPivot(matrix44 &A, matrix44 &I, int diag);
 public:
  matrix44( void );
  matrix44( GLfloat source_value );
  matrix44( GLfloat *source_values );
 // TER -- PHYSIQUE
	void update( GLfloat *source_values );
	void rotationMatrix(vector3 axis, float angle);
 // TER -- PHYSIQUE
  void LoadRotationIdentity( void );
  void LoadIdentity( void );
  GLfloat *GetValue( void );
  void product(matrix44& R , matrix44& D);
  matrix44 operator * (matrix44 &Droite);
  void operator=(matrix44&);
  void operator+=(matrix44&);
  void operator+=(vector3&);
  int operator==(matrix44&);
  matrix44 operator ~( void );
};

#endif
