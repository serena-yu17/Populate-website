#pragma once
#include "stdafx.h"


namespace std
{
	template<>
	struct hash<pair<int, int>>
	{
		size_t operator()(pair<int, int> const& pair) const
		{
			return (pair.first << 16) | pair.second;
		}
	};
}
