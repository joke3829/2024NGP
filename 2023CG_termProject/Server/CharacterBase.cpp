#include "CharacterBase.h"

// ������
CharacterBase::CharacterBase(float hp, float max, float spd, float def, float atk)
	: HP(hp), MAXHP(max), speed(spd), DEF(def), ATK(atk) {};

CharacterBase::~CharacterBase() {};

// ü�� ������Ʈ
void CharacterBase::Update_HP(float received)
{
	if (received > 0) {
		HP += received;
		if (HP >= MAXHP) HP = MAXHP;
	}
	else {
		//std::cout << "���� ���� ������" <<  received + DEF << '\n';
		HP += received + DEF;		// ������ ���߿� ��������
	}
}

// �׾����� üũ
bool CharacterBase::Death_check()
{
	if (HP <= 0)
		return true;

	return false;
}


float CharacterBase::getHP()
{
	return HP;
}