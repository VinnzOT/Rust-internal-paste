#include "aimbot.h"
#include "utils/memory.hpp"

namespace aimbot {
    rust::BasePlayer* getBestPlayerByFov() {
        rust::BasePlayer* bestEnt = nullptr;
        float closest = FLT_MAX;
        for (int i = 0; i < variables::player_list.size(); i++) {
            auto model = variables::player_list[i]->get_model();
          
            if (variables::player_list[i] == variables::local_player)
                continue;
            if (variables::player_list[i]->get_life_state())
                continue;
            if (variables::player_list[i]->is_sleeping())
                continue;
            Vector3 headPos = variables::player_list[i]->get_model()->get_position();
            Vector2 head2D;
            if (functions::WorldToScreen(headPos, head2D) == false) continue;
            float length = sqrt(pow((variables::screen_width / 2) - head2D.x, 2) + pow((variables::screen_height / 2) - head2D.y, 2));

            if (length < closest) {
                closest = length, 300;
                bestEnt = variables::player_list[i];
            }
        }
        return bestEnt;
    }

    	Vector3 sim_prediction(const Vector3& from, int bone, float initial_velocity, float gravity_modifier, float drag, float height)
	{
		if (variables::local_player->get_held_item()->is_melee())
		{
			return Vector3{ 0,0,0 };
		}

		rust::BasePlayer* who = getBestPlayerByFov();

		Vector3 aimpoint = who->get_bone_position(48);
		float distance = Math::Calc3D_Dist(from, aimpoint);
		auto held_weapon = variables::local_player->get_held_item();
		if (!held_weapon)
			return aimpoint;

		if (!held_weapon->is_gun())
			return aimpoint;

		auto weapon_data = held_weapon->get_weapon_data();

		float m_flBulletSpeed = initial_velocity;
		float bullet_gravity = gravity_modifier;
		float bullet_time = distance / m_flBulletSpeed;
		float m_fDrag = drag;

		const float m_flTimeStep = (0.015625f);
		float YTravelled{}, m_flYSpeed{}, m_flBulletTime{}, m_flDivider{};



		for (float distance_to_travel = 1.f; distance_to_travel < distance;)
		{
			float speed_modifier = 1.f - m_flTimeStep * m_fDrag;
			m_flBulletSpeed *= speed_modifier;

			if (m_flBulletSpeed <= 1.f || m_flBulletSpeed >= 10000.f || YTravelled >= 10000.f || YTravelled < 10000.f)
				break;


			if (m_flBulletTime > 2.f)
				break;

			m_flYSpeed += ((9.81f) * bullet_gravity) * m_flTimeStep;
			m_flYSpeed *= speed_modifier;

			distance_to_travel += m_flBulletSpeed * m_flTimeStep;
			YTravelled += m_flYSpeed * m_flTimeStep;
			m_flBulletTime += m_flTimeStep;
		}

		Vector3 velocity = who->get_model()->get_velocity() * 0.9f;
	/*	if (velocity.y > 1.f)
		   velocity.y /= 3.25f;*/

		aimpoint.y += YTravelled;
		aimpoint += velocity * m_flBulletTime;

		return aimpoint;
	}


    int getClosestBoneToCrosshair(rust::BasePlayer* player) {
        int bestBone = 0;
        float closest = FLT_MAX;
        Vector2 curAngles = variables::local_player->get_input()->get_view_angles();
        for (int i = 0; i < rust::valid_bones.size(); i++) {
            Vector3 pos3d = variables::player_list[i]->get_model()->get_position();
            Vector2 pos;
            if (functions::WorldToScreen(pos3d, pos) == false) continue;
            float length = sqrt(pow((variables::screen_width / 2) - pos.x, 2) + pow((variables::screen_height / 2) - pos.y, 2));
            if (length < closest) {
                closest = length;
               
                bestBone = rust::valid_bones[i];
            }
        }
        return bestBone;
    }
    double ToRad(double degree)
    {
        double pi = 3.14159265359;
        return (degree * (pi / 180));
    }
    Vector4 Epicwashere(Vector3 Euler) {

        double heading = ToRad(Euler.x);
        double attitude = ToRad(Euler.y);
        double bank = ToRad(Euler.z);

        double c1 = cos(heading / 2);
        double s1 = sin(heading / 2);
        double c2 = cos(attitude / 2);
        double s2 = sin(attitude / 2);
        double c3 = cos(bank / 2);
        double s3 = sin(bank / 2);
        double c1c2 = c1 * c2;
        double s1s2 = s1 * s2;
        Vector4 Quat;
        Quat.w = c1c2 * c3 - s1s2 * s3;
        Quat.x = c1c2 * s3 + s1s2 * c3;
        Quat.y = s1 * c2 * c3 + c1 * s2 * s3;
        Quat.z = c1 * s2 * c3 - s1 * c2 * s3;
        Vector4 yourmom;
        yourmom = { Quat.y, Quat.z, (Quat.x * -1), Quat.w };
        return yourmom;
    }

    void run(BoneList abone) {
        rust::BasePlayer* player = getBestPlayerByFov();
        if (!player)
        {

            //functions::outline(Vector4(variables::screen_width / 2, variables::screen_height / 2 + 50, 150, 20), xorstr_("Dead"), Vector4(0, 0.5, 1, 1));
            functions::label(Vector4(variables::screen_width / 2, variables::screen_height / 2 + 50, 150, 20), xorstr_("Dead"), Vector4(1, 1, 1, 1));
        }
        int bone = getClosestBoneToCrosshair(player);
        bone = bone ? bone : BoneList::neck;

        Vector3 enemylock = player->get_bone_position(49);
       

        Vector2 AngleToAim = Math::CalcAngle(variables::local_player->get_bone_position(BoneList::head), enemylock);


        Math::Normalize(AngleToAim.y, AngleToAim.x);
        if (isnan(AngleToAim.x) || isnan(AngleToAim.y))
            return;

      
        Vector4 QuatAngles = Epicwashere({ AngleToAim.x, AngleToAim.y, 0.0f });

        variables::local_player->get_eyes()->rotationLook(QuatAngles);
        //variables::local_player->get_input()->set_view_angles(Math::CalcAngle(variables::local_player->get_bone_position(BoneList::head), enemylock )); <--obsolete says epic
    }

}