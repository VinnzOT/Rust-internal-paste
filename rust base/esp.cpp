#include "esp.h"
#include "../rust base/utils/memory.hpp"
#include "../rust base/game/offsets.hpp"
#include <cmath>
#include "Glob.hpp"
#include "aimbot.h"

std::chrono::time_point<std::chrono::steady_clock> m_StartTime;



void Start()
{
    m_StartTime = std::chrono::high_resolution_clock::now();
}

float GetDuration()
{
    std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - m_StartTime;
    return duration.count();
}

void RotatePoint(float* pointToRotate, float* centerPoint, float angle, float* ReturnTo)
{
    angle = (float)(angle * (M_PI / (float)180));

    float cosTheta = (float)cos(angle);
    float sinTheta = (float)sin(angle);

    ReturnTo[0] = cosTheta * (pointToRotate[0] - centerPoint[0]) - sinTheta * (pointToRotate[1] - centerPoint[1]);
    ReturnTo[1] = sinTheta * (pointToRotate[0] - centerPoint[0]) + cosTheta * (pointToRotate[1] - centerPoint[1]);

    ReturnTo[0] += centerPoint[0];
    ReturnTo[1] += centerPoint[1];
}

namespace esp
{
    void radar()
    {
        //if (options::radar)
        {
            float x = 200, y = 200;
            float resx = 200, resy = 200;
            float screenPos[2];
            float RotatedPoint[2];
            float centerx = x + (resx / 2), centery = 300;
            float centerWindowPos[2] = { centerx, centery };

            functions::fill_box({ x, y, resx, resy }, { 0.2f, 0.2f, 0.2f, 0.2f });

            for (int i = 0; i < variables::player_list.size(); i++)
            {
                rust::BasePlayer* player = variables::player_list.at(i);

                // if (player->is_knocked())
                  //   color = { 1,0,0,1 };
                 //if (player->get_model()->is_npc())
                   //  color = { 0.9, 0, 1, 1 };
                 //if (player->is_same_team(variables::local_player))
                   //  color = { 0, 1, 0, 1 };
                 //if (player->is_sleeping())
                   //      
                static int cases = 0;
                static float r = 1.00f, g = 0.00f, b = 1.00f;
                switch (cases)
                {
                case 0: { r -= 0.005f; if (r <= 0) cases += 1; break; }
                case 1: { g += 0.005f; b -= 0.005f; if (g >= 1) cases += 1; break; }
                case 2: { r += 0.005f; if (r >= 1) cases += 1; break; }
                case 3: { b += 0.005f; g -= 0.005f; if (b >= 1) cases = 0; break; }
                default: { r = 1.00f; g = 0.00f; b = 1.00f; break; }
                }
                Vector4 color = { 1,0,0,1 };

                screenPos[0] = variables::local_position.x - player->get_bone_position(BoneList::spine1).x;
                screenPos[1] = variables::local_position.z - player->get_bone_position(BoneList::spine1).z;
                screenPos[0] *= 1;
                screenPos[1] *= 1;
                screenPos[0] *= -1;
                screenPos[0] += centerx;
                screenPos[1] += centery;

                RotatePoint(screenPos, centerWindowPos, 360.0f - variables::local_player->get_input()->get_view_angles().y, RotatedPoint);

                if (RotatedPoint[0] < x) continue;
                if (RotatedPoint[1] < y) continue;
                if (RotatedPoint[0] > x + resx) continue;
                if (RotatedPoint[1] > y + resy) continue;

                functions::fill_box({ RotatedPoint[0] - 1, RotatedPoint[1] - 1, 4, 4 }, { 0,0,0,1 });
                functions::fill_box({ RotatedPoint[0], RotatedPoint[1], 2, 2 }, color);
            }
            functions::fill_box({ centerx, centery, 2, 2 }, { 1,0.5,0,1 });
        }
    }
	uintptr_t shader;
    void do_chams(rust::BasePlayer* player)
    {
 

            if (!variables::asset_bundle)
                variables::asset_bundle = functions::load_bundle_file("C:\\shitcoder\\assets.saph");

            auto multiMesh = player->get_model()->get_multimesh();
            auto ourmesh = variables::local_player->get_model()->get_multimesh();

            if (!multiMesh)
                return;

            auto renderers = functions::get2renderers(multiMesh);
            auto ourrenders = functions::get2renderers(ourmesh);

            if (!renderers)
                return;

            for (int i = 0; i < renderers->get_size(); i++)
            {
                auto renderer = renderers->get_value(i);

                if (!renderer)
                    return;

                const auto material = functions::get_material(renderer);
                if (!material)
                    return;

                if (!shader)
                    shader = functions::load_asset(variables::asset_bundle, "flatshader.shader", il2cpp::type_object("UnityEngine", "Shader")); // there are two flatshader.shader and shader.shader

                if (functions::get_shader(material) == shader)
                    return;

                functions::set_shader(material, shader);



            functions::set_color(material, "ColorBehind", { 255.0f, 0,0, 0.2 });
        }



    }
    void render_box(rust::BasePlayer* player) {
        //for (int i = 0; i < variables::player_list.size(); i++) {
          //  auto player = variables::player_list[i];

            if (player == variables::local_player)
                return;

            auto bounds = get_bounds(player, 2.f);
            float box_width = bounds.right - bounds.left;
            float box_height = bounds.bottom - bounds.top;
            auto half = (bounds.right - bounds.left) / 2;
            Vector2 HeadPos;
            Vector2 NeckPos;
            Vector2 PelvisPos;
            Vector2 LegLPos;
            Vector2 LegRPos;
          
            auto distancea = Math::Calc3D_Dist(variables::local_position, player->get_model()->get_position());

            if (distancea > 200.0f)
                return;

            if (bounds.onscreen) {
                float resp, gesp, besp;
                resp = 0.0f;
                gesp = 1.0f;
                besp = 0.2f;

                if (player->get_model()->is_npc()) {
                    resp = 0.5f;
                    gesp = 0.2f;
                    besp = 0.0f;
                }
                if (player->is_knocked()) {
                    resp = 1.0f;
                    gesp = 0.0f;
                    besp = 0.0f;
                }

                if (player->is_sleeping()) {
                    resp = 0.4f;
                    gesp = 0.0f;
                    besp = 1.0f;
                }
                Vector3 vis = player->get_model()->get_position();
                if (player->is_visible(vis)) {
                    resp = 0.3f;
                    gesp = 0.2f;
                    besp = 0.9f;
                }

             
            }

            //	functions::set_timescale(12.0f);

        }
    void render_name(rust::BasePlayer* player) {
        //for (int i = 0; i < variables::player_list.size(); i++) {
          //  auto player = variables::player_list[i];

            if (player == variables::local_player)
                return;

            auto bounds = get_bounds(player, 2.f);
            float box_width = bounds.right - bounds.left;
            float box_height = bounds.bottom - bounds.top;
            auto half = (bounds.right - bounds.left) / 2;
            Vector2 HeadPos;
            Vector2 NeckPos;
            Vector2 PelvisPos;
            Vector2 LegLPos;
            Vector2 LegRPos;

            auto distancea = Math::Calc3D_Dist(variables::local_position, player->get_model()->get_position());

      /*      if (distancea > 200.0f)
                return;*/

            if (bounds.onscreen) {
                float resp, gesp, besp;
                resp = 0.0f;
                gesp = 1.0f;
                besp = 0.2f;

                if (player->get_model()->is_npc()) {
                    resp = 0.5f;
                    gesp = 0.2f;
                    besp = 0.0f;
                }
                if (player->is_knocked()) {
                    resp = 1.0f;
                    gesp = 0.0f;
                    besp = 0.0f;
                }

                if (player->is_sleeping()) {
                    resp = 0.4f;
                    gesp = 0.0f;
                    besp = 1.0f;
                }
                Vector3 vis = player->get_model()->get_position();
                if (player->is_visible(vis)) {
                    resp = 0.3f;
                    gesp = 0.2f;
                    besp = 0.9f;
                }

                auto name = player->get_username();
                std::string wcs = functions::ws2s(name);
                //functions::outline(Vector4{
                //  bounds.left - 80.f, bounds.bottom + 5, 79.f + half * 2 + 80.f, 13
                //    }, wcs, Vector4(resp, gesp, besp, 1), true, 10);
                functions::label(Vector4{
                bounds.left - 80.f, bounds.top - 20, 79.f + half * 2 + 80.f, 13

                    }, wcs, {
                      1,
                      1,
                      1,
                      1
                    }, true, 10);
               

            }

            //	functions::set_timescale(12.0f);

        }
    void render_distance(rust::BasePlayer* player) {
       
       // for (int i = 0; i < variables::player_list.size(); i++) {
        //    auto player = variables::player_list[i];
           
            //auto Vbase = LI_MODULE(("GameAssembly.dll")).get<uintptr_t>(); //GetModuleHandleA(xorstr_("GameAssembly.dll"));

            //	functions::set_draw_color(Vector4(1, 1, 1, 1));

            //bool is_npc = false;
            //	if (model->is_npc())
            //	return;
            //	else
            //	is_npc = true;
            int espas;

            if (player == variables::local_player)
                return;

            auto bounds = get_bounds(player, 2.f);
            float box_width = bounds.right - bounds.left;
            float box_height = bounds.bottom - bounds.top;
            auto half = (bounds.right - bounds.left) / 2;
            Vector2 HeadPos;
            Vector2 NeckPos;
            Vector2 PelvisPos;
            Vector2 LegLPos;
            Vector2 LegRPos;

            auto distancea = Math::Calc3D_Dist(variables::local_position, player->get_model()->get_position());

            //if (distancea > 200.0f)
            //    return;

            if (bounds.onscreen) {

                std::string playerdistance = "" + std::to_string(int(distancea)) + "m ";

      /*          functions::outline(Vector4{
                  bounds.left - 80.f, bounds.bottom + 25, 79.f + half * 2 + 80.f, 13*/
                    //}, playerdistance, Vector4(0.5, 0, 1.2, 1), false, 10);
                functions::label(Vector4{
                  bounds.left - 80.f, bounds.bottom + 15, 79.f + half * 2 + 80.f, 13
                    }, playerdistance, {
                      1,
                      1,
                      1,
                      1
                    }, true, 10);

            }


       // }
    }   
    void render_Health(rust::BasePlayer* player) {
        if (player == variables::local_player)
            return;

        auto bounds = get_bounds(player, 3.f);
        float box_width = bounds.right - bounds.left;
        float box_height = bounds.bottom - bounds.top;
        auto half = (bounds.right - bounds.left) / 2;
        Vector2 HeadPos;
        Vector2 NeckPos;
        Vector2 PelvisPos;
        Vector2 LegLPos;
        Vector2 LegRPos;

        auto distancea = Math::Calc3D_Dist(variables::local_position, player->get_model()->get_position());


        if (bounds.onscreen) {

            auto health = Vector4(bounds.left - 5, bounds.top + (box_height - box_height * (player->get_health() / 100)), 3, box_height * (player->get_health() / 100));
            const auto health_bg = Vector4(bounds.left - 6, bounds.top, 5, box_height);

            auto health2 = Vector4(bounds.left, bounds.bottom, box_width * (player->get_health() / 100), 3);
            const auto health_bg2 = Vector4(bounds.left, bounds.bottom, box_width, 2);

            functions::fill_box(health_bg, Vector4(0, 0, 0, 1));
            //functions::fill_box(health_bg2, Vector4(0, 0, 0, 1));
            functions::fill_box(health, Vector4(0.2, 0.8, 0, 1));
            //functions::fill_box(health2, Vector4(0.2, 0.8, 0, 1));
        }
    }


    void render_weapon(rust::BasePlayer* player) {
        //for (int i = 0; i < variables::player_list.size(); i++) {
         //   auto player = variables::player_list[i];

            auto bounds = get_bounds(player, 2.f);
            float box_width = bounds.right - bounds.left;
            float box_height = bounds.bottom - bounds.bottom;
            auto half = (bounds.right - bounds.left) / 2;
            Vector2 HeadPos;
            Vector2 NeckPos;
            Vector2 PelvisPos;
            Vector2 LegLPos;
            Vector2 LegRPos;

            auto distancea = Math::Calc3D_Dist(variables::local_position, player->get_model()->get_position());

   /*         if (distancea > 400.0f)
                return;*/

            if (bounds.onscreen) {

                std::string wName = functions::ws2s(player->get_held_item()->get_name());
                //functions::outline(Vector4{
                //  bounds.left - 80.f, bounds.top - 20, 79.f + half * 2 + 80.f, 13
                //    }, wName, Vector4(0, 0.2, 1, 1), true, 10);
                functions::label(Vector4{
                  bounds.left - 80.f, bounds.bottom + 5, 79.f + half * 2 + 80.f, 13
                    }, wName, {
                      1,
                      1,
                      1,
                      1
                    }, true, 10);

            }

        }
    void render_aimbot(rust::BasePlayer* player, BoneList bone) {
        //for (int i = 0; i < variables::player_list.size(); i++) {
          //  auto player = variables::player_list[i];

            //auto Vbase = LI_MODULE(("GameAssembly.dll")).get<uintptr_t>(); //GetModuleHandleA(xorstr_("GameAssembly.dll"));

            //	functions::set_draw_color(Vector4(1, 1, 1, 1));

            //bool is_npc = false;
            //	if (model->is_npc())
            //	return;
            //	else
            //	is_npc = true;
        int espas;

        if (player == variables::local_player)
            return;

        //variables::local_player->get_movement()->fun();
        //variables::local_player->get_movement()->spiderman();

        auto bounds = get_bounds(player, 2.f);
        float box_width = bounds.right - bounds.left;
        float box_height = bounds.bottom - bounds.top;
        auto half = (bounds.right - bounds.left) / 2;
        Vector2 HeadPos;
        Vector2 NeckPos;
        Vector2 PelvisPos;
        Vector2 LegLPos;
        Vector2 LegRPos;
        //aimbot::getClosestBoneToCrosshair(player);
        //aimbot::getBestPlayerByFov();
        //aimbot::run();
        auto distancea = Math::Calc3D_Dist(variables::local_position, player->get_model()->get_position());

  /*      if (distancea > 100.0f)
            return;*/

        if (bounds.onscreen) {
            if (functions::get_key(KeyCode::Mouse1)) {
                aimbot::run(bone);
                espas = 1;
            }
            else {
                espas = 0;
            }

            if (espas == 1) {
                //functions::outline(Vector4(variables::screen_width / 2, variables::screen_height / 2 - 10, 150, 20), xorstr_("Locking on"), Vector4(0, 0.5, 1, 1));
                //functions::label(Vector4(variables::screen_width / 2, variables::screen_height / 2 - 10, 150, 20), xorstr_("Locking on"), Vector4(1, 1, 1, 1));
            }
        }
    }
    

    float flyhack_forgiveness_interia = 10.0f;
    float flyhack_forgiveness = 1.5f;
    float flyhackDistanceVertical = 110.f;
    float flyhack_extrusion = 1.85f;
    float flyhackDistanceHorizontal = 110.f;
    float flyhackPauseTime{};


    float VFlyhack = 0.0f;
    float HFlyhack = 0.0f;
    float VMaxFlyhack = 0.0f;
    float HMaxFlyhack = 0.0f;

    void FlyHack()
    {
        variables::local_player->get_movement()->flying();
    }

    void flyhackbar()
    {
        rust::BasePlayer* plly = variables::local_player;
        float flyhack_forgiveness_interia = 10.0f;
        float flyhack_forgiveness = 1.5f;
        float flyhack_extrusion = 1.85f;
        bool flag = false;
        bool isInAir = false;
        Vector3 vector = (plly->get_lastSentTick()->get_position() + plly->get_transform()->get_position()) * 0.5f;
        flyhackPauseTime = functions::mathf_max(0.f, flyhackPauseTime - functions::get_deltaTime());

        if (plly) {
            float radius = plly->get_radius();
            float height = plly->get_height(false);

            Vector3 vector2 = vector + Vector3(0.0f, radius - flyhack_extrusion, 0.0f);
            Vector3 vector3 = vector + Vector3(0.0f, height - radius, 0.0f);
            float radius2 = radius - 0.05f;

            isInAir = plly->get_movement()->flying();

            if (isInAir)
            {
                Vector3 vector4 = (plly->get_lastSentTick()->get_position() - plly->get_transform()->get_position());

                float num2 = functions::mathf_abs(vector4.y);
                float num3 = sqrt((vector4.x * vector4.x) + (vector4.y * vector4.y) + (vector4.z + vector4.z));

                if (vector4.y >= 0.0f)
                {
                    flyhackDistanceVertical += vector4.y;
                    flag = true;
                }
                if (num2 < num3)
                {
                    flyhackDistanceHorizontal += num3;
                    flag = true;
                }
                if (flag)
                {
                    float num5 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
                    float num6 = (plly->get_jumpheight() + num5);
                    if (flyhackDistanceVertical > num6) {
                        //std::cout << OBFUSCATE_STR("Got Flyhacked") << std::endl;
                    }
                    float num7 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
                    float num8 = (5.f + num7);
                    if (flyhackDistanceHorizontal > num8) {
                        //std::cout << OBFUSCATE_STR("Got Flyhacked") << std::endl;
                    }
                }
            }




            else {
                flyhackDistanceHorizontal = 0.f;
                flyhackDistanceVertical = 0.f;
            }
            float num5 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
            float num6 = ((plly->get_jumpheight() + num5) * 3);
            //num6 = 13.0f;
            VMaxFlyhack = num6;//(num6 * 2) - 2;
            if (flyhackDistanceVertical <= (num6)) {
                VFlyhack = flyhackDistanceVertical;
            }
            if (VFlyhack >= VMaxFlyhack)
                VFlyhack = VMaxFlyhack;
            float num7 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
            float num8 = ((5.f + num7) * 3);
            //num8 = 26.0f;
            HMaxFlyhack = num8;//(num8 * 2) - 2; //- (VFlyhack * 2);//(num6 * 2);
            if (flyhackDistanceHorizontal <= (num8)) {
                HFlyhack = flyhackDistanceHorizontal;// +(VFlyhack * 2);
            }
            if (HFlyhack >= HMaxFlyhack)
                HFlyhack = HMaxFlyhack;

        }
        else {
            flyhackDistanceHorizontal = 0.f;
            flyhackDistanceVertical = 0.f;
        }
        if (!isInAir)
        {
            flyhackDistanceHorizontal = 0.f;
            flyhackDistanceVertical = 0.f;
        }


        float v, h;
        v = VFlyhack / (VMaxFlyhack - 1.0f);
        h = HFlyhack / (HMaxFlyhack - 1.0f);

        static int cases = 0;
        static float r = 1.00f, g = 0.00f, b = 1.00f;
        switch (cases)
        {
        case 0: { r -= 0.005f; if (r <= 0) cases += 1; break; }
        case 1: { g += 0.005f; b -= 0.005f; if (g >= 1) cases += 1; break; }
        case 2: { r += 0.005f; if (r >= 1) cases += 1; break; }
        case 3: { b += 0.005f; g -= 0.005f; if (b >= 1) cases = 0; break; }
        default: { r = 1.00f; g = 0.00f; b = 1.00f; break; }
        }
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 870, 250, 10), Vector4(0, 0, 0, 0.9));
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 850, 250, 10), Vector4(0, 0, 0, 0.9));
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 870, 250 * (v >= 1.0f ? 1.0f : v), 10), Vector4(r, g, b, 1));
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 850, 250 * (h >= 1.0f ? 1.0f : h), 10), Vector4(r, g, b, 1));




    }
    template <typename T>
    void write(uintptr_t address, T data) {
        if (address > 0x40000 && (address + sizeof(T)) < 0x7FFFFFFF0000)
            *(T*)(address) = data;
    }
    void fov(float fov)
    {

        uintptr_t Vbase = LI_MODULE(("GameAssembly.dll")).get<uintptr_t>();

        auto klass2 = *reinterpret_cast<uintptr_t*>(Vbase + 50523688); // convar_admin_c*
        //auto Op = *reinterpret_cast<uintptr_t*>(Vbase + 49981568); // convar_admin_c*

        auto static_fields2 = *reinterpret_cast<uintptr_t*>(klass2 + 0xb8);
        //auto fos = *reinterpret_cast<uintptr_t*>(Op + 0xb8);
    //	write<float>(fos + 0X18, 130.0f);
        write<float>(static_fields2 + 0x0, 12.0f);
    }


    void SafeAdmin() { variables::local_player->add_flag(4); }
    void NoRecoil() { variables::local_player->get_held_item()->Norecoil(0.0f); }

    void nosway() {
        variables::local_player->get_held_item()->set_aim_cone(-10.0f);
        variables::local_player->get_held_item()->set_aim_cone_curve_scale(-10.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_max(-10.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_per_shot(-10.0f);
        variables::local_player->get_held_item()->set_aim_sway(-10.0f);
        variables::local_player->get_held_item()->set_aim_sway_speed(-10.0f);
    }
    void instaEoka() {
        variables::local_player->get_held_item()->set_always_eoka(100.0f);
    }
    void Infjump() {
        variables::local_player->get_movement()->set_jump_time(0);
        variables::local_player->get_movement()->set_land_time(0);
        variables::local_player->get_movement()->set_ground_time(100000);
        variables::local_player->get_held_item()->set_aim_cone(0.0f);
        variables::local_player->get_held_item()->set_aim_cone_curve_scale(0.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_max(0.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_per_shot(0.0f);
        variables::local_player->get_movement()->ShootInAir();
        variables::local_player->get_held_item()->set_aim_cone(0.f / 100.f);
        variables::local_player->get_held_item()->set_aim_cone_curve_scale(-10.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_max(-10.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_per_shot(-10.0f);



        write<float>((uint64_t) + 0x48, 1);
       
        rust::BasePlayer* plly = variables::local_player;
        float flyhack_forgiveness_interia = 10.0f;
        float flyhack_forgiveness = 1.5f;
        float flyhack_extrusion = 1.85f;
        bool flag = false;
        bool isInAir = false;
        Vector3 vector = (plly->get_lastSentTick()->get_position() + plly->get_transform()->get_position()) * 0.5f;
        flyhackPauseTime = functions::mathf_max(0.f, flyhackPauseTime - functions::get_deltaTime());

        if (plly) {
            float radius = plly->get_radius();
            float height = plly->get_height(false);

            Vector3 vector2 = vector + Vector3(0.0f, radius - flyhack_extrusion, 0.0f);
            Vector3 vector3 = vector + Vector3(0.0f, height - radius, 0.0f);
            float radius2 = radius - 0.05f;

            isInAir = plly->get_movement()->flying();

            if (isInAir)
            {
                Vector3 vector4 = (plly->get_lastSentTick()->get_position() - plly->get_transform()->get_position());

                float num2 = functions::mathf_abs(vector4.y);
                float num3 = sqrt((vector4.x * vector4.x) + (vector4.y * vector4.y) + (vector4.z + vector4.z));

                if (vector4.y >= 0.0f)
                {
                    flyhackDistanceVertical += vector4.y;
                    flag = true;
                }
                if (num2 < num3)
                {
                    flyhackDistanceHorizontal += num3;
                    flag = true;
                }
                if (flag)
                {
                    float num5 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
                    float num6 = (plly->get_jumpheight() + num5);
                    if (flyhackDistanceVertical > num6) {
                        //std::cout << OBFUSCATE_STR("Got Flyhacked") << std::endl;
                    }
                    float num7 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
                    float num8 = (5.f + num7);
                    if (flyhackDistanceHorizontal > num8) {
                        //std::cout << OBFUSCATE_STR("Got Flyhacked") << std::endl;
                    }
                }
            }




            else {
                flyhackDistanceHorizontal = 0.f;
                flyhackDistanceVertical = 0.f;
            }
            float num5 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
            float num6 = ((plly->get_jumpheight() + num5) * 3);
            //num6 = 13.0f;
            VMaxFlyhack = num6;//(num6 * 2) - 2;
            if (flyhackDistanceVertical <= (num6)) {
                VFlyhack = flyhackDistanceVertical;
            }
            if (VFlyhack >= VMaxFlyhack)
                VFlyhack = VMaxFlyhack;
            float num7 = functions::mathf_max((flyhackPauseTime > 0.0f) ? flyhack_forgiveness_interia : flyhack_forgiveness, 0.0f);
            float num8 = ((5.f + num7) * 3);
            //num8 = 26.0f;
            HMaxFlyhack = num8;//(num8 * 2) - 2; //- (VFlyhack * 2);//(num6 * 2);
            if (flyhackDistanceHorizontal <= (num8)) {
                HFlyhack = flyhackDistanceHorizontal;// +(VFlyhack * 2);
            }
            if (HFlyhack >= HMaxFlyhack)
                HFlyhack = HMaxFlyhack;

        }
        else {
            flyhackDistanceHorizontal = 0.f;
            flyhackDistanceVertical = 0.f;
        }
        if (!isInAir)
        {
            flyhackDistanceHorizontal = 0.f;
            flyhackDistanceVertical = 0.f;
        }


        float v, h;
        v = VFlyhack / (VMaxFlyhack - 1.0f);
        h = HFlyhack / (HMaxFlyhack - 1.0f);

        static int cases = 0;
        static float r = 1.00f, g = 0.00f, b = 1.00f;
        switch (cases)
        {
        case 0: { r -= 0.005f; if (r <= 0) cases += 1; break; }
        case 1: { g += 0.005f; b -= 0.005f; if (g >= 1) cases += 1; break; }
        case 2: { r += 0.005f; if (r >= 1) cases += 1; break; }
        case 3: { b += 0.005f; g -= 0.005f; if (b >= 1) cases = 0; break; }
        default: { r = 1.00f; g = 0.00f; b = 1.00f; break; }
        }
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 870, 250, 10), Vector4(0, 0, 0, 0.9));
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 850, 250, 10), Vector4(0, 0, 0, 0.9));
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 870, 250 * (v >= 1.0f ? 1.0f : v), 10), Vector4(r, g, b, 1));
        functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 850, 250 * (h >= 1.0f ? 1.0f : h), 10), Vector4(r, g, b, 1));


    }

    void FullAuto() { variables::local_player->get_held_item()->set_automatic(false); }
    void MoonJump() { variables::local_player->get_movement()->fun(); }
    void ShootWhileInAir() { variables::local_player->get_movement()->ShootInAir(); 
    variables::local_player->get_movement()->set_ground_angle(1.f);

    
    
    }

    void longeyes() {
        if (functions::get_key(KeyCode::UpArrow))
        {
            variables::local_player->get_eyes()->set_view_offset({ 0,3.8,0 });

        }
    }
    void NoSpread() {
     
        variables::local_player->get_held_item()->set_aim_cone ( 0.f / 100.f);
        variables::local_player->get_held_item()->set_aim_cone_curve_scale(-10.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_max(-10.0f);
        variables::local_player->get_held_item()->set_aim_cone_penalty_per_shot(-10.0f);
        variables::local_player->get_held_item()->ExtMelee();
        variables::local_player->get_held_item()->instabow();
        variables::local_player->get_held_item()->instacharge();
        variables::local_player->get_held_item()->set_rapid_fire( 1.f);


    };
    void WalkOnWalter() { variables::local_player->get_movement()->Water(); }
    void ExterndMelee() { variables::local_player->get_held_item()->ExtMelee(); }
    void fastbow() { variables::local_player->get_held_item()->instabow(); }
    void instacharge() { variables::local_player->get_held_item()->instacharge(); }
    void longeye() { variables::local_player->get_eyes()->longeyes(); }
    void Dysnic() {
        if (functions::get_key(KeyCode::Mouse3))
        {
            variables::local_player->get_lastSentTick()->SetTicks(.99);
            variables::local_player->get_lastSentTick()->SetTicks(-.0090099);
            variables::local_player->get_lastSentTick()->SetTicks(-00000.9);
            variables::local_player->get_lastSentTick()->SetTicks(12);
        }
        auto loco = variables::local_player;
        float desyncTime = (functions::get_time() - loco->get_lastSentTick()->TicksPerSec() - 0.03125 * 3);

        // std::cout << desyncTime;
        float red, green, blue;
        float Num = desyncTime;


        //  int newcammo = (cammo / mammo) * (250);
          //functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 400, 250, 10), Vector4(0, 0, 0, 0.9));
         // functions::fill_box(Vector4((functions::get_screen_width() / 2) - 125, (functions::get_screen_height() / 2) - 400, (newcammo), 10), Vector4(1, 0, 0, 1));



        if (Num < 0.5)
        {
            red = Num * 2.f * 255.f;
            green = 255.f;
            blue = 0.f;
        }
        else
        {
            red = 255.f;
            green = (2.f - 2.f * Num) * 255.f;
            blue = 0.f;
        }

        if (Num > 1.f)
            Num = 1.f;

        if (desyncTime > 0) {

            functions::outline_box(Vector2((variables::screen_width / 2), (variables::screen_height / 8)), Vector2(100, 8), Vector4(0, 0, 0, 1));
            functions::fill_box(Vector4((variables::screen_width / 2), (variables::screen_height / 8), (desyncTime), 8), Vector4(red, green, blue, 1));
        }

        }
   
}

    
