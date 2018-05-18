#ifndef __REDUCTOR_CPU__
#define __REDUCTOR_CPU__

enum CPU_KERNEL
{
	cpu_simple,
	cpu_reduction,
	cpu_kahansum
};

//Kernel
//1. for ������ ��� ���ϱ� ����
void CPU_simple(float* input, float* output, int size);
//2. reduction ���� 
void CPU_reduction(float* input, float* output, int size);
//3. kahansum ����
void CPU_kahansum(float* input, float* output, int size);

class Reductor_CPU
{
private :
	bool bsuccess = false;

	//kernel �Լ��� ���� �迭
	void (*func_kernel[3])(float*, float*, int);
	//kernel �Լ��� �̸�
	const char* str_kernel[3] = { "Simple", "Reduction", "KahanSum" };

	//host buffer ����
	float* input;
	float* output;
	size_t size;

public:
	Reductor_CPU()
	{
		bsuccess = true;
		func_kernel[0] = CPU_simple;
		func_kernel[1] = CPU_reduction;
		func_kernel[2] = CPU_kahansum;
	}
	~Reductor_CPU()
	{

	}
	//��ü�� ���������� �����ƴ��� �˷��ִ� �޼ҵ�
	inline bool success() { return bsuccess; }

	//host buffer�� �ܺο��� �����Ѵ�.
	void set(float* input, float* output, size_t size);
	//1���� �����͸� CPU ȯ�濡�� ����
	void test(enum CPU_KERNEL kernel_num, int trial);

};

#endif