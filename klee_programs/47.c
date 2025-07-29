#include <klee/klee.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#define EPSILON 1e-9

typedef struct vec_3d_
{
    float x;
    float y;
    float z;
} vec_3d;

typedef struct quaternion_
{
    union
    {
        float w;
        float q0;
    };
    union
    {
        vec_3d dual;
        struct
        {
            float q1, q2, q3;
        };
    };
} quaternion;

typedef struct euler_
{
    union
    {
        float roll;
        float bank;
    };
    union
    {
        float pitch;
        float elevation;
    };
    union
    {
        float yaw;
        float heading;
    };
} euler;

quaternion quat_from_euler(const euler *in_euler)
{
    quaternion out_quat;

    if (!in_euler)
    {
        return out_quat;
    }

    quaternion temp;

    float cy = cosf(in_euler->yaw * 0.5f);
    float sy = sinf(in_euler->yaw * 0.5f);
    float cp = cosf(in_euler->pitch * 0.5f);
    float sp = sinf(in_euler->pitch * 0.5f);
    float cr = cosf(in_euler->roll * 0.5f);
    float sr = sinf(in_euler->roll * 0.5f);

    temp.w = cr * cp * cy + sr * sp * sy;
    temp.q1 = sr * cp * cy - cr * sp * sy;
    temp.q2 = cr * sp * cy + sr * cp * sy;
    temp.q3 = cr * cp * sy - sr * sp * cy;

    return temp;
}

euler euler_from_quat(const quaternion *in_quat)
{
    euler out_euler;
    if (!in_quat)
    {
        return out_euler;
    }

    out_euler.roll = atan2f(
        2.f * (in_quat->w * in_quat->q1 + in_quat->q2 * in_quat->q3),
        1.f - 2.f * (in_quat->q1 * in_quat->q1 + in_quat->q2 * in_quat->q2));
    out_euler.pitch =
        asinf(2.f * (in_quat->w * in_quat->q2 + in_quat->q1 * in_quat->q3));
    out_euler.yaw = atan2f(
        2.f * (in_quat->w * in_quat->q3 + in_quat->q1 * in_quat->q2),
        1.f - 2.f * (in_quat->q2 * in_quat->q2 + in_quat->q3 * in_quat->q3));

    return out_euler;
}

quaternion quaternion_multiply(const quaternion *in_quat1,
                               const quaternion *in_quat2)
{
    quaternion out_quat;
    if (!in_quat1 || !in_quat2)
    {
        return out_quat;
    }

    out_quat.w = in_quat1->w * in_quat2->w - in_quat1->q1 * in_quat2->q1 -
                 in_quat1->q2 * in_quat2->q2 - in_quat1->q3 * in_quat2->q3;
    out_quat.q1 = in_quat1->w * in_quat2->q1 + in_quat1->q1 * in_quat2->w +
                  in_quat1->q2 * in_quat2->q3 - in_quat1->q3 * in_quat2->q2;
    out_quat.q2 = in_quat1->w * in_quat2->q2 - in_quat1->q1 * in_quat2->q3 +
                  in_quat1->q2 * in_quat2->w + in_quat1->q3 * in_quat2->q1;
    out_quat.q3 = in_quat1->w * in_quat2->q3 + in_quat1->q1 * in_quat2->q2 -
                  in_quat1->q2 * in_quat2->q1 + in_quat1->q3 * in_quat2->w;

    return out_quat;
}

int main()
{
    euler input_euler;
    klee_make_symbolic(&input_euler, sizeof(input_euler), "input_euler");
    
    // Constrain Euler angles to reasonable ranges
    klee_assume(input_euler.roll >= -3.14f && input_euler.roll <= 3.14f);
    klee_assume(input_euler.pitch >= -1.57f && input_euler.pitch <= 1.57f);
    klee_assume(input_euler.yaw >= -3.14f && input_euler.yaw <= 3.14f);
    
    // Convert Euler to quaternion
    quaternion quat = quat_from_euler(&input_euler);
    
    // Convert back to Euler
    euler output_euler = euler_from_quat(&quat);
    
    // Verify that the conversion is consistent (within epsilon)
    klee_assert(fabsf(input_euler.roll - output_euler.roll) < 0.1f);
    klee_assert(fabsf(input_euler.pitch - output_euler.pitch) < 0.1f);
    klee_assert(fabsf(input_euler.yaw - output_euler.yaw) < 0.1f);
    
    // Test quaternion multiplication
    quaternion quat1 = {0.7071f, 0.7071f, 0.f, 0.f};
    quaternion quat2 = quaternion_multiply(&quat1, &quat);
    
    // Verify quaternion multiplication properties
    klee_assert(quat2.w >= -1.0f && quat2.w <= 1.0f);
    klee_assert(quat2.q1 >= -1.0f && quat2.q1 <= 1.0f);
    klee_assert(quat2.q2 >= -1.0f && quat2.q2 <= 1.0f);
    klee_assert(quat2.q3 >= -1.0f && quat2.q3 <= 1.0f);
    
    return 0;
} 