#include "particles.h"
#include "../vendor/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// Effect Library - loaded from JSON
// ============================================================================

#define MAX_EFFECTS 32

static ParticleEffect s_effects[MAX_EFFECTS];
static int s_effect_count = 0;

bool particle_effects_load(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open particle effects file: %s\n", filepath);
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* data = (char*)malloc(size + 1);
    if (!data) {
        fclose(f);
        return false;
    }
    fread(data, 1, size, f);
    data[size] = '\0';
    fclose(f);

    cJSON* root = cJSON_Parse(data);
    free(data);

    if (!root) {
        fprintf(stderr, "Failed to parse particle effects JSON\n");
        return false;
    }

    s_effect_count = 0;

    // Iterate over all effects in the JSON object
    cJSON* effect_json = NULL;
    cJSON_ArrayForEach(effect_json, root) {
        if (s_effect_count >= MAX_EFFECTS) break;

        ParticleEffect* effect = &s_effects[s_effect_count];
        memset(effect, 0, sizeof(ParticleEffect));

        // Name is the key
        strncpy(effect->name, effect_json->string, MAX_EFFECT_NAME - 1);

        // Enabled (default true)
        cJSON* enabled = cJSON_GetObjectItem(effect_json, "enabled");
        effect->enabled = enabled ? cJSON_IsTrue(enabled) : true;

        // Intensity (default 1.0)
        cJSON* intensity = cJSON_GetObjectItem(effect_json, "intensity");
        effect->intensity = intensity ? (float)intensity->valuedouble : 1.0f;

        // Trigger settings
        cJSON* slip = cJSON_GetObjectItem(effect_json, "slip_threshold");
        effect->slip_threshold = slip ? (float)slip->valuedouble : 0.15f;

        cJSON* min_vel = cJSON_GetObjectItem(effect_json, "min_velocity");
        effect->min_velocity = min_vel ? (float)min_vel->valuedouble : 1.0f;

        // Colors (start_color/end_color for gradient, or just "color" for static)
        Vec3 default_color = vec3(0.6f, 0.6f, 0.6f);

        cJSON* start_col = cJSON_GetObjectItem(effect_json, "start_color");
        cJSON* end_col = cJSON_GetObjectItem(effect_json, "end_color");
        cJSON* color = cJSON_GetObjectItem(effect_json, "color");

        // Parse start_color (fallback to "color" or default)
        if (start_col && cJSON_IsArray(start_col) && cJSON_GetArraySize(start_col) >= 3) {
            effect->start_color.x = (float)cJSON_GetArrayItem(start_col, 0)->valuedouble;
            effect->start_color.y = (float)cJSON_GetArrayItem(start_col, 1)->valuedouble;
            effect->start_color.z = (float)cJSON_GetArrayItem(start_col, 2)->valuedouble;
        } else if (color && cJSON_IsArray(color) && cJSON_GetArraySize(color) >= 3) {
            effect->start_color.x = (float)cJSON_GetArrayItem(color, 0)->valuedouble;
            effect->start_color.y = (float)cJSON_GetArrayItem(color, 1)->valuedouble;
            effect->start_color.z = (float)cJSON_GetArrayItem(color, 2)->valuedouble;
        } else {
            effect->start_color = default_color;
        }

        // Parse end_color (fallback to start_color for no gradient)
        if (end_col && cJSON_IsArray(end_col) && cJSON_GetArraySize(end_col) >= 3) {
            effect->end_color.x = (float)cJSON_GetArrayItem(end_col, 0)->valuedouble;
            effect->end_color.y = (float)cJSON_GetArrayItem(end_col, 1)->valuedouble;
            effect->end_color.z = (float)cJSON_GetArrayItem(end_col, 2)->valuedouble;
        } else {
            effect->end_color = effect->start_color;  // No gradient by default
        }

        // Size [min, max]
        cJSON* size_arr = cJSON_GetObjectItem(effect_json, "size");
        if (size_arr && cJSON_IsArray(size_arr) && cJSON_GetArraySize(size_arr) >= 2) {
            effect->min_size = (float)cJSON_GetArrayItem(size_arr, 0)->valuedouble;
            effect->max_size = (float)cJSON_GetArrayItem(size_arr, 1)->valuedouble;
        } else {
            effect->min_size = 0.2f;
            effect->max_size = 0.4f;
        }

        // Lifetime [min, max]
        cJSON* life_arr = cJSON_GetObjectItem(effect_json, "lifetime");
        if (life_arr && cJSON_IsArray(life_arr) && cJSON_GetArraySize(life_arr) >= 2) {
            effect->min_lifetime = (float)cJSON_GetArrayItem(life_arr, 0)->valuedouble;
            effect->max_lifetime = (float)cJSON_GetArrayItem(life_arr, 1)->valuedouble;
        } else {
            effect->min_lifetime = 0.5f;
            effect->max_lifetime = 0.9f;
        }

        // Size growth
        cJSON* growth = cJSON_GetObjectItem(effect_json, "size_growth");
        effect->size_growth = growth ? (float)growth->valuedouble : 1.8f;

        // Spread (velocity randomness)
        cJSON* spread = cJSON_GetObjectItem(effect_json, "spread");
        effect->velocity_randomness = spread ? (float)spread->valuedouble : 0.8f;

        // Gravity
        cJSON* grav = cJSON_GetObjectItem(effect_json, "gravity");
        if (grav && cJSON_IsArray(grav) && cJSON_GetArraySize(grav) >= 3) {
            effect->gravity.x = (float)cJSON_GetArrayItem(grav, 0)->valuedouble;
            effect->gravity.y = (float)cJSON_GetArrayItem(grav, 1)->valuedouble;
            effect->gravity.z = (float)cJSON_GetArrayItem(grav, 2)->valuedouble;
        } else {
            effect->gravity = vec3(0.0f, 0.4f, 0.0f);
        }

        // Spawn scatter (position randomness)
        cJSON* scatter = cJSON_GetObjectItem(effect_json, "spawn_scatter");
        effect->spawn_scatter = scatter ? (float)scatter->valuedouble : 0.15f;

        // Vertical velocity [min, max]
        cJSON* vert_vel = cJSON_GetObjectItem(effect_json, "vertical_velocity");
        if (vert_vel && cJSON_IsArray(vert_vel) && cJSON_GetArraySize(vert_vel) >= 2) {
            effect->min_vertical_vel = (float)cJSON_GetArrayItem(vert_vel, 0)->valuedouble;
            effect->max_vertical_vel = (float)cJSON_GetArrayItem(vert_vel, 1)->valuedouble;
        } else {
            effect->min_vertical_vel = 0.2f;
            effect->max_vertical_vel = 0.6f;
        }

        // Alpha [min, max]
        cJSON* alpha_arr = cJSON_GetObjectItem(effect_json, "alpha");
        if (alpha_arr && cJSON_IsArray(alpha_arr) && cJSON_GetArraySize(alpha_arr) >= 2) {
            effect->min_alpha = (float)cJSON_GetArrayItem(alpha_arr, 0)->valuedouble;
            effect->max_alpha = (float)cJSON_GetArrayItem(alpha_arr, 1)->valuedouble;
        } else {
            effect->min_alpha = 0.5f;
            effect->max_alpha = 0.8f;
        }

        // Spawn height (Y offset)
        cJSON* height = cJSON_GetObjectItem(effect_json, "spawn_height");
        effect->spawn_height = height ? (float)height->valuedouble : 0.08f;

        printf("Loaded particle effect: %s\n", effect->name);
        s_effect_count++;
    }

    cJSON_Delete(root);
    printf("Loaded %d particle effects from %s\n", s_effect_count, filepath);
    return true;
}

const ParticleEffect* particle_effect_get(const char* effect_name) {
    for (int i = 0; i < s_effect_count; i++) {
        if (strcmp(s_effects[i].name, effect_name) == 0) {
            return &s_effects[i];
        }
    }
    return NULL;
}

bool particle_emitter_init(ParticleEmitter* e, const char* effect_name) {
    memset(e, 0, sizeof(ParticleEmitter));

    const ParticleEffect* effect = particle_effect_get(effect_name);
    if (!effect) {
        fprintf(stderr, "Particle effect not found: %s\n", effect_name);
        return false;
    }

    // Copy effect definition
    e->effect = *effect;
    return true;
}

// ============================================================================
// Core particle system
// ============================================================================

// Simple random float [0, 1]
static float randf(void) {
    return (float)rand() / (float)RAND_MAX;
}

// Random float in range [min, max]
static float randf_range(float min, float max) {
    return min + randf() * (max - min);
}

// Particle vertex shader - billboarded quads
static const char* particle_vert_src =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aOffset;\n"  // Corner offset (-1 to 1)
    "layout (location = 2) in float aSize;\n"
    "layout (location = 3) in float aAlpha;\n"
    "layout (location = 4) in vec3 aColor;\n"
    "out vec2 texCoord;\n"
    "out float alpha;\n"
    "out vec3 color;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform vec3 cameraRight;\n"
    "uniform vec3 cameraUp;\n"
    "void main() {\n"
    "    // Billboard: expand quad in camera space\n"
    "    vec3 worldPos = aPos + cameraRight * aOffset.x * aSize + cameraUp * aOffset.y * aSize;\n"
    "    gl_Position = projection * view * vec4(worldPos, 1.0);\n"
    "    texCoord = aOffset * 0.5 + 0.5;\n"  // Map -1..1 to 0..1
    "    alpha = aAlpha;\n"
    "    color = aColor;\n"
    "}\n";

// Particle fragment shader - soft circle with alpha fade
static const char* particle_frag_src =
    "#version 330 core\n"
    "in vec2 texCoord;\n"
    "in float alpha;\n"
    "in vec3 color;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    // Soft circle (distance from center)\n"
    "    vec2 center = texCoord - vec2(0.5);\n"
    "    float dist = length(center) * 2.0;\n"  // 0 at center, 1 at edge
    "    float softness = 1.0 - smoothstep(0.3, 1.0, dist);\n"
    "    if (softness <= 0.0) discard;\n"
    "    FragColor = vec4(color, alpha * softness);\n"
    "}\n";

// Vertex data per particle quad (6 vertices = 2 triangles)
// Each vertex: position(3) + offset(2) + size(1) + alpha(1) + color(3) = 10 floats
#define FLOATS_PER_VERTEX 10
#define VERTICES_PER_PARTICLE 6

static bool create_shader(GLuint* program, const char* vert, const char* frag) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vert, NULL);
    glCompileShader(vs);

    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(vs, 512, NULL, log);
        fprintf(stderr, "Particle vertex shader error: %s\n", log);
        return false;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &frag, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(fs, 512, NULL, log);
        fprintf(stderr, "Particle fragment shader error: %s\n", log);
        glDeleteShader(vs);
        return false;
    }

    *program = glCreateProgram();
    glAttachShader(*program, vs);
    glAttachShader(*program, fs);
    glLinkProgram(*program);

    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(*program, 512, NULL, log);
        fprintf(stderr, "Particle shader link error: %s\n", log);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

bool particle_renderer_init(ParticleRenderer* r) {
    memset(r, 0, sizeof(ParticleRenderer));

    if (!create_shader(&r->shader_program, particle_vert_src, particle_frag_src)) {
        return false;
    }

    r->u_view = glGetUniformLocation(r->shader_program, "view");
    r->u_projection = glGetUniformLocation(r->shader_program, "projection");
    r->u_camera_right = glGetUniformLocation(r->shader_program, "cameraRight");
    r->u_camera_up = glGetUniformLocation(r->shader_program, "cameraUp");

    // Create VAO and VBO for dynamic particle data
    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);

    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    // Allocate max buffer size
    size_t max_buffer_size = MAX_PARTICLES * VERTICES_PER_PARTICLE * FLOATS_PER_VERTEX * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, max_buffer_size, NULL, GL_DYNAMIC_DRAW);

    // Position (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Offset (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Size (1 float)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Alpha (1 float)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Color (3 floats)
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    r->valid = true;
    printf("Particle renderer initialized\n");
    return true;
}

void particle_renderer_destroy(ParticleRenderer* r) {
    if (r->valid) {
        glDeleteProgram(r->shader_program);
        glDeleteVertexArrays(1, &r->vao);
        glDeleteBuffers(1, &r->vbo);
        r->valid = false;
    }
}

void particle_emitter_init_smoke(ParticleEmitter* e) {
    memset(e, 0, sizeof(ParticleEmitter));

    // Try to load from JSON first
    if (particle_emitter_init(e, "tire_smoke")) {
        return;  // Success - using JSON-defined effect
    }

    // Fallback: hardcoded tire smoke settings
    strncpy(e->effect.name, "tire_smoke", MAX_EFFECT_NAME - 1);
    e->effect.enabled = true;
    e->effect.intensity = 1.0f;
    e->effect.slip_threshold = 0.15f;
    e->effect.min_velocity = 1.0f;
    e->effect.start_color = vec3(0.6f, 0.58f, 0.55f);
    e->effect.end_color = vec3(0.6f, 0.58f, 0.55f);  // Same color (no gradient)
    e->effect.min_lifetime = 0.5f;
    e->effect.max_lifetime = 0.9f;
    e->effect.min_size = 0.2f;
    e->effect.max_size = 0.4f;
    e->effect.size_growth = 1.8f;
    e->effect.velocity_randomness = 0.8f;
    e->effect.gravity = vec3(0.0f, 0.4f, 0.0f);
    e->effect.spawn_scatter = 0.15f;
    e->effect.min_vertical_vel = 0.2f;
    e->effect.max_vertical_vel = 0.6f;
    e->effect.min_alpha = 0.5f;
    e->effect.max_alpha = 0.8f;
    e->effect.spawn_height = 0.08f;
}

void particle_emitter_spawn(ParticleEmitter* e, Vec3 position, float intensity) {
    ParticleEffect* fx = &e->effect;

    // Spawn position with random offset (cloud effect)
    Vec3 spawn_pos = position;
    spawn_pos.x += randf_range(-fx->spawn_scatter, fx->spawn_scatter);
    spawn_pos.z += randf_range(-fx->spawn_scatter, fx->spawn_scatter);
    spawn_pos.y += randf_range(0.0f, fx->spawn_scatter * 0.67f);

    // Velocity: upward with horizontal spread
    Vec3 vel = vec3(
        randf_range(-fx->velocity_randomness, fx->velocity_randomness),
        randf_range(fx->min_vertical_vel, fx->max_vertical_vel),
        randf_range(-fx->velocity_randomness, fx->velocity_randomness)
    );

    float lifetime = randf_range(fx->min_lifetime, fx->max_lifetime);
    float size = randf_range(fx->min_size, fx->max_size) * (0.5f + intensity * 0.5f);
    float alpha = fx->min_alpha + intensity * (fx->max_alpha - fx->min_alpha);

    if (e->active_count >= MAX_PARTICLES) {
        // Find oldest particle to replace
        int oldest = 0;
        float min_life = e->particles[0].lifetime;
        for (int i = 1; i < MAX_PARTICLES; i++) {
            if (e->particles[i].lifetime < min_life) {
                min_life = e->particles[i].lifetime;
                oldest = i;
            }
        }
        Particle* p = &e->particles[oldest];
        p->position = spawn_pos;
        p->velocity = vel;
        p->max_lifetime = lifetime;
        p->lifetime = lifetime;
        p->size = size;
        p->alpha = alpha;
        return;
    }

    // Add new particle
    Particle* p = &e->particles[e->active_count++];
    p->position = spawn_pos;
    p->velocity = vel;
    p->max_lifetime = lifetime;
    p->lifetime = lifetime;
    p->size = size;
    p->alpha = alpha;
}

void particle_emitter_update(ParticleEmitter* e, float dt) {
    int i = 0;
    while (i < e->active_count) {
        Particle* p = &e->particles[i];

        p->lifetime -= dt;
        if (p->lifetime <= 0.0f) {
            // Remove by swapping with last
            e->particles[i] = e->particles[--e->active_count];
            continue;
        }

        // Update position
        p->position.x += p->velocity.x * dt;
        p->position.y += p->velocity.y * dt;
        p->position.z += p->velocity.z * dt;

        // Apply gravity (upward for smoke)
        p->velocity.x += e->effect.gravity.x * dt;
        p->velocity.y += e->effect.gravity.y * dt;
        p->velocity.z += e->effect.gravity.z * dt;

        // Compute current alpha based on lifetime
        float life_ratio = p->lifetime / p->max_lifetime;
        p->alpha = 0.6f * life_ratio;

        // Grow size over time
        float size_factor = 1.0f + (1.0f - life_ratio) * (e->effect.size_growth - 1.0f);
        // Size is updated based on initial size (stored implicitly)

        i++;
    }
}

void particle_emitter_clear(ParticleEmitter* e) {
    e->active_count = 0;
}

void particle_renderer_draw(ParticleRenderer* r, ParticleEmitter* e,
                            Mat4* view, Mat4* projection, Vec3 camera_pos) {
    if (!r->valid || e->active_count == 0) return;

    // Extract camera right and up from view matrix
    // View matrix columns are: [right, up, forward, pos] (transposed)
    Vec3 camera_right = vec3(view->m[0], view->m[4], view->m[8]);
    Vec3 camera_up = vec3(view->m[1], view->m[5], view->m[9]);

    // Build vertex data
    float* vertex_data = (float*)malloc(e->active_count * VERTICES_PER_PARTICLE * FLOATS_PER_VERTEX * sizeof(float));
    if (!vertex_data) return;

    // Quad corner offsets (2 triangles)
    static const float offsets[6][2] = {
        {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f},  // First triangle
        {-1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}   // Second triangle
    };

    int vertex_count = 0;
    for (int i = 0; i < e->active_count; i++) {
        Particle* p = &e->particles[i];

        // Compute size with growth
        float life_ratio = p->lifetime / p->max_lifetime;
        float size = p->size * (1.0f + (1.0f - life_ratio) * (e->effect.size_growth - 1.0f));

        // Interpolate color over lifetime (start -> end as life decreases)
        float t = 1.0f - life_ratio;  // 0 at spawn, 1 at death
        float r = e->effect.start_color.x + t * (e->effect.end_color.x - e->effect.start_color.x);
        float g = e->effect.start_color.y + t * (e->effect.end_color.y - e->effect.start_color.y);
        float b = e->effect.start_color.z + t * (e->effect.end_color.z - e->effect.start_color.z);

        for (int v = 0; v < 6; v++) {
            float* vert = &vertex_data[vertex_count * FLOATS_PER_VERTEX];

            // Position
            vert[0] = p->position.x;
            vert[1] = p->position.y;
            vert[2] = p->position.z;

            // Offset
            vert[3] = offsets[v][0];
            vert[4] = offsets[v][1];

            // Size
            vert[5] = size;

            // Alpha
            vert[6] = p->alpha;

            // Color (interpolated over lifetime)
            vert[7] = r;
            vert[8] = g;
            vert[9] = b;

            vertex_count++;
        }
    }

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * FLOATS_PER_VERTEX * sizeof(float), vertex_data);
    free(vertex_data);

    // Set up rendering state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  // Don't write to depth buffer

    glUseProgram(r->shader_program);
    glUniformMatrix4fv(r->u_view, 1, GL_FALSE, view->m);
    glUniformMatrix4fv(r->u_projection, 1, GL_FALSE, projection->m);
    glUniform3f(r->u_camera_right, camera_right.x, camera_right.y, camera_right.z);
    glUniform3f(r->u_camera_up, camera_up.x, camera_up.y, camera_up.z);

    glBindVertexArray(r->vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glBindVertexArray(0);

    // Restore state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUseProgram(0);
}
