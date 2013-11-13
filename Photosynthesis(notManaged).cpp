// Photosynthesis(notManaged).cpp : Defines the entry point for the console application.
// This program demonstrates how to call the photosynthesis module
// it reads two files
//   1. a parameter file for a particular plant species
//   2. an environmental file with temperature, radiation, CO2 content, humidity, wind
//       and a flag (0,1) to tell the program if constant temperature is used (or let temperature of leaf
//       vary with stomatal conductance. 
//

#include "stdafx.h"
#include "gas_exchange.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;
// using std::string is a more generic method
using namespace photomod;


int _tmain(int argc, _TCHAR* argv[])
{    
	string DataLine; //!< holds line of data read from file
	string Species ="maize"; //!< Species of plant for calculatioins
	string Remark; //!< from parameter file
	char* context = NULL; //!< needed to use strtok_s functin
	double PFD, Temperature, RelativeHumidity,Wind, CO2, Pressure=100;
	bool ConstantTemperature; //!< parameters sent to photosynthesis module
	// photomod::CGasExchange::tParms *thisParms;
	photomod::CGasExchange::tParms thisParms; //!< declare a photosynthesis object
	const char *token=","; //!< pointer to character that separates entries in the parameter file
	char * pnt; //!< pointer to the next word to be read from parameter file
	bool test; 
	char CharTest ='A'; //!< variable to test if there are characters in the line of data (indicates end of data)
	bool found=false; //!< Boolean to indicate the species line was found in the parameter file
	//char *dup; // need to copy string objects to a char* in order to use strtok_s()
	string temp ; //variable for holding string objects

	ifstream ParamFile, DataFile;  //!< files to hold parameters and variables for a single run
	ofstream OutputFile;            //!<holds data output from photosynthesis module
	string DataFileName="ClimateIn.dat", OutputFileName="Results.dat";
	// Get datafile name, and species name
	cout << "enter name of file with input data and species name separated by commas:" <<endl
		<< "hit Enter with empty string for defaults" <<endl;
	getline(cin,DataLine);
	if (!DataLine.empty())
	{

		pnt=strtok_s((char*)DataLine.c_str(), token, &context );  //pnt is a pointer to the last recently found token in the string
		DataFileName.assign(pnt);
		pnt=strtok_s(NULL, token, &context );  //pnt is a pointer to the last recently found token in the string
		Species.assign(pnt);
		Species.erase(remove(Species.begin(),Species.end(),' '),Species.end());
		pnt=NULL; //finished with these two
	}
	// open file with parameters and data
	ParamFile.open("parameters.csv");
	DataFile.open(DataFileName.c_str());
	FILE * pFile;
	pFile=fopen((char*)OutputFileName.c_str(),"w");
	fprintf(pFile, "ANet     AGross    VPD  Leaf Temp   BoundaryL conduc  InternalCO2  Respiration  StomatalConduct \n");
	OutputFile.open(OutputFileName.c_str());
	std::getline(ParamFile,DataLine); //get header from parameter file
    // last line of file may be empty or contain numbers, this indicates file is at the  end
	while (!ParamFile.eof() && isalpha(CharTest))
	{
		int test=DataLine.size();
		getline(ParamFile,DataLine); // get the first line of data 
		CharTest=DataLine.at(0);     //check that line contains alphabetical text at beginning
		pnt=strtok_s((char*)DataLine.c_str(), token, &context ); // pick off the first word before the token (',')

		//First read file to find the desired plant species
		temp.assign(pnt);
		temp.erase(remove(temp.begin(),temp.end(),' '),temp.end()); // removes spaces from string
		thisParms.ID=temp;
		temp.clear();



		//Eliminate case issues - convert everything to lower case
		transform(thisParms.ID.begin(), thisParms.ID.end(),thisParms.ID.begin(), ::tolower);
		transform(Species.begin(), Species.end(),Species.begin(), ::tolower);

		//Search for the correct species
		if (thisParms.ID.compare(Species)==0 && !found)  //continue to parse string
		{

			pnt = strtok_s( NULL,token, &context ); // iterate to clean string of characters already read
			// This section parses string, each iteration it cleans string of characters already read
			while(pnt!=NULL ) 
			{
				// printf( "Tokenized string using *  is:: %s\n", pnt );

				temp.assign(pnt);
				thisParms.species=temp;
				temp.clear();
				pnt = strtok_s( NULL,token, &context ); 
				temp.assign(pnt);
				thisParms.Type=temp;
				temp.clear();
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Vcm25=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Jm25=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Vpm25=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.TPU25=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Rd25=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Theta=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.EaVc=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Eaj=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Hj=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Sj=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Hv=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Sv=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Eap=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.Ear=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.g0=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.g1=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.stomaRatio=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.LfWidth=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				thisParms.LfAngFact=atof(pnt);
				pnt = strtok_s( NULL,token, &context ); 
				temp.assign(pnt);
				thisParms.Remark=temp;
				temp.clear();
				pnt=strtok_s(NULL,token, &context);
				found=true;
			}

		}
	}
	// Implementation to interact with photosynthesis model is here
	CharTest='1';
	int start=1;
	bool LineEmpty=false;
	//output variables
	double Anet, VPD,Agross,LeafTemperature, Respiration, InternalCO2, StomatalConductance,
		BoundaryLayerConductance;
	// Create a new GasExchange Object
	photomod::CGasExchange *MyLeafGas;
	MyLeafGas= new CGasExchange();
	//Pass the plant species paramters read before
	MyLeafGas->SetParams(&thisParms);
	// loop to read environmental input data and calculate output
	while (!LineEmpty)
		
	{
	     getline(DataFile, DataLine);
		if (DataLine.length()==0) LineEmpty=true;
		
	else
	{
	
		CharTest=DataLine.at(0);
		pnt=strtok_s((char*)DataLine.c_str(), token, &context ); 
		PFD=atof(pnt);
		pnt=strtok_s(NULL,token,&context);
		Temperature=atof(pnt);
		pnt=strtok_s(NULL,token,&context);
		CO2=atof(pnt);
		pnt=strtok_s(NULL,token,&context);
		RelativeHumidity=atof(pnt);
		pnt=strtok_s(NULL,token,&context);
		Wind=atof(pnt);
		pnt=strtok_s(NULL,token,&context);
		ConstantTemperature=atoi(pnt);
		MyLeafGas->SetVal(PFD, Temperature, CO2, RelativeHumidity, 
			Wind, Pressure, ConstantTemperature); 
		//MyLeafGas =new photomod::CGasExchange;
		Anet=MyLeafGas->get_ANet();
		Agross=MyLeafGas->get_AGross();
		VPD=MyLeafGas->get_VPD();
		LeafTemperature=MyLeafGas->get_LeafTemperature();
		BoundaryLayerConductance=MyLeafGas->get_BoundaryLayerConductance();
		InternalCO2=MyLeafGas->get_Ci();
		Respiration=MyLeafGas->get_Respiration();
		StomatalConductance=MyLeafGas->get_StomatalConductance();



		fprintf(pFile,"%.2f   %.2f    %.3f      %4.1f      %.3f              %4.1f       %.2f     %.3f \n", Anet, Agross, VPD, 
			LeafTemperature, BoundaryLayerConductance,InternalCO2,Respiration, 
			StomatalConductance);
	}	
	}
	DataFile.close();
	DataFile.close();
	fclose(pFile);
	return 0;
}

