#pragma once

#include "app/cube_field.h"
#include "raylib.h"

#include <stdint.h>

namespace app {

struct Cube_View_Config {
    uint32_t focused_radius;
    uint32_t face_text_radius;
    uint32_t birds_eye_sample_stride;
};

struct Grid_Region {
    Grid_Coordinate minimum;
    Grid_Coordinate maximum;
};

// Representative cubes share one edge length and are exactly adjacent. Source
// coordinates remain separate from this sparse virtual geometry.
struct Birds_Eye_Lattice {
    Grid_Dimensions sample_count;
    float representative_size;
    Vector3 center;
    Vector3 first_center;
    Vector3 bounds_min;
    Vector3 bounds_max;
};

enum Birds_Eye_Face_Flag {
    BIRDS_EYE_FACE_NONE = 0,
    BIRDS_EYE_FACE_NEGATIVE_X = 1 << 0,
    BIRDS_EYE_FACE_POSITIVE_X = 1 << 1,
    BIRDS_EYE_FACE_NEGATIVE_Y = 1 << 2,
    BIRDS_EYE_FACE_POSITIVE_Y = 1 << 3,
    BIRDS_EYE_FACE_NEGATIVE_Z = 1 << 4,
    BIRDS_EYE_FACE_POSITIVE_Z = 1 << 5
};

struct Birds_Eye_Sample {
    Grid_Coordinate lattice_coordinate;
    Grid_Coordinate source_coordinate;
    Cube_Handle source_cube;
    uint32_t outward_faces;
    Vector3 center;
    Vector3 bounds_min;
    Vector3 bounds_max;
};

struct Cube_Pick_Result {
    Cube_Handle cube;
    float distance;
};

// Clamps a selected cube's candidate box to field bounds without allocation.
Grid_Region cube_view_focused_region(const Cube_Field& field, Cube_Handle selected_cube,
                                     uint32_t radius);
// Applies the spherical filter after the inexpensive clamped box enumeration.
int cube_view_focused_accepts_coordinate(Grid_Coordinate selected_coordinate,
                                         Grid_Coordinate candidate_coordinate, uint32_t radius);

// Returns a sparse count that preserves both endpoints for every axis over one.
uint32_t cube_view_sample_count(uint32_t dimension, uint32_t stride);
// Evenly maps a sample coordinate onto [0, dimension-1], including endpoints.
uint32_t cube_view_source_coordinate(uint32_t sample, uint32_t sample_count, uint32_t dimension);
// Builds adjacent virtual cubes without allocating or storing per-sample data.
void cube_view_build_birds_eye_lattice(Birds_Eye_Lattice& lattice, const Cube_Field& field,
                                       const Cube_View_Config& config);
// Returns only faces directed away from the complete cube-of-cubes shell.
uint32_t cube_view_birds_eye_outward_faces(const Birds_Eye_Lattice& lattice,
                                           Grid_Coordinate sample_coordinate);
// Resolves virtual geometry and its exact immutable source identity together.
int cube_view_birds_eye_sample(Birds_Eye_Sample& sample, const Cube_Field& field,
                               const Birds_Eye_Lattice& lattice, Grid_Coordinate sample_coordinate);
// Dense shell ordinals enumerate every exterior representative exactly once.
// Drawing and picking resolve ordinals through this same source mapping.
uint32_t cube_view_birds_eye_shell_sample_count(const Birds_Eye_Lattice& lattice);
// Invalid ordinals return zero and clear the output sample.
int cube_view_birds_eye_shell_sample(Birds_Eye_Sample& sample, const Cube_Field& field,
                                     const Birds_Eye_Lattice& lattice, uint32_t shell_index);

// Picking tests the same bounded Euclidean candidates used by focused drawing.
Cube_Pick_Result cube_view_pick_focused(const Cube_Field& field, Cube_Handle selected_cube,
                                        uint32_t radius, Ray ray);
// Picking resolves the same exterior sample and source handle used by drawing.
Cube_Pick_Result cube_view_pick_birds_eye(const Cube_Field& field, const Birds_Eye_Lattice& lattice,
                                          Ray ray);

} // namespace app
