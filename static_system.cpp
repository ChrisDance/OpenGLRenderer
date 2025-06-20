#include "static_system.hpp"
#include "Input.hpp"
#include "aabb_renderer.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "model_loader.hpp"
#include "model_setup.hpp"
#include "primitive_factory.hpp"
#include "resource_ids.hpp"
#include "text_renderer.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// === Data Structures ===
struct StaticMeta {
    unsigned int asset;
    unsigned int scale;
};

struct AABBData {
    glm::mat4 transform;
};

struct ParsedEntity {
    unsigned int asset;
    unsigned int scale;
    glm::mat4 transform;
};

enum Dimension { X, Y, Z };
enum Mode { TRANSLATION, ROTATION, META, AABB_TRANSLATE, AABB_ROTATE, AABB_SCALE };

// === Global State ===
static std::vector<entt::entity> static_entities;
static std::vector<StaticMeta> static_meta;
static std::vector<AABBData> aabbs;

static size_t selected_entity_index = 0;
static size_t selected_aabb_index = 0;
static uint current_add_entity_index = 1;

static Mode current_mode = AABB_TRANSLATE;
static Dimension current_dimension = X;

static AABBRenderer* renderer = nullptr;

// === File I/O Functions ===
static std::string serialize_entities() {
    std::stringstream ss;

    for (size_t i = 0; i < static_meta.size(); i++) {
        const Model& model = ecs.get<Model>(static_entities[i]);
        const glm::mat4& transform = model.transform;

        // Write asset and scale
        ss << static_meta[i].asset << "," << static_meta[i].scale << ",";

        // Write transform matrix (16 floats in column-major order)
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                ss << transform[col][row];
                if (col < 3 || row < 3) ss << ",";
            }
        }

        if (i < static_meta.size() - 1) ss << "\n";
    }

    return ss.str();
}

static std::string serialize_aabbs() {
    std::stringstream ss;

    for (size_t i = 0; i < aabbs.size(); i++) {
        const glm::mat4& transform = aabbs[i].transform;

        // Write transform matrix (16 floats in column-major order)
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                ss << transform[col][row];
                if (col < 3 || row < 3) ss << ",";
            }
        }

        if (i < aabbs.size() - 1) ss << "\n";
    }

    return ss.str();
}

static void save_entities(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << serialize_entities();
        file.close();
        std::cout << "Entities saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save entities to " << filename << std::endl;
    }
}

static void save_aabbs(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << serialize_aabbs();
        file.close();
        std::cout << "AABBs saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save AABBs to " << filename << std::endl;
    }
}

static ParsedEntity parse_entity_line(const std::string& line) {
    ParsedEntity entity;
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 18) {
        throw std::runtime_error("Invalid entity line: insufficient data");
    }

    entity.asset = static_cast<unsigned int>(std::stoul(tokens[0]));
    entity.scale = static_cast<unsigned int>(std::stoul(tokens[1]));

    // Parse transform matrix
    entity.transform = glm::mat4(1.0f);
    int idx = 2;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            entity.transform[col][row] = std::stof(tokens[idx++]);
        }
    }

    return entity;
}

static glm::mat4 parse_aabb_line(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 16) {
        throw std::runtime_error("Invalid AABB line: insufficient data");
    }

    glm::mat4 transform = glm::mat4(1.0f);
    int idx = 0;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            transform[col][row] = std::stof(tokens[idx++]);
        }
    }

    return transform;
}

static std::vector<ParsedEntity> load_entities(const std::string& filename) {
    std::vector<ParsedEntity> entities;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "Could not open entities file: " << filename << std::endl;
        return entities;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            try {
                entities.push_back(parse_entity_line(line));
            } catch (const std::exception& e) {
                std::cerr << "Parse error in line: " << line << " - " << e.what() << std::endl;
            }
        }
    }

    file.close();
    return entities;
}

static std::vector<glm::mat4> load_aabbs(const std::string& filename) {
    std::vector<glm::mat4> transforms;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "Could not open AABBs file: " << filename << std::endl;
        return transforms;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            try {
                transforms.push_back(parse_aabb_line(line));
            } catch (const std::exception& e) {
                std::cerr << "Parse error in AABB line: " << line << " - " << e.what() << std::endl;
            }
        }
    }

    file.close();
    return transforms;
}

// === Entity Management ===
static void create_entity(unsigned int asset_id, unsigned int scale) {
    StaticMeta meta = {.asset = asset_id, .scale = scale};
    Model model = load_model(resources::path(static_cast<resources::AssetId>(asset_id)));

    auto transform = model.aabb.getScaleToLengthTransform(1);
    model.transform = transform;

    setupModel(&model, 1);

    auto entity = ecs.create();
    ecs.emplace<Model>(entity, model);
    uploadData(&model, model.transform);

    static_entities.push_back(entity);
    static_meta.push_back(meta);
}

// === Input Handling ===
static void handle_mode_toggle() {
    if (Input::is_key_just_pressed(GLFW_KEY_M)) {
        switch (current_mode) {
            case TRANSLATION: current_mode = ROTATION; break;
            case ROTATION: current_mode = META; break;
            case META: current_mode = AABB_TRANSLATE; break;
            case AABB_TRANSLATE: current_mode = AABB_ROTATE; break;
            case AABB_ROTATE: current_mode = AABB_SCALE; break;
            case AABB_SCALE: current_mode = TRANSLATION; break;
        }
    }
}

static void handle_dimension_toggle() {
    if (Input::is_key_just_pressed(GLFW_KEY_L)) {
        switch (current_dimension) {
            case X: current_dimension = Y; break;
            case Y: current_dimension = Z; break;
            case Z: current_dimension = X; break;
        }
    }
}

static void handle_entity_selection() {
    if (Input::is_key_just_pressed(GLFW_KEY_N) && !static_entities.empty()) {
        selected_entity_index = (selected_entity_index + 1) % static_entities.size();
    }
}

static void handle_aabb_selection() {
    if (Input::is_key_just_pressed(GLFW_KEY_B) && !aabbs.empty()) {
        selected_aabb_index = (selected_aabb_index + 1) % aabbs.size();
    }
}

static void handle_transformations(float dt) {
    bool i_pressed = Input::is_key_just_pressed(GLFW_KEY_I);
    bool o_pressed = Input::is_key_just_pressed(GLFW_KEY_O);

    if (!i_pressed && !o_pressed) return;

    if (current_mode == TRANSLATION || current_mode == ROTATION) {
        if (static_entities.empty()) return;

        Model& model = ecs.get<Model>(static_entities[selected_entity_index]);
        glm::mat4 transform = model.transform;

        if (current_mode == TRANSLATION) {
            glm::vec3 movement(0.0f);
            float speed = 2.0f * dt;
            float direction = i_pressed ? -1.0f : 1.0f;

            switch (current_dimension) {
                case X: movement.x = direction * speed; break;
                case Y: movement.y = direction * speed; break;
                case Z: movement.z = direction * speed; break;
            }

            transform = glm::translate(transform, movement);
        }
        else if (current_mode == ROTATION) {
            float angle = glm::radians((i_pressed ? -90.0f : 90.0f) * dt);
            glm::vec3 axis(0.0f);

            switch (current_dimension) {
                case X: axis = glm::vec3(1, 0, 0); break;
                case Y: axis = glm::vec3(0, 1, 0); break;
                case Z: axis = glm::vec3(0, 0, 1); break;
            }

            glm::vec3 position = glm::vec3(transform[3]);
            transform[3] = glm::vec4(0, 0, 0, 1);
            transform = glm::rotate(glm::mat4(1.0f), angle, axis) * transform;
            transform[3] = glm::vec4(position, 1.0f);
        }

        model.transform = transform;
        uploadData(&model, transform);
    }
    else if ((current_mode == AABB_TRANSLATE || current_mode == AABB_ROTATE || current_mode == AABB_SCALE) && !aabbs.empty()) {
        glm::mat4& transform = aabbs[selected_aabb_index].transform;

        if (current_mode == AABB_TRANSLATE) {
            glm::vec3 movement(0.0f);
            float speed = 2.0f * dt;
            float direction = i_pressed ? -1.0f : 1.0f;

            switch (current_dimension) {
                case X: movement.x = direction * speed; break;
                case Y: movement.y = direction * speed; break;
                case Z: movement.z = direction * speed; break;
            }

            transform = glm::translate(transform, movement);
        }
        else if (current_mode == AABB_ROTATE) {
            float angle = glm::radians((i_pressed ? -90.0f : 90.0f) * dt);
            glm::vec3 axis(0.0f);

            switch (current_dimension) {
                case X: axis = glm::vec3(1, 0, 0); break;
                case Y: axis = glm::vec3(0, 1, 0); break;
                case Z: axis = glm::vec3(0, 0, 1); break;
            }

            // Extract position, rotate around origin, restore position
            glm::vec3 position = glm::vec3(transform[3]);
            transform[3] = glm::vec4(0, 0, 0, 1);
            transform = glm::rotate(glm::mat4(1.0f), angle, axis) * transform;
            transform[3] = glm::vec4(position, 1.0f);
        }
        else if (current_mode == AABB_SCALE) {
            float scale_factor = i_pressed ? 0.9f : 1.1f;
            glm::vec3 scale_vec(1.0f);

            switch (current_dimension) {
                case X: scale_vec.x = scale_factor; break;
                case Y: scale_vec.y = scale_factor; break;
                case Z: scale_vec.z = scale_factor; break;
            }

            // Extract position, scale around origin, restore position
            glm::vec3 position = glm::vec3(transform[3]);
            transform[3] = glm::vec4(0, 0, 0, 1);
            transform = glm::scale(transform, scale_vec);
            transform[3] = glm::vec4(position, 1.0f);
        }
    }
}

static void handle_entity_creation() {
    if (Input::is_key_just_pressed(GLFW_KEY_J)) {
        current_add_entity_index++;
        if (resources::path(static_cast<resources::AssetId>(current_add_entity_index)) == nullptr) {
            current_add_entity_index = 1;
        }
    }

    if (Input::is_key_just_pressed(GLFW_KEY_K)) {
        create_entity(current_add_entity_index, 1);
    }
}

static void handle_aabb_management() {
    if (Input::is_key_just_pressed(GLFW_KEY_U)) {
        Camera& camera = entt::locator<Camera>::value();
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), camera.Position + camera.Front);
        aabbs.push_back({.transform = transform});
    }

    if (Input::is_key_just_pressed(GLFW_KEY_DELETE) && !aabbs.empty()) {
        aabbs.erase(aabbs.begin() + selected_aabb_index);
        if (selected_aabb_index >= aabbs.size() && !aabbs.empty()) {
            selected_aabb_index = aabbs.size() - 1;
        }
    }
}

static void handle_file_operations() {
    if (Input::is_key_just_pressed(GLFW_KEY_Y)) {
        save_entities("entities.txt");
        save_aabbs("aabbs.txt");
    }
}

// === Rendering ===
static void render_ui() {
    Camera& camera = entt::locator<Camera>::value();
    Meta& meta = entt::locator<Meta>::value();
    Shaders& shaders = entt::locator<Shaders>::value();

    int y = 20;
    const int line_height = 25;

    std::string mode_text = "Mode: ";
    switch (current_mode) {
        case TRANSLATION: mode_text += "Translation"; break;
        case ROTATION: mode_text += "Rotation"; break;
        case META: mode_text += "Meta"; break;
        case AABB_TRANSLATE: mode_text += "AABB Translate"; break;
        case AABB_ROTATE: mode_text += "AABB Rotate"; break;
        case AABB_SCALE: mode_text += "AABB Scale"; break;
    }

    std::string dimension_text = "Dimension: ";
    switch (current_dimension) {
        case X: dimension_text += "X"; break;
        case Y: dimension_text += "Y"; break;
        case Z: dimension_text += "Z"; break;
    }

    RenderText(shaders.TEXT, "M: Change mode", 20, y, 0.5f, glm::vec3(1));
    y += line_height;
    RenderText(shaders.TEXT, mode_text, 20, y, 0.5f, glm::vec3(1, 1, 0));
    y += line_height;
    RenderText(shaders.TEXT, "L: Change dimension", 20, y, 0.5f, glm::vec3(1));
    y += line_height;
    RenderText(shaders.TEXT, dimension_text, 20, y, 0.5f, glm::vec3(0, 1, 1));
    y += line_height;

    if (current_mode == TRANSLATION || current_mode == ROTATION) {
        RenderText(shaders.TEXT, "I & O: Transform", 20, y, 0.5f, glm::vec3(1));
        y += line_height;
        RenderText(shaders.TEXT, "N: Next entity", 20, y, 0.5f, glm::vec3(1));
        y += line_height;
        if (!static_entities.empty()) {
            std::string entity_text = "Entity: " + std::to_string(selected_entity_index + 1) +
                                    "/" + std::to_string(static_entities.size());
            RenderText(shaders.TEXT, entity_text, 20, y, 0.5f, glm::vec3(0, 1, 0));
            y += line_height;
        }
    }
    else if (current_mode == META) {
        RenderText(shaders.TEXT, "Y: Save scene", 20, y, 0.5f, glm::vec3(1));
        y += line_height;
        std::string add_text = "K: Add ";
        RenderText(shaders.TEXT, add_text  + resources::path(static_cast<resources::AssetId>(current_add_entity_index)), 20, y, 0.5f, glm::vec3(1));
        y += line_height;
        RenderText(shaders.TEXT, "J: Next add entity", 20, y, 0.5f, glm::vec3(1));
        y += line_height;
    }
    else if (current_mode == AABB_TRANSLATE || current_mode == AABB_ROTATE || current_mode == AABB_SCALE) {
        RenderText(shaders.TEXT, "U: Add AABB", 20, y, 0.5f, glm::vec3(1));
        y += line_height;
        RenderText(shaders.TEXT, "B: Next AABB", 20, y, 0.5f, glm::vec3(1));
        y += line_height;
        RenderText(shaders.TEXT, "Del: Remove AABB", 20, y, 0.5f, glm::vec3(1));
        y += line_height;

        std::string transform_text = "I & O: ";
        switch (current_mode) {
            case AABB_TRANSLATE: transform_text += "Move AABB"; break;
            case AABB_ROTATE: transform_text += "Rotate AABB"; break;
            case AABB_SCALE: transform_text += "Scale AABB"; break;
        }
        RenderText(shaders.TEXT, transform_text, 20, y, 0.5f, glm::vec3(1));
        y += line_height;

        if (!aabbs.empty()) {
            std::string aabb_text = "AABB: " + std::to_string(selected_aabb_index + 1) +
                                  "/" + std::to_string(aabbs.size());
            RenderText(shaders.TEXT, aabb_text, 20, y, 0.5f, glm::vec3(1, 0, 1));
            y += line_height;
        }
    }
}

static void render_scene() {
    Camera& camera = entt::locator<Camera>::value();
    Meta& meta = entt::locator<Meta>::value();

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.Zoom),
        meta.WindowDimensions.x / meta.WindowDimensions.y,
        0.1f, 100.0f
    );
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 vp = projection * view;

    // Render selected entity's AABB
    if (!static_entities.empty()) {
        const Model& model = ecs.get<Model>(static_entities[selected_entity_index]);
        renderer->drawAABB(model.aabb.transform(model.transform), vp);
    }

    // Render all AABBs
    for (size_t i = 0; i < aabbs.size(); i++) {
        glm::vec3 color = (i == selected_aabb_index) ? glm::vec3(1, 0, 0) : glm::vec3(1, 1, 0);
        renderer->drawAABB2(aabbs[i].transform, vp, color);
    }
}

// === Public Interface ===
void static_system_init() {
    renderer = new AABBRenderer();

    // Load entities
    auto loaded_entities = load_entities("entities.txt");
    for (const auto& entity : loaded_entities) {
        Model model = load_model(resources::path(static_cast<resources::AssetId>(entity.asset)));
        model.transform = entity.transform;
        setupModel(&model, 1);

        auto ent = ecs.create();
        ecs.emplace<Model>(ent, model);
        uploadData(&model, model.transform);

        static_entities.push_back(ent);
        StaticMeta meta = {.asset = entity.asset, .scale = entity.scale};
        static_meta.push_back(meta);
    }

    // Load AABBs
    auto loaded_aabb_transforms = load_aabbs("aabbs.txt");
        tCollidables colliders;
    for (const auto& transform : loaded_aabb_transforms) {
        aabbs.push_back({.transform = transform});
        colliders.aabbs.push_back(transform);
    }

    entt::locator<tCollidables>::emplace(colliders);


    // Create ground plane
    // Model plane = ModelFactory::createPlane();
    // plane.transform = glm::rotate(glm::mat4(1.0f), static_cast<float>(M_PI) * 1.5f, glm::vec3(1, 0, 0));
    // setupModel(&plane, 1);
    // auto plane_entity = ecs.create();
    // ecs.emplace<Model>(plane_entity, plane);
    // uploadData(&plane, plane.transform);

    std::cout << "Static system initialized - Entities: " << static_entities.size()
              << ", AABBs: " << aabbs.size() << std::endl;
}

void static_system_update(float dt) {
    handle_mode_toggle();
    handle_dimension_toggle();
    handle_entity_selection();
    handle_aabb_selection();
    handle_transformations(dt);
    handle_entity_creation();
    handle_aabb_management();
    handle_file_operations();

    render_scene();
    render_ui();
}
