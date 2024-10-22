#include "battery.h"
#include <cmath>

// test&debug logging
#include <iostream>
#include <fstream>

Battery::Battery(double E0, double KRes, double KPol, double Qmax, double Av, double Bcap)
	:_E0(E0), _KRes(KRes), _KPol(KPol), _Qmax(Qmax), _AV(Av), _Bcap(Bcap), _charge(Qmax) {}

double Battery::calc_I_bat(double load_power) {
	
	// if load is applied to battery, we have 2 equations
	// 1) f1(it, i*, i) = U
	// 2) i*f1 = P
	// => i*(E1(it) - m(it)*i) = P

	// TODO: during normal discharge m*i << E1 (?)
	// neglecting m*i term now (!)
	double E1 = calc_U_full(_charge, 0, 0);
	return load_power / E1;
}

double Battery::calc_I_star(double ibat) const{
	// TODO: low-pass filter must be here
	// now just return as is
	return ibat;
}

// discharge model for istar>0
double Battery::calc_U_full(double it, double istar, double i) {

	double f1;
	double qc = _Qmax / (_Qmax - _charge);

	f1 = _E0 - qc * (_KRes*istar + _KPol*_charge);
	f1 = f1 + _AV * std::exp(-_Bcap * _charge);
	return f1;
}

void Battery::get_current_charge_and_v(double& charge, double& v) {
	charge = _charge;
	v = calc_U_full(_charge, 0, 0);
}

void Battery::tick(double dt, double load_power) {

	double i_batt = calc_I_bat(load_power);

	// TODO: general HLA integration
	// now use 1st order
	_charge -= i_batt * dt;

}

void test_battery() {

	// TODO: constants
	double E0 = 0.0; 
	double KRes = 0.0;
	double KPol = 0.0; 
	double Qmax = 0.0; 
	double Av = 0.0;
	double Bcap = 0.0;

	Battery bat(E0, KRes, KPol, Qmax, Av, Bcap);

	double T = 3600.0;
	double dt = 1.0;
	double load = 1.0;
	int n = (int)T / dt;

	std::ofstream ofstr("bat.dat");
	ofstr << "time\tcharge\tpower\n";
	double time, charge, power;
	for (int i = 0; i < n; i++) {
		time = i * dt;
		bat.tick(dt, load);
		bat.get_current_charge_and_v(charge, power);
		ofstr << time << "\t" << charge << "\t" << power << "\n";
	}
	ofstr.close();
}