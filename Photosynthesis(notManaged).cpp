/*! @file 
*  Defines the entry point for the console application.
   @author $Author \n
*/

#include "stdafx.h"
#include "gas_exchange.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;
// uses std::string, a more generic method than CString

 /*! \namespace photomod
     \details photomod is the namespace and contains the code needed to run the model. In includes an interface
	 \b _tmain (named by Visual Studio), and \b gasexchange.cpp, the model itself */ 

using namespace photomod; //Photosynthesis module


/*! \b Program _tmain
* \page Interface
* \par Interface to photosynthesis module
* \details This program demonstrates how to call the photosynthesis module 
* 
*
* \par Use of this interface  
* Two input files are needed (@b parameters.csv and @b ClimateIn.dat)  and one output file is created (@b Results.dat). ClimateIn.dat is the 
default name, any file can be input when prompted by the program. A detailed description of the input files follows.
*    -# A comma delimited parameter file containing the parameters for the photosynthesis module, one line for each species (Parameters.csv)
*    -# An comma delimted environmental  file @b (ClimateIn.dat) each line should have these variables (separated by commas):
*    \li  temperature (C)
*	 \li  PAR (umol photons m-2 s-1)
	 \li  CO2 content (umol mol-1)
	 \li  humidity (%)
	 \li  wind (m s-1)
     \li  a flag (0,1) to tell the program if constant temperature is used (or let temperature of leaf
          vary with stomatal conductance). 

*    -# an output file is produced with results @b (Results.dat) each line is written as:
*     \li  PAR               (umol photons m-2 s-1)
      \li  Net Photosynthesis (umol CO2 m-2 s-1)
      \li Gross Photosynthesis
	  \li VPD                 (kPa)
	  \li LeafTemperature     (C)
	  \li BoundaryLayerConductance (mol m-2 s-1)
      \li Internal CO2        (umol mol-1)
	 \li Respiration          (umolCO2 m-2 s-1)
	 \li Transpiration        (umol H2O m-2 s-1) \n

     -# In your calling program, execute the function SetParams first to initialize the calculator for a specific plant species 
*     Then execute the function:
      \b SetVal(PFD, Temperature, CO2, RelativeHumidity, Wind, Pressure, ConstantTemperature)
      to pass environmental parameters needed
      to calculate carbon assimilation and transpiration. This will call the function \b GasEx() which carries out the calculations \n
*    -# Use the public get functions described in the CGasExchange Class to retrieve the calculated
*     variables. 
      
*/

int _tmain(int argc, _TCHAR* argv[]) 
{    
	/*! \brief these are the variables for the interface */

	string DataLine;         //!< \b DataLine, holds line of data read from file with climate data
	string Remark;           //!< \b Remark, remark from parameter file
	char* context = NULL;    //!< \b context, pointer needed to use strtok_s function
	bool ConstantTemperature; //!< \b ConstantTemperature, if set to 1, model does not solve for leaf temperature 
	photomod::CGasExchange::tParms thisParms; //!< \b thisParms, object to hold parameters sent to photosynthesis module
	const char *pDelim=",";    //!< \b pDelim, -pointer to character (delimiter) that separates entries in the parameter file
	char * pnt;               //!< \b pnt, -pointer to the next word to be read from parameter file
	char CharTest ='A';       //!< \b CharTest -variable to test if there are characters in the line of data (indicates end of data)
	bool found=false;         //!< \b found -Boolean to indicate the species line was found in the parameter file
	string temp ;             //!< \b temp -temporary variable for holding string objects
	/*! \brief \li these variables hold data sent to model */
	double PFD,               /*!< \b PFD light umol ppfd m-2 s-1- */ 
		Temperature,          /*!< \b Temperature leaf temperature C*/ 
		RelativeHumidity,Wind, CO2, Pressure=100;

	ifstream ParamFile, DataFile;  //!< \b ParamFile, \b DataFile - files to hold parameters and variables for a single run
	ofstream OutputFile;            //!< \b OutputFile holds data output from photosynthesis module
	//Define a pointer to a new variable as a GasExchange Object
	photomod::CGasExchange *MyLeafGasEx; //!< \b MyLeafGasEx - define a gas exchange object type
	// Create a new GasExchange Object
	MyLeafGasEx= new CGasExchange();     //create a new gas exchange object on the stack

	//variables to hold results from gas exchange module.
	double Anet, VPD,Agross,LeafTemperature, Respiration, InternalCO2, StomatalConductance,
		BoundaryLayerConductance, Transpiration;


//assign defaults in case user does not want to enter other information (DataLine is empty)
	string DataFileName="ClimateIn.dat", OutputFileName="Results.dat"; 
	string Species ="Maize"; //!< \b Species of plant for calculatioins

	// Get datafile name, and species name if entered by user.
	cout << "enter name of file with input data and species name separated by commas:" <<endl
		<< "hit Enter with empty string for defaults" <<endl;
	getline(cin,DataLine);
	if (!DataLine.empty()) //if empty defaults to string names assigned above
	{

		pnt=strtok_s((char*)DataLine.c_str(), pDelim, &context );  //pnt is a pointer to the last recently found token in the string
		DataFileName.assign(pnt);                                // first token is a file name
		pnt=strtok_s(NULL, pDelim, &context );                    //get ready for the next token by getting pointer to recently found token in the string
		Species.assign(pnt);                                     // next token is species
		Species.erase(remove(Species.begin(),Species.end(),' '),Species.end()); //remove blanks from species name in case any are present
		pnt=NULL; //finished with these two
	}
	// open file with parameters and data
	  ParamFile.open("parameters.csv", std::ifstream::in);
	
      if (!ParamFile)
	  { 
		  std::cerr << "Parameter file not found \n";
	    return 0;
	  }

	DataFile.open(DataFileName.c_str());
	if (!DataFile)
	{
		std::cerr << "Data file with input not found \n";
		 return 0;
	}

	FILE * pFile;
	pFile=fopen((char*)OutputFileName.c_str(),"w");
	fprintf(pFile, "PAR   ANet     AGross    VPD  Leaf_Temp   BoundaryL_Conduc  InternalCO2  Respiration  StomatalConduct Transpiration\n");
	OutputFile.open(OutputFileName.c_str());
	std::getline(ParamFile,DataLine); //get header from parameter file
    // last line of file may be empty or contain numbers, this indicates file is at the  end
	while (!ParamFile.eof() && isalpha(CharTest))  //loops through file reads each line and tests if the correct species is found
	{
		getline(ParamFile,DataLine); // get the first line of data 
		CharTest=DataLine.at(0);     //check that line contains alphabetical text at beginning
		pnt=strtok_s((char*)DataLine.c_str(), pDelim, &context ); // pick off the first word before the token (',')

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

			pnt = strtok_s( NULL,pDelim, &context ); // iterate to clean string of characters already read
			// This section parses string, each iteration it cleans string of characters already read
			while(pnt!=NULL ) 
			{
				// printf( "Tokenized string using *  is:: %s\n", pnt ); // for debugging

				temp.assign(pnt);
				thisParms.species=temp;
				temp.clear();
				pnt = strtok_s( NULL,pDelim, &context ); 
				temp.assign(pnt);
				thisParms.Type=temp;
				temp.clear();
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Vcm25=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Jm25=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Vpm25=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.TPU25=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Rd25=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Theta=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.EaVc=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Eaj=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Hj=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Sj=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Hv=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.EaVp=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Sv=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Eap=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.Ear=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.g0=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.g1=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.stomaRatio=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.LfWidth=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				thisParms.LfAngFact=atof(pnt);
				pnt = strtok_s( NULL,pDelim, &context ); 
				temp.assign(pnt);
				thisParms.Remark=temp;
				temp.clear();
				pnt=strtok_s(NULL,pDelim, &context);
				found=true;
			}

		}
	}
	// Implementation to interact with photosynthesis model is here
	CharTest='1'; //initialize CharTest
	int start=1;
	bool LineEmpty=false; //Checks if file with environmental data is finished.
	//output variables
	
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
		pnt=strtok_s((char*)DataLine.c_str(), pDelim, &context );  //token is the delimiter
		PFD=atof(pnt);
		pnt=strtok_s(NULL,pDelim,&context);
		Temperature=atof(pnt);
		pnt=strtok_s(NULL,pDelim,&context);
		CO2=atof(pnt);
		pnt=strtok_s(NULL,pDelim,&context);
		RelativeHumidity=atof(pnt);
		pnt=strtok_s(NULL,pDelim,&context);
		Wind=atof(pnt);
		pnt=strtok_s(NULL,pDelim,&context);
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
		Transpiration=MyLeafGasEx->get_Transpiration();



		fprintf(pFile,"%8.2f  %6.2f   %6.2f    %8.3f      %4.1f      %8.3f    %4.1f    %6.2f   %8.3f  %8.3f\n", PFD, Anet, Agross, VPD, 
			LeafTemperature, BoundaryLayerConductance,InternalCO2,Respiration, 
			StomatalConductance, Transpiration);
	}	
	}
	DataFile.close();
	DataFile.close();
	fclose(pFile);
	return 0;
}



