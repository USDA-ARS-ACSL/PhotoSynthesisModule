


/* !
@file Gas_exchange.cpp
class CGas_Exchange 
   Additional functions and variables
   file GasExchange.CPP Holds class definitions
*/
//using namespace System.math;
#include "stdafx.h"
#include "gas_exchange.h"
#include <cmath>
using namespace std;

namespace photomod
{
	// General fixed parameters
#define R 8.314  //!< idealgasconstant
#define maxiter 200 //!< maximum number of iterations
#define epsilon 0.97   //!<emissivity See Campbell and Norman, 1998, page 163 (CHECK) 
#define sbc 5.6697e-8  //!<stefan-Boltzmann constant Wm-2 k-4. Actually varies somewhat with temperature
#define scatt 0.15     //!<leaf reflectance + transmittance
#define    f 0.15      //!<spectral correction
#define    O 205.0     //!< gas units are mbar
//#define theta 0.70	   //!<for potato //Initial slope of CO2 response umol m2 s-1; de Pury (1997)
	//!<Empirical Curvature factor for calculation of J Eq 5 in Soo, 200
#define Q10 2.0        //!< Q10 factor



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

	inline double Square(double a) { return a * a; } 
	inline double Min(double a, double b, double c) {return (__min(__min(a,b),c));}
 
	//note that '<' indicates member is before the block and not after for Doxygen

	CGasExchange::CGasExchange()
		//! This is the constructor
		
		/** Constructor - Some initialization is done here.
		
		*/ 
		{ 

		
		 isCiConverged=false;
		 errTolerance = 0.001;
	     eqlTolerance = 1.0e-6;
		
		}


	CGasExchange::~CGasExchange()
		//! This is the destructor
		/**Destructor - nothing is done here
		*/
		{
		}
	void CGasExchange::SetParams(struct tParms *sParms)
	{		

		//In Cameron (2000), Vpm25=120, Vcm25=60,Jm25=400
		//In Soo et al.(2006), under elevated CO2, Vpm25=91.9, Vcm25=71.6, Jm25=354.2 YY
       /*! @see SetParams for definitions*/
		PlantType=sParms->Type;
		if (PlantType.compare("C4")==0)
		{

			    this->sParms.Vcm25	=	sParms->Vcm25;
				this->sParms.Jm25	=	sParms->Jm25;
				this->sParms.Vpm25	=	sParms->Vpm25;
				this->sParms.Rd25	=	sParms->Rd25;
				this->sParms.Theta  =   sParms->Theta;
				this->sParms.EaVc	=	sParms->EaVc;
				this->sParms.Eaj	=	sParms->Eaj;
				this->sParms.Hj	    =	sParms->Hj;
				this->sParms.Sj	    =	sParms->Sj;
				this->sParms.EaVp	=	sParms->EaVp;
				this->sParms.Ear	=	sParms->Ear;
				this->sParms.g0  	=	sParms->g0;
				this->sParms.g1	    =	sParms->g1;
				this->sParms.stomaRatio	=	sParms->stomaRatio;
				this->sParms.LfWidth	=	sParms->LfWidth;

		}

		if (PlantType.compare("C3")==0)
		{

			    this->sParms.Vcm25  =	sParms->Vcm25;
				this->sParms.Jm25	=	sParms->Jm25;
				this->sParms.TPU25	=	sParms->TPU25;
				this->sParms.Rd25	=	sParms->Rd25;
				this->sParms.Theta  =   sParms->Theta;
				this->sParms.EaVc	=	sParms->EaVc;
				this->sParms.Eaj	=	sParms->Eaj;
				this->sParms.Hj	    =	sParms->Hj;
				this->sParms.Sj	    =	sParms->Sj;
				this->sParms.Hv	    =	sParms->Hv;
				this->sParms.Sv   	=	sParms->Sv;
				this->sParms.Eap	=	sParms->Eap;
				this->sParms.Ear	=	sParms->Ear;
				this->sParms.g0 	=	sParms->g0;
				this->sParms.g1	   =	sParms->g1;
				this->sParms.stomaRatio	=	sParms->stomaRatio;
				this->sParms.LfWidth	=	sParms->LfWidth;


		}


	}

	void CGasExchange::SetVal(double PhotoFluxDensity, double Tair, double CO2, double RH, double wind,  double Press, bool ConstantTemperature)
		/**Sets environment variables for a single execution of the module /
		// * Calls GasEx() to calculate photosynthetic rate and stomatal conductance.

		* @param[in] PhotoFluxDensity	Photosynthetic Flux Density (umol Quanta m-2 s-1) (check)
		* @param[in] Tair	Air Temperature (C)
		* @param[in] CO2	CO2 concentration of the air (umol mol-1)
		* @param[in] RH	    Relative Humidity (%)
		* @param[in] wind	Windspeed at 2.5 m, m s-1
		* @param[in] Press	Atmospheric pressure (kpa m-2)
		* @param[in] ConstantTemperature boolian if true, leaf temperature=air temperature when calculating gas exchange
		\return nothing
		*/


		{
		this->PhotoFluxDensity = PhotoFluxDensity;
		double PAR = (PhotoFluxDensity/4.55); //PAR is watts m-2
		double NIR = PAR; // If total solar radiation unavailable, assume NIR the same energy as PAR waveband
		this->R_abs = (1-scatt)*PAR + 0.15*NIR + 2*(epsilon*sbc*pow(Tair+273,4)); // times 2 for projected area basis
		// shortwave radiation (PAR (=0.85) + NIR (=0.15) solar radiation absorptivity of leaves: =~ 0.5
		//transfer variables to local scope
		this->CO2 = CO2;
		this->RH = __min(100.0, __max(RH, 10.0))/100;
		this->Tair = Tair;
		this->wind = wind;
		this->Press = Press;
		ConstantLeafTemperature=ConstantTemperature;
		GasEx();   // Gas exchange calculations here
		}

	void CGasExchange::GasEx(void)
		/** 
		* carries out calculations for photosynthesis and stomatal conductance.
		* no parameters, returns nothing

		* @see SearchCi(), @see EnergyBalance(), @see CalcStomatalConductance()
		\return nothing
		*/
		{
		double Tleaf_old;  //previous leaf temperture (for iteration)
		int   iter=1;
		iter_total=0;
		Tleaf = Tair; Tleaf_old = 0;
		Ci = 0.7*CO2;
		BoundaryLayerConductance = CalcTurbulentVaporConductance();
		StomatalConductance = CalcStomatalConductance();
		while ((abs(Tleaf_old -Tleaf)>0.01) && (iter < maxiter))
			{
			Tleaf_old=Tleaf;
			Ci=SearchCi(Ci);
			StomatalConductance=CalcStomatalConductance();
			EnergyBalance();
			iter2 =++iter; //iter=iter+1, iter2=iter; 
			if (ConstantLeafTemperature) Tleaf=Tair;
			} 

		}
	void CGasExchange::PhotosynthesisC3(double Ci)    
		/**
		*Calculates photosynthesis for C3 plants \n
		*Uses as input incident PhotoFluxDensity, Air temp in C, CO2 in ppm, RH in per percent.
		@see SetVal() 
		@param[in] Ci - internal CO2 concentration, umol mol-1

		\return nothing.
		
		*/
		//** \code
		{
		//parameters for C3 Photosythesis; 
		const double curvature=0.999 ; //!< Curvature -factor of Av and Aj colimitation 
			
		const int		Kc25 = 404;//!< Kc25, MM Constant of rubisco for CO2 of C3 plants (de Pury and Farquar, 1997) (umol m-2 s-1) 
		const int		Ko25 = 278;//!< Ko25, MM Constant of rubiscuo for O2 from above reference (umol m-2 s-1) 
		const long      Eac = 59400;//!< Eac, Energy Activation kJ mol-1

		const long       Eao = 36000;//!< Eao, activation energy values 
		//** \endcode
		// These variables hold temporary calculations
		double alpha, Kc, Ko, gamma, Ia,Jmax, Vcmax, TPU, J, Av, Aj, Ap, Ac, Km, Ca, Cc, P;
		gamma = 36.9 + 1.88*(Tleaf-25)+0.036*Square(Tleaf-25);  // CO2 compensation point in the absence of mitochondirial respiration, in ubar}
//* Light response function parameters */
		Ia = PhotoFluxDensity*(1-scatt);    //* absorbed irradiance */
		alpha = (1-f)/2; // *!apparent quantum efficiency, params adjusted to get value 0.3 for average C3 leaf

		AssimilationNet = 0;

				//* other input parameters and constants */
		P  = Press/100; //Press is kPa. Used to convert mole fraction to partial pressure
		Ca = CO2*P; //* conversion to partial pressure */ 
		Kc = Kc25*exp(Eac*(Tleaf-25)/(298*R*(Tleaf+273)));
		Ko = Ko25*exp(Eao*(Tleaf-25)/(298*R*(Tleaf+273)));
		Km = Kc*(1+O/Ko); //* effective M-M constant for Kc in the presence of O2 */
		DarkRespiration = sParms.Rd25*exp(sParms.Ear*(Tleaf-25)/(298*R*(Tleaf+273)));
		Jmax = sParms.Jm25*exp(((Tleaf-25)*sParms.Eaj)/(R*(Tleaf+273)*298))*
			(1+exp((sParms.Sj*298-sParms.Hj)/(R*298)))/
			(1+exp((sParms.Sj*(Tleaf+273)-sParms.Hj)/(R*(Tleaf+273)))); // de Pury 1997
		Vcmax = sParms.Vcm25*exp(((Tleaf-25)*sParms.EaVc)/(R*(Tleaf+273)*298))*
			(1+exp((sParms.Sv*298-sParms.Hv)/(R*298)))/
			(1+exp((sParms.Sv*(Tleaf+273)-sParms.Hv)/(R*(Tleaf+273)))); // Used peaked response, DHF
		TPU = sParms.TPU25*exp(sParms.Eap*(Tleaf-25)/(298*R*(Tleaf+273)));
		Cc = Ci; // assume infinite gi

		StomatalConductance = CalcStomatalConductance(); // Initial value
		BoundaryLayerConductance=  CalcTurbulentVaporConductance();
		Av = (Vcmax*(Cc-gamma))/(Cc+Km);
		J =  (((alpha*Ia + Jmax) - sqrt(Square(alpha*Ia+Jmax) - 4*alpha*Ia*(Jmax)*sParms.Theta)) / (2*sParms.Theta)) ;
		Aj = J*(Cc-gamma)/(4*(Cc+2*gamma));
		Ap = 3*TPU;
		Ac = ((Av+Aj) - sqrt(Square(Av+Aj)-4*curvature*Av*Aj))/(2*curvature); // curvatureaccount for colimitation between Av and Aj */
		if (Cc > gamma) 
			AssimilationNet = min(Ac, Ap) -DarkRespiration; 
		else
		{
			AssimilationNet = Av-DarkRespiration;
		}
     	
		    AssimilationGross = max(AssimilationNet+DarkRespiration,0.0);
        	StomatalConductance = CalcStomatalConductance(); // Update StomatalConductance using new value of AssimilationNet
		
		}

	void CGasExchange::PhotosynthesisC4(double Ci)    
		/**
		* Calculates photosynthesis for C4 plants. 
		* Requires Incident PhotoFluxDensity, Air temp in C, CO2 in ppm, RH in percent
		@see SetVal()
		* 
		@param[in] Ci - internal CO2 concentration, umol mol-1

		\return nothing
		*/
		{
		const double    curvature=0.995; //!<curvature factor of Av and Aj colimitation

       
		const int       Kc25 = 650,    //!< Kc25, Michaelis constant of rubisco for CO2 of C4 plants (2.5 times that of tobacco), ubar, Von Caemmerer 2000 
		                Ko25 = 450,    //!< Ko25, Michaelis constant of rubisco for O2 (2.5 times C3), mbar 
		        		Kp25 = 57;     /*!< Kp25, Michaelis constant for PEP caboxylase for CO2 - was 60 in Soo's paper */
		const long       Eao = 36000;   /*!< EAO, activation energy for Ko */
		const int	    Vpr25 = 80; /*!<   Vpr25, PEP regeneration limited Vp at 25C, value adopted from vC book */
		const double	gbs = 0.003; /*!< gbs, bundle sheath conductance to CO2, umol m-2 s-1 gbs x Cm is the inward diffusion of CO2 into the bundle sheath  */
		const double    x = 0.4;  /*!< x, Partitioning factor of J, yield maximal J at this value */
		const double    alpha = 0.001; /*!< alpha, fraction of PSII activity in the bundle sheath cell, very low for NADP-ME types  */
		const double    gi = 5.0; /*!< gi, conductance to CO2 from intercelluar to mesophyle, mol m-2 s-1, assumed  was 1, changed to 5 as per Soo 6/2012*/
		const double    beta = 0.99; /*!< beta, smoothing factor */
		const double    gamma1 = 0.193; /*!< gamma1, half the reciprocal of rubisco specificity, to account for O2 dependence of CO2 comp point, note that this become the same as that in C3 model when multiplied by [O2] */

		double Kp, Kc, Ko, Km; //!<Kp, Kc, Ko, Km, Calculated Michaelis params as a function of temperature
		double Ia, I2;       // Calculated light variables
		double Vpmax, Jmax, Vcmax, Eac, Om, Rm, J, Ac1, Ac2, Ac, Aj1,
			Aj2, Aj, Vp1, Vp2, Vp, P,  Ca, Cm, Vpr,
			Os, GammaStar, Gamma, a1, b1, c1; //secondary calculated variables
			
		//* Light response function parameters */
		Ia = PhotoFluxDensity*(1-scatt);    //* absorbed irradiance */
		I2 = Ia*(1-f)/2;    //* useful light absorbed by PSII */
		//* other input parameters and constants */
		P  = Press/100;
		Ca = CO2*P; //* conversion to partial pressure Atmospheric partial pressure of CO2, kPa*/
		Om = O;   //* mesophyle O2 partial pressure */
		Eac=sParms.EaVc;

		Kp = Kp25*pow(Q10,(Tleaf-25.0)/10.0);
		Vpr = Vpr25*pow(Q10,(Tleaf-25.0)/10.0);
		Kc = Kc25*exp(Eac*(Tleaf-25)/(298*R*(Tleaf+273))); //Kc adjusted for temperature
		Ko = Ko25*exp(Eao*(Tleaf-25)/(298*R*(Tleaf+273)));
		Km = Kc*(1+Om/Ko); //* effective M-M constant for Kc in the presence of O2 */
		DarkRespiration = sParms.Rd25*exp(sParms.Ear*(Tleaf-25)/(298*R*(Tleaf+273)));
		// The following are Arrhenius Equations for parameter temperature dependencies
		// Vpm25 (PEPC activity rate) , Vcm25  (Rubisco Capacity rate) and Jm25 (Whole chain electron transport rate) are the rates at 25C for Vp, Vc and Jm
		Vpmax = sParms.Vpm25*exp(sParms.EaVp*(Tleaf-25)/(298*R*(Tleaf+273)));
		Vcmax = sParms.Vcm25*exp(sParms.EaVc*(Tleaf-25)/(298*R*(Tleaf+273)));
		Jmax = sParms.Jm25*exp(((Tleaf-25)*sParms.Eaj)/(R*(Tleaf+273)*298))*(1+exp((sParms.Sj*298-sParms.Hj)/(R*298)))
			/(1+exp((sParms.Sj*(Tleaf+273)-sParms.Hj)/(R*Tleaf+273.0)));
		Rm = 0.5*DarkRespiration;

		Cm=Ci; //* mesophyle CO2 partial pressure, ubar, one may use the same value as Ci assuming infinite mesohpyle conductance */
		double gs_last=0;

		StomatalConductance = CalcStomatalConductance();
		Vp1 = (Cm*Vpmax)/(Cm+Kp); //* PEP carboxylation rate, that is the rate of C4 acid generation  Eq 1 in Kim 2007*/
		Vp2 = Vpr;
		Vp = __max(__min(Vp1, Vp2),0);
		//* Enzyme limited A (Rubisco or PEP carboxylation */
		Ac1 = (Vp+gbs*Cm-Rm);
		Ac2 = (Vcmax-DarkRespiration);
		//* Quadratic expression to solve for Ac */
		a1 = 1-(alpha/0.047)*(Kc/Ko);
		b1 = -(Ac1 + Ac2 + gbs*Km + (alpha/0.047)*(gamma1*Vcmax + DarkRespiration*Kc/Ko));
		c1 = Ac1*Ac2-(Vcmax*gbs*gamma1*Om+DarkRespiration*gbs*Km);
		Ac = QuadSolnLower(a1,b1,c1);
		Ac = __min(Ac1,Ac2);
		//* Light and electron transport limited  A mediated by J */
		J=minh(I2,Jmax,sParms.Theta);  //* rate of electron transport */
		Aj1 = (x*J/2-Rm+gbs*Cm);  // Eq 4 in Kim, 2007
		Aj2 = (1-x)*J/3-DarkRespiration;       //Eq 4 in Kim, 2007
		Aj = __min(Aj1,Aj2);      //Eq 4 in Kim, 2007
		AssimilationNet = ((Ac+Aj) - sqrt(Square(Ac+Aj)-4*beta*Ac*Aj))/(2*beta); //* smooting the transition between Ac and Aj */
		AssimilationNet=minh(Ac,Aj, curvature);
		gs_last=StomatalConductance;
		Os = alpha*AssimilationNet/(0.047*gbs)+Om; //* Bundle sheath O2 partial pressure, mbar */
		GammaStar = gamma1*Os;
		Gamma = (DarkRespiration*Km + Vcmax*GammaStar)/(Vcmax-DarkRespiration);
		AssimilationGross = __max(0, AssimilationNet + DarkRespiration); 

		}


	void CGasExchange::EnergyBalance()
		/** 
		    Calculates Transpiration rate (T) and leaf temperature (Tleaf). Iterates by recalculating
			photosynthesis until leaf temperatures converge

		    See Campbell and Norman (1998) pp 224-225
			
			Does not have input 

			\return nothing but calculates transpiration (T) and leaf temperature (Tleaf)
					    
			Because Stefan-Boltzman constant is for unit surface area by denifition,
			all terms including sbc are multilplied by 2 (i.e., RadiativeConductance, thermal radiation)
			*/

		{
		const long Lambda = 44000; //latent heat of vaporization of water J mol-1 - not used in this implementation
		const double Cp = 29.3; // thermodynamic psychrometer constant and specific hear of air, J mol-1 C-1
		const double psc = 6.66e-4; //psycrometric constant units are C-1
		//psc=Cp/Lambda = 29.3/44000 See Campbell and Norman, pg 232, after eq 14.11

		//The following are secondary variables used in the energy balance
		double HeatConductance,  //heat conductance J m-2 s-1
			VaporConductance, //vapor conductance ratio of stomatal and heat conductance mol m-2 s-1
			RadiativeConductance, //radiative conductance J m-2 s-1
			RadiativeAndHeatConductance, //radiative+heat conductance
			psc1,  // apparent psychrometer constant Campbell and Norman, page 232 after eq 14.11
			Ea,   //ambient vapor pressure kPa
			thermal_air; // emitted thermal radiation Watts  m-2
			double lastTi, newTi;
		int    iter;

		HeatConductance = BoundaryLayerConductance*(0.135/0.147);  // heat conductance, HeatConductance = 1.4*.135*sqrt(u/d), u is the wind speed in m/s} Boundary Layer Conductance to Heat
		// Since BoundaryLayerConductance is .147*sqrt(u/d) this scales to 0.135*sqrt(u/d) - HeatConductance on page 109 of Campbell and Norman, 1998
		// Wind was accounted for in BoundaryLayerConductance already  as BoundaryLayerConductance (turbulent vapor transfer) was calculated from CalcTurbulentVaporConductance() in GasEx. 
		// units are J m-2 s-1 
		VaporConductance = StomatalConductance*BoundaryLayerConductance/(StomatalConductance+BoundaryLayerConductance);      //vapor conductance, StomatalConductance is stomatal conductance and is given as gvs in Campbell and Norman.      
		RadiativeConductance = (4*epsilon*sbc*pow(273+Tair,3)/Cp)*2; // radiative conductance, *2 account for both sides
		RadiativeAndHeatConductance = HeatConductance + RadiativeConductance;
		thermal_air = epsilon*sbc*pow(Tair+273,4)*2; //Multiply by 2 for both surfaces
		psc1 = psc*RadiativeAndHeatConductance/VaporConductance; 
		this->VPD = Es(Tair)*(1-RH); // vapor pressure deficit Es is saturation vapor pressure at air temperature
		// iterative version
		newTi=-10;
		iter=0;
		lastTi=Tleaf;
		double Res, dRes; //temporary variables
		double thermal_leaf;
		Ea = Es(Tair)*RH; // ambient vapor pressure
		while ((abs(lastTi-newTi)>0.001) && (iter <maxiter)) 
			{
			lastTi=newTi;
			Tleaf= Tair + (R_abs- thermal_air-Lambda*VaporConductance*this->VPD/Press)/(Cp*RadiativeAndHeatConductance+Lambda*Slope(Tair)*VaporConductance); // eqn 14.6a
			thermal_leaf=epsilon*sbc*pow(Tleaf+273,4)*2;
			Res = R_abs - thermal_leaf - Cp*HeatConductance*(Tleaf - Tair) - Lambda*VaporConductance*0.5*(Es(Tleaf)-Ea)/Press; // Residual function: f(Ti), KT Paw (1987)
			dRes= -4*epsilon*sbc*pow(273+Tleaf,3)*2-Cp*HeatConductance*Tleaf-Lambda*VaporConductance*Slope(Tleaf); // derivative of residual: f'(Ti)
			newTi = Tleaf + Res/dRes; // newton-rhapson iteration
			iter++;
			}
		Tleaf=newTi;

		Transpiration =1000*VaporConductance*(Es(Tleaf)-Ea)/Press; //Don't need Lambda - cancels out see eq 14.10 in Campbell and Norman, 1998
		// umol m-2 s-1. note 1000 converts from moles to umol
		}



	double CGasExchange::CalcStomatalConductance()  
		/**
		  * calculates and returns stomatal conductance for water vapor in mol m-2 s-1 
		  * Uses Ball-Berry model.
		  * @see Es()
		  \return stomatal conductance for water vapor
		  */
		  //*! \fn CalcStomatalConductance()
		{
	//** \code
		double Ds, //! Ds, VPD at leaf surface 
		 	aa,    //! aa, a value in quadratic equation 
			bb,    //! bb, b value in quadratic equation 
			cc,    //! cc, calcuation variable (x) in quadratic equation
			hs,    //! hs, solution for relative humidity
			Cs,    //! Cs, estimate of mole fraction of CO2 at the leaf surface
			Gamma, //! Gamma, CO2 compensation point in the absence of mitochondirial respiration, in ubar
			StomatalConductance;    //! StomatalConductance, temporary variable to hold stomatal conductance
		Gamma = 10.0; 
        //** \endcode

		double P=Press/100;  
		Cs = (CO2 - (1.37*AssimilationNet/BoundaryLayerConductance))*P; // surface CO2 in mole fraction
		if (Cs == Gamma) Cs = Gamma + 1;
		if (Cs <= Gamma) Cs = Gamma + 1;
		// Quadratic equation to obtain hs by combining StomatalConductance with diffusion equation
		aa = sParms.g1*AssimilationNet/Cs;
		bb = sParms.g0+BoundaryLayerConductance-(sParms.g1*AssimilationNet/Cs);
		cc = (-RH*BoundaryLayerConductance)-sParms.g0;
		hs = QuadSolnUpper(aa,bb,cc);
		if (hs > 1) hs = 1;
		if (hs<0) hs = 0;
		Ds = (1-hs)*Es(Tleaf); // VPD at leaf surface
		StomatalConductance = (sParms.g0+sParms.g1*(AssimilationNet*hs/Cs));
		if (StomatalConductance < sParms.g0) StomatalConductance=sParms.g0; //Limit StomatalConductance to mesophyll conductance 
		return StomatalConductance;
		}





	double CGasExchange::CalcTurbulentVaporConductance(void)
		{
		/**
		  * calculates and returns conductance for turbulant vapor transfer in air - forced convection
		  *  units are mol m-2 s-1
		  \return conductance for turbulent vapor transfer in air
		  */

		double ratio;
		double d;
		ratio = Square(sParms.stomaRatio+1)/(Square(sParms.stomaRatio)+1);
		d = sParms.LfWidth*0.72; // characteristic dimension of a leaf, leaf width in m
		// wind is in m per second
		return (1.4*0.147*sqrt(__max(0.1,wind)/d))*ratio; 
		// multiply by 1.4 for outdoor condition, Campbell and Norman (1998), p109, gva
		// multiply by ratio to get the effective blc (per projected area basis), licor 6400 manual p 1-9
		}

	double CGasExchange::Es(double Temperature) //Campbell and Norman (1998), p 41 Saturation vapor pressure in kPa
		{
		/**
		 * calculates and returns Saturation vapor pressure (kPa)
		 @param[in] Temperature
		 \return saturated vapor pressure
		 */

		double result;
		// a=0.611 kPa, b=17.502 C and c=240.97 C 
		//Units of Es are kPa
	    result=(0.611*exp(17.502*Temperature/(240.97+Temperature)));
		return result;
		}

	double CGasExchange::Slope(double Temperature) 
		/**
		   Calculates and returns the slope of the sat vapor pressure curve: 
		   first order derivative of Es with respect to T

		   @param[in] Temperature (C)
		   @see Es()
		   \return slope of the vapor pressure curve kPa T-1 (?)
		  */

		{
		double TSlope;
		// units of b and c are  degrees C
		const double b= 17.502; const double c= 240.97;
		TSlope=(Es(Temperature)*(b*c)/Square(c+Temperature)/Press);
		return TSlope; 
		}

	double CGasExchange::SearchCi(double CO2i)
		{
		 /**
		   * does a secant search to find the optimal internal CO2 concentration (ci
		   * Calls:
		   * @see EvalCi()
		   @param[in] CO2i - internal CO2 concentration, umol mol-1
		   \return Ci
		   */

		int iter;
		double fprime, Ci1, Ci2, Ci_low, Ci_hi, Ci_m;
		double temp;
		Ci1 = CO2i;
		Ci2 = CO2i + 1.0;
		Ci_m = (Ci1+Ci2)/2.0;
		iter_Ci = 0;
		iter = 0;
		isCiConverged = true;

		do 
			{
			iter++;
			//Secant search method
			if (abs(Ci1-Ci2) <= errTolerance) {break;}
			if (iter >= maxiter) 
				{
				isCiConverged = false;
				break;
				}
			fprime = (EvalCi(Ci2)-EvalCi(Ci1))/(Ci2-Ci1);  // f'(Ci)
			if (fprime != 0.0) 
				{
				Ci_m = max(errTolerance, Ci1-EvalCi(Ci1)/fprime);
				}
			else
				Ci_m = Ci1;
			Ci1 = Ci2;
			Ci2 = Ci_m;
			temp=EvalCi(Ci_m);
			double temp2=maxiter;
			} while ((abs(EvalCi(Ci_m)) >= errTolerance) || (iter < maxiter));




			// C4 photosynthesis fails to converge at low soil water potentials using secant search, 6/8/05 SK
			// Bisectional type search is slower but more secure
			//Bisectional search
			if (iter > maxiter)
				{
				Ci_low = 0.0;
				Ci_hi = 2.0*CO2;
				isCiConverged = false;

				while (abs(Ci_hi-Ci_low) <= errTolerance || iter > (maxiter*2))
					{
					Ci_m = (Ci_low + Ci_hi)/2;
					if (abs(EvalCi(Ci_low)*EvalCi(Ci_m)) <= eqlTolerance) break;
					else if (EvalCi(Ci_low)*EvalCi(Ci_m) < 0.0) {Ci_hi = max(Ci_m, errTolerance);}
					else if (EvalCi(Ci_m)*EvalCi(Ci_hi) < 0.0)  {Ci_low = max(Ci_m, errTolerance);}
					else {isCiConverged = false; break;}
					}

				}

			CO2i = Ci_m;
			Ci_Ca = CO2i/CO2;
			iter_Ci = iter_Ci + iter;
			iter_total = iter_total + iter;
			return CO2i;

		}
	double CGasExchange::EvalCi(double Ci)
		{
		/**
		  * calculates a new value of Ci for the current values of photosynthesis and stomatal conductance
		  * determined using parameters from a previous step where the energy balance was solved
		  @param[in] Ci **, internal CO2 concentration, umol mol-1

		  \return the difference between the passed value and the new one. 

		  */
		double newCi;

		if (PlantType.compare("C3")== 0) PhotosynthesisC3(Ci);
		if (PlantType.compare("C4")== 0) PhotosynthesisC4(Ci);
		if (abs(StomatalConductance) > eqlTolerance) 
			{
			newCi = max(1.0,CO2 - AssimilationNet*(1.6/StomatalConductance+1.37/BoundaryLayerConductance)*(Press/100.0));
			}
		else
			newCi = max(1.0,CO2 - AssimilationNet*(1.6/eqlTolerance+1.37/BoundaryLayerConductance)*(Press/100.0));
		return (newCi-Ci);
		}

	//These two functions solve the quadratic equation.
	double CGasExchange::QuadSolnUpper (double a, double b, double c )
		{
		 
		  /** solves the uppper part of the quadratic equation ax2+bx2=c
		   
		    @param[in] a
			@param[in] b
			@param[in] c 
			
			\return lower portion of x		*/
		if (a==0) return 0;
		else if ((b*b - 4*a*c) < 0) return -b/a;   //imaginary roots
		else  return (-b+sqrt(b*b-4*a*c))/(2*a);
		}

	double CGasExchange::QuadSolnLower (double a, double b, double c )
		{
		/** solves the lower part of the quadratic equation ax2+bx=c
		  
		   @param[in] a
			@param[in] b
			@param[in] c 
			\return lower portion of x
		  */
		if (a==0) return 0;
		else if ((b*b - 4*a*c) < 0) return -b/a;   //imaginary roots
		else  return (-b-sqrt(b*b-4*a*c))/(2*a);
		}
 
	//*! hyperbolic min
	double CGasExchange::minh(double fn1,double fn2,double theta2)
		{
		
		/**  
		    @param [in] fn1 first value to be compared for min
		    @param [in] fn2 second value to be compared for min
			@param [in] theta2  curvature factor

			\return hyperbolic minimum
			*/
		double x, res;

		x = ((fn1+fn2)*(fn1+fn2)-4*theta2*fn1*fn2);
		if (x<0)
			{
			res = min(fn1,fn2); 
			return res;
			}
		if (theta2==0.0)
			{
			res= fn1*fn2/(fn1+fn2);
			return res;
			}
		else
			{
			res = ((fn1+ fn2) - sqrt(x))/(2*theta2); // hyperbolic minimum
			return res;
			}
		}
} // end namespace

