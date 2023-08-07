#ifndef GAME_SHIPPING
    #include "../include_all.h"
#endif



float GetExtremeValue(const Vector3& a) {
    const float absX = abs(a.x);
    const float absY = abs(a.y);
    const float absZ = abs(a.z);

    return max(max(absX, absY), absZ);
}


string getFileExtension(string filePath)
{
    filesys::path pathObj(filePath);

    if (pathObj.has_extension()) {
        return pathObj.extension().string();
    }
    return "no file extension";
}





string read_file_to_string(const string& filename) {
    ifstream file(filename);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

py::scoped_interpreter guard{}; // Start interpreter






bool EntityRunScriptFirstTime = true;
bool Entity_already_registered = false;

std::mutex script_mutex;


class Entity {
public:
    bool initialized = false;
    string name = "Entity";
    Color color;
    float size = 1;
    Vector3 position = { 0, 0, 0 };
    Vector3 rotation = { 0, 0, 0 };
    Vector3 scale = { 1, 1, 1 };

    Vector3 relative_position = { 0, 0, 0 };
    Vector3 relative_rotation = { 0, 0, 0 };
    Vector3 relative_scale = { 1, 1, 1 };

    string script = "";
    string model_path = "";
    Model model;

    BoundingBox bounds;



    std::filesystem::path texture_path;
    Texture2D texture;

    std::filesystem::path normal_texture_path;
    Texture2D normal_texture;

    std::filesystem::path roughness_texture_path;
    Texture2D roughness_texture;


    SurfaceMaterial surface_material;


    bool collider = true;
    bool visible = true;
    bool isChild = false;
    bool isParent = false;
    bool running = false;

    int id = 0;

    Entity* parent = nullptr;
    vector<Entity*> children;


    Entity(Color color = { 255, 255, 255, 255 }, Vector3 scale = { 1, 1, 1 }, Vector3 rotation = { 0, 0, 0 }, string name = "entity", Vector3 position = {0, 0, 0}, string script = "")
        : color(color), scale(scale), rotation(rotation), name(name), position(position), script(script)
    {   
        initialized = true;
    }

    bool operator==(const Entity& other) const {
        return this->id == other.id;
    }


    void addChild(Entity& child) {
        Entity* newChild = new Entity(child);
        newChild->relative_position = Vector3Subtract(newChild->position, this->position);
        newChild->parent = selected_entity;
        printf("Parent x position: %f", newChild->parent->position.x);
        children.push_back(newChild);
    }

    void update_children()
    {
        if (children.empty()) return;
        for (Entity* child : children)
        {
            child->render();
            #ifndef GAME_SHIPPING
                if (child == selected_entity) return;
            #endif

            child->position = Vector3Add(this->position, child->relative_position);

            child->update_children();

        }
    }


    void remove() {

        for (Entity* child : children) {
            delete child;
        }
        children.clear();


        entities_list_pregame.erase(
            std::remove_if(entities_list_pregame.begin(), entities_list_pregame.end(),
                [this](const Entity& entity) {
                    return entity.id == this->id;
                }),
            entities_list_pregame.end());

    }

    void setColor(Color newColor) {
        color = newColor;
    }

    void setName(const string& newName) {
        name = newName;
    }

    string getName() const {
        return name;
    }

    void setScale(Vector3 newScale) {
        scale = newScale;
    }

    void initializeDefaultModel() {
        Mesh mesh = GenMeshCube(scale.x, scale.y, scale.z);
        model = LoadModelFromMesh(mesh);
    }

    void loadModel(const char* filename, const char* textureFilename = NULL) {
        model = LoadModel(filename);
    }

    void ReloadTextures()
    {
        if (IsTextureReady(this->texture))
            model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
        
        if (IsTextureReady(this->normal_texture))
        {
            model.materials[0].maps[MATERIAL_MAP_NORMAL].texture = normal_texture;
        }

        if (IsTextureReady(this->roughness_texture))
        {
            model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = roughness_texture;
        }
    }

    void setModel(const char* modelPath = "", Model entity_model = Model())
    {
        model_path = modelPath;
    
        if (modelPath == "")
            model = entity_model;
        else
            model = LoadModel(modelPath);
    
        model.materials[0].shader = shader;

        bounds = GetMeshBoundingBox(model.meshes[0]);
    }

    bool hasModel()
    {

        if (model.meshCount > 0)
            return true;
        else
            return false;
    }

    void setShader(Shader shader)
    {
        model.materials[0].shader = shader;
    }

    void runScript(std::reference_wrapper<Entity> entityRef, Camera3D* rendering_camera)
    {
        if (script.empty()) return;
        running = true;
        std::lock_guard<std::mutex> lock(script_mutex);
        
        py::gil_scoped_release release;
        py::gil_scoped_acquire acquire;

        if (!Entity_already_registered)
        {
            Entity_already_registered = true;
            py::module entity_module("entity_module");
            py::class_<Entity>(entity_module, "Entity")
                .def(py::init<>())
                .def_readwrite("name", &Entity::name, py::call_guard<py::gil_scoped_release>())
                .def_readwrite("position", &Entity::position, py::call_guard<py::gil_scoped_release>())
                .def_readwrite("scale", &Entity::scale, py::call_guard<py::gil_scoped_release>())
                .def_readwrite("rotation", &Entity::rotation, py::call_guard<py::gil_scoped_release>())
                .def_readwrite("color", &Entity::color, py::call_guard<py::gil_scoped_release>())
                .def_readwrite("visible", &Entity::visible, py::call_guard<py::gil_scoped_release>())
                .def_readwrite("id", &Entity::id, py::call_guard<py::gil_scoped_release>())
                .def_readwrite("collider", &Entity::collider, py::call_guard<py::gil_scoped_release>());
        }
        Entity& this_entity = entityRef.get();

        py::object entity_obj = py::cast(&this_entity);
        py::module input_module = py::module::import("input_module");
        py::module collisions_module = py::module::import("collisions_module");
        py::module camera_module = py::module::import("camera_module");
        py::module time_module = py::module::import("time_module");
        py::module color_module = py::module::import("color_module");
        py::module math_module = py::module::import("math_module");
        
        rendering_camera->position.x = 100;
        std::cout << "Camera position: " << camera.position.x << std::endl;

        thread_local py::dict locals = py::dict(
            "entity"_a = entity_obj,
            "IsMouseButtonPressed"_a = input_module.attr("IsMouseButtonPressed"),
            "IsKeyDown"_a = input_module.attr("IsKeyDown"),
            "KeyboardKey"_a = input_module.attr("KeyboardKey"),
            "raycast"_a = collisions_module.attr("raycast"),
            "Vector3"_a = collisions_module.attr("Vector3"),
            "Color"_a = color_module.attr("Color"),
            "time"_a = py::cast(&time_instance),
            "lerp"_a = math_module.attr("lerp"),
            "camera"_a = py::cast(rendering_camera)
        );


        try {
            pybind11::gil_scoped_acquire acquire;
            string script_content = read_file_to_string(script);
            

            py::module module("__main__");

            for (auto item : locals) {
                module.attr(item.first) = item.second;
            }

            py::eval<py::eval_statements>(script_content, module.attr("__dict__"));


            if (module.attr("__dict__").contains("update")) {
                py::object update_func = module.attr("update");


                script_mutex.unlock();
                
                /*
                We only want to call the update function every frame, but because the runScript()
                is being called in a new thread the update_func may be called more than once per
                frame. This solution will fix it.
                */

                float last_frame_count = 0;
                while (running) {
                    if (time_instance.dt - last_frame_count != 0) {
                        locals["time"] = py::cast(&time_instance);
                        update_func();
                        last_frame_count = time_instance.dt;
                    }
                }
            } else {
                std::cout << "The 'update' function is not defined in the script.\n";
                return 1;
            }

            py::gil_scoped_release release;
        } catch (const py::error_already_set& e) {
            py::print(e.what());
        }
    }




    void render() {
        if (!hasModel())
            initializeDefaultModel();

        update_children();

        // model.transform = MatrixScale(scale.x, scale.y, scale.z);

        if (visible)
        {

            PassSurfaceMaterials();

            glUseProgram((GLuint)shader.id);

            bool normalMapInit = IsTextureReady(this->normal_texture);
            glUniform1i(glGetUniformLocation((GLuint)shader.id, "normalMapInit"), normalMapInit);

            bool roughnessMapInit = IsTextureReady(this->roughness_texture);
            glUniform1i(glGetUniformLocation((GLuint)shader.id, "roughnessMapInit"), roughnessMapInit);

            DrawModelEx(model, position, rotation, GetExtremeValue(rotation), scale, color);

        }
    }

private:
    void PassSurfaceMaterials()
    {
        glGenBuffers(1, &surface_material_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, surface_material_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(SurfaceMaterial), &this->surface_material, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Bind the buffer to a specific binding point (for example, binding point 0)
        GLuint bindingPoint = 0;
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, surface_material_ubo);


        // Update the uniform buffer data
        glBindBuffer(GL_UNIFORM_BUFFER, surface_material_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SurfaceMaterial), &this->surface_material);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

    }
};


bool operator==(const Entity& e, const Entity* ptr) {
    return &e == ptr;
}

#ifndef GAME_SHIPPING
    void AddEntity(
        bool create_immediatly = false,
        bool is_create_entity_a_child = is_create_entity_a_child,
        const char* model_path = "assets/models/tree.obj",
        Model model = Model()
    )
    {
        const int POPUP_WIDTH = 600;
        const int POPUP_HEIGHT = 650;

        int popupX = GetScreenWidth() / 4.5;
        int popupY = (GetScreenHeight() - POPUP_HEIGHT) / 6;

        if (create || create_immediatly)
        {
            Color entity_color_raylib = {
                static_cast<unsigned char>(entity_create_color.x * 255),
                static_cast<unsigned char>(entity_create_color.y * 255),
                static_cast<unsigned char>(entity_create_color.z * 255),
                static_cast<unsigned char>(entity_create_color.w * 255)
            };

            Entity entity_create;
            entity_create.setColor(entity_color_raylib);
            entity_create.setScale(Vector3{entity_create_scale, entity_create_scale, entity_create_scale});
            entity_create.setName(name);
            entity_create.isChild = is_create_entity_a_child;
            entity_create.setModel(model_path, model);
            entity_create.setShader(shader);

            if (!entities_list_pregame.empty())
            {
                int id = entities_list_pregame.back().id + 1;
                entity_create.id = id;
            }
            else
                entity_create.id = "0";

            if (!is_create_entity_a_child)
            {
                entities_list_pregame.reserve(1);
                entities_list_pregame.emplace_back(entity_create);
            }
            else
            {
                if (selected_game_object_type == "entity")
                {
                    if (selected_entity->isChild)
                        selected_entity->addChild(entity_create);
                    else
                        entities_list_pregame.back().addChild(entity_create);
                }
            }

            selected_entity = &entity_create;

            int last_entity_index = entities_list_pregame.size() - 1;
            listViewExActive = last_entity_index;

            create = false;
            is_create_entity_a_child = false;
            canAddEntity = false;
        }
        else if (canAddEntity)
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.6f, 0.6f, 0.6f, 0.6f)); // light gray

            ImGui::Begin("Entities");

            ImVec2 size = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 50.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

            ImGui::SetCursorPosX(size.x / 2 - 250);
            ImGui::SetCursorPosY(size.y / 4);
            ImGui::Button("Entity Add Menu", ImVec2(500,100));

            ImGui::PopStyleColor(4);

            /* Scale Slider */
            ImGui::Text("Scale: ");
            ImGui::SameLine();
            ImGui::SliderFloat(" ", &entity_create_scale, 0.1f, 100.0f);

            /* Name Input */
            ImGui::InputText("##text_input_box", name, sizeof(name));
            
            /* Color Picker */
            ImGui::Text("Color: ");
            ImGui::SameLine();
            ImGui::ColorEdit4(" ", (float*)&entity_create_color, ImGuiColorEditFlags_NoInputs);

            /* Is Children */
            ImGui::Checkbox("Is Children: ", &is_create_entity_a_child);

            /* Create BTN */
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14, 0.37, 0.15, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18, 0.48, 0.19, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

            ImGui::SetCursorPosX(size.x / 2);
            ImGui::SetCursorPosY(size.y / 1.1);
            bool create_entity_btn = ImGui::Button("Create", ImVec2(200,50));
            if (create_entity_btn)
            {
                canAddEntity = false;
                create = true;
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();

            ImGui::End();
        }
    }
#endif
    

py::module entity_module("entity_module");







HitInfo raycast(Vector3 origin, Vector3 direction, bool debug=false, std::vector<Entity> ignore = {})
{
    std::lock_guard<std::mutex> lock(script_mutex);
    pybind11::gil_scoped_acquire acquire;

    HitInfo _hitInfo;
    _hitInfo.hit = false;

    Ray ray;
    ray.position = origin;
    ray.direction = direction;

    if (debug)
        DrawRay(ray, RED);


    Entity entity;

    for (int index = 0; index < entities_list.size(); index++)
    {
        entity = entities_list[index];

        if (std::find(ignore.begin(), ignore.end(), entity) != ignore.end())
            continue;

        if (!entity.collider)
            continue;

        float extreme_rotation = GetExtremeValue(entity.rotation);

        Matrix matScale = MatrixScale(entity.scale.x, entity.scale.y, entity.scale.z);
        Matrix matRotation = MatrixRotate(entity.rotation, extreme_rotation*DEG2RAD);
        Matrix matTranslation = MatrixTranslate(entity.position.x, entity.position.y, entity.position.z);

        Matrix modelMatrix = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
        RayCollision meshHitInfo = { 0 };

        for (int mesh_i = 0; mesh_i < entity.model.meshCount; mesh_i++)
        {
            meshHitInfo = GetRayCollisionMesh(ray, entity.model.meshes[mesh_i], modelMatrix);
            if (meshHitInfo.hit)
            {
                _hitInfo.hit = true;
                _hitInfo.distance = meshHitInfo.distance;
                _hitInfo.entity = &entity;
                _hitInfo.worldPoint = meshHitInfo.point;
                _hitInfo.worldNormal = meshHitInfo.normal;
               _hitInfo.hitColor = entity.color;

                return _hitInfo;
            }
        }
    }

    pybind11::gil_scoped_release release;
    
    return _hitInfo;
}