//==========================================================
// Field.h
// �ʵ� ���� ���
//==========================================================

#pragma once

#include "stdafx.h"

class Mesh;
class FieldMap {
public:
	FieldMap();
	~FieldMap();

	Mesh* gethouse_1();
	Mesh* gethouse_2();
	Mesh* gethouse_3();
	Mesh* gethouse_4();


protected:
private:
	Mesh* bottom{ nullptr };
	// �ǹ��� �߰�
	Mesh* house_1;
	Mesh* house_2;
	Mesh* house_3;
	Mesh* house_4;
};