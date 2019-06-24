#include "stdafx.h"

int main(int argc, char** argv)
{
	int32 rtn;
	::testing::InitGoogleTest(&argc, argv);
	rtn = RUN_ALL_TESTS();

	return rtn;
}