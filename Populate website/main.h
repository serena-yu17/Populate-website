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

void readFile(char* path, std::unordered_map<std::string, int>& geneid, std::unordered_map<std::pair<int, int>, int>& record);
void populateDB(std::unordered_map<std::string, int>& geneid, std::unordered_map<std::pair<int, int>, int>& record);
void insertID(int tid, std::pair<std::string, int> const& gene, _ConnectionPtr& pConn);
void insertRecord(int tid, std::pair<std::pair<int, int>, int> const& item, _ConnectionPtr& pConn);