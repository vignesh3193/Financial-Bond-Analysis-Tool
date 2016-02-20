#ifndef CALCULATION_H
#define CALCULATION_H

class Calculator{

	public:
		double yieldRate;
		double couponRate;
		int numPayments;
		int amount;
		
		Calculator(){}
		~Calculator(){}
		double calculate_risk(double sensitivity);
		virtual double calculate(){return 0;}
		virtual double sensitivity_calculate(double price){return 0;}
};



class Coupon_Bearing_Calculator: public Calculator{
public:
	double calculate();
	double sensitivity_calculate(double price);

	Coupon_Bearing_Calculator(double yr, double cr, int numP, int amt){
		yieldRate = yr;
		couponRate = cr;
		numPayments = numP;	
		amount = amt;
	}
};

class Zero_Coupon_Calculator: public Calculator{
public:

	Zero_Coupon_Calculator(double yr,int amt, int numP)
	{
		yieldRate = yr;
		amount = amt;
		numPayments = numP;
	}
	double calculate();
	double sensitivity_calculate(double price);
};

class Spread_Calculator: public Calculator{
public:
	double bp;

	Spread_Calculator(double yr, double cr, int numP, int amt)
	{
		yieldRate = yr;
		couponRate = cr;
		numPayments = numP;	
		amount = amt;
		//bp = basePt/100;
	}
	double calculate();
	double sensitivity_calculate(double price);
};







#endif