#include "Reductor_GPU.hpp"

void CPU_SUM(float *data, size_t size)
{
	int i, j;
	
	for (i = size / 2; i > 0; i >>= 1)
	{
		for (j = i - 1; j >= 0; j--)
		{		
			data[j] += data[j + i];
		}
	}
}


void Reductor_GPU::init()
{
	getDevice();
	getContext();
	getCommandQueue();
	buildProgram();
	getKernel();
}

void Reductor_GPU::destroy()
{
	for (int i = 0; i < NUM_DEVICE; i++)
	{
		clReleaseKernel(preset[i].kernel[0]);
		clReleaseKernel(preset[i].kernel[1]);
		clReleaseProgram(preset[i].program);
		clReleaseCommandQueue(preset[i].cmd_queue);
		clReleaseContext(preset[i].context);
	}

	delete source[0].string;
	delete source[1].string;
}

void Reductor_GPU::getDevice()
{
	
	cl_int errcode_ret;

	//Platform List ������.
	errcode_ret = clGetPlatformIDs(2, platform, NULL);
	CHECK_ERROR_CODE(errcode_ret);  
	
	//GPU Device ������
	errcode_ret = clGetDeviceIDs(platform[INDEX_GPU_1D], CL_DEVICE_TYPE_GPU, 1, &(preset[INDEX_GPU_1D].device), NULL);
	CHECK_ERROR_CODE(errcode_ret);
	//GPU Device ���� ���
	fprintf(stdout, "\n^^^ The first GPU device on the platform for 1d kernel^^^\n");
	print_device_0(preset[INDEX_GPU_1D].device);

	//GPU Device ������
	errcode_ret = clGetDeviceIDs(platform[INDEX_GPU_1D], CL_DEVICE_TYPE_GPU, 1, &(preset[INDEX_GPU_2D].device), NULL);
	CHECK_ERROR_CODE(errcode_ret);
	//GPU Device ���� ���
	fprintf(stdout, "\n^^^ The second GPU device on the platform for 2d kernel ^^^\n");
	print_device_0(preset[INDEX_GPU_2D].device);

	//CPU Device ������
	errcode_ret = clGetDeviceIDs(platform[INDEX_CPU_1D], CL_DEVICE_TYPE_CPU, 1, &(preset[INDEX_CPU_1D].device), NULL);
	CHECK_ERROR_CODE(errcode_ret);
	//CPU DEvice ���� ���
	fprintf(stdout, "\n^^^ The first CPU device on the platform for 1d kernel ^^^\n");
	print_device_0(preset[INDEX_CPU_1D].device);
	
	//CPU Device ������
	errcode_ret = clGetDeviceIDs(platform[INDEX_CPU_1D], CL_DEVICE_TYPE_CPU, 1, &(preset[INDEX_CPU_2D].device), NULL);
	CHECK_ERROR_CODE(errcode_ret);
	//CPU DEvice ���� ���
	fprintf(stdout, "\n^^^ The second CPU device on the platform for 2d kernel  ^^^\n");
	print_device_0(preset[INDEX_CPU_2D].device);

}


void Reductor_GPU::getContext()
{
	//Context�� �����.
	cl_int errcode_ret;
	fprintf(stdout, "\n^^^ Getting Context! ^^^\n");
	for (int i = 0; i < NUM_DEVICE; i++)
	{
		preset[i].context = clCreateContext(NULL, 1, &(preset[i].device), NULL, NULL, &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);
	}
	fprintf(stdout, "\n^^^ Getting Context Complete! ^^^\n");
}
void Reductor_GPU::getCommandQueue()
{
	cl_int errcode_ret;
	//Command Queue�� �����.
	fprintf(stdout, "\n^^^ Getting Command Queue! ^^^\n");
	for (int i = 0; i < NUM_DEVICE; i++)
	{
		preset[i].cmd_queue= clCreateCommandQueue(preset[i].context, preset[i].device, CL_QUEUE_PROFILING_ENABLE, &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);
	}
	fprintf(stdout, "\n^^^ Getting Command Queue Complete! ^^^\n");
}

void Reductor_GPU::buildProgram()
{
	cl_int errcode_ret;

	//1-1. 1D file�� �д´�.
	source[0].length = read_kernel_from_file(source_name[0], &(source[0].string));
	//1-2. 2D file�� �д´�.
	source[1].length = read_kernel_from_file(source_name[1], &(source[1].string));

	
	fprintf(stdout, "\n^^^ Building Program! ^^^\n");
	for (int i = 0; i < NUM_DEVICE; i++)
	{
		//2. ���α׷��� �ҽ��κ��� �����.
		preset[i].program = clCreateProgramWithSource(preset[i].context, 1, (const char **)&source[i/2].string, &source[i/2].length, &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);

		//3. ���α׷��� �����Ѵ�.
		errcode_ret = clBuildProgram(preset[i].program, 1, &(preset[i].device), NULL, NULL, NULL);
		CHECK_ERROR_CODE(errcode_ret);
		if (errcode_ret != CL_SUCCESS) {
			print_build_log(preset[i].program, preset[i].device, (i & 1) ? "GPU" : "CPU");
			exit(-1);
		}

		fprintf(stdout, "Build Success : %s %d\n", (i & 1) ? "CPU" : "GPU", i / 2 + 1);
	}
	fprintf(stdout, "^^^ Building Program! ^^^\n");
	
}

void Reductor_GPU::getKernel()
{
	cl_int errcode_ret;
	//�� Program���� Kernel�� �����.
	fprintf(stdout, "\n^^^ Making Kernel! ^^^\n");
	for (int i = 0; i < NUM_DEVICE; i++)
	{
		preset[i].kernel[0] = clCreateKernel(preset[i].program, kernel_name[0], &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);
		preset[i].kernel[1] = clCreateKernel(preset[i].program, kernel_name[1], &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);

		fprintf(stdout, "Making Kernel Success : %s %d\n", (i & 1) ? "CPU" : "GPU", i / 2 + 1);
	}
	fprintf(stdout, "^^^ Making Kernel Complete! ^^^\n");
}

void Reductor_GPU::allocBuffer()
{
	cl_int errcode_ret;
	for (int i = 0; i < NUM_DEVICE; i++)
	{
		preset[i].buffer_input = clCreateBuffer(preset[i].context, CL_MEM_READ_WRITE, sizeof(float)*this->size, NULL, &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);

		preset[i].buffer_temp = clCreateBuffer(preset[i].context, CL_MEM_READ_WRITE, sizeof(float)*this->size, NULL, &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);

		preset[i].buffer_output = clCreateBuffer(preset[i].context, CL_MEM_READ_WRITE, sizeof(float)*this->size, NULL, &errcode_ret);
		CHECK_ERROR_CODE(errcode_ret);
	}
	
}

void Reductor_GPU::removeBuffer()
{
	for (int i = 0; i < NUM_DEVICE; i++)
	{
		clReleaseMemObject(preset[i].buffer_input);
		clReleaseMemObject(preset[i].buffer_temp);
		clReleaseMemObject(preset[i].buffer_output);
	}
}


void Reductor_GPU::transfer(int device)
{
	cl_int errcode_ret;
	float compute_time;

	fprintf(stdout, "   [Data Transfer Host to Device(%s %d)] \n", (device & 1) ? "CPU" : "GPU", device / 2 + 1);

	CHECK_TIME_START;
	// Move the input data from the host memory to the GPU device memory.
	errcode_ret = clEnqueueWriteBuffer(preset[device].cmd_queue, preset[device].buffer_input, CL_FALSE, 0,
		sizeof(float)*this->size, input, 0, NULL, NULL);
	CHECK_ERROR_CODE(errcode_ret);

	/* Wait until all data transfers finish. */
	// Blocks until all previously queued OpenCL commands in a command-queue are issued to
	// the associated device and have completed. clFinish does not return until all previously
	// queued commands in command_queue have been processed and completed. clFinish is also
	// a synchronization point.
	clFinish(preset[device].cmd_queue); // What if this line is removed?
	CHECK_TIME_END(compute_time);
	CHECK_ERROR_CODE(errcode_ret);

	fprintf(stdout, "     * Time by host clock = %.3fms\n\n", compute_time);
}


//Device Buffer�� �ִ� ������ Host Buffer�� �ٽ� ������.
void Reductor_GPU::receive(int device)
{
	cl_int errcode_ret;
	float compute_time;

	fprintf(stdout, "   [Data Transfer Device to Host(%s %d)] \n", (device & 1) ? "CPU" : "GPU", device / 2 + 1);

	/* Read back the device buffer to the host array. */
	// Enqueue commands to read from a buffer object to host memory.
	CHECK_TIME_START;
	errcode_ret = clEnqueueReadBuffer(preset[device].cmd_queue, preset[device].buffer_output, CL_TRUE, 0,
		sizeof(float)*this->size, output, 0, NULL, &event_for_timing);
	CHECK_TIME_END(compute_time);
	CHECK_ERROR_CODE(errcode_ret);
	// In this case, you do not need to call clFinish() for a synchronization.

	fprintf(stdout, "     * Time by host clock = %.3fms\n\n", compute_time);
	print_device_time(event_for_timing);

}

void Reductor_GPU::set(float* input, float* output, size_t size)
{
	this->input = input;
	this->output = output;
	this->size = size;
}

//1���� �����Ϳ��� global/local memory���� Ȱ���Ͽ� ����.
void Reductor_GPU::test_1d(enum GPU_KERNEL kernel_num, int trial, size_t work_group_size_gpu)
{
	fprintf(stdout, "\n^^^ Test : Computing on OpenCL GPU 1D Device (%s)^^^\n",kernel_name[kernel_num]);

	printf_KernelWorkGroupInfo(preset[INDEX_GPU_1D].kernel[(int)kernel_num], preset[INDEX_GPU_1D].device);

	/* 1. Data �ű�� : Host -> Device */
	transfer(INDEX_GPU_1D);

	/* 2. Kernel ���� */

	cl_int errcode_ret;
	float compute_time;

	cl_kernel& kernel = preset[INDEX_GPU_1D].kernel[(int)kernel_num];

	int n = -1;
	while (work_group_size_gpu != 1 << (++n));

	size_t len;

	int count=trial;
	while (trial--)
	{
		if (kernel_num == gpu_reduction_global)
		{
			errcode_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &(preset[INDEX_GPU_1D].buffer_input));
			CHECK_ERROR_CODE(errcode_ret);
			errcode_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &(preset[INDEX_GPU_1D].buffer_output));
			CHECK_ERROR_CODE(errcode_ret);

			/* Execute the kernel on the device. */
			// Enqueues a command to execute a kernel on a device.
			CHECK_TIME_START;
			for (len = size; len >= work_group_size_gpu; len >>= n)
			{
				errcode_ret = clEnqueueNDRangeKernel(preset[INDEX_GPU_1D].cmd_queue, kernel, 1, NULL,
					&len, &work_group_size_gpu, 0, NULL, &event_for_timing);
				CHECK_ERROR_CODE(errcode_ret);
				clFinish(preset[INDEX_GPU_1D].cmd_queue);  // What would happen if this line is removed?
													   // or clWaitForEvents(1, &event_for_timing);
				clEnqueueCopyBuffer(preset[INDEX_GPU_1D].cmd_queue, preset[INDEX_GPU_1D].buffer_output, preset[INDEX_GPU_1D].buffer_input,
					0, 0, sizeof(float)*size, 0, NULL, NULL);
				clFinish(preset[INDEX_GPU_1D].cmd_queue);
			}
			

			/* 3. Data �ű�� - device -> host */
			receive(INDEX_GPU_1D);

			/* 4. Kernel ������ ���� �� CPU�� ������*/
			CPU_SUM(output, len);

		}


		else // (kernel_num == gpu_reduction_local)
		{
			errcode_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &(preset[INDEX_GPU_1D].buffer_input));
			CHECK_ERROR_CODE(errcode_ret);
			errcode_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &(preset[INDEX_GPU_1D].buffer_output));
			CHECK_ERROR_CODE(errcode_ret);
			errcode_ret = clSetKernelArg(kernel, 2, (sizeof(float) * work_group_size_gpu), NULL);
			CHECK_ERROR_CODE(errcode_ret);

			/* Execute the kernel on the device. */
			// Enqueues a command to execute a kernel on a device.
			CHECK_TIME_START;


			for (len = size; len >= work_group_size_gpu; len >>= n)
			{
				errcode_ret = clEnqueueNDRangeKernel(preset[INDEX_GPU_1D].cmd_queue, kernel, 1, NULL,
					&len, &work_group_size_gpu, 0, NULL, &event_for_timing);
				CHECK_ERROR_CODE(errcode_ret);
				clFinish(preset[INDEX_GPU_1D].cmd_queue);  // What would happen if this line is removed?
														   // or clWaitForEvents(1, &event_for_timing);			
				clEnqueueCopyBuffer(preset[INDEX_GPU_1D].cmd_queue, preset[INDEX_GPU_1D].buffer_output, preset[INDEX_GPU_1D].buffer_input,
					0, 0, sizeof(float)*size, 0, NULL, NULL);
				clFinish(preset[INDEX_GPU_1D].cmd_queue);
			}

			/* 3. Data �ű�� - device -> host */
			receive(INDEX_GPU_1D);

			/* 4. Kernel ������ ���� �� CPU�� ������*/
			CPU_SUM(output, len);
			
		}
	}
	clFinish(preset[INDEX_GPU_1D].cmd_queue);
	CHECK_TIME_END(compute_time);
	CHECK_ERROR_CODE(errcode_ret);

	fprintf(stdout, "     * Time by host clock = %.3fms\n\n", compute_time / (float)trial);

	print_device_time(event_for_timing);
	
	/* 4. ��� üũ */
	fprintf(stdout, "   [Check Results] \n");
	fprintf(stdout, "output[%d] = %f\n", 0, output[0]);
	
}
//2���� �����Ϳ��� global/local memory���� Ȱ���Ͽ� ����.
void Reductor_GPU::test_2d(enum GPU_KERNEL kernel_num, int trial, size_t global_width, size_t work_group_width, size_t work_group_height)
{
	size_t work_group_size[2];
	work_group_size[0] = work_group_width;
	work_group_size[1] = work_group_height;

	size_t global_size[2];
	global_size[0] = global_width;
	global_size[1] = this->size / global_width;

	fprintf(stdout, "\n^^^ Test : Computing on OpenCL GPU 2D Device (%s)^^^\n", kernel_name[kernel_num]);

	printf_KernelWorkGroupInfo(preset[INDEX_GPU_2D].kernel[(int)kernel_num], preset[INDEX_GPU_2D].device);

	/* 1. Data �ű�� : Host -> Device */
	transfer(INDEX_GPU_2D);

	/* 2. Kernel ���� */

	cl_int errcode_ret;
	float compute_time;

	cl_kernel& kernel = preset[INDEX_GPU_2D].kernel[(int)kernel_num];

	int n = -1, m = -1;
	while (work_group_size[0] != 1 << (++n));
	while (work_group_size[1] != 1 << (++m));
	
	size_t len[2];

	int count = trial;

	while (count--)
	{
		if (kernel_num == gpu_reduction_global)
		{
			errcode_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &(preset[INDEX_GPU_2D].buffer_input));
			CHECK_ERROR_CODE(errcode_ret);
			errcode_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &(preset[INDEX_GPU_2D].buffer_output));
			CHECK_ERROR_CODE(errcode_ret);

			/* Execute the kernel on the device. */
			// Enqueues a command to execute a kernel on a device.
			CHECK_TIME_START;
			for (len[0] = global_size[0], len[1] = global_size[1];
				len[0] >= work_group_size[0] && len[1] >= work_group_size[1];
				len[0] >>= n, len[1] >>= m)
			{
				errcode_ret = clEnqueueNDRangeKernel(preset[INDEX_GPU_2D].cmd_queue, kernel, 2, NULL,
					len, work_group_size, 0, NULL, &event_for_timing);
				CHECK_ERROR_CODE(errcode_ret);
				clFinish(preset[INDEX_GPU_2D].cmd_queue);  // What would happen if this line is removed?
														   // or clWaitForEvents(1, &event_for_timing);
				clEnqueueCopyBuffer(preset[INDEX_GPU_2D].cmd_queue, preset[INDEX_GPU_2D].buffer_output, preset[INDEX_GPU_2D].buffer_input,
					0, 0, sizeof(float)*size, 0, NULL, NULL);
				clFinish(preset[INDEX_GPU_2D].cmd_queue);
			}

			/* 3. Data �ű�� - device -> host */
			receive(INDEX_GPU_2D);

			/* 4. Kernel ������ ���� �� CPU�� ������*/
			CPU_SUM(output, len[0] * len[1]);

		}


		else // (kernel_num == gpu_reduction_local)
		{
			errcode_ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &(preset[INDEX_GPU_2D].buffer_input));
			CHECK_ERROR_CODE(errcode_ret);
			errcode_ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &(preset[INDEX_GPU_2D].buffer_output));
			CHECK_ERROR_CODE(errcode_ret);
			errcode_ret = clSetKernelArg(kernel, 2, sizeof(float) * work_group_size[0] * work_group_size[1], NULL);
			CHECK_ERROR_CODE(errcode_ret);

			/* Execute the kernel on the device. */
			// Enqueues a command to execute a kernel on a device.
			CHECK_TIME_START;
			for (len[0] = global_size[0], len[1] = global_size[1];
				len[0] >= work_group_size[0] && len[1] >= work_group_size[1];
				len[0] >>= n, len[1] >>= m)
			{
				errcode_ret = clEnqueueNDRangeKernel(preset[INDEX_GPU_2D].cmd_queue, kernel, 2, NULL,
					len, work_group_size, 0, NULL, &event_for_timing);
				CHECK_ERROR_CODE(errcode_ret);
				clFinish(preset[INDEX_GPU_2D].cmd_queue);  // What would happen if this line is removed?
														   // or clWaitForEvents(1, &event_for_timing);
				clEnqueueCopyBuffer(preset[INDEX_GPU_2D].cmd_queue, preset[INDEX_GPU_2D].buffer_output, preset[INDEX_GPU_2D].buffer_input,
					0, 0, sizeof(float)*size, 0, NULL, NULL);
				clFinish(preset[INDEX_GPU_2D].cmd_queue);
			}

			/* 3. Data �ű�� - device -> host */
			receive(INDEX_GPU_2D);

			/* 4. Kernel ������ ���� �� CPU�� ������*/
			CPU_SUM(output, len[0] * len[1]);
		}
	}
	clFinish(preset[INDEX_GPU_2D].cmd_queue);
	CHECK_TIME_END(compute_time);
	CHECK_ERROR_CODE(errcode_ret);

	fprintf(stdout, "     * Time by host clock = %.3fms\n\n", compute_time/(float)trial);

	print_device_time(event_for_timing);


	/* 4. ��� üũ */
	fprintf(stdout, "   [Check Results] \n");
	fprintf(stdout, "output[%d] = %f\n", 0, output[0]);

}

//1���� �����Ϳ��� CPU�� Ȱ���Ͽ� ����.
void Reductor_GPU::test_1d_cpu(enum GPU_KERNEL kernel_num, int trial, size_t work_grou_size_cpu)
{

}
void Reductor_GPU::test_2d_cpu(enum GPU_KERNEL kernel_num, int trial, size_t global_width, size_t work_group_width, size_t work_group_height)
{

}
