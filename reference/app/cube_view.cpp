#include "app/cube_view.h"

#include <float.h>
#include <limits.h>
#include <string.h>

namespace app {

static uint32_t axis_region_minimum(uint32_t coordinate, uint32_t radius) {
    if (coordinate > radius) {
        return coordinate - radius;
    }

    return 0u;
}

static uint32_t axis_region_maximum(uint32_t coordinate, uint32_t radius, uint32_t dimension) {
    if (dimension == 0u) {
        return 0u;
    }

    uint32_t field_maximum = dimension - 1u;
    if (radius < field_maximum - coordinate) {
        return coordinate + radius;
    }

    return field_maximum;
}

Grid_Region cube_view_focused_region(const Cube_Field& field, Cube_Handle selected_cube,
                                     uint32_t radius) {
    Grid_Region region = {};
    if (!cube_field_contains_handle(field, selected_cube)) {
        return region;
    }

    Grid_Coordinate selected_coordinate = cube_field_coordinate(field, selected_cube);
    region.minimum.x = axis_region_minimum(selected_coordinate.x, radius);
    region.minimum.y = axis_region_minimum(selected_coordinate.y, radius);
    region.minimum.z = axis_region_minimum(selected_coordinate.z, radius);
    region.maximum.x = axis_region_maximum(selected_coordinate.x, radius, field.dimensions.x);
    region.maximum.y = axis_region_maximum(selected_coordinate.y, radius, field.dimensions.y);
    region.maximum.z = axis_region_maximum(selected_coordinate.z, radius, field.dimensions.z);
    return region;
}

static uint64_t coordinate_difference_squared(uint32_t left, uint32_t right) {
    uint64_t difference = 0u;
    if (left >= right) {
        difference = (uint64_t)left - right;
    } else {
        difference = (uint64_t)right - left;
    }

    return difference * difference;
}

static uint64_t axis_side_count(uint64_t count) {
    if (count > 1u) {
        return 2u;
    }

    return 1u;
}

static uint64_t axis_interior_count(uint64_t count) {
    if (count > 2u) {
        return count - 2u;
    }

    return 0u;
}

int cube_view_focused_accepts_coordinate(Grid_Coordinate selected_coordinate,
                                         Grid_Coordinate candidate_coordinate, uint32_t radius) {
    uint64_t remaining_distance_squared = (uint64_t)radius * radius;
    uint64_t x_distance_squared =
        coordinate_difference_squared(selected_coordinate.x, candidate_coordinate.x);
    if (x_distance_squared > remaining_distance_squared) {
        return 0;
    }

    remaining_distance_squared -= x_distance_squared;
    uint64_t y_distance_squared =
        coordinate_difference_squared(selected_coordinate.y, candidate_coordinate.y);
    if (y_distance_squared > remaining_distance_squared) {
        return 0;
    }

    remaining_distance_squared -= y_distance_squared;
    uint64_t z_distance_squared =
        coordinate_difference_squared(selected_coordinate.z, candidate_coordinate.z);
    return z_distance_squared <= remaining_distance_squared;
}

uint32_t cube_view_sample_count(uint32_t dimension, uint32_t stride) {
    if (dimension <= 1u) {
        return dimension;
    }

    if (stride == 0u) {
        stride = 1u;
    }

    uint32_t sample_count = 1u + (dimension - 1u) / stride;
    if (sample_count < 2u) {
        sample_count = 2u;
    }

    return sample_count;
}

uint32_t cube_view_source_coordinate(uint32_t sample, uint32_t sample_count, uint32_t dimension) {
    if (dimension <= 1u || sample_count <= 1u) {
        return 0u;
    }

    if (sample >= sample_count) {
        sample = sample_count - 1u;
    }

    uint64_t interval_count = sample_count - 1u;
    uint64_t numerator = (uint64_t)sample * (dimension - 1u) + interval_count / 2u;
    return (uint32_t)(numerator / interval_count);
}

void cube_view_build_birds_eye_lattice(Birds_Eye_Lattice& lattice, const Cube_Field& field,
                                       const Cube_View_Config& config) {
    memset(&lattice, 0, sizeof(lattice));
    if (field.cube_count == 0u) {
        return;
    }

    uint32_t stride = config.birds_eye_sample_stride;
    if (stride == 0u) {
        stride = 1u;
    }

    lattice.sample_count.x = cube_view_sample_count(field.dimensions.x, stride);
    lattice.sample_count.y = cube_view_sample_count(field.dimensions.y, stride);
    lattice.sample_count.z = cube_view_sample_count(field.dimensions.z, stride);
    lattice.representative_size = (float)stride * field.spacing;
    lattice.center = cube_field_center(field);
    lattice.first_center.x = lattice.center.x - 0.5f * (float)(lattice.sample_count.x - 1u) *
                                                    lattice.representative_size;
    lattice.first_center.y = lattice.center.y - 0.5f * (float)(lattice.sample_count.y - 1u) *
                                                    lattice.representative_size;
    lattice.first_center.z = lattice.center.z - 0.5f * (float)(lattice.sample_count.z - 1u) *
                                                    lattice.representative_size;

    float half_representative_size = 0.5f * lattice.representative_size;
    lattice.bounds_min = Vector3{lattice.first_center.x - half_representative_size,
                                 lattice.first_center.y - half_representative_size,
                                 lattice.first_center.z - half_representative_size};
    lattice.bounds_max.x = lattice.first_center.x +
                           (float)(lattice.sample_count.x - 1u) * lattice.representative_size +
                           half_representative_size;
    lattice.bounds_max.y = lattice.first_center.y +
                           (float)(lattice.sample_count.y - 1u) * lattice.representative_size +
                           half_representative_size;
    lattice.bounds_max.z = lattice.first_center.z +
                           (float)(lattice.sample_count.z - 1u) * lattice.representative_size +
                           half_representative_size;
}

uint32_t cube_view_birds_eye_outward_faces(const Birds_Eye_Lattice& lattice,
                                           Grid_Coordinate sample_coordinate) {
    if (sample_coordinate.x >= lattice.sample_count.x ||
        sample_coordinate.y >= lattice.sample_count.y ||
        sample_coordinate.z >= lattice.sample_count.z) {
        return BIRDS_EYE_FACE_NONE;
    }

    uint32_t outward_faces = BIRDS_EYE_FACE_NONE;
    if (sample_coordinate.x == 0u) {
        outward_faces |= BIRDS_EYE_FACE_NEGATIVE_X;
    }
    if (sample_coordinate.x + 1u == lattice.sample_count.x) {
        outward_faces |= BIRDS_EYE_FACE_POSITIVE_X;
    }
    if (sample_coordinate.y == 0u) {
        outward_faces |= BIRDS_EYE_FACE_NEGATIVE_Y;
    }
    if (sample_coordinate.y + 1u == lattice.sample_count.y) {
        outward_faces |= BIRDS_EYE_FACE_POSITIVE_Y;
    }
    if (sample_coordinate.z == 0u) {
        outward_faces |= BIRDS_EYE_FACE_NEGATIVE_Z;
    }
    if (sample_coordinate.z + 1u == lattice.sample_count.z) {
        outward_faces |= BIRDS_EYE_FACE_POSITIVE_Z;
    }

    return outward_faces;
}

int cube_view_birds_eye_sample(Birds_Eye_Sample& sample, const Cube_Field& field,
                               const Birds_Eye_Lattice& lattice,
                               Grid_Coordinate sample_coordinate) {
    memset(&sample, 0, sizeof(sample));
    uint32_t outward_faces = cube_view_birds_eye_outward_faces(lattice, sample_coordinate);
    if (outward_faces == BIRDS_EYE_FACE_NONE || lattice.representative_size <= 0.0f) {
        return 0;
    }

    sample.lattice_coordinate = sample_coordinate;
    sample.source_coordinate.x = cube_view_source_coordinate(
        sample_coordinate.x, lattice.sample_count.x, field.dimensions.x);
    sample.source_coordinate.y = cube_view_source_coordinate(
        sample_coordinate.y, lattice.sample_count.y, field.dimensions.y);
    sample.source_coordinate.z = cube_view_source_coordinate(
        sample_coordinate.z, lattice.sample_count.z, field.dimensions.z);
    sample.source_cube = cube_field_handle(field, sample.source_coordinate);
    if (sample.source_cube == CUBE_HANDLE_NONE) {
        memset(&sample, 0, sizeof(sample));
        return 0;
    }

    sample.outward_faces = outward_faces;
    sample.center.x =
        lattice.first_center.x + (float)sample_coordinate.x * lattice.representative_size;
    sample.center.y =
        lattice.first_center.y + (float)sample_coordinate.y * lattice.representative_size;
    sample.center.z =
        lattice.first_center.z + (float)sample_coordinate.z * lattice.representative_size;
    float half_representative_size = 0.5f * lattice.representative_size;
    sample.bounds_min = Vector3{sample.center.x - half_representative_size,
                                sample.center.y - half_representative_size,
                                sample.center.z - half_representative_size};
    sample.bounds_max = Vector3{sample.center.x + half_representative_size,
                                sample.center.y + half_representative_size,
                                sample.center.z + half_representative_size};
    return 1;
}

uint32_t cube_view_birds_eye_shell_sample_count(const Birds_Eye_Lattice& lattice) {
    uint64_t count_x = lattice.sample_count.x;
    uint64_t count_y = lattice.sample_count.y;
    uint64_t count_z = lattice.sample_count.z;
    if (count_x == 0u || count_y == 0u || count_z == 0u) {
        return 0u;
    }

    uint64_t z_sides = axis_side_count(count_z);
    uint64_t y_sides = axis_side_count(count_y);
    uint64_t x_sides = axis_side_count(count_x);
    uint64_t interior_z = axis_interior_count(count_z);
    uint64_t interior_y = axis_interior_count(count_y);
    uint64_t shell_count = count_x * count_y * z_sides;
    shell_count += count_x * interior_z * y_sides;
    shell_count += interior_y * interior_z * x_sides;
    if (shell_count > UINT32_MAX) {
        return 0u;
    }

    return (uint32_t)shell_count;
}

int cube_view_birds_eye_shell_sample(Birds_Eye_Sample& sample, const Cube_Field& field,
                                     const Birds_Eye_Lattice& lattice, uint32_t shell_index) {
    memset(&sample, 0, sizeof(sample));
    uint32_t shell_count = cube_view_birds_eye_shell_sample_count(lattice);
    if (shell_index >= shell_count) {
        return 0;
    }

    uint64_t count_x = lattice.sample_count.x;
    uint64_t count_y = lattice.sample_count.y;
    uint64_t count_z = lattice.sample_count.z;
    uint64_t z_sides = axis_side_count(count_z);
    uint64_t y_sides = axis_side_count(count_y);
    uint64_t x_sides = axis_side_count(count_x);
    uint64_t interior_z = axis_interior_count(count_z);
    uint64_t interior_y = axis_interior_count(count_y);
    uint64_t index = shell_index;
    Grid_Coordinate sample_coordinate = {};

    uint64_t z_face_count = count_x * count_y * z_sides;
    if (index < z_face_count) {
        uint64_t face_size = count_x * count_y;
        uint64_t side = index / face_size;
        uint64_t face_index = index % face_size;
        sample_coordinate.x = (uint32_t)(face_index % count_x);
        sample_coordinate.y = (uint32_t)(face_index / count_x);
        if (side != 0u) {
            sample_coordinate.z = (uint32_t)(count_z - 1u);
        }
        return cube_view_birds_eye_sample(sample, field, lattice, sample_coordinate);
    }

    index -= z_face_count;
    uint64_t y_face_count = count_x * interior_z * y_sides;
    if (index < y_face_count) {
        uint64_t layer_size = count_x * y_sides;
        uint64_t layer_index = index % layer_size;
        sample_coordinate.z = (uint32_t)(index / layer_size + 1u);
        sample_coordinate.x = (uint32_t)(layer_index % count_x);
        if (layer_index / count_x != 0u) {
            sample_coordinate.y = (uint32_t)(count_y - 1u);
        }
        return cube_view_birds_eye_sample(sample, field, lattice, sample_coordinate);
    }

    index -= y_face_count;
    uint64_t layer_size = interior_y * x_sides;
    sample_coordinate.z = (uint32_t)(index / layer_size + 1u);
    uint64_t layer_index = index % layer_size;
    sample_coordinate.y = (uint32_t)(layer_index / x_sides + 1u);
    if (layer_index % x_sides != 0u) {
        sample_coordinate.x = (uint32_t)(count_x - 1u);
    }
    return cube_view_birds_eye_sample(sample, field, lattice, sample_coordinate);
}

static void test_pick_candidate(Cube_Pick_Result& result, float& nearest_distance, Ray ray,
                                Cube_Handle cube, Vector3 bounds_min, Vector3 bounds_max) {
    RayCollision collision = GetRayCollisionBox(ray, BoundingBox{bounds_min, bounds_max});
    if (collision.hit && collision.distance < nearest_distance) {
        result.cube = cube;
        result.distance = collision.distance;
        nearest_distance = collision.distance;
    }
}

Cube_Pick_Result cube_view_pick_focused(const Cube_Field& field, Cube_Handle selected_cube,
                                        uint32_t radius, Ray ray) {
    Cube_Pick_Result result = {};
    if (!cube_field_contains_handle(field, selected_cube)) {
        return result;
    }

    float nearest_distance = FLT_MAX;
    Grid_Coordinate selected_coordinate = cube_field_coordinate(field, selected_cube);
    Grid_Region region = cube_view_focused_region(field, selected_cube, radius);
    for (uint32_t z = region.minimum.z; z <= region.maximum.z; ++z) {
        for (uint32_t y = region.minimum.y; y <= region.maximum.y; ++y) {
            for (uint32_t x = region.minimum.x; x <= region.maximum.x; ++x) {
                Grid_Coordinate coordinate = {x, y, z};
                if (!cube_view_focused_accepts_coordinate(selected_coordinate, coordinate,
                                                          radius)) {
                    continue;
                }

                Cube_Handle cube = cube_field_handle(field, coordinate);
                Vector3 bounds_min = {};
                Vector3 bounds_max = {};
                cube_field_cube_bounds(field, cube, bounds_min, bounds_max);
                test_pick_candidate(result, nearest_distance, ray, cube, bounds_min, bounds_max);
            }
        }
    }

    return result;
}

Cube_Pick_Result cube_view_pick_birds_eye(const Cube_Field& field, const Birds_Eye_Lattice& lattice,
                                          Ray ray) {
    Cube_Pick_Result result = {};
    float nearest_distance = FLT_MAX;
    uint32_t shell_count = cube_view_birds_eye_shell_sample_count(lattice);
    for (uint32_t shell_index = 0u; shell_index < shell_count; ++shell_index) {
        Birds_Eye_Sample sample = {};
        if (!cube_view_birds_eye_shell_sample(sample, field, lattice, shell_index)) {
            continue;
        }

        test_pick_candidate(result, nearest_distance, ray, sample.source_cube, sample.bounds_min,
                            sample.bounds_max);
    }

    return result;
}

} // namespace app
