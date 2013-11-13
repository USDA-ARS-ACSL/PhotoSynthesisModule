/*
 This is the Gas Exchange class. It encapsulates all the calculations to estimate photsynthetic rate, CO2 assimilation,
  stomatal conductance, and transpiration for a square meter of leaf.
 Use SetParams to initialize with parameters for a specific variety of plant
 Use SetVal to pass environmental variables and return a structure with output.
*/
//#include "Stdafx.h"
//! @brief This class simulates gas exchange in plant leaves
//! @class CGasExchange 

#include "stdafx.h"

using namespace std;
namespace photomod
{
class CGasExchange
{
  
	
public:
  CGasExchange();

	~CGasExchange(void);
	   struct tParms    //!Structure to hold parameters for the model
		{
		 string   ID;
		 std::string species;
		 std::string Type;
 /*!
		 *   parameter        description
		 * note some parameters are specific for C3 or C4 type Plants
		@param 	ID	 1		Name of plant
		@param 		species	2 		Species Name
		@param 		type	3			C3 or C4
		@param 		Vcm25	4		Photosynthetic Rubisco Capacity at 25C (umol m-2 s-1)
		@param 		Jm25	5		Potential Rate of electron transport at 25C  (umol m-2 s-1)
		@param 		Vpm25	6		C4 Carboxylation rate at 25C (C4, umol m-2 s-1)
		@param 		TPU25	7		Rate if Triose Phosphate Utilization at 25C (C3, umol m-2 s-1)
		@param 		RD25	8		Mitochondrial respiration in the light at 25C (umol m-2 s-1)
		@param 		theta  10		Initial slope of CO2 response (umol m2 s-1)
		@param 		EaVc   11		Activation energy for Arrhenius function used to calculate temperature dependence for Vcmax (kJ mol-1)	
		@param 		Eaj    12		Activation energy for Arrhenius function used to calculate temperature dependence for J (kJ mol-1)
		@param 		Hj     13		Curvature parameter of the temperature dpendence of Jmax (kJ mol-1)
		@param		Sj	   14		Electron transport temperature response parameter for Jmax (J mole-1 K-1)
		@param      Hv              Need descriptot ToDo
		@param 	    EaVp   15		Activation energy for Arrhenius function used to calculate temperature dependence for Vpmax (kJ mol-1)
		@param 	    Sv	   16		Electron transpor temperature response parameter for Vcmax (J mole-1 K-1)
		@param 		EAP	   17		Activation energy for Arrhenius function used to calculate temperature dependence for TPU (kJ mol-1)
		@param 		EAR	   18	 	Activation energy for Arrhenius function used to calculate temperature dependence for respiration (kJ mol-1) 
		@param 		g0	   19		Minimum stomatal conductance to water vapor at the light compensation point in the BWB model (mol m-2 s-1)	
		@param 	    g1	      20	Empirical coefficient for the sensitivity of StomatalConductance to A, Cs and hs in BWB model (no units?)
		@param 		StomRatio	21	Stomatal Ratio
		@param  	LfWidth   22	Leaf Width (m)
		@param 		LfAngFact	23	Leaf Angle Factor		
		@param 		Remark	24	Text 
	*/	
		double  
			    Vcm25, 
			    Jm25, 
				Vpm25, 
				TPU25,
				Rd25,
				Theta,
				EaVc,
				Eaj,
				Hj,
				Sj,
				Hv,
				EaVp,
				Sv,
				Eap,
				Ear,
				g0,
				g1,
			    stomaRatio,
				LfWidth,
				LfAngFact;
		std::string Remark;
		}sParms ;

		///    @param tParms - a structure to hold input parameters for the variety


    void SetVal(double PhotoFluxDensity, double Tair, double CO2, double RH, 
	double wind, double Press, bool ConstantTemperature); 
	            //!< sets input values for calculations for a particular set of environmental variables
	double get_VPD(){ return VPD;}         //!<returns vapor pressure deficit (kpa)
	double get_ANet() {return AssimilationNet;}      //!< returns net photosynthesis (umol CO2 m-2 s-1)
	double get_AGross() {return AssimilationGross;}  //!< returns gross photosynthesis  (umol CO2 m-2 s-1)
	double get_Transpiration()     {return ET;}   //!< returns transpiration rate (umol H2O m-2 s-1)
	double get_LeafTemperature() {return Tleaf;} //!< returns leaf temperature (C)
	double get_Ci()    {return Ci;}        //!< returns internal CO2 concentration (umol mol-1)
	double get_StomatalConductance()    {return StomatalConductance;}    //!< returns stomatal conductance to water vapor (mol m-2 s-1)
	double get_BoundaryLayerConductance()    {return BoundaryLayerConductance;}    //!< returns boundary layer conductance (mol m-2 s-1)
	double get_Respiration() {return DarkRespiration;}   //!< returns respiration rate (umol CO2 m-2 s-1)
	void  SetParams(tParms *sParms);              //!<used to pass structure with parameters from the main program

protected:
  double  PhotoFluxDensity,  //!< Photosynthetic Flux Density (umol photons m-2 s-1 
	     R_abs, //!< absorbed incident radiation (watts m-2)        
		 Tair,  //!< Air temperature at 2m, (C) 
		 CO2,   //!< CO2 concentration (umol mol-1 air) 
		 RH,   //!<  Relative Humidity (%, i.e., 80) 
		 wind, //!<  Windspeed at 2 meters (km s-1) 
		 age,  //!< leaf age (days)(not used now) 
		 width, //!< Leaf width (m) 
		 Press;  //!<  Air pressure (kPa) 
   
   //! These variables hold the output of calculations 

   double AssimilationNet,    //!< net photosynthesis (umol CO2 m-2 s-1) 
	      AssimilationGross, //!< gross photosynthesis (umol CO2 m-2 s-1) (Adjusted for respiration)
	      ET,     //!< evapotranspiration mol h2o m-2 s-1 
		  Tleaf,  //!< Leaf temperature C 
		  Ci,     //!< Internal CO2 concentration umol mol-1 
		  StomatalConductance,     //!< stomatal conductance umol m-2 s-1 
		  BoundaryLayerConductance,    //!< boundary layer conductance umol m-2 s-1 
		  DarkRespiration,    //!< plant respiration    umol m-2 s-1 
		  VPD,    //!< Vapor Pressure Density, kPa */
		  Ci_Ca;  //!< ratio of internal to external CO2, unitless
		  double errTolerance; /*!< error tolerance for iterations */
	      double eqlTolerance; /*!< equality tolerance */
   
   int iter_total;      //!< holds total number of iterations */
    ///  @param PlantType string that holds the type of plant, C3 or C4

   std::string PlantType;       //!< For C3 or C4 designation */
   bool ConstantLeafTemperature; //!< if true, uses constant temperature - if true, does not solve for leaf temperature */


   void GasEx();  //!<Main module to calculate gas exchange rates
   void PhotosynthesisC3(double Ci);  //!<C3 photosynthesis module
   void PhotosynthesisC4(double Ci);  //!<C4 photosynthesis module
   void EnergyBalance();     //!<calculates leaf temperature and transpiration
   double SearchCi(double CO2i);          //!< called iterively to find optimal internal CO2 concentration
   double EvalCi(double Ci);   //!<Calls photosynthesis modules to evaluate Ci dependent calculations

   double CalcStomatalConductance();             //!< calculates and returns stomatal conductance (mol m-2 s-1)
   double CalcTurbulentVaporConductance();             //!<  calculates and returns conductance for turbulant vapor transfer in air - forced convection (mol m-2 s-1)
   double Es(double Temperature);      //!< calculates and returns saturated vapor pressure at temperature (T). kPa
   double Slope(double Temperature);  //!< calculates and returns slope of the vapor pressure curve
   double QuadSolnUpper (double a, double b, double c ); //!< returns upper part of quadratic equation solution
   double QuadSolnLower (double a, double b, double c ); //!< returns lower part of quadratic equation solution
    double minh(double fn1,double fn2,double theta2); //!< hyperbolic minimum
	double get_CiCaRatio()  {return Ci_Ca;} //!< returns ratio between internal and atmospheric CO2
   int iter1, iter2;         //!< holds iteration counters
   int  iter_Ci;   /*!< iteration value for Ci */
	bool isCiConverged; /*!< true if Ci iterations have converged */
  

 
};


} //end namespace photomod
 
