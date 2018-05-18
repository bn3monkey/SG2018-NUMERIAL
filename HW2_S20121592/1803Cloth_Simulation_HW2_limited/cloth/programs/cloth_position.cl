enum direct_part
{
	CENTER,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	LEFTUP,
	LEFTDOWN,
	RIGHTUP,
	RIGHTDOWN,
};

inline int direct(int i, enum direct_part d, int NUM_PARTICLES_X, int NUM_PARTICLES_Y)
{
	int x, y;
	y = i / NUM_PARTICLES_X;
	x = i - y * NUM_PARTICLES_X;

	switch (d)
	{
	case CENTER: break;
	case LEFT: x -= 1; break;
	case RIGHT: x += 1; break;
	case UP: y -= 1; break;
	case DOWN: y += 1; break;
	case LEFTUP: x -= 1; y -= 1; break;
	case LEFTDOWN:x -= 1; y += 1; break;
	case RIGHTUP: x += 1; y -= 1; break;
	case RIGHTDOWN: x += 1; y += 1; break;
	}

	if (x < 0 || x >= NUM_PARTICLES_X || y < 0 || y >= NUM_PARTICLES_Y)
		return -1;
	else
		return y * NUM_PARTICLES_X + x;
}

inline float4 getFs(int i, __global float4* pos_in, float SpringK, float RestLengthHoriz, float RestLengthVert, float RestLengthDiag, float NUM_PARTICLES_X, float NUM_PARTICLES_Y)
{
	int idx;
	float4 diff;
	float distance;
	//����ϱ� �� Fs�� Clear
	float4 Fs = (float4)(0);

	idx = direct(i, LEFT, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);

		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);

		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthHoriz)) * diff;
	}
	idx = direct(i, DOWN, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);

		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);

		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthVert)) * diff;
	}

	idx = direct(i, RIGHT, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);
		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);
		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthHoriz)) * diff;
	}
	idx = direct(i, UP, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�s
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);
		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);
		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthVert)) * diff;
	}

	idx = direct(i, RIGHTDOWN, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);
		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);
		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthDiag)) * diff;
	}
	idx = direct(i, LEFTDOWN, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);
		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);
		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthDiag)) * diff;
	}
	idx = direct(i, LEFTUP, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);
		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);
		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthDiag)) * diff;
	}
	idx = direct(i, RIGHTUP, NUM_PARTICLES_X, NUM_PARTICLES_Y);
	if (idx >= 0)
	{
		//Fs�� ������� �����ش�.
		// diff = r ���ϱ�
		diff = pos_in[idx] - pos_in[i];
		// distance = |r| 
		distance = fast_length(diff.xyz);
		// diff = r/|r|
		diff = (float4)(normalize(diff.xyz), 0.0f);
		// F = K *(|r| - R)*(r/|r|)
		Fs += (SpringK * (distance - RestLengthDiag)) * diff;
	}

	return Fs;
}

__kernel
void cloth_position_global_euler(
    __global float4* pos_in, __global float4* pos_out,
    __global float4* vel_in, __global float4* vel_out,
    __local float4* local_data,
    float3 Gravity,
    float ParticleMass,
    float ParticleInvMass,
    float SpringK,
    float RestLengthHoriz,
    float RestLengthVert,
    float RestLengthDiag,
    float DeltaT,
    float DampingConst) {

	int x = get_global_id(0);
	int y = get_global_id(1);
	int width = get_global_size(0);
	int height = get_global_size(1);
    int idx = x + width * y;
	float4 Fs, Fg, Fd, a;

	/*********************** Fs ��� ****************************/
	Fs = getFs(idx, pos_in, SpringK, RestLengthHoriz, RestLengthVert, RestLengthDiag, width, height);

	/*********************** Fg ��� ****************************/
	Fg = ParticleMass * (float4)(Gravity, 0.0f);

	/*********************** Fd ��� ****************************/
	Fd = -DampingConst * vel_in[idx];

	/*********************** ���� �� ��� ****************************/
	Fs = Fs + Fg + Fd;

	/*********************** ���ӵ� ��� ****************************/
	a = Fs * ParticleInvMass;

	/*********************** First Order ��� ****************************/
	vel_out[idx] = vel_in[idx] + a * DeltaT;
	pos_out[idx] = pos_in[idx] + vel_in[idx] * DeltaT;


	if (y == height -1)
	{
		int span = (width >> 2);
		int span2 = span + span;
		int span3 = span + span2;
		int span4 = width - 1;
		
		if (x == 0 || x == span || x == span2 || x == span3 || x == span4)
		{
			vel_out[idx] = vel_in[idx];
			pos_out[idx] = pos_in[idx];
		}
	}

}

__kernel
void cloth_position_global_cookbook(
	__global float4* pos_in, __global float4* pos_out,
	__global float4* vel_in, __global float4* vel_out,
	__local float4* local_data,
	float3 Gravity,
	float ParticleMass,
	float ParticleInvMass,
	float SpringK,
	float RestLengthHoriz,
	float RestLengthVert,
	float RestLengthDiag,
	float DeltaT,
	float DampingConst) {

	int x = get_global_id(0);
	int y = get_global_id(1);
	int width = get_global_size(0);
	int height = get_global_size(1);
	int idx = x + width * y;
	float4 Fs, Fg, Fd, a;

	/*********************** Fs ��� ****************************/
	Fs = getFs(idx, pos_in, SpringK, RestLengthHoriz, RestLengthVert, RestLengthDiag, width, height);

	/*********************** Fg ��� ****************************/
	Fg = ParticleMass * (float4)(Gravity, 0.0f);

	/*********************** Fd ��� ****************************/
	Fd = -DampingConst * vel_in[idx];

	/*********************** ���� �� ��� ****************************/
	Fs = Fs + Fg + Fd;

	/*********************** ���ӵ� ��� ****************************/
	a = Fs * ParticleInvMass;

	/*********************** First Order ��� ****************************/
	vel_out[idx] = vel_in[idx] + a * DeltaT;
	pos_out[idx] = pos_in[idx] + DeltaT * (vel_in[idx] + 0.5f * DeltaT * a);


	if (y == height - 1)
	{
		int span = (width >> 2);
		int span2 = span + span;
		int span3 = span + span2;
		int span4 = width - 1;

		if (x == 0 || x == span || x == span2 || x == span3 || x == span4)
		{
			vel_out[idx] = vel_in[idx];
			pos_out[idx] = pos_in[idx];
		}
	}

}

__kernel
void cloth_position_global_modified(
	__global float4* pos_in, __global float4* pos_out,
	__global float4* vel_in, __global float4* vel_out,
	__local float4* local_data,
	float3 Gravity,
	float ParticleMass,
	float ParticleInvMass,
	float SpringK,
	float RestLengthHoriz,
	float RestLengthVert,
	float RestLengthDiag,
	float DeltaT,
	float DampingConst) {

	int x = get_global_id(0);
	int y = get_global_id(1);
	int width = get_global_size(0);
	int height = get_global_size(1);
	int idx = x + width * y;
	float4 Fs, Fg, Fd, a;

	/*********************** Fs ��� ****************************/
	Fs = getFs(idx, pos_in, SpringK, RestLengthHoriz, RestLengthVert, RestLengthDiag, width, height);

	/*********************** Fg ��� ****************************/
	Fg = ParticleMass * (float4)(Gravity, 0.0f);

	/*********************** Fd ��� ****************************/
	Fd = -DampingConst * vel_in[idx];

	/*********************** ���� �� ��� ****************************/
	Fs = Fs + Fg + Fd;

	/*********************** ���ӵ� ��� ****************************/
	a = Fs * ParticleInvMass;

	/*********************** First Order ��� ****************************/
	vel_out[idx] = vel_in[idx] + a * DeltaT;
	pos_out[idx] = pos_in[idx] + vel_in[idx] * DeltaT;


	if (y == height - 1)
	{
		int span = (width >> 2);
		int span2 = span + span;
		int span3 = span + span2;
		int span4 = width - 1;

		if (x == 0 || x == span || x == span2 || x == span3 || x == span4)
		{
			vel_out[idx] = vel_in[idx];
			pos_out[idx] = pos_in[idx];
		}
	}

}