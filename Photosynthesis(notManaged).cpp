/*! \class _tmain 
* interface to photosynthesis module
* \par
*  This program demonstrates how to call the photosynthesis module
*  Photosynthesis(notManaged).cpp : Defines the entry point for the console application.
*
* \par Use of this interface
* Two files are needed:
*    1. a parameter file containing the parameters for the photosynthesis module, one line for each species
*    2. an environmental file with temperature, radiation, CO2 content, humidity, wind
*       and a flag (0,1) to tell the program if constant temperature is used (or let temperature of leaf
*       vary with stomatal conductance. 
*
*/
  

#include "stdafx.h"
#include "gas_exchange.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;
// uses std::string, a more generic method than CString
using namespace photomod; //Photosynthesis module

//* @author Dennis Timlin
int _tmain(int argc, _TCHAR* argv[]) 
{    
	string DataLine;         //!< holds line of data read from file
	string Remark;           //!< from parameter file
	char* context = NULL;    //!< needed to use strtok_s function
	double PFD, Temperature, RelativeHumidity,Wind, CO2, Pressure=100;
	bool ConstantTemperature; //!< parameters sent to photosynthesis module
	// photomod::CGasExchange::tParms *thisParms;
	photomod::CGasExchange::tParms thisParms; //!< declare a photosynthesis object
	const char *delim=",";    //!< pointer to character (delimiter) that separates entries in the parameter file
	char * pnt;               //!< pointer to the next word to be read from parameter file
	char CharTest ='A';       //!< variable to test if there are characters in the line of data (indicates end of data)
	bool found=false;         //!< Boolean to indicate the species line was found in the parameter file
	//char *dup;              // need to copy string objects to a char* in order to use strtok_s()
	string temp ;             //!< temporary variable for holding string objects

	ifstream ParamFile, DataFile;  //!< files to hold parameters and variables for a single run
	ofstream OutputFile;            //!<holds data output from photosynthesis module

	//variables to hold results from gas exchange module.
	double Anet, VPD,Agross,LeafTemperature, Respiration, InternalCO2, StomatalConductance,
		BoundaryLayerConductance;


//assign defaults in case user does not want to enter other information (DataLine is empty)
	string DataFileName="ClimateIn.dat", OutputFileName="Results.dat"; 
	string Species ="maize"; //!< Species of plant for calculatioins

	// Get datafile name, and species name if entered by user.
	cout << "enter name of file with input data and species name separated by commas:" <<endl
		<< "hit Enter with empty string for defaults" <<endl;
	getline(cin,DataLine);
	if (!DataLine.empty()) //if empty defaults to string names assigned above
	{

		pnt=strtok_s((char*)DataLine.c_str(), delim, &context );  //pnt is a pointer to the last recently found token in the string
		DataFileName.assign(pnt);                                // first token is a file name
		pnt=strtok_s(NULL, delim, &context );                    //get ready for the next token by getting pointer to recently found token in the string
		Species.assign(pnt);                                     // next token is species
		Species.erase(remove(Species.begin(),Species.end(),' '),Species.end()); //remove blanks from species name in case any are present
		pnt=NULL; //finished with these two
	}
	// open file with parameters and data
	ParamFile.open("parameters.csv");
	DataFile.open(DataFileName.c_str());
	FILE * pFile;
	pFile=fopen((char*)OutputFileName.c_str(),"w");
	fprintf(pFile, "ANet     AGross    VPD  Leaf Temp   BoundaryL_Conduc  InternalCO2  Respiration  StomatalConduct \n");
	OutputFile.open(OutputFileName.c_str());
	std::getline(ParamFile,DataLine); //get header from parameter file
    // last line of file may be empty or contain numbers, this indicates file is at the  end
	while (!ParamFile.eof() && isalpha(CharTest))  //loops through file reads each line and tests if the correct species is found
	{
		getline(ParamFile,DataLine); // get the first line of data 
		CharTest=DataLine.at(0);     //check that line contains alphabetical text at beginning
		pnt=strtok_s((char*)DataLine.c_str(), delim, &context ); // pick off the first word before the token (',')

		//First read file to find the desired plant species
		temp.assign(pnt);
		temp.erase(remove(temp.begin(),temp.end(),' '),temp.end()); // removes spaces from string
		thisParms.ID=temp;
		temp.clear();



		//Eliminate case issues - convert everything to lower case
		transform(thisParms.ID.begin(), thisParms.ID.end(),thisParms.ID.begin(), ::tolower);
		transform(Species.begin(), Species.end(),Species.begin(), ::tolower);

		//Search for the correct species in file
		if (thisParms.ID.compare(Species)==0 && !found)  //continue to parse string 
		{

			pnt = strtok_s( NULL,delim, &context ); // iterate to clean string of characters already read
			// This section parses string, each iteration it cleans string of characters already read
			while(pnt!=NULL ) 
			{
				// printf( "Tokenized string using *  is:: %s\n", pnt ); // for debugging

				temp.assign(pnt);
				thisParms.species=temp;
				temp.clear();
				pnt = strtok_s( NULL,delim, &context ); 
				temp.assign(pnt);
				thisParms.Type=temp;
				temp.clear();
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Vcm25=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Jm25=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Vpm25=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.TPU25=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Rd25=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Theta=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.EaVc=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Eaj=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Hj=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Sj=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Hv=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Sv=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Eap=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.Ear=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.g0=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.g1=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.stomaRatio=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.LfWidth=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				thisParms.LfAngFact=atof(pnt);
				pnt = strtok_s( NULL,delim, &context ); 
				temp.assign(pnt);
				thisParms.Remark=temp;
				temp.clear();
				pnt=strtok_s(NULL,delim, &context);
				found=true;
			}

		}
	}
	// Implementation to interact with photosynthesis model is here
	CharTest='1'; //initialize CharTest
	int start=1;
	bool LineEmpty=false; //Checks if file with environmental data is finished.
	//output variables
	//Define a pointer to a new variable as a GasExchange Object
	photomod::CGasExchange *MyLeafGasEx;
	// Create a new GasExchange Object
	MyLeafGasEx= new CGasExchange();
	//Initialize Gas exchange object by passing species and relavent parameters read earlier
	MyLeafGasEx->SetParams(&thisParms);
	// loop to read environmental input data and call object to calculate results
	while (!LineEmpty)
		
	{
	     getline(DataFile, DataLine);
		if (DataLine.length()==0) LineEmpty=true;
		
	else
	{
	
		//CharTest=DataLine.at(0); // check for valid data
		pnt=strtok_s((char*)DataLine.c_str(), delim, &context );  //token is the delimiter
		PFD=atof(pnt);
		pnt=strtok_s(NULL,delim,&context);
		Temperature=atof(pnt);
		pnt=strtok_s(NULL,delim,&context);
		CO2=atof(pnt);
		pnt=strtok_s(NULL,delim,&context);
		RelativeHumidity=atof(pnt);
		pnt=strtok_s(NULL,delim,&context);
		Wind=atof(pnt);
		pnt=strtok_s(NULL,delim,&context);
		ConstantTemperature=atoi(pnt);
		// pass relavent environmental variables to gas exchange object and execute module
		MyLeafGasEx->SetVal(PFD, Temperature, CO2, RelativeHumidity, 
			Wind, Pressure, ConstantTemperature); 
		// Return calculated variables from gas exchange object
		Anet=MyLeafGasEx->get_ANet();
		Agross=MyLeafGasEx->get_AGross();
		VPD=MyLeafGasEx->get_VPD();
		LeafTemperature=MyLeafGasEx->get_LeafTemperature();
		BoundaryLayerConductance=MyLeafGasEx->get_BoundaryLayerConductance();
		InternalCO2=MyLeafGasEx->get_Ci();
		Respiration=MyLeafGasEx->get_Respiration();
		StomatalConductance=MyLeafGasEx->get_StomatalConductance();



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

