#pragma once
/*
Piston Engine 
Input: 
Throttle lever: number in range [0;1],
Aircraft speed [m/s],
Air density [kg/m^3],
Air pressure [Pa],
Output: 
Thrust [N], Fuel Consumption [kg/s]
*/
class PistonEngine {
protected:
	// current throttle_lever
	double _throt_lv;
	// current rpm, si units [rad/s]
	double _rot_speed;
	// fuel remain, [kg]
	double _fuel_remain;
	// fuel consumption [kg/s] 
	double _fuel_consum;
public:
	//PistonEngine();
	double calc_manifold_pressure(double p_base);
	double calc_static_horse_power();
	double calc_fuel_to_air_ratio();
	double calc_power_func(double far);
	double calc_power_func();
};
