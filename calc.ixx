//import main;

export module calc;

constexpr float PI = 3.1415927f;

export inline float RadToDeg(float radian)
{
	return radian * (180 / PI);
}

export inline float DegToRad(float degree)
{
	return degree * (PI / 180);
}