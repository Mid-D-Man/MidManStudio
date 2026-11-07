// raymath as static inline functions, so this file needs nothing from libraylib: the headless tests link without raylib (the game's
// own build is unaffected, these are private copies).
#define RAYMATH_STATIC_INLINE
#include "ball_xform.h"
#include "squash.h"
#include "raymath.h"

Matrix ball_xform_axis_scale(Vector3 a, float along)
{
    // A symmetric matrix, so it does not matter whether raylib reads its Matrix rows or columns.
    const float across = squash_across(along), d = along - across;
    Matrix m = MatrixIdentity();
    m.m0 = across + d * a.x * a.x;  m.m5 = across + d * a.y * a.y;  m.m10 = across + d * a.z * a.z;
    m.m1 = m.m4 = d * a.x * a.y;    m.m2 = m.m8 = d * a.x * a.z;    m.m6 = m.m9 = d * a.y * a.z;
    return m;
}

Matrix ball_xform(Quaternion spin, Vector3 normal, Vector3 travel, float along_normal, float along_motion, float lift, float bx)
{
    Matrix m = QuaternionToMatrix(spin);
    m = MatrixMultiply(m, ball_xform_axis_scale(normal, along_normal));
    m = MatrixMultiply(m, ball_xform_axis_scale(travel, along_motion));
    return MatrixMultiply(m, MatrixTranslate(bx + normal.x * lift, normal.y * lift, normal.z * lift));
}
