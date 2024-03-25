#include "../../../include_all.h"


void EntityInspector()
{
    ImVec2 window_size = ImGui::GetWindowSize();

    selectedEntity = std::get<Entity*>(objectInInspector);

    if (selectedEntity == nullptr || !selectedEntity->initialized) return;

    selectedEntityPosition = selectedEntity->position;
    if (selectedEntity->isChild)
        selectedEntityPosition = selectedEntity->relativePosition;

    selectedEntityScale = selectedEntity->scale;

    std::string entityName;


        
    selectedEntityName = selectedEntity->name;


    ImGui::Text("Inspecting '");
    ImGui::SameLine();

    float textWidth = ImGui::CalcTextSize(entityName.c_str()).x + 10.0f;
    textWidth = std::max(textWidth, 100.0f);
    ImGui::SetNextItemWidth(textWidth);

    int name_size = 0;
    
    if (selectedEntity->name.empty())
        name_size = 10;
    else
        name_size = selectedEntity->name.size();
    
    char inputBuffer[255];
    size_t bufferSize = sizeof(inputBuffer);
    strncpy(inputBuffer, selectedEntityName.c_str(), bufferSize - 1);
    inputBuffer[bufferSize - 1] = '\0';

    if (ImGui::InputText("##Title Part 2", inputBuffer, sizeof(inputBuffer)))
        selectedEntity->name = inputBuffer;



    ImGui::SameLine();
    ImGui::Text("'");

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 30));
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    if (ImGui::CollapsingHeader("Entity Properties"))
    {
        ImGui::Indent(30.0f);

        ImGui::Text("Scale:");
        ImGui::Indent(20.0f);

        ImGui::Text("X:");
        ImGui::SameLine();
        if (ImGui::InputFloat("##ScaleX", &selectedEntityScale.x))
            selectedEntity->reloadRigidBody();
        
        ImGui::Text("Y:");
        ImGui::SameLine();
        if (ImGui::InputFloat("##ScaleY", &selectedEntityScale.y))
            selectedEntity->reloadRigidBody();
        
        ImGui::Text("Z:");
        ImGui::SameLine();
        if (ImGui::InputFloat("##ScaleZ", &selectedEntityScale.z))
            selectedEntity->reloadRigidBody();

        selectedEntity->scale = selectedEntityScale;
            
        ImGui::Unindent(20.0f);
        ImGui::Dummy(ImVec2(0.0f, 30.0f));

        ImGui::Text("Position:");
        ImGui::Indent(20.0f);

        ImGui::Text("X:");
        ImGui::SameLine();
        ImGui::InputFloat("##PositionX", &selectedEntityPosition.x);

        ImGui::Text("Y:");
        ImGui::SameLine();
        ImGui::InputFloat("##PositionY", &selectedEntityPosition.y);

        ImGui::Text("Z:");
        ImGui::SameLine();
        ImGui::InputFloat("##PositionZ", &selectedEntityPosition.z);

        ImGui::Unindent(20.0f);
        ImGui::Dummy(ImVec2(0.0f, 30.0f));

        if (selectedEntity->isChild)
        {
            selectedEntity->relativePosition = selectedEntityPosition;
        }
        else
        {
            if (selectedEntity->isDynamic && selectedEntity->calcPhysics)
                selectedEntity->applyForce(selectedEntityPosition);
            else
                selectedEntity->position = selectedEntityPosition;
        }

        ImGui::Text("Rotation:");
        ImGui::Indent(20.0f);

        ImGui::Text("X:");
        ImGui::SameLine();
        if (EntityRotationXInputModel)
        {
            if (ImGui::InputFloat("##RotationX", &selectedEntity->rotation.x, -180.0f, 180.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            {
                EntityRotationXInputModel = false;
                selectedEntity->reloadRigidBody();
            }
        }
        else
        {
            if (ImGui::SliderFloat("##RotationX", &selectedEntity->rotation.x, -180.0f, 180.0f, "%.3f"))
                selectedEntity->reloadRigidBody();

            EntityRotationXInputModel = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);
        }

        ImGui::Text("Y:");
        ImGui::SameLine();
        if (EntityRotationYInputModel)
        {
            if (ImGui::InputFloat("##RotationY", &selectedEntity->rotation.y, -180.0f, 180.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            {
                selectedEntity->reloadRigidBody();
                EntityRotationYInputModel = false;
            }
        }
        else
        {
            if (ImGui::SliderFloat("##RotationY", &selectedEntity->rotation.y, -180.0f, 180.0f, "%.3f"))
                selectedEntity->reloadRigidBody();

            EntityRotationYInputModel = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);
        }

        ImGui::Text("Z:");
        ImGui::SameLine();
        if (EntityRotationZInputModel)
        {
            if (ImGui::InputFloat("##RotationZ", &selectedEntity->rotation.z, -180.0f, 180.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
            {
                EntityRotationZInputModel = false;
                selectedEntity->reloadRigidBody();
            }
        }
        else
        {
            if (ImGui::SliderFloat("##RotationZ", &selectedEntity->rotation.z, -180.0f, 180.0f, "%.3f"))
                selectedEntity->reloadRigidBody();
                
            EntityRotationZInputModel = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);
        }
        
        selectedEntity->setRot(selectedEntity->rotation);
        
        ImGui::Unindent(20.0f);
        ImGui::Dummy(ImVec2(0.0f, 30.0f));

        const float margin = 40.0f;
        const float LODWidth = ImGui::CalcTextSize("Level of Detail: ").x + margin;


        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 30));
        ImGui::Text("Model: ");
        ImGui::SameLine(LODWidth);

        if (ImGui::Button(
            ("##Drag'nDropModelPath"),
            ImVec2(200, 25)
            ));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_PAYLOAD"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int payload_n = *(const int*)payload->Data;

                std::string path = dirPath.string();
                path += "/" + filesTextureStruct[payload_n].name;

                selectedEntity->model_path = path;
                selectedEntityModelPathIndex = 0;
                selectedEntity->model_path = selectedEntity->model_path;

                selectedEntity->setModel(selectedEntity->model_path.c_str());
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopStyleVar();


        ImGui::Text("Script: ");
        ImGui::SameLine(LODWidth);

        const char* scriptName = selectedEntity->script.c_str();
        if (selectedEntity->script.empty()) scriptName = "##ScriptPath";
        
        ImGui::Button(scriptName, ImVec2(200,25));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCRIPT_PAYLOAD"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int payload_n = *(const int*)payload->Data;

                std::string path = dirPath.string();
                path += "/" + filesTextureStruct[payload_n].name;

                selectedEntityScriptPath = path;
                selectedEntityScriptPathIndex = 0;
                selectedEntity->script = selectedEntityScriptPath;
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();

        if (ImGui::Button("x##ScriptEmptyButton", ImVec2(25, 25)))
        {
            selectedEntity->script = "";
            selectedEntity->scriptIndex = "NONE";
        }

        ImGui::Dummy(ImVec2(0.0f, 30.0f));

        ImGui::Text("Collisions: ");
        ImGui::SameLine(LODWidth);
        ImGui::Checkbox("##Collisions", &selectedEntity->collider);

        ImGui::Text("Visible: ");
        ImGui::SameLine(LODWidth);
        ImGui::Checkbox("##Visible", &selectedEntity->visible);

        ImGui::Text("Level of Detail: ");
        ImGui::SameLine(LODWidth);
        ImGui::Checkbox("##Lod", &selectedEntity->lodEnabled);

        ImGui::Unindent(30.0f);
    }




    ImGui::Dummy(ImVec2(0.0f, 5.0f));




    if (ImGui::CollapsingHeader("Materials"))
    {
        ImGui::Indent(30.0f);

        ImGui::Text("Material:");
        ImGui::SameLine();

        if (ImGui::Button(
            ("##Drag'nDropMaterialPath"),
            ImVec2(200, 25)
            ));

        if (ImGui::BeginDragDropTarget())
        {

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MATERIAL_PAYLOAD"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int payload_n = *(const int*)payload->Data;

                std::string path = dirPath.string();
                path += "/" + filesTextureStruct[payload_n].name;

                selectedEntity->surfaceMaterial_path = path;
                DeserializeMaterial(&selectedEntity->surfaceMaterial, selectedEntity->surfaceMaterial_path.string().c_str());
            }
            ImGui::EndDragDropTarget();
        }

        if (!selectedEntity->surfaceMaterial_path.empty() && selectedEntity->surfaceMaterial_path.has_filename())
        {
            MaterialInspector(&selectedEntity->surfaceMaterial, selectedEntity->surfaceMaterial_path.string());
        }

        ImGui::Unindent(30.0f);


    }


    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    if (ImGui::CollapsingHeader("Physics"))
    {
        ImGui::Indent(30.0f);

        ImGui::Text("Collision Type");
        ImGui::SameLine();

        const char* collisionShapeNames[] = {"Box", "HighPolyMesh", "None"};
        int currentItem = static_cast<int>(Entity::CollisionShapeType::None);

        if (selectedEntity->currentCollisionShapeType)
            currentItem = static_cast<int>(*selectedEntity->currentCollisionShapeType);

        const float comboWidth = ImGui::GetContentRegionAvail().x - 30.0f;

        if (ImGui::BeginCombo("##CollisionType", collisionShapeNames[currentItem]))
        {
            for (int i = 0; i < IM_ARRAYSIZE(collisionShapeNames); i++)
            {
                const bool isSelected = (currentItem == i);

                if (ImGui::Selectable(collisionShapeNames[i], isSelected))
                {
                    *selectedEntity->currentCollisionShapeType = static_cast<Entity::CollisionShapeType>(i);
                    selectedEntity->reloadRigidBody();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        const float sliderWidth = comboWidth;
        const float marginLeft  = 30.0f;

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Is Dynamic");
        ImGui::SameLine();

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (sliderWidth + marginLeft));

        if (ImGui::Checkbox("##doPhysics", &selectedEntity->isDynamic))
        {
            if (selectedEntity->isDynamic)
                selectedEntity->makePhysicsDynamic();
            else
                selectedEntity->makePhysicsStatic();
        }

        ImGui::Dummy(ImVec2(0.0f, 15.0f));

        ImGui::Text("Mass:");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (sliderWidth + marginLeft));
        ImGui::SetNextItemWidth(sliderWidth);
        if (ImGui::SliderFloat("##Mass", &selectedEntity->mass, 0.0f, 100.0f, "%.1f"))
        {
            selectedEntity->updateMass();
        }

        ImGui::Text("Friction:");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (sliderWidth + marginLeft));
        ImGui::SetNextItemWidth(sliderWidth);
        if (ImGui::SliderFloat("##Friction", &selectedEntity->friction, 0.0f, 10.0f, "%.1f"))
        {
            selectedEntity->setFriction(selectedEntity->friction);
        }

        ImGui::Text("Damping:");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (sliderWidth + marginLeft));
        ImGui::SetNextItemWidth(sliderWidth);
        if (ImGui::SliderFloat("##Damping", &selectedEntity->damping, 0.0f, 5.0f, "%.1f"))
        {
            selectedEntity->applyDamping(selectedEntity->damping);
        }

        ImGui::Unindent(30.0f);
    }



    ImGui::Dummy(ImVec2(0.0f, 5.0f));



    if (ImGui::CollapsingHeader("Advanced Settings"))
    {
        ImGui::Indent(30.0f);

        ImGui::TextWrapped("Warning!\n"
"These experimental features won't save or load.\n"
"Avoid Advanced Settings until the alpha state.");

        if (ImGui::Button("Set all children instanced"))
        {
            selectedEntity->makeChildrenInstances();
        }

        ImGui::Unindent(30.0f);
    }
}