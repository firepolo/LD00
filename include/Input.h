#ifndef INPUTS_H
#define INPUTS_H

#define MAX_KEYS 128

struct Input
{
	static bool *KEYBOARD;
};

bool *Input::KEYBOARD = 0;

#endif