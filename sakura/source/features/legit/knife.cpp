#include "../../client.h"

constexpr auto MAX_DISTANCE = 8192.f;

int		Sakura::Knifebot::iTargetKnife;
int		Sakura::Knifebot::iHitboxKnife;
Vector	Sakura::Knifebot::vAimOriginKnife;

void Sakura::Knifebot::SelectTarget(playeraim_t Aim)
{
	bool hitboxselected = false;
	for (const model_aim_select_t& Model_Selected : Model_Aim_Select)
	{
		if (strcmp(Model_Selected.checkmodel, Aim.modelname))
			continue;

		bool skip = false;
		for (const playeraimlegit_t& AimLegit : PlayerAimLegit)
		{
			if (strcmp(AimLegit.checkmodel, Model_Selected.checkmodel))
				continue;
			if (AimLegit.numhitbox != Model_Selected.numhitbox)
				continue;
			if (AimLegit.m_iWeaponID != g_Local.weapon.m_iWeaponID)
				continue;
			skip = true;
		}
		if (skip)
			continue;

		hitboxselected = true;

		pmtrace_t tr;

		g_Engine.pEventAPI->EV_SetTraceHull(2);

		Vector vEye = pmove->origin + pmove->view_ofs;

		if (cvar.bypass_trace_knife)
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[Model_Selected.numhitbox].Hitbox, PM_WORLD_ONLY, -1, &tr);
		else
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[Model_Selected.numhitbox].Hitbox, PM_GLASS_IGNORE, -1, &tr);

		int detect = g_Engine.pEventAPI->EV_IndexFromTrace(&tr);

		if ((cvar.bypass_trace_knife && tr.fraction == 1 && !detect) || (!cvar.bypass_trace_knife && detect == Aim.index))
		{
			if (Aim.PlayerAimHitbox[Model_Selected.numhitbox].HitboxFOV <= cvar.knifebot_fov)
			{
				float fDistance = (Aim.PlayerAimHitbox[Model_Selected.numhitbox].Hitbox - (pmove->origin + pmove->view_ofs)).Length();

				if (fDistance < MAX_DISTANCE)
				{
					//flDist = fDistance;
					iTargetKnife = Aim.index;
					vAimOriginKnife = Aim.PlayerAimHitbox[Model_Selected.numhitbox].Hitbox;
					iHitboxKnife = Model_Selected.numhitbox;
					break;
				}
			}
		}
	}
	if (!hitboxselected)
	{
		pmtrace_t tr;

		g_Engine.pEventAPI->EV_SetTraceHull(2);

		Vector vEye = pmove->origin + pmove->view_ofs;

		if (cvar.bypass_trace_knife)
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[HeadBox[Aim.index]].Hitbox, PM_WORLD_ONLY, -1, &tr);
		else
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[HeadBox[Aim.index]].Hitbox, PM_GLASS_IGNORE, -1, &tr);

		int detect = g_Engine.pEventAPI->EV_IndexFromTrace(&tr);

		if ((cvar.bypass_trace_knife && tr.fraction == 1 && !detect) || (!cvar.bypass_trace_knife && detect == Aim.index))
		{
			if (Aim.PlayerAimHitbox[HeadBox[Aim.index]].HitboxFOV <= cvar.knifebot_fov)
			{
				float fDistance = (Aim.PlayerAimHitbox[Aim.index].Hitbox - (pmove->origin + pmove->view_ofs)).Length();
				if (fDistance < MAX_DISTANCE)
				{
					//flDist = fDistance;
					iTargetKnife = Aim.index;
					vAimOriginKnife = Aim.PlayerAimHitbox[HeadBox[Aim.index]].Hitbox;
					iHitboxKnife = HeadBox[Aim.index];
				}
			}
		}
	}
}

void Sakura::Knifebot::Knife(usercmd_s* cmd)
{
	iTargetKnife = 0;

	if (!IsCurWeaponKnife() || !cvar.knifebot_active)
		return;

	for (const playeraim_t& Aim : PlayerAim)
	{
		if (!Sakura::Player::IsAlive(Aim.index))
			continue;

		if (!cvar.knifebot_team && g_Player[Aim.index].iTeam == g_Local.iTeam)
			continue;

		if (cvar.aim_id_mode == 2 || (IdHook::FirstKillPlayer[Aim.index] == 1 || cvar.aim_id_mode == 0))
		{
			SelectTarget(Aim);
			continue;
		}

		if (!iTargetKnife && cvar.aim_id_mode != 2 && IdHook::FirstKillPlayer[Aim.index] < 2)
			SelectTarget(Aim);
	}

	if (!iTargetKnife)
		return;

	int dist = (cvar.knifebot_attack == 0) ? cvar.knifebot_attack_distance : cvar.knifebot_attack2_distance;
	float distanceSquared = (vAimOriginKnife - (pmove->origin + pmove->view_ofs)).LengthSqr();

	if (distanceSquared < dist * dist)
	{
		if (CanAttack())
		{
			QAngle QAimAngles;
			Vector vEye = pmove->origin + pmove->view_ofs;
			VectorAngles(vAimOriginKnife - vEye, QAimAngles);
			QAimAngles.Normalize();

			if (cvar.knifebot_perfect_silent)
			{
				MakeAngle(QAimAngles, cmd);
				bSendpacket(false);
			}
			else if (cvar.knifebot_silent)
			{
				MakeAngle(QAimAngles, cmd);
			}
			else
			{
				cmd->viewangles = QAimAngles;
				g_Engine.SetViewAngles(QAimAngles);
			}

			if (cvar.knifebot_attack == 0)
				cmd->buttons |= IN_ATTACK;
			else if (cvar.knifebot_attack == 1)
				cmd->buttons |= IN_ATTACK2;
		}
		else
		{
			if (cvar.knifebot_attack == 0)
				cmd->buttons &= ~IN_ATTACK;
			else if (cvar.knifebot_attack == 1)
				cmd->buttons &= ~IN_ATTACK2;
		}
	}
}

void Sakura::Knifebot::Draw()
{
	if (!cvar.knifebot_active || !IsCurWeaponKnife() || !Sakura::Player::Local::IsAlive() || !cvar.knifebot_draw_aim || !iTargetKnife)
		return;

	for (playeraim_t Aim : PlayerAim)
	{
		if (Aim.index != iTargetKnife)
			continue;

		float CalcAnglesMin[2], CalcAnglesMax[2];
		for (size_t i = 0; i < 12; ++i)
		{
			if (WorldToScreen(Aim.PlayerAimHitbox[iHitboxKnife].HitboxMulti[SkeletonHitboxMatrix[i][0]], CalcAnglesMin) && WorldToScreen(Aim.PlayerAimHitbox[iHitboxKnife].HitboxMulti[SkeletonHitboxMatrix[i][1]], CalcAnglesMax))
				ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Sakura::Colors::Green());
		}
	}
}