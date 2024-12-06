#pragma once

/* Atmosphere params */
/* P - pressure Pa */
/* T - temperature K */
/* rho - density kg/m^3*/
/* Cs - speed of sound m/s */

struct t_AtmoParams {
	double P, T, rho, Cs;
};

void test_atmo_params();