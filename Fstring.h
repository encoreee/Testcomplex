#pragma once

class Fstring
{

public:

	Fstring();
	~Fstring();

	//Setters

	void SetTermo(int Termo);
	void SetSratio(int Sratio);
	void SetUsignal(int Usignal);
	void SetUref_signal(int Uref_signal);
	void SetSo_Z(int So_Z);
	void SetSo_z(int So_z);
	void SetSo_zkt(int So_zkt);
	void SetStatus(int Status);

	//Accessors

	int GetTermo();
	int GetSratio();
	int GetUsignal();
	int GetUref_signal();
	int GetSo_Z();
	int GetSo_z();
	int GetSo_zkt();
	int GetStatus();

	void display();

private:

	int Termo;
	int Sratio;
	int Usignal;
	int Uref_signal;
	int So_Z;
	int So_z;
	int So_zkt;
	int Status;

};

