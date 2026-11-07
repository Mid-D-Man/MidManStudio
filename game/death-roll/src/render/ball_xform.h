// The ball's draw transform: spin, then the cartoon squash and stretch, then the move that keeps the ball touching the ground.
// Kept apart from scene.c so tests/test_render.c can check the maths with raymath alone (no GL needed).
#ifndef DR_BALL_XFORM_H
#define DR_BALL_XFORM_H

#include "raylib.h"

// Scale by `along` in the direction `a` (unit length) and by 1 / sqrt(along) across it: the volume stays the same.
Matrix ball_xform_axis_scale(Vector3 a, float along);

// spin -> squash along `normal` (factor along_normal) -> stretch along `travel` (factor along_motion) -> translate to (bx, 0, 0),
// moved on by normal * lift, where lift = squash_anchor() * the ball's radius: that keeps the squashed ball touching the ground
// at the same point as the round one.
Matrix ball_xform(Quaternion spin, Vector3 normal, Vector3 travel, float along_normal, float along_motion, float lift, float bx);

#endif
