#include "app/cube_field.h"

#include <stdio.h>

static int expect_int(int actual, int expected, const char* label)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL %s: actual=%d expected=%d\n", label, actual, expected);
        return 1;
    }

    return 0;
}

static int expect_float(float actual, float expected, const char* label)
{
    float diff = actual - expected;
    if (diff < 0.0f) diff = -diff;

    if (diff > 0.0001f) {
        fprintf(stderr, "FAIL %s: actual=%f expected=%f\n", label, actual, expected);
        return 1;
    }

    return 0;
}

int main(void)
{
    int failures = 0;

    app::Cube_Field field = {};
    app::cube_field_init(field, 4, 3, 2, 1.0f, 5.0f);

    app::Cube_Handle first = app::cube_handle_from_coords(field, 0, 0, 0);
    app::Cube_Handle second = app::cube_handle_from_coords(field, 1, 0, 0);
    app::Cube_Handle z_next = app::cube_handle_from_coords(field, 0, 0, 1);

    failures += expect_int((int)first, 1, "first handle is one-based");
    failures += expect_int((int)second, 2, "x is innermost");
    failures += expect_int((int)z_next, 13, "z stride is x*y");
    failures += expect_int((int)field.stride_y, 4, "stored y stride");
    failures += expect_int((int)field.stride_z, 12, "stored z stride");

    app::Cube_Coords coords = app::cube_coords_from_handle(field, z_next);
    failures += expect_int((int)coords.x, 0, "coords x");
    failures += expect_int((int)coords.y, 0, "coords y");
    failures += expect_int((int)coords.z, 1, "coords z");

    Vector3 center0 = app::cube_center(field, 0, 0, 0);
    Vector3 center3 = app::cube_center(field, 3, 2, 1);

    failures += expect_float(center0.x, -7.5f, "even x min centered");
    failures += expect_float(center3.x,  7.5f, "even x max centered");
    failures += expect_float(center0.y, -5.0f, "odd y min centered");
    failures += expect_float(center3.y,  5.0f, "odd y max centered");
    failures += expect_float(center0.z, -2.5f, "even z min centered");
    failures += expect_float(center3.z,  2.5f, "even z max centered");

    app::Cube_Data empty_data = {};
    app::Cube_Field empty_field = {};
    failures += expect_int((int)app::cube_value_at(empty_data, app::CUBE_HANDLE_STUB), (int)app::CUBE_VALUE_NONE, "zero data returns stub value");
    failures += expect_int((int)app::cube_handle_from_world(empty_field, Vector3{0, 0, 0}), (int)app::CUBE_HANDLE_STUB, "zero field world lookup returns stub handle");

    return failures ? 1 : 0;
}
