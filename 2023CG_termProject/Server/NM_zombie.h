//==========================================================
// NM_zombie.h
// �Ϲ� ���� ���� ���
//==========================================================

#pragma once

#include "Enemy.h"
#include "NMMesh.h"
#include "Player.h"


class Player;

class NM_zombie : public EnemyBase {
public:
	NM_zombie();
	NM_zombie(float hp, float max, float spd, float def, float atk, int type);
	~NM_zombie();
	void setPlayer(std::vector<Player*>& players);
	void walk_ani(EnemyBase* t_list[], int myNum);
	
	void setHit(bool);
	void UpdateMatrix();

	void z_heal(std::vector<EnemyBase*>&);
	void z_boom();
	NM_Mesh* gethead();
	NM_Mesh* getbody();
	NM_Mesh* getlarm();
	NM_Mesh* getrarm();
	NM_Mesh* getlleg();
	NM_Mesh* getrleg();

	bool Death_check();
	
	
protected:
private:
	int z_type;
	clock_t h_start_time{};
	clock_t h_current_time{};
	double p_delay{};
	NM_Mesh* head;		// �Ӹ�
	NM_Mesh* body;		// ����
	NM_Mesh* arm[2];	// ����, ������
	NM_Mesh* leg[2];	// �޴ٸ�, �����ٸ�

	bool hithead;
};