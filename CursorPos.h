
#pragma once

class CCursorPos
{
public:
	CCursorPos()
	{
		row = 0;
		column = 0;
		digit = 0;
	}

	bool operator == (CCursorPos const &x) const
	{
		return row == x.row && column == x.column && digit == x.digit;
	}

	bool operator != (CCursorPos const &x) const { return !(*this == x); }


public:
	int row;
	int column;
	int digit;
};
