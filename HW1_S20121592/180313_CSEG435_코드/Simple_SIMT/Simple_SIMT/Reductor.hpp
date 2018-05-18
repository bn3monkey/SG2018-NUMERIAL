#ifndef __REDUCTOR__
#define __REDUCTOR__

#include "Reductor_GPU.hpp"
#include "Reductor_CPU.hpp"

//ERROR �޽��� ��ü
class ERROR_
{
public:
	const char* msg;
	ERROR_(const char* _msg) : msg(_msg) {};
};

class Reductor
{
	//�� Reductor Class�� ���� ��ü
	Reductor_CPU* pCPU;
	Reductor_GPU* pGPU;

	//input data ����
	float* data_input;
	float* data_temp;
	size_t data_size;



public:
	Reductor()
	{
		pCPU = nullptr;
		pGPU = nullptr;

		pCPU = new Reductor_CPU();
		if (!pCPU->success())
			throw ERROR_("CPU Initialize ERROR!");

		pGPU = new Reductor_GPU();
		if (!pGPU->success())
			throw ERROR_("GPU Initialize ERROR!");

	}
	~Reductor()
	{
		if(pCPU != nullptr)
			delete pCPU;
		if(pGPU != nullptr)
			delete pGPU;
	}

	//inputData�� ���Ƿ� �����Ѵ�.
	void makeTestSet(size_t size);
	//inputData�� �Ҵ������Ѵ�.
	void removeTestSet();
	//CPU ���� ��ü�� ���´�.
	inline Reductor_CPU* getCPU() { return pCPU; }
	//GPU ���� ��ü�� ���´�.
	inline Reductor_GPU* getGPU() { return pGPU; }
};

#endif