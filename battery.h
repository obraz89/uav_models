#pragma once

/* 
battery discharge model
TODO: source article
*/
class Battery {
protected:
	// constant voltage from paper (V)
	double _E0;

	// internal resistance
	double _RInt;

	// polarization constant for modified model (V)
	double _KPol_V;

	// exponential voltage (V)
	double _AV;

	// exponentail capacity (A*s)
	double _Bcap;

	// maximum battery capacity (A*s) 
	double _Qmax;

	// charge spent, positive if discharge
	// charge = i*t if i=const
	double _charge_spent;
public:
	Battery(double E0, double KRes, double KPol, double Qmax, double Av, double Bcap);
	// calculate internal resistance
	double calc_internal_resistance();

	// calc i star
	double calc_I_star(double ibat) const;

	// calculate current voltage of the battery
	double calc_U_batt(double i);

	// calculate current with given power of load (W)
	double calc_I_bat(double load_power);

	void get_current_charge_and_v(double i, double& charge, double& v);

	void tick(double dt, double load_power);

};

extern void test_battery_LiIon();
