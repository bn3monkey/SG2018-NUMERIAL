#ifndef __REDUCTOR_GPU__
#define __REDUCTOR_GPU__

#include "my_OpenCL_util.h"

#define COALESCED_GLOBAL_MEMORY_ACCESS 

#define INDEX_GPU_1D 0
#define INDEX_CPU_1D 1
#define INDEX_GPU_2D 2
#define INDEX_CPU_2D 3

#define NUM_DEVICE 4

enum GPU_KERNEL
{
	gpu_reduction_global,
	gpu_reduction_local,
};

typedef struct _OPENCL_C_PROG_SRC {
	size_t length;
	char *string;
} OPENCL_C_PROG_SRC;


typedef struct _preset
{
	cl_device_id device;
	cl_context context;
	cl_command_queue cmd_queue;

	cl_program program;
	cl_kernel kernel[2];

	cl_mem buffer_input;
	cl_mem buffer_temp;
	cl_mem buffer_output;

} Preset;

class Reductor_GPU
{
private:
	//OpenCL ���� ��ü
	cl_platform_id platform[2];
	
	const char* source_name[2] = 
	{
		"sum_kernel_1d.cl",
		"sum_kernel_2d.cl",
	};
	OPENCL_C_PROG_SRC source[2];
	const char* kernel_name[2] =
	{
		"reduction_global",
		"reduction_local",
	};

	Preset preset[NUM_DEVICE];

	cl_event event_for_timing;

	//��ü�� �������� ���� ����
	bool bsuccess = false;

	// host buffer
	float* input;
	float* output;
	size_t size;

	// Device ������ �������� �޼ҵ�
	void getDevice();
	// �� Device�� ���Ͽ� Context�� �����ϴ� �޼ҵ�
	void getContext();
	// �� Device�� ���Ͽ� Command_queue�� �����ϴ� �޼ҵ�
	void getCommandQueue();
	//Source file�� �о�� OpenCL program�� build�ϴ� �޼ҵ�
	void buildProgram();
	//Kernel�� �������� Program
	void getKernel();
	
	//Test ���� 
	//Host Buffer�� �ִ� ������ Device Buffer�� ������.
	void transfer(int device);

	//Device Buffer�� �ִ� ������ Host Buffer�� �ٽ� ������.
	void receive(int device);

public :

	Reductor_GPU()
	{
		bsuccess = true;
		init();
	}

	~Reductor_GPU()
	{
		destroy();
	}

	//��ü�� ���������� �����ƴ��� �˷��ִ� �޼ҵ�
	inline bool success() { return bsuccess; }

	//OpenCL �ʿ亯���� �ʱ�ȭ���ִ� �޼ҵ�.
	void init();
	//OpenCL �ʿ亯���� �����ϴ� �޼ҵ�.
	void destroy();
	
	//host buffer�� �ܺο��� �����Ѵ�.
	void set(float* input, float* output, size_t size);

	//Buffer���� �Ҵ����ִ� �޼ҵ�.
	void allocBuffer();
	//Buffer���� �Ҵ��������ִ� �޼ҵ�.
	void removeBuffer();

	//1���� �����Ϳ��� global/local memory���� Ȱ���Ͽ� ����.
	void test_1d(enum GPU_KERNEL kernel_num, int trial, size_t work_group_size_gpu);
	//2���� �����Ϳ��� global/local memory���� Ȱ���Ͽ� ����.
	void test_2d(enum GPU_KERNEL kernel_num, int trial, size_t global_width, size_t work_group_width, size_t work_group_height);
	//1���� �����Ϳ��� CPU�� Ȱ���Ͽ� ����.
	void test_1d_cpu(enum GPU_KERNEL kernel_num, int trial, size_t work_grou_size_cpu);
	//2���� �����Ϳ��� CPU�� Ȱ���Ͽ� ����.
	void test_2d_cpu(enum GPU_KERNEL kernel_num, int trial, size_t global_width, size_t work_group_width, size_t work_group_height);
	
};

#endif