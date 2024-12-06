#include <iostream>

#include "IniFile.hpp"
#include "atmosphere.h"

// test&debug logging
#include <iostream>
#include <iomanip>
#include <fstream>

static const double Rs = 8314.32; /* Nm/(kmol K), gas constant */
static const double M0 = 28.9644; /* kg/kmol, mean molecular weight of air */
static const double g0 = 9.80665; /* m/s^2, accn of gravity at 45.5425 deg lat. */
static const double r0 = 6356766.0; /* m, Earth radius at g0 */
static const double P0 = 101325.0; /* Pa, air pressure at g0 */
static const double T0 = 288.15; /* K, standard sea-level temperature */
static const double Td = 273.15; /* K, 0 degrees C */

const double Gamma = 1.40; /* Ratio of Specific heats for an ideal diatomic gas */

/* Z - geometric height m */
void calc_atmo_vars(double Z, t_AtmoParams& params)
{
	double As; /* constant */
	double H; /* geopotential height m */

	As = g0 * M0 / Rs;
	{
		H = r0 * Z / (r0 + Z);
		// TODO: 0.0065 to constants
		params.T = T0 - 0.0065 * H;
		params.P = P0 * exp(As * log(T0 / params.T) / (-0.0065));
		params.rho = (params.P / params.T) * (M0 / Rs);
		params.Cs = sqrt((Gamma * Rs / M0) * params.T);
//		printf("%5d %6.2f %8.4f %8.4f %6.2f\n", Z, T, P, rho, Cs);
	}
}


void test_atmo_params() {

    std::string fn("params.ini");

    TIniAutoDefaults iniAD(fn);
    // example of usage of config file with default vals
    iniAD.set_section("init");
    double xl = iniAD.read_float("xl", 0.0);
    double xr = iniAD.read_float("xr", 0.5);

    iniAD.save_if_updated();

    t_AtmoParams prm;

	std::ofstream ofstr("atmo.dat");
	ofstr << "Z\tTemp[K]\tPressure[Pa]\tDensity[kg/m^3]\tSpeed of Sound[m/s]\n";
	ofstr << std::fixed << std::setprecision(6) << std::scientific;
    for (double Z=0; Z <= 10800; Z+=300){
        calc_atmo_vars(Z, prm);
		ofstr << Z << "\t"
			<< prm.T << "\t"
			<< prm.P << "\t"
			<< prm.rho << "\t"
			<< prm.Cs << "\n";
	//printf("%5d %6.2f %8.4f %8.4f %6.2f\n", Z, prm.T, prm.P, prm.rho, prm.Cs);
    }
	ofstr.close();
}
