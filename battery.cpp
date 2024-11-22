#include "battery.h"
#include <cmath>

// test&debug logging
#include <iostream>
#include <iomanip>
#include <fstream>

Battery::Battery(double E0, double RInt, double KPolV, double Qmax, double Av, double Bcap)
	:_E0(E0), _RInt(RInt), _KPol_V(KPolV), _Qmax(Qmax), _AV(Av), _Bcap(Bcap), _charge_spent(0.0) {}

double Battery::calc_I_bat(double load_power) {
	
	// if load is applied to battery, we have 2 equations
	// 1) f1(it, i*, i) = U
	// 2) i*f1 = P
	// => i*(E1(it) - m(it)*i) = P

	// TODO: during normal discharge m*i << E1 (?)
	// neglecting m*i term now (!)
	double E1 = calc_U_batt(0.0);
	return load_power / E1;
}

double Battery::calc_I_star(double ibat) const{
	// TODO: low-pass filter must be here
	// now just return as is
	return ibat;
}

// discharge model for istar>0
// https://doi.org/10.4236/epe.2018.101002
double Battery::calc_U_batt(double i) {

	// avoid problems with zero division
	// TODO: better way to avoid problems with zero division
	double ch99 = 0.99 * _Qmax;
	double ch = _charge_spent > ch99 ? ch99 : _charge_spent;
	double qc = _Qmax / (_Qmax - ch);

	double E = _E0 - _KPol_V * qc + _AV * std::exp(-_Bcap * _charge_spent);
	double U_batt = E - i * _RInt;
	return U_batt;
}

void Battery::get_current_charge_and_v(double i, double& charge, double& v) {
	charge = _charge_spent;
	v = calc_U_batt(i);
}

void Battery::tick(double dt, double i) {

	// TODO: general HLA integration
	// now use 1st order
	_charge_spent += i * dt;

	// check for discharge (can't spend more than capacity)
	if (_charge_spent >= _Qmax) _charge_spent = _Qmax;

}

void test_battery_LiIon() {

	// TODO: constants
	double E0 = 3.73;
	double RInt = 0.09;
	double KPolV = 8.76e-03; 
	// Qmax in SI units, A*s
	double Qmax = 1.04*3600.0; 
	double Av = 0.468;
	// B in SI units, 1/(A*s)
	double Bcap = 3.53/3600.0;

	Battery bat(E0, RInt, KPolV, Qmax, Av, Bcap);

	double T = 3600.0*3;
	double load = 1.0;
	int n = 501;
	double dt = T / ((double)n - 1);
	//double i_batt = calc_I_bat(load_power);
	// for tests, discharge current
	double i_batt = 1.0;
	std::ofstream ofstr("bat.dat");
	ofstr << "time\tcharge\tpower\n";
	double time, charge, voltage;
	ofstr << std::fixed << std::setprecision(6) << std::scientific;
	for (int i = 0; i < n; i++) {
		time = i * dt;
		bat.tick(dt, i_batt);
		bat.get_current_charge_and_v(i_batt, charge, voltage);
		ofstr << time << "\t" << charge << "\t" << voltage << "\n";
	}
	ofstr.close();
}