// Populate website.cpp : Defines the entry point for the console application.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

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
*/

void readFile(char* path, unordered_map<string, int>& geneid, unordered_map<pair<int, int>, int>& record, unordered_map<pair<int, int>, unordered_map<string, string>>& references)
{
	FILE* fp;
	char line[2048];
	fp = fopen(path, "r");
	if (!fp)
	{
		cout << "Cannot open file" << path << endl;
		return;
	}
	int genecount = 0;
	const char ht = (char)9;
	while (fgets(line, 2048, fp) != NULL)
	{
		string gene1, gene2, species1, species2, author, refID;		  //string buffers
		if (line[0] != '#')
		{
			int i = 0;
			int TabCount = 0;
			while (line[i] && TabCount < 7)
			{
				if (line[i] == ht)
					TabCount++;
				i++;
			}
			while (line[i] && line[i] != ht)
			{
				gene1.push_back(line[i]);
				i++;
			}

			i++;
			while (line[i] && line[i] != ht)
			{
				gene2.push_back(line[i]);
				i++;
			}
			i++;
			if (gene1[0] != '\0' && gene1[0] != '-' && gene2[0] != '\0' && gene2[0] != '-')
			{
				TabCount = 8;
				while (line[i] && TabCount < 12)
				{
					if (line[i] == ht)
						TabCount++;
					i++;
				}
				while (line[i] && line[i] != ht)
				{
					if (line[i] == '\'')
						line[i] = ' ';
					author.push_back(line[i]);
					i++;
				}
				i++;
				while (line[i] && line[i] != ht)
				{
					refID.push_back(line[i]);
					i++;
				}
				i++;
				while (line[i] && line[i] != ht)
				{
					species1.push_back(line[i]);
					i++;
				}
				i++;
				while (line[i] && line[i] != ht)
				{
					species2.push_back(line[i]);
					i++;
				}
				if (species1 == "9606" && species2 == "9606" && gene1 != gene2)					 //9606: human
				{
					int id1, id2;
					if (geneid.find(gene1) == geneid.end())
					{
						geneid[gene1] = genecount;
						id1 = genecount;
						genecount++;
					}
					else
						id1 = geneid[gene1];
					if (geneid.find(gene2) == geneid.end())
					{
						geneid[gene2] = genecount;
						id2 = genecount;
						genecount++;
					}
					else
						id2 = geneid[gene2];
					auto genepair = make_pair(id1, id2);
					if (record.find(genepair) == record.end())
						record[genepair] = 1;
					else
						record[genepair]++;
					if (references.find(genepair) == references.end())
						references[genepair] = unordered_map<string, string>();
					references[genepair][refID] = author;
				}
			}
		}
	}
}

void insertID(pair<string, int> const& gene, _ConnectionPtr& pConn)
{
	_bstr_t cmdStr = "INSERT INTO GeneID VALUES(" + _bstr_t(gene.second) + ", '" + _bstr_t(gene.first.c_str()) + "');";
	pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
}

void insertRecord(pair<pair<int, int>, int> const& item, _ConnectionPtr& pConn)
{
	_bstr_t cmdStr = "INSERT INTO Reactomes VALUES(" + _bstr_t(item.first.first) + ", " + _bstr_t(item.first.second) + ", " + _bstr_t(item.second) + ");";
	pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
}

void insertRef(pair<pair<int, int>, unordered_map<string, string>> const& refItem, _ConnectionPtr& pConn)
{
	_bstr_t baseStr = "INSERT INTO Reference (gene1,gene2,refID,author) VALUES(" + _bstr_t(refItem.first.first) + ", " + _bstr_t(refItem.first.second) + ", ";
	for (auto& entry : refItem.second)
	{
		_bstr_t  cmdStr = baseStr + _bstr_t(entry.first.c_str()) + ", '" + _bstr_t(entry.second.c_str()) + "');";
		pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
	}
}

void populateDB(unordered_map<string, int>& geneid, unordered_map<pair<int, int>, int>& record, unordered_map<pair<int, int>, unordered_map<string, string>>& references)
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
		pConn->Execute("delete from GeneID", NULL, adExecuteNoRecords);
		pConn->Execute("delete from Reference", NULL, adExecuteNoRecords);
		pConn->Execute("delete from Reactomes", NULL, adExecuteNoRecords);
		for (auto& key : geneid)
			insertID(key, pConn);
		for (auto& key : record)
			insertRecord(key, pConn);
		for (auto& key : references)
			insertRef(key, pConn);
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
	unordered_map<pair<int, int>, int> record;
	unordered_map<pair<int, int>, unordered_map<string, string>> references;
	readFile("C:\\OneDrive\\projects\\Web\\BIOGRID.txt", geneid, record, references);
	populateDB(geneid, record, references);
	cout << "Done.";
	getchar();
}

