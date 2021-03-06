// Populate website.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"	
#include "main.h"

using namespace std;

/*
Schema:

create table GeneID(
	ID integer primary key,
	Symbol varchar(50) not null
);

create table Reactomes(
	Gene1 integer,
	Gene2 integer,
	Confidence integer not null,
	primary key(gene1,gene2)
);

create table Ref(
	Gene1 integer,
	Gene2 integer,
	RefID integer not null,
	Author varchar(200) not null,
	foreign key(gene1,gene2) references Reactomes(gene1,gene2),
	primary key(gene1,gene2, RefID)
);

create table Synon(
	Symbol varchar(50) primary key,
	ID integer not null
);

*/

void readFile(char* path, unordered_map<string, int>& geneid, unordered_map<int, string>& idgene, unordered_map<pair<int, int>, int>& record, unordered_map<pair<int, int>, unordered_map<string, string>>& references)
{
	FILE* fp;
	char line[5000];
	fp = fopen(path, "r");
	if (!fp)
	{
		cout << "Cannot open file" << path << endl;
		return;
	}
	const char ht = (char)9;
	size_t maxId = 0;
	while (fgets(line, 5000, fp) != NULL)
	{
		if (line[0] == '#')
			continue;
		string splitLine[17];
		int p = 0;
		int i = 0;
		while (line[i] && i < 5000 && p < 17)
		{
			string section;
			while (line[i] && i < 5000 && line[i] != ht)
			{
				section.push_back(line[i]);
				i++;
			}
			splitLine[p++] = section;
			i++;
		}
		if (splitLine[15] != "9606" || splitLine[16] != "9606")
			continue;
		int id1 = stoi(splitLine[1]);
		int id2 = stoi(splitLine[2]);
		if (id1 > maxId)
			maxId = id1;
		if (id2 > maxId)
			maxId = id2;
		for (auto& ch : splitLine[7])
			ch = toupper(ch);
		for (auto& ch : splitLine[8])
			ch = toupper(ch);
		string* symbol1 = &splitLine[7];
		string* symbol2 = &splitLine[8];
		string* syn1 = &(splitLine[9]);
		string* syn2 = &(splitLine[10]);
		vector<string> synonym1, synonym2;
		i = 0;
		if ((*syn1)[0] != '-')
		{
			while (i < syn1->size())
			{
				string sym;
				while (i < syn1->size() && (*syn1)[i] != '|')
				{
					sym.push_back(toupper((*syn1)[i]));
					i++;
				}
				if (sym.size())
					synonym1.push_back(sym);
				i++;
			}
		}
		i = 0;
		if ((*syn2)[0] != '-')
		{
			while (i < syn2->size())
			{
				string sym;
				while (i < syn2->size() && (*syn2)[i] != '|')
				{
					sym.push_back(toupper((*syn2)[i]));
					i++;
				}
				if (sym.size())
					synonym2.push_back(sym);
				i++;
			}
		}
		pair<int, int> genepair = make_pair(id1, id2);
		geneid[*symbol1] = id1;
		geneid[*symbol2] = id2;
		idgene[id1] = *symbol1;
		idgene[id2] = *symbol2;
		string refStr;
		for (auto ch : splitLine[13])
		{
			if (ch == 39)
				refStr.push_back(ch);
			refStr.push_back(ch);
		}
		if (record.find(genepair) == record.end())
			record[genepair] = 0;
		record[genepair]++;
		if (references.find(genepair) == references.end())
			references[genepair] = unordered_map<string, string>();
		references[genepair][splitLine[14]] = refStr;
		for (string& sym : synonym1)
			geneid[sym] = id1;
		for (string& sym : synonym2)
			geneid[sym] = id2;
	}
	cout << "Max Gene ID: " << maxId << '\n';
}

void insertSynon(pair<string, int> const& gene, _ConnectionPtr& pConn)
{
	_bstr_t cmdStr = _bstr_t("INSERT INTO Synon VALUES(") + "'" + _bstr_t(gene.first.c_str()) + "', " + _bstr_t(gene.second) + ");";
	pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
}

void insertRecord(pair<pair<int, int>, int> const& item, _ConnectionPtr& pConn)
{
	_bstr_t cmdStr = _bstr_t("INSERT INTO Reactomes VALUES(") + _bstr_t(item.first.first) + ", " + _bstr_t(item.first.second) + ", " + _bstr_t(item.second) + ");";
	pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
}

void insertRef(pair<pair<int, int>, unordered_map<string, string>> const& refItem, _ConnectionPtr& pConn)
{
	_bstr_t baseStr = _bstr_t("INSERT INTO Ref (gene1,gene2,refID,author) VALUES(") + _bstr_t(refItem.first.first) + ", " + _bstr_t(refItem.first.second) + ", ";
	for (auto& entry : refItem.second)
	{
		_bstr_t  cmdStr = baseStr + _bstr_t(entry.first.c_str()) + ", '" + _bstr_t(entry.second.c_str()) + "');";
		pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
	}
}

void insertID(pair<int, string> const& gene, _ConnectionPtr& pConn)
{
	_bstr_t cmdStr = _bstr_t("INSERT INTO GeneID VALUES(") + _bstr_t(gene.first)  + ", '" + _bstr_t(gene.second.c_str()) + "');";
	pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
}


void populateDB(unordered_map<string, int>& geneid, unordered_map<int, string>& idgene, unordered_map<pair<int, int>, int>& record, unordered_map<pair<int, int>, unordered_map<string, string>>& references)
{
	HRESULT hr = S_OK;
	_ConnectionPtr pConn = NULL;
	_CommandPtr pMD;
	const _bstr_t strCon("Provider=SQLNCLI11; Data Source='(localdb)\\MSSQLLocalDB'; Initial Catalog='ReactomeDB'; Integrated Security=SSPI;");
	_RecordsetPtr pDB = NULL;
	CoInitialize(NULL);
	try {
		hr = pConn.CreateInstance((__uuidof(Connection)));
		if (FAILED(hr)) {
			cout << "Error instantiating Connection object\n";
			CoUninitialize();
			return;
		}
		hr = pConn->Open(strCon, "", "", 0);
		if (FAILED(hr)) {
			printf("Error Opening Database objectn");
			CoUninitialize();
			return;
		}
		pMD.CreateInstance(__uuidof(Command));
		pMD->ActiveConnection = pConn;
		pMD->CommandType = adCmdText;
		pConn->Execute("delete from Ref", NULL, adExecuteNoRecords);
		pConn->Execute("delete from Reactomes", NULL, adExecuteNoRecords);
		pConn->Execute("delete from GeneID", NULL, adExecuteNoRecords);
		pConn->Execute("delete from Synon", NULL, adExecuteNoRecords);
		for (auto& key : idgene)
			insertID(key, pConn);
		for (auto& key : record)
			insertRecord(key, pConn);
		for (auto& key : references)
			insertRef(key, pConn);
		for (auto& key : geneid)
			insertSynon(key, pConn);
	}
	catch (_com_error &ce) {
		cout << "Error:" << ce.Description() << '\n';
	}
	pConn->Close();
	CoUninitialize();
}


int main()
{
	ios::sync_with_stdio(false);
	cout << "Running\n";
	unordered_map<string, int> geneid;
	unordered_map<int, string> idgene;
	unordered_map<pair<int, int>, int> record;
	unordered_map<pair<int, int>, unordered_map<string, string>> references;
	readFile("C:\\OneDrive\\projects\\Web\\BIOGRID.txt", geneid, idgene, record, references);
	populateDB(geneid, idgene, record, references);
	cout << "Done.";
	getchar();
}

