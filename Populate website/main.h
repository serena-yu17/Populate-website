#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unordered_map>

#include "ctpl_stl.h"
#import "C:\Program Files\Common Files\system\ado\msado15.dll" no_namespace rename("EOF", "EndOfFile")

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
