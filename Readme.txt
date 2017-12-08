This program is the data analysis component of the website http://www.iprotein.info 
It analyses a local data file (~300k records) and populate a local SQL Server database using the genome data obtained. After offline testing, the databased will then be deployed online to Azure through the MS SQL migration tool.
It was written in C++ using MS ADO to operate the database. Much of its code may be reusable.

