#pragma once

class HandleInput
{
public:
	HandleInput(int key);
	int GetKey();
	bool WasReleased();
	bool _mWasPressed;
private:
	int _mKey;
	
};
