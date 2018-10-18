#include "Fstring.h"
#include <iostream>

using namespace std;

Fstring::Fstring()
{
	int Termo = 0;
	int Sratio = 0;
	int Usignal = 0;
	int Uref_signal = 0;
	int So_Z = 0;
	int So_z = 0;
	int So_zkt = 0;
	int Status = 0;
}

Fstring::~Fstring()
{
}

void Fstring::SetTermo(int Termo)
{
	this->Termo = Termo;
}

void Fstring::SetSratio(int Sratio)
{
	this->Sratio = Sratio;
}

void Fstring::SetUsignal(int Usignal)
{
	this->Usignal = Usignal;
}

void Fstring::SetUref_signal(int Uref_signal)
{
	this->Uref_signal = Uref_signal;
}

void Fstring::SetSo_Z(int So_Z)
{
	this->So_Z = So_Z;
}

void Fstring::SetSo_z(int So_z)
{
	this->So_z = So_z;
}

void Fstring::SetSo_zkt(int So_zkt)
{
	this->So_zkt = So_zkt;
}

void Fstring::SetStatus(int Status)
{
	this->Status = Status;
}

int Fstring::GetTermo()
{
	return Termo;
}

int Fstring::GetSratio()
{
	return Sratio;
}

int Fstring::GetUsignal()
{
	return Usignal;
}

int Fstring::GetUref_signal()
{
	return Uref_signal;
}

int Fstring::GetSo_Z()
{
	return So_Z;
}

int Fstring::GetSo_z()
{
	return So_z;
}

int Fstring::GetSo_zkt()
{
	return So_zkt;
}

int Fstring::GetStatus()
{
	return Status;
}

void Fstring::display() 
{
	 cout <<
		Termo << "\t" <<
		Sratio << "\t" <<
		Usignal << "\t" <<
		Uref_signal << "\t" <<
		So_Z << "\t" <<
		So_z << "\t" <<
		So_zkt << "\t" << endl;
}
