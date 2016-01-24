/*****************************************************************************
File: cg-lib.cpp

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
/////////////////////////////////////////////////////////
// POINTS AND VECTORS
/////////////////////////////////////////////////////////

#include <GL/gl.h>           
#include "cg-lib.h"

/////////////////////////////////////////////////////////
// point3
/////////////////////////////////////////////////////////
point3::point3( void ) {
  x = 0;
  y = 0;
  z = 0;
  IDSelectable = -1;
  estSelectionne = false;
}
point3::point3( GLfloat X , GLfloat Y , GLfloat Z ) {
  x = X;
  y = Y;
  z = Z;
  IDSelectable = -1;
  estSelectionne = false;
}
void point3::operator=(point3& p) {
  x = p.x;
  y = p.y;
  z = p.z;
  IDSelectable = p.IDSelectable;
  estSelectionne = p.estSelectionne;
}
void point3::operator+=(vector3 &v) {
  x += v.x;
  y += v.y;
  z += v.z;
}
void point3::operator+(point3 &p) {
  x += p.x;
  y += p.y;
  z += p.z;
}
void point3::update( GLfloat X , GLfloat Y , GLfloat Z ) {
  x = X;
  y = Y;
  z = Z;
}
void point3::middle( point3 &p1 , point3 &p2 )
{
  x = .5 * (p1.x + p2.x);
  y = .5 * (p1.y + p2.y);
  z = .5 * (p1.z + p2.z);
}
GLfloat point3::distance( point3 &p1 ) {
  vector3 v;
  v.x = p1.x - x;
  v.y = p1.y - y;
  v.z = p1.z - z;
  return v.norme();
}
GLfloat point3::distancexz( point3 &p1 ) {
  vector3 v;
  v.x = p1.x - x;
  v.y = 0;
  v.z = p1.z - z;
  return v.norme();
}
void point3::linearCombination( point3 *points , 
				float w1 , float w2 ) {
  x = w1 * points[0].x + w2 * points[1].x;
  y = w1 * points[0].y + w2 * points[1].y;
  z = w1 * points[0].z + w2 * points[1].z;
} 
// rotation d'un angle alpha autour Oy d'un point ou d'un vector
void point3::rotationv( point3 &source , float alpha ) {
  x = source.x * cos( alpha );
  y = source.y;
  z = source.x * sin( alpha );
}
void point3::linearCombination( point3 *points , 
				float w1 , float w2 , float w3 ) {
  x = w1 * points[0].x + w2 * points[1].x + w3 * points[2].x;
  y = w1 * points[0].y + w2 * points[1].y + w3 * points[2].y;
  z = w1 * points[0].z + w2 * points[1].z + w3 * points[2].z;
} 
void point3::linearCombination( point3 *points , 
				float w1 , float w2 , float w3 , float w4 ) {
  x = w1 * points[0].x + w2 * points[1].x + w3 * points[2].x
    + w4 * points[3].x;
  y = w1 * points[0].y + w2 * points[1].y + w3 * points[2].y
    + w4 * points[3].y;
  z = w1 * points[0].z + w2 * points[1].z + w3 * points[2].z
    + w4 * points[3].z;
} 

void point3::product( point3 &p , matrix44 &m ) {
  float * value = m.GetValue();
  x = p.x * value[0] + p.y * value[4] + p.z * value[8] + value[12];
  y = p.x * value[1] + p.y * value[5] + p.z * value[9] + value[13];
  z = p.x * value[2] + p.y * value[6] + p.z * value[10] + value[14];
}

/////////////////////////////////////////////////////////
// vector3
/////////////////////////////////////////////////////////
vector3::vector3( void ) {
  x = 0;
  y = 0;
  z = 0;
}
vector3::vector3( GLfloat X , GLfloat Y, GLfloat Z ) {
  x = X;
  y = Y;
  z = Z;
}
vector3::vector3( GLfloat v[3] ) {
  x = v[0];
  y = v[1];
  z = v[2];
}
vector3::vector3( point3 &p1 , point3 &p2 ) {
  x = p2.x - p1.x;
  y = p2.y - p1.y;
  z = p2.z - p1.z;
}
void vector3::operator=(vector3& v) {
  x = v.x;
  y = v.y;
  z = v.z;
}
void vector3::operator+=(vector3& v) {
  x += v.x;
  y += v.y;
  z += v.z;
}
void vector3::operator*=(double d) {
  x *= d;
  y *= d;
  z *= d;
}
void vector3::operator/=(double d) {
  x /= d;
  y /= d;
  z /= d;
}
void vector3::operator*(float f) {
  x = f * x;
  y = f * y;
  z = f * z;
}
int vector3::operator==(vector3& v) {
  return((x == v.x) && (y == v.y) && (z == v.z));
}
void vector3::update( GLfloat X , GLfloat Y , GLfloat Z ) {
  x = X;
  y = Y;
  z = Z;
}
void vector3::update( point3 &p1 , point3 &p2 ) {
  x = p2.x - p1.x;
  y = p2.y - p1.y;
  z = p2.z - p1.z;
}
GLfloat vector3::norme( void ) {
  return sqrt( x * x + y * y + z * z );
}

// normalisation de vectors
void vector3::normalize()
{
  GLfloat norme = this->norme();

  if( norme != 0 ) {
    x /= norme;
    y /= norme;
    z /= norme;
  }
}

// average
void vector3::averageVectors( vector3 *vectors , int nbVectors ) {
  x = 0; y = 0; z = 0;
  if( nbVectors <= 0 ) {
    return;
  }
  for( int ind = 0 ; ind < nbVectors ; ind++ ) {
    x += vectors[ ind ].x;
    y += vectors[ ind ].y;
    z += vectors[ ind ].z;
  }
  float inv = 1.0 / (float)nbVectors;
  x *= inv;
  y *= inv;
  z *= inv;
}

GLfloat vector3::cosinus( vector3 &v1 , vector3 &v2 ) {
  GLfloat norm = v1.norme() * v2.norme();
  if( norm != 0 ) {
    return ( v1.x * v2.x +  v1.y * v2.y +  v1.z * v2.z )/norm;
  }
  else {
    return 0;
  }
}

// vector that is the half sum of 2 vectors
void vector3::averageVector( vector3 &v1 , vector3 &v2 ) {
  x = v1.x * .5 + v2.x * .5;
  y = v1.y * .5 + v2.y * .5;
  z = v1.z * .5 + v2.z * .5;
}
// vector that is the half sum of 2 vectors
void vector3::averageNormalizedVector( vector3 &v1 , vector3 &v2 ) {
  x = v1.x * .5 + v2.x * .5;
  y = v1.y * .5 + v2.y * .5;
  z = v1.z * .5 + v2.z * .5;
  normalize();
}

// calcul du produit vectoriel
//       2 vectors vus de l'avant
//
//       <---v1---
//                |
//               v2
//                |
//                v
// 2 vectors
void vector3::prodVect( vector3 &v1 , vector3 &v2 )
{
  x = v1.y * v2.z - v2.y * v1.z;
  y = v2.x * v1.z - v1.x * v2.z;
  z = v2.y * v1.x - v1.y * v2.x;
}

//       3 points vus de l'avant
//
//       p1     p2
//
//              p3
//
// 3 points
void vector3::prodVect( point3 &p1 , point3 &p2 , 
			 point3 &p3 )
{
  vector3 v1( p2 , p1 );
  vector3 v2( p2 , p3 );

  prodVect( v1 , v2 );
  //  pv->x = (p1.y - p2.y) * (p3.z - p2.z) - (p3.y - p2.y) * (p1.z - p2.z);
  //  pv->y = (p3.x - p2.x) * (p1.z - p2.z) - (p1.x - p2.x) * (p3.z - p2.z);
  //  pv->z = (p3.y - p2.y) * (p1.x - p2.x) - (p1.y - p2.y) * (p3.x - p2.x);
}

void vector3::prodVect( point3 &p1 , point3 &p2 , vector3 &v ) {
  vector3 v1( p1 , p2 );
  prodVect( v1 , v );
}

GLfloat vector3::prodScal( vector3 &v1 ) {
  return x * v1.x + y * v1.y + z * v1.z;
}

GLfloat vector3::prodScal( point3 &p1 , point3 &p2 ) {
  vector3 v1( p1 , p2 );
  return prodScal( v1 );
}

GLfloat vector3::prodScalxz( vector3 &v1 ) {
  return x * v1.x + z * v1.z;
}

GLfloat vector3::prodScalxz( point3 &p1 , point3 &p2 ) {
  vector3 v1( p1 , p2 );
  return prodScalxz( v1 );
}

void vector3::product( vector3 &v , matrix44 &m ) {
  float * value = m.GetValue();
  x = v.x * value[0] + v.y * value[4] + v.z * value[8];
  y = v.x * value[1] + v.y * value[5] + v.z * value[9];
  z = v.x * value[2] + v.y * value[6] + v.z * value[10];
}

// rotation d'un angle alpha autour Oy d'un point ou d'un vector
void vector3::rotationv( vector3 &source , float alpha ) {
  x = source.x * cos( alpha );
  y = source.y;
  z = source.x * sin( alpha );
}

/////////////////////////////////////////////////////////
// matrices
/////////////////////////////////////////////////////////
matrix44::matrix44( void ) {
  LoadIdentity();
}
matrix44::matrix44( GLfloat source_value ) {
  for( int ind = 0 ; ind < 16 ; ind++ ) {
    values[ind] = source_value;
  }
}
matrix44::matrix44( GLfloat *source_values ) {
  for( int ind = 0 ; ind < 16 ; ind++ ) {
    values[ind] = source_values[ind];
  }
}
// TER -- PHYSIQUE
// Mets a jour les valeurs de la matrice 
// << a priori c'est inutile maintenant qu'il y a GetModelViewMatrix44() >>
void matrix44::update ( GLfloat *source_values ) {
	for( int ind = 0 ; ind < 16 ; ind++ ) {
    values[ind] = source_values[ind];
  }
}

// Construit une matrice fausse de rotation suivant l'axe axis et d'angle angle !!!!
void matrix44::rotationMatrix(vector3 axis, float angle) {
	float cost = cos(M_PI*angle/180.0);
	float sint = sin(M_PI*angle/180.0);
	float mcost = 1 - cost;
	
	values[0] = cost + mcost*axis.x*axis.x;
	values[1] = mcost*axis.x*axis.y + sint*axis.z;
	values[2] = mcost*axis.x*axis.z - sint*axis.y;
	values[3] = 0;
	values[4] = mcost*axis.y*axis.x - sint*axis.z;
	values[5] = cost + mcost*axis.y*axis.y;
	values[6] = mcost*axis.y*axis.z + sint*axis.x;
	values[7] = 0;
	values[8] = mcost*axis.x*axis.z + sint*axis.y;
	values[9] = mcost*axis.z*axis.y - sint*axis.x;
	values[10] = cost + mcost*axis.z*axis.z;
	values[11] = 0;
	values[12] = 0;
	values[13] = 0;
	values[14] = 0;
	values[15] = 1;
}
// TER -- PHYSIQUE

void matrix44::LoadIdentity( void ) {
  for( int ind = 0 ; ind < 16 ; ind++ ) {
    values[ind] = 0.0;
  }
  values[0] = 1.0; values[5] = 1.0; values[10] = 1.0;
  values[15] = 1.0;
}
void matrix44::LoadRotationIdentity( void ) {
  for( int ind = 0 ; ind < 12 ; ind++ ) {
    values[ind] = 0.0;
  }
  values[0] = 1.0; values[5] = 1.0; values[10] = 1.0;
}
GLfloat *matrix44::GetValue( void ) {
  return values;
}
void matrix44::product(matrix44& R , matrix44& D) {
  R.values[0]  = values[0]*D.values[0] + values[4]*D.values[1] + values[8]*D.values[2] + values[12]*D.values[3];
  R.values[1]  = values[1]*D.values[0] + values[5]*D.values[1] + values[9]*D.values[2] + values[13]*D.values[3];
  R.values[2]  = values[2]*D.values[0] + values[6]*D.values[1] + values[10]*D.values[2] + values[14]*D.values[3];
  R.values[3]  = values[3]*D.values[0] + values[7]*D.values[1] + values[11]*D.values[2] + values[15]*D.values[3];
  
  R.values[4]  = values[0]*D.values[4] + values[4]*D.values[5] + values[8]*D.values[6] + values[12]*D.values[7];
  R.values[5]  = values[1]*D.values[4] + values[5]*D.values[5] + values[9]*D.values[6] + values[13]*D.values[7];
  R.values[6]  = values[2]*D.values[4] + values[6]*D.values[5] + values[10]*D.values[6] + values[14]*D.values[7];
  R.values[7]  = values[3]*D.values[4] + values[7]*D.values[5] + values[11]*D.values[6] + values[15]*D.values[7];
  
  R.values[8]  = values[0]*D.values[8] + values[4]*D.values[9] + values[8]*D.values[10] + values[12]*D.values[11];
  R.values[9]  = values[1]*D.values[8] + values[5]*D.values[9] + values[9]*D.values[10] + values[13]*D.values[11];
  R.values[10] = values[2]*D.values[8] + values[6]*D.values[9] + values[10]*D.values[10] + values[14]*D.values[11];
  R.values[11] = values[3]*D.values[8] + values[7]*D.values[9] + values[11]*D.values[10] + values[15]*D.values[11];
  
  R.values[12] = values[0]*D.values[12] + values[4]*D.values[13] + values[8]*D.values[14] + values[12]*D.values[15];
  R.values[13] = values[1]*D.values[12] + values[5]*D.values[13] + values[9]*D.values[14] + values[13]*D.values[15];
  R.values[14] = values[2]*D.values[12] + values[6]*D.values[13] + values[10]*D.values[14] + values[14]*D.values[15];
  R.values[15] = values[3]*D.values[12] + values[7]*D.values[13] + values[11]*D.values[14] + values[15]*D.values[15];
}
matrix44 matrix44::operator * (matrix44 &Droite) {
  matrix44 r;
  product(r, Droite);
  return (matrix44(r)); 
}
void matrix44::operator=(matrix44& m) {
  for( int ind = 0 ; ind < 16 ; ind++ ) {
    values[ind] = m.values[ind];
  }
}
void matrix44::operator+=(matrix44& m) {
  for( int ind = 0 ; ind < 16 ; ind++ ) {
    values[ind] += m.values[ind];
  }
}
void matrix44::operator+=(vector3& v) {
  values[12] += v.x;
  values[13] += v.y;
  values[14] += v.z;
}
int matrix44::operator==(matrix44& m) {
  for( int ind = 0 ; ind < 16 ; ind++ ) {
    if( values[ind] != m.values[ind] ) {
      return false;
    }
  }
  return true;
}

// computes the inverse of a matrix
// taken from glu library
GLboolean invert_matrix(const GLfloat * m, GLfloat * out) {
/* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { GLfloat *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

   GLfloat wtmp[4][8];
   GLfloat m0, m1, m2, m3, s;
   GLfloat *r0, *r1, *r2, *r3;

   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
      r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
      r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
      r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
      r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
      r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
      r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
      r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
      r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
      r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
      r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
      r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabs(r3[0]) > fabs(r2[0]))
      SWAP_ROWS(r3, r2);
   if (fabs(r2[0]) > fabs(r1[0]))
      SWAP_ROWS(r2, r1);
   if (fabs(r1[0]) > fabs(r0[0]))
      SWAP_ROWS(r1, r0);
   if (0.0 == r0[0])
      return GL_FALSE;

   /* eliminate first variable     */
   m1 = r1[0] / r0[0];
   m2 = r2[0] / r0[0];
   m3 = r3[0] / r0[0];
   s = r0[1];
   r1[1] -= m1 * s;
   r2[1] -= m2 * s;
   r3[1] -= m3 * s;
   s = r0[2];
   r1[2] -= m1 * s;
   r2[2] -= m2 * s;
   r3[2] -= m3 * s;
   s = r0[3];
   r1[3] -= m1 * s;
   r2[3] -= m2 * s;
   r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) {
      r1[4] -= m1 * s;
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r0[5];
   if (s != 0.0) {
      r1[5] -= m1 * s;
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r0[6];
   if (s != 0.0) {
      r1[6] -= m1 * s;
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r0[7];
   if (s != 0.0) {
      r1[7] -= m1 * s;
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabs(r3[1]) > fabs(r2[1]))
      SWAP_ROWS(r3, r2);
   if (fabs(r2[1]) > fabs(r1[1]))
      SWAP_ROWS(r2, r1);
   if (0.0 == r1[1])
      return GL_FALSE;

   /* eliminate second variable */
   m2 = r2[1] / r1[1];
   m3 = r3[1] / r1[1];
   r2[2] -= m2 * r1[2];
   r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3];
   r3[3] -= m3 * r1[3];
   s = r1[4];
   if (0.0 != s) {
      r2[4] -= m2 * s;
      r3[4] -= m3 * s;
   }
   s = r1[5];
   if (0.0 != s) {
      r2[5] -= m2 * s;
      r3[5] -= m3 * s;
   }
   s = r1[6];
   if (0.0 != s) {
      r2[6] -= m2 * s;
      r3[6] -= m3 * s;
   }
   s = r1[7];
   if (0.0 != s) {
      r2[7] -= m2 * s;
      r3[7] -= m3 * s;
   }

   /* choose pivot - or die */
   if (fabs(r3[2]) > fabs(r2[2]))
      SWAP_ROWS(r3, r2);
   if (0.0 == r2[2])
      return GL_FALSE;

   /* eliminate third variable */
   m3 = r3[2] / r2[2];
   r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
      r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

   /* last check */
   if (0.0 == r3[3])
      return GL_FALSE;

   s = 1.0 / r3[3];		/* now back substitute row 3 */
   r3[4] *= s;
   r3[5] *= s;
   r3[6] *= s;
   r3[7] *= s;

   m2 = r2[3];			/* now back substitute row 2 */
   s = 1.0 / r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
      r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
      r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
      r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

   m1 = r1[2];			/* now back substitute row 1 */
   s = 1.0 / r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
      r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
      r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

   m0 = r0[1];			/* now back substitute row 0 */
   s = 1.0 / r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
      r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(out, 0, 0) = r0[4];
   MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
   MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
   MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
   MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
   MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
   MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
   MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
   MAT(out, 3, 3) = r3[7];

   return GL_TRUE;

#undef MAT
#undef SWAP_ROWS
}


matrix44 matrix44::operator ~( void ) { 
  float inv[16];

  invert_matrix(values, inv);

  return (matrix44(inv));
}


