#ifndef PARTICLES_H
#define PARTICLES_H

#include <stdbool.h>
#include <GL/glew.h>
#include "../math/vec3.h"
#include "../math/mat4.h"

// Maximum particles per emitter
#define MAX_PARTICLES 256
#define MAX_EFFECT_NAME 32

// Single particle
typedef struct {
    Vec3 position;
    Vec3 velocity;
    float lifetime;      // Remaining life in seconds
    float max_lifetime;  // Initial lifetime (for alpha calc)
    float size;          // Current size
    float alpha;         // Current alpha (computed from lifetime)
} Particle;

// Particle effect definition (loaded from JSON)
typedef struct {
    char name[MAX_EFFECT_NAME];  // Effect ID (e.g., "tire_smoke", "explosion")
    bool enabled;                // Global enable/disable

    // Visual properties
    Vec3 start_color;            // Color at spawn (beginning of life)
    Vec3 end_color;              // Color at death (end of life)
    float min_lifetime;
    float max_lifetime;
    float min_size;
    float max_size;
    float size_growth;
    float velocity_randomness;  // Horizontal spread
    Vec3 gravity;

    // Spawn properties
    float spawn_scatter;        // Position randomness (cloud effect)
    float min_vertical_vel;     // Vertical velocity range
    float max_vertical_vel;
    float min_alpha;            // Alpha range based on intensity
    float max_alpha;
    float spawn_height;         // Y offset from spawn point

    // Trigger settings (for automatic effects like tire smoke)
    float intensity;         // Base intensity multiplier
    float slip_threshold;    // Min slip to trigger (for tire effects)
    float min_velocity;      // Min vehicle speed to trigger
} ParticleEffect;

// Particle emitter - manages a pool of particles with an effect definition
typedef struct {
    Particle particles[MAX_PARTICLES];
    int active_count;

    // Effect definition (copied from loaded effect)
    ParticleEffect effect;
} ParticleEmitter;

// Particle renderer - shared rendering resources
typedef struct {
    GLuint shader_program;
    GLuint vao;
    GLuint vbo;

    // Uniform locations
    GLint u_view;
    GLint u_projection;
    GLint u_camera_right;
    GLint u_camera_up;

    bool valid;
} ParticleRenderer;

// Initialize particle renderer (call once)
bool particle_renderer_init(ParticleRenderer* r);
void particle_renderer_destroy(ParticleRenderer* r);

// Load all particle effects from JSON file
// Call once at startup, stores effects in internal library
bool particle_effects_load(const char* filepath);

// Initialize emitter with a named effect from the library
// Returns false if effect not found
bool particle_emitter_init(ParticleEmitter* e, const char* effect_name);

// Legacy: Initialize an emitter with hardcoded smoke settings
void particle_emitter_init_smoke(ParticleEmitter* e);

// Spawn a particle at position with intensity multiplier
void particle_emitter_spawn(ParticleEmitter* e, Vec3 position, float intensity);

// Update all particles (call once per frame with delta time)
void particle_emitter_update(ParticleEmitter* e, float dt);

// Clear all particles
void particle_emitter_clear(ParticleEmitter* e);

// Render all particles from an emitter
void particle_renderer_draw(ParticleRenderer* r, ParticleEmitter* e,
                            Mat4* view, Mat4* projection, Vec3 camera_pos);

// Get a loaded effect by name (for reading trigger settings)
const ParticleEffect* particle_effect_get(const char* effect_name);

#endif // PARTICLES_H
