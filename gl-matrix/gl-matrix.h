#ifndef MAT2_H
#define MAT2_H

#include <stdint.h>

/**
 * Set a mat2 to the identity matrix
 *
 * @param {mat2} out the receiving matrix
 */
void mat2_identity(float* dst);

/**
 * Copy a mat2 to another mat2
 *
 * @param {mat2} out the receiving matrix
 * @param {mat2} out the source matrix
 */
void mat2_copy(float* dst, float* src);

/**
 * Transpose the values of a mat2
 *
 * @param {mat2} the matrix
 */
void mat2_transpose(float* dst);

/**
 * Inverts a mat2
 *
 * @param {mat2} the matrix
 */
void mat2_invert(float* dst);

/**
 * Calculates the adjugate of a mat2
 *
 * @param {mat2} the matrix
 */
void mat2_adjoint(float* dst);

/**
 * Calculates the determinant of a mat2
 *
 * @param {mat2} a the source matrix
 * @returns {float} determinant of a
 */
float mat2_determinant(float* dst);

/**
 * Multiplies two mat2's
 *
 * @param {mat2} out the receiving matrix
 * @param {mat2} the operand
 */
void mat2_multiply(float* dst, float* op);

/**
 * Rotates a mat2 by the given angle
 *
 * @param {mat2} out the receiving matrix
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat2_rotate(float* dst, float rad);

/**
 * Scales the mat2 by the dimensions in the given vec2
 *
 * @param {mat2} out the receiving matrix
 * @param {vec2} v the vec2 to scale the matrix by
 **/
void mat2_scale(float* dst, float* v);

/**
 * Creates a matrix from a given angle
 * This is equivalent to (but much faster than):
 *
 *     mat2_identity(dst);
 *     mat2_rotate(dst, rad);
 *
 * @param {mat2} out mat2 receiving operation result
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat2_fromRotation(float* dst, float rad);

/**
 * Creates a matrix from a vector scaling
 * This is equivalent to (but much faster than):
 *
 *     mat2_identity(dst);
 *     mat2_scale(dst, dst, vec);
 *
 * @param {mat2} out mat2 receiving operation result
 * @param {vec2} v Scaling vector
 */
void mat2_fromScaling(float* dst, float* v);

/**
 * Adds two mat2's
 *
 * @param {mat2} the receiving matrix
 * @param {mat2} the operand
 */
void mat2_add(float* dst, float* a);

/**
 * Subtracts matrix b from matrix a
 *
 * @param {mat2} the receiving matrix
 * @param {mat2} the operand
 */
void mat2_subtract(float* dst, float* b);

/**
 * Returns whether or not the matrices have exactly the same elements.
 *
 * @param {mat2} a The first matrix.
 * @param {mat2} b The second matrix.
 * @returns {uint8_t} 1 if the matrices are equal, 0 otherwise.
 */
uint8_t mat2_equals(float* a, float* b);

/**
 * Multiply each element of the matrix by a scalar.
 *
 * @param {mat2} out the receiving matrix
 * @param {Number} b amount to scale the matrix's elements by
 */
void mat2_multiplyScalar(float* dst, float b);

/**
 * Adds two mat2's after multiplying each element of the second operand by a scalar value.
 *
 * @param {mat2} out the receiving vector
 * @param {mat2} b the second operand
 * @param {Number} scale the amount to scale b's elements by before adding
 */
void mat2_multiplyScalarAndAdd(float* dst, float* b, float scale);

#endif
#ifndef MAT4_H
#define MAT4_H

#include <stdint.h>

/**
 * Set a mat4 to the identity matrix
 *
 * @param {mat4} out the receiving matrix
 */
void mat4_identity(float dst[16]);

/**
 * Copy the values from one mat4 to another
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} a the source matrix
 */
void mat4_copy(float* dst, float* src);

/**
 * Set the components of a mat4 to the given values
 *
 * @param {mat4} out the receiving matrix
 * @param {Number} m00 Component in column 0, row 0 position (index 0)
 * @param {Number} m01 Component in column 0, row 1 position (index 1)
 * @param {Number} m02 Component in column 0, row 2 position (index 2)
 * @param {Number} m03 Component in column 0, row 3 position (index 3)
 * @param {Number} m10 Component in column 1, row 0 position (index 4)
 * @param {Number} m11 Component in column 1, row 1 position (index 5)
 * @param {Number} m12 Component in column 1, row 2 position (index 6)
 * @param {Number} m13 Component in column 1, row 3 position (index 7)
 * @param {Number} m20 Component in column 2, row 0 position (index 8)
 * @param {Number} m21 Component in column 2, row 1 position (index 9)
 * @param {Number} m22 Component in column 2, row 2 position (index 10)
 * @param {Number} m23 Component in column 2, row 3 position (index 11)
 * @param {Number} m30 Component in column 3, row 0 position (index 12)
 * @param {Number} m31 Component in column 3, row 1 position (index 13)
 * @param {Number} m32 Component in column 3, row 2 position (index 14)
 * @param {Number} m33 Component in column 3, row 3 position (index 15)
 */
void mat4_set(float* dst, float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);

/**
 * Transpose the values of a mat4
 *
 * @param {mat4} out the receiving matrix
 */
void mat4_transpose(float* dst);

/**
 * Inverts a mat4
 *
 * @param {mat4} out the receiving matrix
 */
void mat4_invert(float* dst);

/**
 * Calculates the adjugate of a mat4
 *
 * @param {mat4} out the receiving matrix
 */
void mat4_adjoint(float* dst);

/**
 * Calculates the determinant of a mat4
 *
 * @param {mat4} a the source matrix
 * @returns {Number} determinant of a
 */
float mat4_determinant(float* dst);

/**
 * Multiplies two mat4s
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} b the first operand
 */
void mat4_multiply(float* dst, float* b);

/**
 * Translate a mat4 by the given vector
 *
 * @param {mat4} out the receiving matrix
 * @param {vec3} v vector to translate by
 */
void mat4_translate(float dst[16], float v[3]);

/**
 * Scales the mat4 by the dimensions in the given vec3 not using vectorization
 *
 * @param {mat4} out the receiving matrix
 * @param {vec3} v the vec3 to scale the matrix by
 **/
void mat4_scale(float* dst, float* v);

/**
 * Rotates a mat4 by the given angle around the given axis
 *
 * @param {mat4} out the receiving matrix
 * @param {Number} rad the angle to rotate the matrix by
 * @param {vec3} axis the axis to rotate around
 */
void mat4_rotate(float* dst, float rad, float* axis);

/**
 * Rotates a matrix by the given angle around the X axis
 *
 * @param {mat4} out the receiving matrix
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat4_rotateX(float* dst, float rad);

/**
 * Rotates a matrix by the given angle around the Y axis
 *
 * @param {mat4} out the receiving matrix
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat4_rotateY(float* dst, float rad);

/**
 * Rotates a matrix by the given angle around the Z axis
 *
 * @param {mat4} out the receiving matrix
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat4_rotateZ(float* dst, float rad);

/**
 * Initializes a matrix from a vector translation
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_translate(dest, vec);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {vec3} v Translation vector
 */
void mat4_fromTranslation(float* dst, float* v);

/**
 * Initializes a matrix from a vector scaling
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_scale(dest, vec);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {vec3} v Scaling vector
 */
void mat4_fromScaling(float* dst, float* v);

/**
 * Initializes a matrix from a given angle around a given axis
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_rotate(dest, rad, axis);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {Number} rad the angle to rotate the matrix by
 * @param {vec3} axis the axis to rotate around
 */
void mat4_fromRotation(float* dst, float rad, float* axis);

/**
 * Initializes a matrix from the given angle around the X axis
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_rotateX(dest, rad);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat4_fromXRotation(float* dst, float rad);

/**
 * Initializes a matrix from the given angle around the Y axis
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_rotateY(dest, rad);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat4_fromYRotation(float* dst, float rad);

/**
 * Initializes a matrix from the given angle around the Z axis
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_rotateZ(dest, rad);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat4_fromZRotation(float* dst, float rad);

/**
 * Initializes a matrix from a quaternion rotation and vector translation
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_translate(dest, vec);
 *     float quatMat = mat4.create();
 *     quat4_toMat4(quat, quatMat);
 *     mat4_multiply(dest, quatMat);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {quat4} q Rotation quaternion
 * @param {vec3} v Translation vector
 */
void mat4_fromRotationTranslation(float* dst, float* q, float* v);

/**
 * Creates a new mat4 from a dual quat.
 *
 * @param {mat4} out Matrix
 * @param {quat2} a Dual Quaternion
 */
void mat4_fromQuat2(float* dst, float* a);

/**
 * Returns the translation vector component of a transformation
 *  matrix. If a matrix is built with fromRotationTranslation,
 *  the returned vector will be the same as the translation vector
 *  originally supplied.
 * @param  {vec3} out Vector to receive translation component
 * @param  {mat4} mat Matrix to be decomposed (input)
 */
void mat4_getTranslation(float* dst, float* mat);

/**
 * Returns the scaling factor component of a transformation
 *  matrix. If a matrix is built with fromRotationTranslationScale
 *  with a normalized Quaternion paramter, the returned vector will be
 *  the same as the scaling vector
 *  originally supplied.
 * @param  {vec3} out Vector to receive scaling factor component
 * @param  {mat4} mat Matrix to be decomposed (input)
 */
void mat4_getScaling(float* dst, float* mat);

/**
 * Returns a quaternion representing the rotational component
 *  of a transformation matrix. If a matrix is built with
 *  fromRotationTranslation, the returned quaternion will be the
 *  same as the quaternion originally supplied.
 * @param {quat} out Quaternion to receive the rotation component
 * @param {mat4} mat Matrix to be decomposed (input)
 */
void mat4_getRotation(float* dst, float* mat);

/**
 * Initializes a matrix from a quaternion rotation, vector translation and vector scale
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_translate(dest, vec);
 *     float quatMat = mat4_create();
 *     quat4_toMat4(quat, quatMat);
 *     mat4_multiply(dest, quatMat);
 *     mat4_scale(dest, scale)
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {quat4} q Rotation quaternion
 * @param {vec3} v Translation vector
 * @param {vec3} s Scaling vector
 */
void mat4_fromRotationTranslationScale(float* dst, float* q, float* v, float* s);

/**
 * Initializes a matrix from a quaternion rotation, vector translation and vector scale, rotating and scaling around the given origin
 * This is equivalent to (but much faster than):
 *
 *     mat4_identity(dest);
 *     mat4_translate(dest, vec);
 *     mat4_translate(dest, origin);
 *     float quatMat = mat4_create();
 *     quat4_toMat4(quat, quatMat);
 *     mat4_multiply(dest, quatMat);
 *     mat4_scale(dest, scale)
 *     mat4_translate(dest, negativeOrigin);
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {quat4} q Rotation quaternion
 * @param {vec3} v Translation vector
 * @param {vec3} s Scaling vector
 * @param {vec3} o The origin vector around which to scale and rotate
 */
void mat4_fromRotationTranslationScaleOrigin(float* dst, float* q, float* v, float* s, float* o);

/**
 * Calculates a 4x4 matrix from the given quaternion
 *
 * @param {mat4} out mat4 receiving operation result
 * @param {quat} q Quaternion to create matrix from
 *
 * @returns {mat4} out
 */
void mat4_fromQuat(float* dst, float* q);

/**
 * Generates a frustum matrix with the given bounds
 *
 * @param {mat4} out mat4 frustum matrix will be written into
 * @param {Number} left Left bound of the frustum
 * @param {Number} right Right bound of the frustum
 * @param {Number} bottom Bottom bound of the frustum
 * @param {Number} top Top bound of the frustum
 * @param {Number} near Near bound of the frustum
 * @param {Number} far Far bound of the frustum
 */
void mat4_frustum(float* dst, float left, float right, float bottom, float top, float near, float far);

/**
 * Generates a perspective projection matrix with the given bounds.
 * Passing null/undefined/no value for far will generate infinite projection matrix.
 *
 * @param {mat4} out mat4 frustum matrix will be written into
 * @param {number} fovy Vertical field of view in radians
 * @param {number} aspect Aspect ratio. typically viewport width/height
 * @param {number} near Near bound of the frustum
 * @param {number} far Far bound of the frustum, can be 0 or FLT_MAX
 */
void mat4_perspective(float* dst, float fovy, float aspect, float near, float far);

/**
 * Generates a orthogonal projection matrix with the given bounds
 *
 * @param {mat4} out mat4 frustum matrix will be written into
 * @param {number} left Left bound of the frustum
 * @param {number} right Right bound of the frustum
 * @param {number} bottom Bottom bound of the frustum
 * @param {number} top Top bound of the frustum
 * @param {number} near Near bound of the frustum
 * @param {number} far Far bound of the frustum
 */
void mat4_ortho(float* dst, float left, float right, float bottom, float top, float near, float far);

/**
 * Generates a look-at matrix with the given eye position, focal point, and up axis.
 * If you want a matrix that actually makes an object look at another object, you should use targetTo instead.
 *
 * @param {mat4} out mat4 frustum matrix will be written into
 * @param {vec3} eye Position of the viewer
 * @param {vec3} center Point the viewer is looking at
 * @param {vec3} up vec3 pointing up
 * @returns {mat4} out
 */
void mat4_lookAt(float* dst, float* eye, float* center, float* up);

/**
 * Generates a matrix that makes something look at something else.
 *
 * @param {mat4} out mat4 frustum matrix will be written into
 * @param {vec3} eye Position of the viewer
 * @param {vec3} center Point the viewer is looking at
 * @param {vec3} up vec3 pointing up
 * @returns {mat4} out
 */
void mat4_targetTo(float* dst, float* eye, float* target, float* up);

/**
 * Returns Frobenius norm of a mat4
 *
 * @param {mat4} a the matrix to calculate Frobenius norm of
 * @returns {Number} Frobenius norm
 */
float mat4_frob(float* a);

/**
 * Adds two mat4's
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} b the second operand
 */
void mat4_add(float* dst, float* b);

/**
 * Subtracts matrix b from matrix a
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} b the second operand
 */
void mat4_subtract(float* dst, float* b);

/**
 * Multiply each element of the matrix by a scalar.
 *
 * @param {mat4} out the receiving matrix
 * @param {Number} b amount to scale the matrix's elements by
 */
void mat4_multiplyScalar(float* dst, float b);

/**
 * Adds two mat4's after multiplying each element of the second operand by a scalar value.
 *
 * @param {mat4} out the receiving vector
 * @param {mat4} b the second operand
 * @param {Number} scale the amount to scale b's elements by before adding
 */
void mat4_multiplyScalarAndAdd(float* dst, float* b, float scale);

/**
 * Returns whether or not the matrices have exactly the same elements.
 *
 * @param {mat4} a The first matrix.
 * @param {mat4} b The second matrix.
 * @returns {uint8_t} True if the matrices are equal, false otherwise.
 */
uint8_t mat4_equals(float* a, float* b);

#endif
#ifndef MAT3_H
#define MAT3_H

#include <stdint.h>

/**
 * Copies the upper-left 3x3 values into the given mat3.
 *
 * @param {mat3} out the receiving 3x3 matrix
 * @param {mat4} a   the source 4x4 matrix
 */
void mat3_fromMat4(float* dst, float* a);

/**
 * Copy the values from one mat3 to another
 *
 * @param {mat3} out the receiving matrix
 * @param {mat3} a the source matrix
 */
void mat3_copy(float* dst, float* a);

/**
 * Set the components of a mat3 to the given values
 *
 * @param {mat3} out the receiving matrix
 * @param {Number} m00 Component in column 0, row 0 position (index 0)
 * @param {Number} m01 Component in column 0, row 1 position (index 1)
 * @param {Number} m02 Component in column 0, row 2 position (index 2)
 * @param {Number} m10 Component in column 1, row 0 position (index 3)
 * @param {Number} m11 Component in column 1, row 1 position (index 4)
 * @param {Number} m12 Component in column 1, row 2 position (index 5)
 * @param {Number} m20 Component in column 2, row 0 position (index 6)
 * @param {Number} m21 Component in column 2, row 1 position (index 7)
 * @param {Number} m22 Component in column 2, row 2 position (index 8)
 */
void mat3_set(float* dst, float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22);

/**
 * Set a mat3 to the identity matrix
 *
 * @param {mat3} out the receiving matrix
 */
void mat3_identity(float* dst);

/**
 * Transpose the values of a mat3
 *
 * @param {mat3} out the receiving matrix
 */
void mat3_transpose(float* dst);

/**
 * Inverts a mat3
 *
 * @param {mat3} out the receiving matrix
 * @returns {mat3} out
 */
void mat3_invert(float* dst);

/**
 * Calculates the adjugate of a mat3
 *
 * @param {mat3} out the receiving matrix
 */
void mat3_adjoint(float* dst);

/**
 * Calculates the determinant of a mat3
 *
 * @param {mat3} a the source matrix
 * @returns {Number} determinant of a
 */
float mat3_determinant(float* dst);

/**
 * Multiplies two mat3's
 *
 * @param {mat3} out the receiving matrix
 * @param {mat3} b the second operand
 */
void mat3_multiply(float* dst, float* b);

/**
 * Translate a mat3 by the given vector
 *
 * @param {mat3} out the receiving matrix
 * @param {vec2} v vector to translate by
 */
void mat3_translate(float* dst, float* v);

/**
 * Rotates a mat3 by the given angle
 *
 * @param {mat3} out the receiving matrix
 * @param {Number} rad the angle to rotate the matrix by
 */
void mat3_rotate(float* dst, float rad);

/**
 * Scales the mat3 by the dimensions in the given vec2
 *
 * @param {mat3} out the receiving matrix
 * @param {vec2} v the vec2 to scale the matrix by
 **/
void mat3_scale(float* dst, float* v);

/**
 * Creates a matrix from a vector translation
 * This is equivalent to (but much faster than):
 *
 *     mat3_identity(dest);
 *     mat3_translate(dest, vec);
 *
 * @param {mat3} out mat3 receiving operation result
 * @param {vec2} v Translation vector
 */
void mat3_fromTranslation(float* dst, float* v);

/**
 * Creates a matrix from a given angle
 * This is equivalent to (but much faster than):
 *
 *     mat3_identity(dest);
 *     mat3_rotate(dest, dest, rad);
 *
 * @param {mat3} out mat3 receiving operation result
 * @param {Number} rad the angle to rotate the matrix by
 * @returns {mat3} out
 */
void mat3_fromRotation(float* dst, float rad);

/**
 * Creates a matrix from a vector scaling
 * This is equivalent to (but much faster than):
 *
 *     mat3_identity(dest);
 *     mat3_scale(dest, vec);
 *
 * @param {mat3} out mat3 receiving operation result
 * @param {vec2} v Scaling vector
 */
void mat3_fromScaling(float* dst, float* v);

/**
 * Copies the values from a mat2d into a mat3
 *
 * @param {mat3} out the receiving matrix
 * @param {mat2d} a the matrix to copy
 **/
void mat3_fromMat2d(float* dst, float* a);

/**
* Calculates a 3x3 matrix from the given quaternion
*
* @param {mat3} out mat3 receiving operation result
* @param {quat} q Quaternion to create matrix from
*/
void mat3_fromQuat(float* dst, float* q);

/**
* Calculates a 3x3 normal matrix (transpose inverse) from the 4x4 matrix
*
* @param {mat3} out mat3 receiving operation result
* @param {mat4} a Mat4 to derive the normal matrix from
*/
void mat3_normalFromMat4(float* dst, float* a);

/**
 * Generates a 2D projection matrix with the given bounds
 *
 * @param {mat3} out mat3 frustum matrix will be written into
 * @param {number} width Width of your gl context
 * @param {number} height Height of gl context
 */
void mat3_projection(float* dst, float width, float height);

/**
 * Returns Frobenius norm of a mat3
 *
 * @param {mat3} a the matrix to calculate Frobenius norm of
 * @returns {Number} Frobenius norm
 */
float mat3_frob(float* a);

/**
 * Adds two mat3's
 *
 * @param {mat3} out the receiving matrix
 * @param {mat3} b the second operand
 */
void mat3_add(float* dst, float* b);

/**
 * Subtracts matrix b from matrix a
 *
 * @param {mat3} out the receiving matrix
 * @param {mat3} b the second operand
 */
void mat3_subtract(float* dst, float* b);

/**
 * Multiply each element of the matrix by a scalar.
 *
 * @param {mat3} out the receiving matrix
 * @param {Number} b amount to scale the matrix's elements by
 */
void mat3_multiplyScalar(float* dst, float b);

/**
 * Adds two mat3's after multiplying each element of the second operand by a scalar value.
 *
 * @param {mat3} out the receiving vector
 * @param {mat3} b the second operand
 * @param {Number} scale the amount to scale b's elements by before adding
 */
void mat3_multiplyScalarAndAdd(float* dst, float* b, float scale);

/**
 * Returns whether or not the matrices have exactly the same elements.
 *
 * @param {mat3} a The first matrix.
 * @param {mat3} b The second matrix.
 * @returns {uint8_t} 1 if the matrices are equal, 0 otherwise.
 */
uint8_t mat3_equals(float* a, float* b);

#endif
#ifndef VEC3_H
#define VEC3_H

#include <stdint.h>

/**
 * Calculates the length of a vec3
 *
 * @param {vec3} a vector to calculate length of
 * @returns {Number} length of a
 */
float vec3_length(float* a);

/**
 * Copy the values from one vec3 to another
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} a the source vector
 */
void vec3_copy(float* dst, float* a);

/**
 * Set the components of a vec3 to the given values
 *
 * @param {vec3} out the receiving vector
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 */
void vec3_set(float* dst, float x, float y, float z);

/**
 * Adds two vec3's
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 */
void vec3_add(float* dst, float* b);

/**
 * Subtracts vector b from vector a
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 */
void vec3_subtract(float* dst, float* b);

/**
 * Multiplies two vec3's
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 */
void vec3_multiply(float* dst, float* b);

/**
 * Divides two vec3's
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 */
void vec3_divide(float* dst, float* b);

/**
 * Math.ceil the components of a vec3
 *
 * @param {vec3} out the receiving vector
 */
void vec3_ceil(float* dst);

/**
 * Math.floor the components of a vec3
 *
 * @param {vec3} out the receiving vector
 */
void vec3_floor(float* dst);

/**
 * Returns the minimum of two vec3's
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 */
void vec3_min(float* dst, float* b);

/**
 * Returns the maximum of two vec3's
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 */
void vec3_max(float* dst, float* b);

/**
 * Math.round the components of a vec3
 *
 * @param {vec3} out the receiving vector
 */
void vec3_round(float* dst);

/**
 * Scales a vec3 by a scalar number
 *
 * @param {vec3} out the receiving vector
 * @param {Number} b amount to scale the vector by
 */
void vec3_scale(float* dst, float b);

/**
 * Adds two vec3's after scaling the second operand by a scalar value
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 * @param {Number} scale the amount to scale b by before adding
 */
void vec3_scaleAndAdd(float* dst, float* b, float scale);

/**
 * Calculates the euclidian distance between two vec3's
 *
 * @param {vec3} a the first operand
 * @param {vec3} b the second operand
 * @returns {Number} distance between a and b
 */
float vec3_distance(float* a, float* b);

/**
 * Calculates the squared euclidian distance between two vec3's
 *
 * @param {vec3} a the first operand
 * @param {vec3} b the second operand
 * @returns {Number} squared distance between a and b
 */
float vec3_squaredDistance(float* a, float* b);

/**
 * Calculates the squared length of a vec3
 *
 * @param {vec3} a vector to calculate squared length of
 * @returns {Number} squared length of a
 */
float vec3_squaredLength(float* a);

/**
 * Negates the components of a vec3
 *
 * @param {vec3} out the receiving vector
 */
void vec3_negate(float* dst);

/**
 * Returns the inverse of the components of a vec3
 *
 * @param {vec3} out the receiving vector
 */
void vec3_inverse(float* dst);

/**
 * Normalize a vec3
 *
 * @param {vec3} out the receiving vector
 */
void vec3_normalize(float* dst);

/**
 * Calculates the dot product of two vec3's
 *
 * @param {vec3} a the first operand
 * @param {vec3} b the second operand
 * @returns {Number} dot product of a and b
 */
float vec3_dot(float* a, float* b);

/**
 * Computes the cross product of two vec3's
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 */
void vec3_cross(float* dst, float* b);

/**
 * Performs a linear interpolation between two vec3's
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 */
void vec3_lerp(float* dst, float* b, float t);

/**
 * Performs a hermite interpolation with two control points
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 * @param {vec3} c the third operand
 * @param {vec3} d the fourth operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 */
void vec3_hermite(float* dst, float* b, float* c, float* d, float t);

/**
 * Performs a bezier interpolation with two control points
 *
 * @param {vec3} out the receiving vector
 * @param {vec3} b the second operand
 * @param {vec3} c the third operand
 * @param {vec3} d the fourth operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 */
void vec3_bezier(float* dst, float* b, float* c, float* d, float t);

/**
 * Transforms the vec3 with a mat4.
 * 4th vector component is implicitly '1'
 *
 * @param {vec3} out the receiving vector
 * @param {mat4} m matrix to transform with
 */
void vec3_transformMat4(float* dst, float* m);

/**
 * Transforms the vec3 with a mat3.
 *
 * @param {vec3} out the receiving vector
 * @param {mat3} m the 3x3 matrix to transform with
 */
void vec3_transformMat3(float* dst, float* m);

/**
 * Transforms the vec3 with a quat
 * Can also be used for dual quaternions. (Multiply it with the real part)
 *
 * @param {vec3} out the receiving vector
 * @param {quat} q quaternion to transform with
 */
void vec3_transformQuat(float* dst, float* q);

/**
 * Rotate a 3D vector around the x-axis
 * @param {vec3} out The receiving vec3
 * @param {vec3} b The origin of the rotation
 * @param {Number} c The angle of rotation
 */
void vec3_rotateX(float* dst, float* b, float c);

/**
 * Rotate a 3D vector around the y-axis
 * @param {vec3} out The receiving vec3
 * @param {vec3} b The origin of the rotation
 * @param {Number} c The angle of rotation
 */
void vec3_rotateY(float* dst, float* b, float c);

/**
 * Rotate a 3D vector around the z-axis
 * @param {vec3} out The receiving vec3
 * @param {vec3} b The origin of the rotation
 * @param {Number} c The angle of rotation
 */
void vec3_rotateZ(float* dst, float* b, float c);

/**
 * Get the angle between two 3D vectors
 * @param {vec3} a The first operand
 * @param {vec3} b The second operand
 * @returns {Number} The angle in radians
 */
float vec3_angle(float* a, float* b);

/**
 * Returns whether or not the vectors have exactly the same elements
 *
 * @param {vec3} a The first vector.
 * @param {vec3} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
uint8_t vec3_equals(float* a, float* b);

#endif
#ifndef VEC2_H
#define VEC2_H

#include <stdint.h>

/**
 * Copy the values from one vec2 to another
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} a the source vector
 */
void vec2_copy(float* dst, float* a);

/**
 * Set the components of a vec2 to the given values
 *
 * @param {vec2} out the receiving vector
 * @param {Number} x X component
 * @param {Number} y Y component
 */
void vec2_set(float* dst, float x, float y);

/**
 * Adds two vec2's
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 */
void vec2_add(float* dst, float* b);

/**
 * Subtracts vector b from vector a
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 */
void vec2_subtract(float* dst, float* b);

/**
 * Multiplies two vec2's
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 */
void vec2_multiply(float* dst, float* b);

/**
 * Divides two vec2's
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 */
void vec2_divide(float* dst, float* b);

/**
 * ceilf the components of a vec2
 *
 * @param {vec2} out the receiving vector
 */
void vec2_ceil(float* dst);

/**
 * floorf the components of a vec2
 *
 * @param {vec2} out the receiving vector
 */
void vec2_floor(float* dst);

/**
 * Returns the minimum of two vec2's
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 */
void vec2_min(float* dst, float* b);

/**
 * Returns the maximum of two vec2's
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 */
void vec2_max(float* dst, float* b);

/**
 * roundf the components of a vec2
 *
 * @param {vec2} out the receiving vector
 */
void vec2_round(float* dst);

/**
 * Scales a vec2 by a scalar number
 *
 * @param {vec2} out the receiving vector
 * @param {Number} b amount to scale the vector by
 */
void vec2_scale(float* dst, float b);

/**
 * Adds two vec2's after scaling the second operand by a scalar value
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 * @param {Number} scale the amount to scale b by before adding
 */
void vec2_scaleAndAdd(float* dst, float* b, float scale);

/**
 * Calculates the euclidian distance between two vec2's
 *
 * @param {vec2} a the first operand
 * @param {vec2} b the second operand
 * @returns {Number} distance between a and b
 */
float vec2_distance(float* a, float* b);

/**
 * Calculates the squared euclidian distance between two vec2's
 *
 * @param {vec2} a the first operand
 * @param {vec2} b the second operand
 * @returns {Number} squared distance between a and b
 */
float vec2_squaredDistance(float* a, float* b);

/**
 * Calculates the length of a vec2
 *
 * @param {vec2} a vector to calculate length of
 * @returns {Number} length of a
 */
float vec2_length(float* a);

/**
 * Calculates the squared length of a vec2
 *
 * @param {vec2} a vector to calculate squared length of
 * @returns {Number} squared length of a
 */
float vec2_squaredLength(float* a);

/**
 * Negates the components of a vec2
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} a vector to negate
 * @returns {vec2} out
 */
void vec2_negate(float* dst);

/**
 * Returns the inverse of the components of a vec2
 *
 * @param {vec2} out the receiving vector
 */
void vec2_inverse(float* dst);

/**
 * Normalize a vec2
 *
 * @param {vec2} out the receiving vector
 */
void vec2_normalize(float* dst);

/**
 * Calculates the dot product of two vec2's
 *
 * @param {vec2} a the first operand
 * @param {vec2} b the second operand
 * @returns {Number} dot product of a and b
 */
float vec2_dot(float* a, float* b);

/**
 * Computes the cross product of two vec2's
 * Note that the cross product must by definition produce a 3D vector
 *
 * @param {vec3} out the receiving vector
 * @param {vec2} b the second operand
 */
void vec2_cross(float* dst, float* b);

/**
 * Performs a linear interpolation between two vec2's
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 */
void vec2_lerp(float* dst, float* b, float t);

/**
 * Transforms the vec2 with a mat2
 *
 * @param {vec2} out the receiving vector
 * @param {mat2} m matrix to transform with
 */
void vec2_transformMat2(float* dst, float* m);

/**
 * Transforms the vec2 with a mat2d
 *
 * @param {vec2} out the receiving vector
 * @param {mat2d} m matrix to transform with
 */
void vec2_transformMat2d(float* dst, float* m);

/**
 * Transforms the vec2 with a mat3
 * 3rd vector component is implicitly '1'
 *
 * @param {vec2} out the receiving vector
 * @param {mat3} m matrix to transform with
 */
void vec2_transformMat3(float* dst, float* m);

/**
 * Transforms the vec2 with a mat4
 * 3rd vector component is implicitly '0'
 * 4th vector component is implicitly '1'
 *
 * @param {vec2} out the receiving vector
 * @param {vec2} a the vector to transform
 * @param {mat4} m matrix to transform with
 */
void vec2_transformMat4(float* dst, float* m);

/**
 * Rotate a 2D vector
 * @param {vec2} out The receiving vec2
 * @param {vec2} b The origin of the rotation
 * @param {Number} c The angle of rotation
 */
void vec2_rotate(float* dst, float* b, float c);

/**
 * Get the angle between two 2D vectors
 * @param {vec2} a The first operand
 * @param {vec2} b The second operand
 * @returns {Number} The angle in radians
 */
float vec2_angle(float* a, float* b);

/**
 * Returns whether or not the vectors exactly have the same elements
 *
 * @param {vec2} a The first vector.
 * @param {vec2} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
uint8_t vec2_exactEquals(float* a, float* b);

#endif
#ifndef VEC4_H
#define VEC4_H

#include <stdint.h>

/**
 * Copy the values from one vec4 to another
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the source vector
 */
void vec4_copy(float* dst, float* a);

/**
 * Set the components of a vec4 to the given values
 *
 * @param {vec4} out the receiving vector
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 * @param {Number} w W component
 */
void vec4_set(float* dst, float x, float y, float z, float w);

/**
 * Adds two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 */
void vec4_add(float* dst, float* b);

/**
 * Subtracts vector b from vector a
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 */
void vec4_subtract(float* dst, float* b);

/**
 * Multiplies two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 */
void vec4_multiply(float* dst, float* b);

/**
 * Divides two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 */
void vec4_divide(float* dst, float* b);

/**
 * ceilf the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to ceil
 */
void vec4_ceil(float* dst);

/**
 * floorf the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to floor
 */
void vec4_floor(float* dst);

/**
 * Returns the minimum of two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 */
void vec4_min(float* dst, float* b);

/**
 * Returns the maximum of two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 */
void vec4_max(float* dst, float* b);

/**
 * roundf the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to round
 */
void vec4_round(float* dst);

/**
 * Scales a vec4 by a scalar number
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the vector to scale
 * @param {Number} b amount to scale the vector by
 */
void vec4_scale(float* dst, float b);

/**
 * Adds two vec4's after scaling the second operand by a scalar value
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} b the second operand
 * @param {Number} scale the amount to scale b by before adding
 */
void vec4_scaleAndAdd(float* dst, float* b, float scale);

/**
 * Calculates the euclidian distance between two vec4's
 *
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {Number} distance between a and b
 */
float vec4_distance(float* a, float* b);

/**
 * Calculates the squared euclidian distance between two vec4's
 *
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {Number} squared distance between a and b
 */
float vec4_squaredDistance(float* a, float* b);

/**
 * Calculates the length of a vec4
 *
 * @param {vec4} a vector to calculate length of
 * @returns {Number} length of a
 */
float vec4_length(float* a);

/**
 * Calculates the squared length of a vec4
 *
 * @param {vec4} a vector to calculate squared length of
 * @returns {Number} squared length of a
 */
float vec4_squaredLength(float* a);

/**
 * Negates the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to negate
 */
void vec4_negate(float* dst);

/**
 * Returns the inverse of the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to invert
 */
void vec4_inverse(float* dst);

/**
 * Normalize a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to normalize
 */
void vec4_normalize(float* dst);

/**
 * Calculates the dot product of two vec4's
 *
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {Number} dot product of a and b
 */
float vec4_dot(float* a, float* b);

/**
 * Performs a linear interpolation between two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 */
void vec4_lerp(float* dst, float* b, float t);

/**
 * Transforms the vec4 with a mat4.
 *
 * @param {vec4} out the receiving vector
 * @param {mat4} m matrix to transform with
 */
void vec4_transformMat4(float* dst, float* m);

/**
 * Transforms the vec4 with a quat
 *
 * @param {vec4} out the receiving vector
 * @param {quat} q quaternion to transform with
 */
void vec4_transformQuat(float* dst, float* q);

/**
 * Returns whether or not the vectors have exactly the same elements
 *
 * @param {vec4} a The first vector.
 * @param {vec4} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
uint8_t vec4_equals(float* a, float* b);

#endif
#ifndef QUAT_H
#define QUAT_H

#include <stdint.h>

/**
 * Set a quat to the identity quaternion
 *
 * @param {quat} out the receiving quaternion
 */
void quat_identity(float* dst);

/**
 * Sets a quat from the given angle and rotation axis,
 * then returns it.
 *
 * @param {quat} out the receiving quaternion
 * @param {vec3} axis the axis around which to rotate
 * @param {Number} rad the angle in radians
 **/
void quat_setAxisAngle(float* dst, float* axis, float rad);

/**
 * Gets the rotation axis and angle for a given
 *  quaternion. If a quaternion is created with
 *  setAxisAngle, this method will return the same
 *  values as providied in the original parameter list
 *  OR functionally equivalent values.
 * Example: The quaternion formed by axis [0, 0, 1] and
 *  angle -90 is the same as the quaternion formed by
 *  [0, 0, 1] and 270. This method favors the latter.
 * @param  {vec3} out_axis  Vector receiving the axis of rotation
 * @param  {quat} q     Quaternion to be decomposed
 * @return {Number}     Angle, in radians, of the rotation
 */
float quat_getAxisAngle(float* out_axis, float* q);

/**
 * Multiplies two quat's
 *
 * @param {quat} out the receiving quaternion
 * @param {quat} b the second operand
 */
void quat_multiply(float* dst, float* b);

/**
 * Rotates a quaternion by the given angle about the X axis
 *
 * @param {quat} out quat receiving operation result
 * @param {number} rad angle (in radians) to rotate
 */
void quat_rotateX(float* dst, float rad);

/**
 * Rotates a quaternion by the given angle about the Y axis
 *
 * @param {quat} out quat receiving operation result
 * @param {number} rad angle (in radians) to rotate
 */
void quat_rotateY(float* dst, float rad);

/**
 * Rotates a quaternion by the given angle about the Z axis
 *
 * @param {quat} out quat receiving operation result
 * @param {number} rad angle (in radians) to rotate
 */
void quat_rotateZ(float* dst, float rad);

/**
 * Calculates the W component of a quat from the X, Y, and Z components.
 * Assumes that quaternion is 1 unit in length.
 * Any existing W component will be ignored.
 *
 * @param {quat} out the receiving quaternion
 */
void quat_calculateW(float* dst);

/**
 * Performs a spherical linear interpolation between two quat
 *
 * @param {quat} out the receiving quaternion
 * @param {quat} b the second operand
 * @param {Number} t interpolation amount, in the range [0-1], between the two inputs
 */
void quat_slerp(float* dst, float* b, float t);

/**
 * Calculates the inverse of a quat
 *
 * @param {quat} out the receiving quaternion
 */
void quat_invert(float* dst);

/**
 * Calculates the conjugate of a quat
 * If the quaternion is normalized, this function is faster than quat.inverse and produces the same result.
 *
 * @param {quat} out the receiving quaternion
 * @param {quat} a quat to calculate conjugate of
 */
void quat_conjugate(float* dst);

/**
 * Creates a quaternion from the given 3x3 rotation matrix.
 *
 * NOTE: The resultant quaternion is not normalized, so you should be sure
 * to renormalize the quaternion yourself where necessary.
 *
 * @param {quat} out the receiving quaternion
 * @param {mat3} m rotation matrix
 */
void quat_fromMat3(float* dst, float* m);

/**
 * Creates a quaternion from the given euler angle x, y, z.
 *
 * @param {quat} out the receiving quaternion
 * @param {x} Angle to rotate around X axis in degrees.
 * @param {y} Angle to rotate around Y axis in degrees.
 * @param {z} Angle to rotate around Z axis in degrees.
 */
void quat_fromEuler(float* dst, float x, float y, float z);

#endif
