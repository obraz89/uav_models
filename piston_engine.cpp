#include "piston_engine.h"

// manifold pressure = base pressure + delta_p
double PistonEngine::calc_manifold_pressure(double p_base) {
	// TODO: correct formula for dp
	// dp from Allerton is broken (?)
	// double dp = (0.04635 * _throt_lv - 0.0469) * _rot_speed;
	double dp = 0.0;
	return p_base + dp;
}

// mass ratio of fuel to air (far) in combustion chamber
// TODO: correct way to compute far
double PistonEngine::calc_fuel_to_air_ratio() {
	// avg value for typical car engine is 1/14.7 = 0.068
	// for now just take the average between very lean (0.05) and very  rich (0.125)
	// 0.5* (0.05 + 0.125) = 0.0875
	return 0.0875;
}

// if fuel to air is know, compute power function (pf) in range [0;1]
// when far is near optimum pf = 1
// when far is out of work region pf = 0 and engine stops
double PistonEngine::calc_power_func(double far) {
	// TODO: approximation in wide range
	// if we are near optimum just return 1
	return 1.0;
}

double PistonEngine::calc_static_horse_power() {}