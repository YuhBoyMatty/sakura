#include "../../client.h"

int		Sakura::Aimbot::Rage::iTargetRage;
int		Sakura::Aimbot::Rage::iHitboxRage;
bool	Sakura::Aimbot::Rage::RageKeyStatus;
Vector	Sakura::Aimbot::Rage::vAimOriginRage;

void Sakura::Aimbot::Rage::Target(playeraim_t Aim, float& m_flBestDist, float& m_flBestFOV, int hitbox)
{
	if (IsCurWeaponKnife())
	{
		float fDistance = (Aim.PlayerAimHitbox[hitbox].Hitbox - (pmove->origin + pmove->view_ofs)).Length();
		if (fDistance < m_flBestDist)
		{
			m_flBestDist = fDistance;
			iTargetRage = Aim.index;
			vAimOriginRage = Aim.PlayerAimHitbox[hitbox].Hitbox;
			iHitboxRage = hitbox;
		}
	}
	else
	{
		if (cvar.rage_target_selection == 0)
		{
			if (Aim.PlayerAimHitbox[hitbox].HitboxFOV < m_flBestFOV)
			{
				m_flBestFOV = Aim.PlayerAimHitbox[hitbox].HitboxFOV;
				iTargetRage = Aim.index;
				vAimOriginRage = Aim.PlayerAimHitbox[hitbox].Hitbox;
				iHitboxRage = hitbox;
			}
		}
		if (cvar.rage_target_selection == 1)
		{
			float fDistance = (Aim.PlayerAimHitbox[hitbox].Hitbox - (pmove->origin + pmove->view_ofs)).Length();
			if (fDistance < m_flBestDist)
			{
				m_flBestDist = fDistance;
				iTargetRage = Aim.index;
				vAimOriginRage = Aim.PlayerAimHitbox[hitbox].Hitbox;
				iHitboxRage = hitbox;
			}
		}
		if (cvar.rage_target_selection == 2)
		{
			if (Aim.PlayerAimHitbox[hitbox].HitboxFOV < m_flBestFOV)
			{
				float fDistance = (Aim.PlayerAimHitbox[hitbox].Hitbox - (pmove->origin + pmove->view_ofs)).Length();
				if (fDistance < m_flBestDist)
				{
					m_flBestFOV = Aim.PlayerAimHitbox[hitbox].HitboxFOV;
					m_flBestDist = fDistance;
					iTargetRage = Aim.index;
					vAimOriginRage = Aim.PlayerAimHitbox[hitbox].Hitbox;
					iHitboxRage = hitbox;
				}
			}
		}
	}
}

void Sakura::Aimbot::Rage::SelectHitbox(playeraim_t Aim, float& m_flBestDist, float& m_flBestFOV)
{
	bool hitboxselected = false;

	for (const model_aim_select_t& Model_Selected : Model_Aim_Select)
	{
		if (strcmp(Model_Selected.checkmodel, Aim.modelname))
			continue;

		hitboxselected = true;

		pmtrace_t tr;

		g_Engine.pEventAPI->EV_SetTraceHull(2);

		Vector vEye = pmove->origin + pmove->view_ofs;

		if (cvar.bypass_trace_rage)
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[Model_Selected.numhitbox].Hitbox, PM_WORLD_ONLY, -1, &tr);
		else
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[Model_Selected.numhitbox].Hitbox, PM_GLASS_IGNORE, -1, &tr);

		int detect = g_Engine.pEventAPI->EV_IndexFromTrace(&tr);

		if ((cvar.bypass_trace_rage && tr.fraction == 1 && !detect) || (!cvar.bypass_trace_rage && detect == Aim.index))
		{
			if (Aim.PlayerAimHitbox[Model_Selected.numhitbox].HitboxFOV <= cvar.rage_fov)
			{
				Target(Aim, m_flBestDist, m_flBestFOV, Model_Selected.numhitbox);
				break;
			}
		}
		else
		{
			int iOriginalPenetration = CurPenetration();

			if (iOriginalPenetration && cvar.rage_wall)
			{
				int iDamage = CurDamage();
				int iBulletType = CurBulletType();
				float flDistance = CurDistance();
				float flRangeModifier = CurWallPierce();

				int iCurrentDamage = Sakura::Aimbot::FireBullets(vEye, Aim.PlayerAimHitbox[Model_Selected.numhitbox].Hitbox, flDistance, iOriginalPenetration, iBulletType, iDamage, flRangeModifier);

				if (iCurrentDamage > 0)
				{
					if (Aim.PlayerAimHitbox[Model_Selected.numhitbox].HitboxFOV <= cvar.rage_fov)
					{
						Target(Aim, m_flBestDist, m_flBestFOV, Model_Selected.numhitbox);
						break;
					}
				}
			}
		}
	}
	if (!hitboxselected)
	{
		pmtrace_t tr;

		g_Engine.pEventAPI->EV_SetTraceHull(2);

		Vector vEye = pmove->origin + pmove->view_ofs;

		if (cvar.bypass_trace_rage)
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[HeadBox[Aim.index]].Hitbox, PM_WORLD_ONLY, -1, &tr);
		else
			g_Engine.pEventAPI->EV_PlayerTrace(vEye, Aim.PlayerAimHitbox[HeadBox[Aim.index]].Hitbox, PM_GLASS_IGNORE, -1, &tr);

		int detect = g_Engine.pEventAPI->EV_IndexFromTrace(&tr);

		if ((cvar.bypass_trace_rage && tr.fraction == 1 && !detect) || (!cvar.bypass_trace_rage && detect == Aim.index))
		{
			if (Aim.PlayerAimHitbox[HeadBox[Aim.index]].HitboxFOV <= cvar.rage_fov)
				Target(Aim, m_flBestDist, m_flBestFOV, HeadBox[Aim.index]);
		}
		else
		{
			int iOriginalPenetration = CurPenetration();

			if (iOriginalPenetration && cvar.rage_wall)
			{
				int iDamage = CurDamage();
				int iBulletType = CurBulletType();
				float flDistance = CurDistance();
				float flRangeModifier = CurWallPierce();

				int iCurrentDamage = Sakura::Aimbot::FireBullets(vEye, Aim.PlayerAimHitbox[HeadBox[Aim.index]].Hitbox, flDistance, iOriginalPenetration, iBulletType, iDamage, flRangeModifier);

				if (iCurrentDamage > 0)
				{
					if (Aim.PlayerAimHitbox[HeadBox[Aim.index]].HitboxFOV <= cvar.rage_fov)
						Target(Aim, m_flBestDist, m_flBestFOV, HeadBox[Aim.index]);
				}
			}
		}
	}
}

void Sakura::Aimbot::Rage::Aim(usercmd_s* cmd)
{
	iTargetRage = 0;

	if (IsCurWeaponNonAttack())
		return;

	if (cvar.rage_tapping_mode && (g_Local.vPunchangle.Length2D() > 0.f || g_Local.weapon.m_iShotsFired))
		return;

	float m_flBestFOV = 180;
	float m_flBestDist = 8192;

	for (const playeraim_t& Aim : PlayerAim)
	{
		if (!Sakura::Player::IsAlive(Aim.index))
			continue;

		if (!cvar.rage_team && g_Player[Aim.index].iTeam == g_Local.iTeam)
			continue;

		if (!cvar.rage_shield_attack && (Aim.sequence == 97 || Aim.sequence == 98))
			continue;

		if (cvar.aim_id_mode == 2 || (IdHook::FirstKillPlayer[Aim.index] == 1 || cvar.aim_id_mode == 0))
		{
			SelectHitbox(Aim, m_flBestDist, m_flBestFOV);
			continue;
		}

		if (!iTargetRage && cvar.aim_id_mode != 2 && IdHook::FirstKillPlayer[Aim.index] < 2)
			SelectHitbox(Aim, m_flBestDist, m_flBestFOV);
	}

	if (iTargetRage)
	{
		QAngle QAimAngles;

		Vector vEye = pmove->origin + pmove->view_ofs;

		VectorAngles(vAimOriginRage - vEye, QAimAngles);

		static DWORD delay = 0;

		static int tickcount = 0;

		if (cvar.route_auto && cvar.route_mode)
		{
			if (g_Local.weapon.m_iClip)
			{
				if (IsCurWeaponKnife())
				{
					if (cvar.rage_knife_attack == 0)
						cmd->buttons |= IN_ATTACK;
					else if (cvar.rage_knife_attack == 1)
						cmd->buttons |= IN_ATTACK2;
				}
				else
					cmd->buttons |= IN_ATTACK;

				cmd->viewangles = QAimAngles;
				g_Engine.SetViewAngles(QAimAngles);
			}
		}
		else
		{
			if (cvar.rage_always_fire)
			{
				if (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2 && IsCurWeaponKnife())
				{
					if (cvar.rage_perfect_silent)
					{
						MakeAngle(QAimAngles, cmd);
						bSendpacket(false);
					}
					else if (cvar.rage_silent)
					{
						MakeAngle(QAimAngles, cmd);
					}
					else
					{
						cmd->viewangles = QAimAngles;
						g_Engine.SetViewAngles(QAimAngles);
					}
				}
			}
			else if (CanAttack())
			{
				if (GetTickCount() - delay > cvar.rage[g_Local.weapon.m_iWeaponID].rage_delay_shot)
				{
					if (cvar.rage_auto_fire || RageKeyStatus)
					{
						if (IsCurWeaponKnife())
						{
							if (cvar.rage_knife_attack == 0)
								cmd->buttons |= IN_ATTACK;
							else if (cvar.rage_knife_attack == 1)
								cmd->buttons |= IN_ATTACK2;
						}
						else
						{
							cmd->buttons |= IN_ATTACK;
							if (cmd->buttons & IN_ATTACK)tickcount++;
							static int random = rand() % (int)cvar.rage[g_Local.weapon.m_iWeaponID].rage_random_max + 1;
							if (tickcount >= (!cvar.rage[g_Local.weapon.m_iWeaponID].rage_shot_type ? cvar.rage[g_Local.weapon.m_iWeaponID].rage_shot_count : random))
							{
								random = rand() % (int)cvar.rage[g_Local.weapon.m_iWeaponID].rage_random_max + 1;
								delay = GetTickCount();
								tickcount = 0;
							}
						}
					}
					if (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2 && IsCurWeaponKnife())
					{
						if (cvar.rage_perfect_silent)
						{
							MakeAngle(QAimAngles, cmd);
							bSendpacket(false);
						}
						else if (cvar.rage_silent)
						{
							MakeAngle(QAimAngles, cmd);
						}
						else
						{
							cmd->viewangles = QAimAngles;
							g_Engine.SetViewAngles(QAimAngles);
						}
					}
				}
			}
			else
			{
				if (IsCurWeaponKnife())
				{
					if (cvar.rage_knife_attack == 0)
						cmd->buttons &= ~IN_ATTACK;
					else if (cvar.rage_knife_attack == 1)
						cmd->buttons &= ~IN_ATTACK2;
				}
				else
					cmd->buttons &= ~IN_ATTACK;
			}
		}
	}
}

void Sakura::Aimbot::Rage::Draw()
{
	if (IsCurWeaponNonAttack() || !Sakura::Player::Local::IsAlive() || !cvar.rage_draw_aim || !iTargetRage)
		return;

	for (playeraim_t Aim : PlayerAim)
	{
		if (Aim.index != iTargetRage)
			continue;

		float CalcAnglesMin[2], CalcAnglesMax[2];
		for (unsigned int i = 0; i < 12; ++i)
		{
			if (WorldToScreen(Aim.PlayerAimHitbox[iHitboxRage].HitboxMulti[SkeletonHitboxMatrix[i][0]], CalcAnglesMin) && WorldToScreen(Aim.PlayerAimHitbox[iHitboxRage].HitboxMulti[SkeletonHitboxMatrix[i][1]], CalcAnglesMax))
				ImGui::GetCurrentWindow()->DrawList->AddLine({ IM_ROUND(CalcAnglesMin[0]), IM_ROUND(CalcAnglesMin[1]) }, { IM_ROUND(CalcAnglesMax[0]), IM_ROUND(CalcAnglesMax[1]) }, Sakura::Colors::Green());
		}
	}
}