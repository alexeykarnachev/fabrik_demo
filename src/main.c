#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <string.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024

#define MAX_N_BONES 4
#define MAX_N_JOINTS (MAX_N_BONES + 1)

#define MAX_N_FABRIK_STEPS 20
#define FABRIK_ERROR_MARGIN 0.01

static const Color BACKGROUND_COLOR = {20, 20, 20, 255};

static Camera2D CAMERA = {
    .offset = {0.5 * SCREEN_WIDTH, 0.5 * SCREEN_HEIGHT},
    .target = {0.0, 0.0},
    .rotation = 0.0,
    .zoom = 100.0,
};

typedef struct Arm {
    Vector2 start;
    Vector2 target;

    int n_bones;
    float bone_angles[MAX_N_BONES];
    float bone_lengths[MAX_N_BONES];
} Arm;

static Arm ARM;

void load(void) {
    // raylib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FABRIK Demo");
    SetTargetFPS(60);
    rlSetLineWidth(3.0);

    // arm
    ARM.start = (Vector2){1.0, -1.0};
    ARM.n_bones = 4;
    ARM.bone_angles[0] = -0.25 * PI;
    ARM.bone_angles[1] = -0.25 * PI;
    ARM.bone_angles[2] = -0.25 * PI;
    ARM.bone_angles[3] = -0.25 * PI;
    ARM.bone_lengths[0] = 0.9;
    ARM.bone_lengths[1] = 1.0;
    ARM.bone_lengths[2] = 1.5;
    ARM.bone_lengths[3] = 0.7;
}

int get_arm_joints(Arm *arm, Vector2 joints[MAX_N_JOINTS]) {
    joints[0] = arm->start;
    int n_joints = arm->n_bones + 1;

    float angle_total = 0.0;

    for (int i = 1; i < n_joints; ++i) {
        float length = arm->bone_lengths[i - 1];
        angle_total += arm->bone_angles[i - 1];

        Vector2 bone = {length, 0.0};
        bone = Vector2Rotate(bone, angle_total);
        bone = Vector2Add(bone, joints[i - 1]);

        joints[i] = bone;
    }

    return n_joints;
}

void draw_arm(Arm *arm) {
    static Vector2 joints[MAX_N_JOINTS];
    int n_joints = get_arm_joints(&ARM, joints);

    for (int i = 0; i < n_joints; ++i) {
        if (i > 0) DrawLineV(joints[i - 1], joints[i], RAYWHITE);
        DrawCircleV(joints[i], 0.1, RED);
    }
}

float fabrik_step(Arm *arm) {
    static Vector2 joints[MAX_N_JOINTS];
    int n_joints = get_arm_joints(&ARM, joints);

    float length_total = 0.0;
    for (int i = 0; i < arm->n_bones; ++i) {
        length_total += arm->bone_lengths[i];
    }

    float dist_to_target = Vector2Distance(arm->start, arm->target);
    if (dist_to_target >= length_total) {
        memset(arm->bone_angles, 0, sizeof(arm->bone_angles));

        Vector2 direction = Vector2Normalize(Vector2Subtract(arm->target, arm->start));
        arm->bone_angles[0] = Vector2Angle((Vector2){1.0, 0.0}, direction);

        return 0.0;
    } else {
        // backward
        joints[n_joints - 1] = arm->target;
        for (int i = n_joints - 2; i >= 0; --i) {
            float length = arm->bone_lengths[i];
            Vector2 direction = Vector2Normalize(Vector2Subtract(joints[i], joints[i + 1])
            );
            joints[i] = Vector2Add(joints[i + 1], Vector2Scale(direction, length));
        }

        // forward
        joints[0] = arm->start;
        for (int i = 1; i < n_joints; ++i) {
            float length = arm->bone_lengths[i - 1];
            Vector2 direction = Vector2Normalize(Vector2Subtract(joints[i], joints[i - 1])
            );
            joints[i] = Vector2Add(joints[i - 1], Vector2Scale(direction, length));
        }

        // update arm angles
        float angle_total = 0.0;
        for (int i = 1; i < n_joints; ++i) {
            Vector2 a = joints[i - 1];
            Vector2 b = joints[i];

            float angle = Vector2Angle((Vector2){1.0, 0.0}, Vector2Subtract(b, a));
            angle -= angle_total;
            angle_total += angle;

            arm->bone_angles[i - 1] = angle;
        }

        float error = Vector2Distance(joints[n_joints - 1], arm->target);
        return error;
    }
}

void update(void) {
    ARM.target = GetScreenToWorld2D(GetMousePosition(), CAMERA);

    for (int i = 0; i < MAX_N_FABRIK_STEPS; ++i) {
        float error = fabrik_step(&ARM);
        if (error < FABRIK_ERROR_MARGIN) break;
    }
}

void draw(void) {
    BeginDrawing();
    ClearBackground(BACKGROUND_COLOR);

    BeginMode2D(CAMERA);

    draw_arm(&ARM);

    EndMode2D();

    DrawFPS(10, 10);
    EndDrawing();
}

int main(void) {
    load();

    while (!WindowShouldClose()) {
        update();
        draw();
    }

    CloseWindow();
}
