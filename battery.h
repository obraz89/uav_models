#pragma once

/* 
battery discharge model
TODO: source article
*/
class Battery {
protected:
	// constant voltage from paper (V)
	const double _E0;

	// polarization Resistance (Ohm)
	const double _KRes;

	// polarization constant (V/(Ah))
	const double _KPol;

	// maximum battery capacity (Ah) 
	const double _Qmax;

	// exponential voltage (V)
	const double _AV;

	// exponentail capacity (Ah)
	const double _Bcap;

	// current charge
	double _charge;
public:
	Battery(double E0, double KRes, double KPol, double Qmax, double Av, double Bcap);
	// calculate internal resistance
	double calc_internal_resistance();

	// calc i star
	double calc_I_star(double ibat) const;

	// calculate current voltage of the battery - f1 function from paper
	double calc_U_full(double it, double istar, double i);

	// calculate current with given power of load (W)
	double calc_I_bat(double load_power);

	void get_current_charge_and_v(double& charge, double& v);

	void tick(double dt, double load_power);

};

extern void test_battery();
