#pragma once

using std::string;
using std::deque;

class sound
{
public:
	sound(void);
	~sound(void);

	bool init(void);
	void playfile(string fspec);

private:
	bool is_initialized;
	deque<DWORD> channels;
};

