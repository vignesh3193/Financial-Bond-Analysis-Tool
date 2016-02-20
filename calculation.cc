#include "calculation.h"
#include <iostream>
//#include <array>
#include <math.h>
#include <cmath>



double Calculator::calculate_risk(double sensitivity){	
	//amount adjusted by /100
	return amount/100*sensitivity;
}


double Coupon_Bearing_Calculator::calculate()
{	
	//yieldrate to decimal and /2
	double YieldRate = yieldRate/200; 
	//couponrate to decimal
	double coupon_perc = couponRate/100;
	double coupon = amount*(coupon_perc/2);
	//adjust the amount by 100
	int adjustment = std::abs(amount/(100));

	double price = std::abs(coupon * ( (1- ( 1 / pow((1+YieldRate),numPayments) ) )/YieldRate) + (amount / pow((1+YieldRate), numPayments)))/adjustment;
	return price;
}

double Coupon_Bearing_Calculator::sensitivity_calculate(double price){
	//set yield rate and coupon rate to decimal
	double YieldRate = yieldRate/100;
	double coupon_perc = couponRate/100;
	double coupon = amount*(coupon_perc/2);
	//+/- base point
	double upYield = YieldRate+0.0001;
	double downYield = YieldRate-0.0001;
	//adjust amount by 100
	int adjustment = std::abs(amount/(100));

	//divide yield and adjusted by 2 (freq)
	YieldRate /=2;
	upYield /=2;
	downYield /=2;

	//Sensitivity Calculation
	double up = std::abs(coupon * ( (1- ( 1 / pow((1+upYield),numPayments) ) )/upYield) + (amount / pow((1+upYield), numPayments)))/adjustment; 
	double down = std::abs(coupon * ( (1- ( 1 / pow((1+downYield),numPayments) ) )/downYield) + (amount / pow((1+downYield), numPayments)))/adjustment;

	double sensitivity = std::abs(((up-price) + (price-down))/2.0);
	return sensitivity;

}


double Zero_Coupon_Calculator::calculate()
{
	double YieldRate = yieldRate/200;
	int adjustment = std::abs(amount/(100));
	double price = std::abs(amount / pow((1+YieldRate), numPayments))/adjustment;
	return price;
}

double Zero_Coupon_Calculator::sensitivity_calculate(double price)
{
	double yield_perc = yieldRate/100;
	double upYield = yield_perc+0.0001;
	double downYield = yield_perc-0.0001;
	int adjustment = std::abs(amount/(100));

	yield_perc /=2;
	upYield/=2;
	downYield/=2;

	//Sensitivity Calculation
	double up = std::abs(amount / pow((1+upYield), numPayments))/adjustment; 
	double down = std::abs(amount / pow((1+downYield), numPayments))/adjustment;
	
	double sensitivity = std::abs((up - price) + (price-down))/2.0;
	return sensitivity;
}

double Spread_Calculator::calculate(){
	double YieldRate =yieldRate/200;
	double coupon_perc = couponRate/100;
	double coupon = amount*(coupon_perc/2);;
	int adjustment = std::abs(amount/(100));

	double price = std::abs(coupon * ( (1- ( 1 / pow((1+YieldRate),numPayments) ) )/YieldRate) + (amount / pow((1+YieldRate), numPayments)))/adjustment;

	return price;
}

double Spread_Calculator::sensitivity_calculate(double price){
	// double coupon = 0;
	double YieldRate = yieldRate/100;
	double coupon_perc = couponRate/100;
	double coupon = amount*(coupon_perc/2);

	double upYield = YieldRate+0.0001;
	double downYield = YieldRate-0.0001;

	int adjustment = std::abs(amount/(100));


	YieldRate /=2;
	upYield /=2;
	downYield /=2;

	//Sensitivity Calculation
	double up = std::abs(coupon * ( (1- ( 1 / pow((1+upYield),numPayments) ) )/upYield) + (amount / pow((1+upYield), numPayments)))/adjustment; 
	double down = std::abs(coupon * ( (1- ( 1 / pow((1+downYield),numPayments) ) )/downYield) + (amount / pow((1+downYield), numPayments)))/adjustment;

	double sensitivity = std::abs(((up-price) + (price-down))/2.0);

	return sensitivity;

}



