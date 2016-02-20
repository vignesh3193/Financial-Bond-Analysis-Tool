#include "SBB_date.cc"
#include "SBB_io.cc"
#include "calculation.cc"
#include "SBB_util.cc"
#include "SBB_ratings.cc"
#include "sbb_socket.h"
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <cmath>
#include <vector>
#include <set>
#include <algorithm>
//#include <unordered_map>
#include <tr1/unordered_map>
#include <iterator>

using namespace std::tr1;
using namespace std;
//using namespace std;


/* file handler - opens trading book and creates results files */
//===================================================================================================
void handleFiles(SBB_instrument_fields* &data_opening, SBB_instrument_fields* &data_closing, SBB_instrument_fields* &yieldCurve, 
	SBB_output_file* &book_result, SBB_output_file* &result, int &collectionSizeTBOpening, int &collectionSizeTBClosing, int &collectionSizeYC, 
	string bookfile_opening, string bookfile_closing, string curvefile){


//	SBB_instrument_input_file* dataFile_mt = new SBB_instrument_input_file((char*)bookfile_midtermbook.c_str());
	SBB_instrument_input_file* dataFile_op = new SBB_instrument_input_file((char*)bookfile_opening.c_str());
	SBB_instrument_input_file* dataFile_cl = new SBB_instrument_input_file((char*)bookfile_closing.c_str());

	SBB_yield_curve_input_file* yieldFile = new SBB_yield_curve_input_file((char*)curvefile.c_str());

	//sets length of the collections
//	collectionSizeTBMidterm = get_row_count_minus_comments((char*)bookfile_midtermbook.c_str());
	collectionSizeTBOpening = get_row_count_minus_comments((char*)bookfile_opening.c_str());
	collectionSizeTBClosing = get_row_count_minus_comments((char*)bookfile_closing.c_str());

	//reading data
//	data_midterm = (dataFile_mt->records(collectionSizeTBMidterm));
	data_opening = (dataFile_op->records(collectionSizeTBOpening));
	data_closing = (dataFile_cl-> records(collectionSizeTBClosing));
	
	collectionSizeYC = get_row_count_minus_comments((char*)curvefile.c_str());
	yieldCurve = (yieldFile->curve_records(collectionSizeYC));

	//handling output files
	book_result = new SBB_output_file("book_results.txt");
	result = new SBB_output_file("results.txt");

}
//===================================================================================================

/* Calculations: */
//===================== Calculates the dv01 of a specific treasury =====================
double calc_T_dv01(SBB_instrument_fields* yieldCurve, int curveSize, char* treasuryYr, double yield, int spreadAmount){
	double price;
	double dv01;
	SBB_date from;
	SBB_date to;
	int numPayments = 0;
	for(int i = 0; i < curveSize; i ++){
		if(!strcmp(yieldCurve[i].SecurityID(), treasuryYr)){
			from.set_from_yyyymmdd(yieldCurve[i].SettlementDate());
			to.set_from_yyyymmdd(yieldCurve[i].MaturityDate());
			numPayments = get_number_of_periods_semiannual(from, to);
			//printf("found  is %s \n", yieldCurve[i].SecurityID());
			Calculator* calculator_ptr = new Coupon_Bearing_Calculator(yield+(spreadAmount/100), yieldCurve[i].Coupon(), numPayments, 1000);
			price = calculator_ptr->calculate();
			dv01 = calculator_ptr->sensitivity_calculate(price);
			//printf("dv01 is %.6f \n", dv01);			
		}
	}	
		return dv01;
}


void get_yieldrate(SBB_instrument_fields* yieldCurve, int curveSize, double &yr2, double &yr5, double &yr10, double &yr30){
	for(int i = 0; i < curveSize; i++){	
		if(!strcmp(yieldCurve[i].SecurityID(), "T2")){
			yr2 = yieldCurve[i].Yield();
		}
		else if(!strcmp(yieldCurve[i].SecurityID(), "T5")){
			yr5 = yieldCurve[i].Yield();
		}
		else if(!strcmp(yieldCurve[i].SecurityID(), "T10")){
			yr10 = yieldCurve[i].Yield();
		}
		else{
			yr30 = yieldCurve[i].Yield();
		}
	}
}

double calc_T_price(SBB_instrument_fields* yieldCurve, int curveSize, char* treasuryYr){
	double price;
	double dv01;
	SBB_date from;
	SBB_date to;
	int numPayments = 0;
	for(int i = 0; i < curveSize; i ++){
		if(!strcmp(yieldCurve[i].SecurityID(), treasuryYr)){
			from.set_from_yyyymmdd(yieldCurve[i].SettlementDate());
			to.set_from_yyyymmdd(yieldCurve[i].MaturityDate());
			numPayments = get_number_of_periods_semiannual(from, to);
			//printf("found  is %s \n", yieldCurve[i].SecurityID());
			Calculator* calculator_ptr = new Coupon_Bearing_Calculator(yieldCurve[i].Yield(), yieldCurve[i].Coupon(), numPayments, 1000);
			price = calculator_ptr->calculate();
			
			//printf("dv01 is %.6f \n", dv01);			
		}
	}	
		return price;
}

double calc_T_price_bpchange(SBB_instrument_fields* yieldCurve, int curveSize, char* treasuryYr, double yieldchange){
	double price;
	double dv01;
	SBB_date from;
	SBB_date to;
	int numPayments = 0;
	for(int i = 0; i < curveSize; i ++){
		if(!strcmp(yieldCurve[i].SecurityID(), treasuryYr)){
			from.set_from_yyyymmdd(yieldCurve[i].SettlementDate());
			to.set_from_yyyymmdd(yieldCurve[i].MaturityDate());
			numPayments = get_number_of_periods_semiannual(from, to);
			//printf("found  is %s \n", yieldCurve[i].SecurityID());
			Calculator* calculator_ptr = new Coupon_Bearing_Calculator(yieldCurve[i].Yield()+yieldchange, yieldCurve[i].Coupon(), numPayments, 1000);
			price = calculator_ptr->calculate();
			
			//printf("dv01 is %.6f \n", dv01);			
		}
	}	
		return price;
}

//===================== Gets yield rate according to remaining term (SPREAD) ===============
double get_BenchMark(int period, SBB_instrument_fields* yieldCurve, int curveSize){
	double yieldTmp = 0.0;
	int yrs = 100;
	int index = 0;
	int bMark = 0;
	int currentBMark = 0;
	for (int i = 0; i < curveSize; i++){
		if(!strcmp(yieldCurve[i].SecurityID(),"T2")){
			bMark=2;
		}
		else if(!strcmp(yieldCurve[i].SecurityID(),"T5")){
			bMark=5;
		}
		else if(!strcmp(yieldCurve[i].SecurityID(),"T10")){
			bMark=10;
		}
		else{
			bMark=30;
		}
		if(std::abs(bMark-period) < yrs){
			
			yrs = std::abs(bMark-period);
			index = i;
			currentBMark = bMark;
			
		}
		else if(abs(bMark-period) == yrs){
			if(bMark < currentBMark){
				yrs = std::abs(bMark-period);
				index = i;
				currentBMark = bMark;
			}
		}
	}
	//yieldCurve[index].show();
	//std:: cout << bMark << "    " << yieldCurve[index].Yield() << std::endl; 
	yieldTmp = yieldCurve[index].Yield();
	return yieldTmp;
}

int get_BenchMark_yr(int period, SBB_instrument_fields* yieldCurve, int curveSize){
	double yieldTmp = 0.0;
	int yrs = 100;
	int index = 0;
	int bMark = 0;
	int currentBMark = 0;
	for (int i = 0; i < curveSize; i++){
		if(!strcmp(yieldCurve[i].SecurityID(),"T2")){
			bMark=2;
		}
		else if(!strcmp(yieldCurve[i].SecurityID(),"T5")){
			bMark=5;
		}
		else if(!strcmp(yieldCurve[i].SecurityID(),"T10")){
			bMark=10;
		}
		else{
			bMark=30;
		}
		if(std::abs(bMark-period) < yrs){
			
			yrs = std::abs(bMark-period);
			index = i;
			currentBMark = bMark;
			
		}
		else if(abs(bMark-period) == yrs){
			if(bMark < currentBMark){
				yrs = std::abs(bMark-period);
				index = i;
				currentBMark = bMark;
			}
		}
	}
	//yieldCurve[index].show();
	//std:: cout << bMark << "    " << yieldCurve[index].Yield() << std::endl; 
	return bMark;
}

//===================== Putting bonds into buckets =====================
void bucketize(vector<vector<SBB_instrument_fields> > &v, SBB_instrument_fields* data, int collectionSizeTB){
	SBB_date from,to;
	for (int i=0;i<collectionSizeTB; ++i){
		from.set_from_yyyymmdd(data[i].SettlementDate());
		to.set_from_yyyymmdd(data[i].MaturityDate());
		int x=get_number_of_periods_semiannual(from,to)/2;
		//cout<<"data["<<i<<"]: "<<" "<<x<<endl;
		if(x<=2)
			v[0].push_back(data[i]);
		else if(x>2 && x<=5)
			v[1].push_back(data[i]);
		else if(x>5 && x<=10)
			v[2].push_back(data[i]);
		else
			v[3].push_back(data[i]);
	}
}

/*
//===================== printing bucket (for debug) =====================

void printbuckets(vector<vector<SBB_instrument_fields> > v){
	for(int i=0;i<v.size();++i){
		cout<<"BUCKET["<<i<<"]:"<<endl;
		for(int j=0;j<v[i].size();++j)
			v[i][j].show();
	}
}
*/

//================ Quick sort code taken from: http://codereview.stackexchange.com/questions/77782/quick-sort-implementation ======================
//For sorting the PnL vector
int partition(double *arr, const int left, const int right) {
    const int mid = left + (right - left) / 2;
    const int pivot = arr[mid];
    // move the mid point value to the front.
    std::swap(arr[mid],arr[left]);
    int i = left + 1;
    int j = right;
    while (i <= j) {
        while(i <= j && arr[i] <= pivot) {
            i++;
        }

        while(i <= j && arr[j] > pivot) {
            j--;
        }

        if (i < j) {
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i - 1],arr[left]);
    return i - 1;
}

void quicksort(double *arr, const int left, const int right, const int sz){

    if (left >= right) {
        return;
    }


    int part = partition(arr, left, right);

    

    quicksort(arr, left, part - 1, sz);
    quicksort(arr, part + 1, right, sz);
}



int compare (const void *elem1, const void *elem2){
    return *(double*)elem1 - *(double*)elem2;
}

//============================================================================

//============================== Hedge bucket ================================

void hedge(SBB_instrument_fields data, SBB_instrument_fields* yieldCurve, int curveSize, double &bucketRisk, double &bucketdv01, double &bucketMarketValue,
	double yr2, double yr5, double yr10, double yr30, int bpchange){
		SBB_date from, to;
		int numPayments = 0;
		double price;
		double dv01;
		double risk;
		double marketvalue;
		
		//calculate time
		from.set_from_yyyymmdd(data.SettlementDate());
		to.set_from_yyyymmdd(data.MaturityDate());
		numPayments = get_number_of_periods_semiannual(from, to);
		
		//char* something = new char[SBB_LINE_BUFFER_LENGTH];
		Calculator* calculator_ptr;
		//Calculator* calculator_ptr_T2;
		//YIELD CALCULATION
		if(data.is_yield_priced()){
			if(data.Coupon() == 0.0){
				calculator_ptr = new Zero_Coupon_Calculator(data.Yield()+(bpchange/100), data.Amount(), numPayments);
				//printf("Sensitivity: %.3f \n", calculator_ptr->sensitivity_calculate());
			}
			else{
				calculator_ptr = new Coupon_Bearing_Calculator(data.Yield()+(bpchange/100), data.Coupon(), numPayments, data.Amount());
				//printf("Amount: %d\tRisk: %.3f \n", data.Amount(),calculator_ptr->calculate_risk());
			}
			price = calculator_ptr->calculate();
			dv01 = calculator_ptr->sensitivity_calculate(price);
			risk = calculator_ptr->calculate_risk(dv01);
			marketvalue = (price/100)*data.Amount();

		}
		//SPREAD CALCULATION
		else{
			int yr = get_BenchMark_yr(numPayments/2, yieldCurve, curveSize);
			double ytmp=0;
			if(yr == 2) ytmp = yr2;
			else if(yr == 5) ytmp = yr5;
			else if (yr == 10) ytmp = yr10;
			else ytmp = yr30;
			
			calculator_ptr = new Spread_Calculator(ytmp+(data.Spread()/100)+(bpchange/100), data.Coupon(), numPayments, data.Amount());
			//std::cout << "yield "<< ytmp+(data.Spread()/100) << "# "<< numPayments << std::endl;
			price = calculator_ptr->calculate();
			dv01 = calculator_ptr->sensitivity_calculate(price);
			risk = calculator_ptr->calculate_risk(dv01); //Since US ticker
			marketvalue = (price/100)*data.Amount();
			
			//std::cout << "bmark "<< tmp.period<< std::endl;
		}
		
		bucketRisk += risk;
		// double ytmp = get_BenchMark(2, yieldCurve, curveSize);
		// calculator_ptr_T2 = new Spread_Calculator(ytmp, data.Coupon(), numPayments, data.Amount());
		// price = calculator_ptr->calculate();
		// dv01 = calculator_ptr->sensitivity_calculate(price);
		bucketdv01 += dv01;
		bucketMarketValue+=marketvalue;
}

//=============================================================================



//****************************************************************************************************************
//************************************************* FIXED YIELD **************************************************
//===================== Calculate Price & write price, dv01, risk, LGD to book result file =====================
void calculate_price_dv01_risk_LGD_to_file(SBB_instrument_fields data, SBB_instrument_fields* yieldCurve, 
	int curveSize, SBB_bond_ratings rat, SBB_output_file* file, double &price, double &dv01, double &risk, double &LGD, double &marketvalue){
	SBB_date from;
	SBB_date to;
	int numPayments;

	// double price;
	// double dv01;
	// double risk;

	//calculate number of payments
	from.set_from_yyyymmdd(data.SettlementDate());
	to.set_from_yyyymmdd(data.MaturityDate());
	numPayments = get_number_of_periods_semiannual(from, to);

	char* line_to_write = new char[SBB_LINE_BUFFER_LENGTH];
	Calculator* calculator_ptr;

	if(data.is_yield_priced()){
		//YIELD CALCULATION
		if(data.Coupon() == 0.0){
			calculator_ptr = new Zero_Coupon_Calculator(data.Yield(), data.Amount(), numPayments);
		}
		else{
			calculator_ptr = new Coupon_Bearing_Calculator(data.Yield(), data.Coupon(), numPayments, data.Amount());
		}
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
		//SPREAD CALCULATION
	else{
		double ytmp = get_BenchMark(numPayments/2, yieldCurve, curveSize);
		
		calculator_ptr = new Spread_Calculator(ytmp+(data.Spread()/100), data.Coupon(), numPayments, data.Amount());
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
	marketvalue = (price/100)*data.Amount();
	LGD=rat.LGD_given_SnP_Fitch(data.Quality());
	LGD = LGD * data.Amount()/100;
	if(LGD == -0) LGD =0;

	// totalRisk+=risk;
	// total_marketvalue+=marketvalue;

	sprintf(line_to_write, "%s %.2f %.3f %.3f %.3f\n", data.Line_buffer(), price, dv01, risk, LGD);
	file->write_line(line_to_write);
}


//===================== Calculate price, dv01, risk, and LGD ** NOT ** write to file =====================

void calculate_price_dv01_risk_LGD(SBB_instrument_fields data, SBB_instrument_fields* yieldCurve, 
	int curveSize, SBB_bond_ratings rat, double &price, double &dv01, double &risk, double &LGD, double &marketvalue){
	SBB_date from;
	SBB_date to;
	int numPayments;

	//calculate number of payments
	from.set_from_yyyymmdd(data.SettlementDate());
	to.set_from_yyyymmdd(data.MaturityDate());
	numPayments = get_number_of_periods_semiannual(from, to);

	Calculator* calculator_ptr;

	if(data.is_yield_priced()){
		//YIELD CALCULATION
		if(data.Coupon() == 0.0){
			calculator_ptr = new Zero_Coupon_Calculator(data.Yield(), data.Amount(), numPayments);
		}
		else{
			calculator_ptr = new Coupon_Bearing_Calculator(data.Yield(), data.Coupon(), numPayments, data.Amount());
		}
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
		//SPREAD CALCULATION
	else{
		double ytmp = get_BenchMark(numPayments/2, yieldCurve, curveSize);
		
		calculator_ptr = new Spread_Calculator(ytmp+(data.Spread()/100), data.Coupon(), numPayments, data.Amount());
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
	marketvalue = (price/100)*data.Amount();
	LGD=rat.LGD_given_SnP_Fitch(data.Quality());
	LGD = LGD * data.Amount()/100;
	if(LGD == -0) LGD =0;

}
//****************************************************************************************************************
//****************************************************************************************************************



//****************************************************************************************************************
//******************************************* BasePoint change enabled *******************************************
//===================== Calculate Price & write price, dv01, risk, LGD to book result file =====================
void calculate_price_dv01_risk_LGD_to_file_changerate(SBB_instrument_fields data, SBB_instrument_fields* yieldCurve, 
	int curveSize, SBB_bond_ratings rat, SBB_output_file* file, double &price, double &dv01, double &risk, double &LGD, 
	double &marketvalue, double bpchange){
	SBB_date from;
	SBB_date to;
	int numPayments;

	// double price;
	// double dv01;
	// double risk;

	//calculate number of payments
	from.set_from_yyyymmdd(data.SettlementDate());
	to.set_from_yyyymmdd(data.MaturityDate());
	numPayments = get_number_of_periods_semiannual(from, to);

	char* line_to_write = new char[SBB_LINE_BUFFER_LENGTH];
	Calculator* calculator_ptr;

	if(data.is_yield_priced()){
		//YIELD CALCULATION
		if(data.Coupon() == 0.0){
			calculator_ptr = new Zero_Coupon_Calculator((data.Yield()+(bpchange/100)), data.Amount(), numPayments);
		}
		else{
			calculator_ptr = new Coupon_Bearing_Calculator((data.Yield()+(bpchange/100)), data.Coupon(), numPayments, data.Amount());
		}
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
		//SPREAD CALCULATION
	else{
		double ytmp = get_BenchMark(numPayments/2, yieldCurve, curveSize);
		
		calculator_ptr = new Spread_Calculator(ytmp+(data.Spread()/100)+(bpchange/100), data.Coupon(), numPayments, data.Amount());
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
	marketvalue = (price/100)*data.Amount();
	LGD=rat.LGD_given_SnP_Fitch(data.Quality());
	LGD = LGD * data.Amount()/100;
	if(LGD == -0) LGD =0;

	// totalRisk+=risk;
	// total_marketvalue+=marketvalue;

	sprintf(line_to_write, "%s %.2f %.3f %.3f %.3f\n", data.Line_buffer(), price, dv01, risk, LGD);
	file->write_line(line_to_write);
}


//===================== Calculate price, dv01, risk, and LGD NOT write to file =====================

void calculate_price_dv01_risk_LGD_chagerate(SBB_instrument_fields data, SBB_instrument_fields* yieldCurve, 
	int curveSize, SBB_bond_ratings rat, double &price, double &dv01, double &risk, double &LGD, 
	double &marketvalue, double bpchange){
	SBB_date from;
	SBB_date to;
	int numPayments;

	//calculate number of payments
	from.set_from_yyyymmdd(data.SettlementDate());
	to.set_from_yyyymmdd(data.MaturityDate());
	numPayments = get_number_of_periods_semiannual(from, to);


	Calculator* calculator_ptr;

	if(data.is_yield_priced()){
		//YIELD CALCULATION
		if(data.Coupon() == 0.0){
			calculator_ptr = new Zero_Coupon_Calculator((data.Yield()+(bpchange/100)), data.Amount(), numPayments);
		}
		else{
			calculator_ptr = new Coupon_Bearing_Calculator((data.Yield()+(bpchange/100)), data.Coupon(), numPayments, data.Amount());
		}
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
		//SPREAD CALCULATION
	else{
		double ytmp = get_BenchMark(numPayments/2, yieldCurve, curveSize);
		
		calculator_ptr = new Spread_Calculator((ytmp+(data.Spread()/100)+(bpchange/100)), data.Coupon(), numPayments, data.Amount());
		price = calculator_ptr->calculate();
		dv01 = calculator_ptr->sensitivity_calculate(price);
		risk = calculator_ptr->calculate_risk(dv01);
	}
	marketvalue = (price/100)*data.Amount();
	LGD=rat.LGD_given_SnP_Fitch(data.Quality());
	LGD = LGD * data.Amount()/100;
	if(LGD == -0) LGD =0;

}


void calculate_spread_var(SBB_instrument_fields data, SBB_instrument_fields* yieldCurve, 
	int curveSize, SBB_bond_ratings rat, double &price, double &dv01, double &risk, double &LGD, 
	double &marketvalue, double yieldchange, double bpchange){
	SBB_date from;
	SBB_date to;
	int numPayments;

	//calculate number of payments
	from.set_from_yyyymmdd(data.SettlementDate());
	to.set_from_yyyymmdd(data.MaturityDate());
	numPayments = get_number_of_periods_semiannual(from, to);


	Calculator* calculator_ptr;


	double ytmp = get_BenchMark(numPayments/2, yieldCurve, curveSize);
		
	calculator_ptr = new Spread_Calculator((ytmp+yieldchange+(bpchange/100)), data.Coupon(), numPayments, data.Amount());
	price = calculator_ptr->calculate();
	dv01 = calculator_ptr->sensitivity_calculate(price);
	risk = calculator_ptr->calculate_risk(dv01);

	marketvalue = (price/100)*data.Amount();
	LGD=rat.LGD_given_SnP_Fitch(data.Quality());
	LGD = LGD * data.Amount()/100;
	if(LGD == -0) LGD =0;

}

class sample{
public:
	string tker;
	string quality;
	int amt;
	double risk;
	double lgd;
};


class obj{
public:
	int amt;
	double risk;
	double lgd;
};


//****************************************************************************************************************
//****************************************************************************************************************

void runcode(char (&message)[MSGSIZE], char (&recd_msg)[MSGSIZE]){
	std::cout<<recd_msg<<endl;
	vector <string> fields;

	const char* p;
  	vector<string> commands;
  	for (p = strtok( recd_msg, "," );  p;  p = strtok( NULL, "," ))
  	{
    // printf( "%s\n", p );
    	commands.push_back(p);
  	}
  	//std::cout << commands.size();
	//files:
	SBB_bond_ratings ratings;
	SBB_output_file* result;
	SBB_output_file* book_result;

	int collectionSizeTB_opening;
	int collectionSizeTB_closing;
	int collectionSizeYC;

	string book_opening = "tradingbook_opening.txt";
	string book_closing = "tradingbook_closing.txt";
	string curvefile = "midterm_curve.txt";

	SBB_instrument_fields* data_opening;
	SBB_instrument_fields* data_closing;
	SBB_instrument_fields* yieldCurve;

	//variables 
	vector<vector<SBB_instrument_fields> > buckets(4); //index: 0:(0,2],1:(2,5],2:(5,10],3:(10,30]

	//============== files now handled all together ========
	handleFiles(data_opening, data_closing, yieldCurve, book_result, result, collectionSizeTB_opening, collectionSizeTB_closing, collectionSizeYC,
		book_opening, book_closing, curvefile);


	//********************************************************************************
	//======== using opening book ===========
	//********************************************************************************
	bucketize(buckets, data_opening, collectionSizeTB_opening);
	/*if(strcmp(recd_msg,"RUN22")==0){
		std::cout<<"got 2"<<endl;
		string r="reply 2";
		const char *res2=(const char*)r.c_str();
		sprintf(message,res2);
	}*/
	if(commands[0]== "RUN2")
	{	
		int bucketNumToHedge = 0;

		int spreadamt;

		int bpchange;

		int spreadamt2;
		int spreadamt5;
		int spreadamt10;
		int spreadamt30;




		double yr_2;
		double yr_5;
		double yr_10;
		double yr_30;


		// double yr_2_cp;
		// double yr_5_cp;
		// double yr_10_cp;
		// double yr_30cp;
		//loading yield rate from yield curve
		get_yieldrate(yieldCurve, collectionSizeYC, yr_2, yr_5, yr_10, yr_30);


		if(commands[1]=="0"){//original RUN2,0,0,0
			std::cout << "in command[1]==0"<< endl;
			//bucketNumToHedge = 0;
			spreadamt = 0;
			spreadamt2 = 0;
			spreadamt5 = 0;
			spreadamt10 = 0;
			spreadamt30 = 0;
			bpchange = 0;
		}
		else if(commands[1] == "1"){//bpchange up / down RUN2,1,x,x
			printf("entering bpchange \n");
			if(commands[2]=="0"){//shifting 50 up RUN2,1,0,x
				//bucketNumToHedge = 0;
				//spreadamt = 0;
				bpchange += 50;		
		
			}
			else if(commands[2]=="1"){//down 50 RUN22,1,1,x
				//bucketNumToHedge = 0;
				printf("down \n");
				//spreadamt = 0;
				bpchange += (-50);


				//printf("bpchange %d after command reads %f %f %f %f\n", bpchange, yr_2, yr_5, yr_10, yr_30);


			}
		}
		else if (commands[1]=="2"){ //RUN2,2,x,x
			//printf("entering spread change \n");
			bpchange =0;
			if(commands[2]=="0"){//RUN2,2,0,x -> val in first column x is bp
				spreadamt2 += atoi(commands[3].c_str());
				yr_2 += spreadamt2/100;

			}
			else if(commands[2]=="1"){//RUN2,2,1,x -> val in 2nd column x is bp
				spreadamt5 += atoi(commands[3].c_str());
				yr_5 += spreadamt5/100;
				//printf("in yr 5 change");
			}
			else if(commands[2]=="2"){ //RUN2,2,2,x -> val in 3rd column x is bp
				spreadamt10 += atoi(commands[3].c_str());
				yr_10 += spreadamt10/100;
			}
			else{//RUN2,2,3,x -> val in 4th column x is bp
				spreadamt30 += atoi(commands[3].c_str());
				//std::cout<<"spreadamt30:"<<spreadamt30<<endl;
				yr_30 += spreadamt30/100;
				//std::cout<<"yr30:"<<yr_30<<endl;
			}
		}

		// yr_2 += bpchange/100;
		// yr_5 += bpchange/100;
		// yr_10 += bpchange/100;
		// yr_30 += bpchange/100;
		printf("bpchange %d after command reads %f %f %f %f\n", bpchange, yr_2, yr_5, yr_10, yr_30);
		// std::string myString = "45";
		// int value = atoi(myString.c_str()); //value = 45

		
		char* TYear = new char[3];

		double t2dv01 = 0;

		sprintf(TYear, "T2");
		t2dv01 = calc_T_dv01(yieldCurve, collectionSizeYC, TYear, yr_2, spreadamt);

		bucketNumToHedge = 0;
		double totalBucketDv01_2=0;
		double totalBucketRisk_2=0;
		double totalBucketAmt_2=0;
			double totalBucketMarketValue_2=0;
		for (int i = 0; i < buckets[bucketNumToHedge].size(); i++){
			hedge(buckets[bucketNumToHedge][i], yieldCurve, collectionSizeYC, totalBucketRisk_2, totalBucketDv01_2, totalBucketMarketValue_2, yr_2, yr_5, yr_10, yr_30, bpchange);
			totalBucketAmt_2 += buckets[bucketNumToHedge][i].Amount();
		}
		double amount2 = 0-(totalBucketDv01_2*totalBucketAmt_2/100/t2dv01);


		bucketNumToHedge = 1;
		double totalBucketDv01_5=0;
		double totalBucketRisk_5=0;
		double totalBucketAmt_5=0;
		double totalBucketMarketValue_5=0;

		for (int i = 0; i < buckets[bucketNumToHedge].size(); i++){
			hedge(buckets[bucketNumToHedge][i], yieldCurve, collectionSizeYC, totalBucketRisk_5, totalBucketDv01_5, totalBucketMarketValue_5, yr_2, yr_5, yr_10, yr_30,bpchange);
			totalBucketAmt_5 += buckets[bucketNumToHedge][i].Amount();
		}
		double amount5 = 0-(totalBucketDv01_5*totalBucketAmt_5/100/t2dv01);

		bucketNumToHedge = 2;
		double totalBucketDv01_10=0;
		double totalBucketRisk_10=0;
		double totalBucketAmt_10=0;
		double totalBucketMarketValue_10=0;

		for (int i = 0; i < buckets[bucketNumToHedge].size(); i++){
			hedge(buckets[bucketNumToHedge][i], yieldCurve, collectionSizeYC, totalBucketRisk_10, totalBucketDv01_10, totalBucketMarketValue_10, yr_2, yr_5, yr_10, yr_30, bpchange);
			totalBucketAmt_10 += buckets[bucketNumToHedge][i].Amount();
		}
		double amount10 = 0-(totalBucketDv01_10*totalBucketAmt_10/100/t2dv01);

		bucketNumToHedge = 3;
		double totalBucketDv01_30=0;
		double totalBucketRisk_30=0;
		double totalBucketAmt_30=0;
		double totalBucketMarketValue_30=0;

		for (int i = 0; i < buckets[bucketNumToHedge].size(); i++){
			hedge(buckets[bucketNumToHedge][i], yieldCurve, collectionSizeYC, totalBucketRisk_30, totalBucketDv01_30, totalBucketMarketValue_30, yr_2, yr_5, yr_10, yr_30, bpchange);
			totalBucketAmt_30 += buckets[bucketNumToHedge][i].Amount();
		}
		double amount30 = 0-(totalBucketDv01_30*totalBucketAmt_30/100/t2dv01);



		printf("Amount of 2 year treasury to hedge the 30 yr bucket: %.3f \nMarket Value: %.3f \nRisk: %.3f\n", amount30, totalBucketMarketValue_30, totalBucketRisk_30);
		printf("Amount of 2 year treasury to hedge the 10 yr bucket: %.3f \nMarket Value: %.3f \nRisk: %.3f\n", amount10, totalBucketMarketValue_10, totalBucketRisk_10);
		printf("Amount of 2 year treasury to hedge the 5 yr bucket: %.3f \nMarket Value: %.3f \nRisk: %.3f\n", amount5, totalBucketMarketValue_5, totalBucketRisk_5);
		printf("Amount of 2 year treasury to hedge the 2 yr bucket: %.3f \nMarket Value: %.3f \nRisk: %.3f\n", amount2, totalBucketMarketValue_2, totalBucketRisk_2);
		printf("%d \n" ,bpchange);

		cout<<"RUN2 received"<<endl;
		string res="";
		stringstream ss;
		ss<<"2YR "<<totalBucketRisk_2<<" "<<totalBucketMarketValue_2<<" "<<amount2<<"\n";
		ss<<"5YR "<<totalBucketRisk_5<<" "<<totalBucketMarketValue_5<<" "<<amount5<<"\n";
		ss<<"10YR "<<totalBucketRisk_10<<" "<<totalBucketMarketValue_10<<" "<<amount10<<"\n";
		ss<<"30YR "<<totalBucketRisk_30<<" "<<totalBucketMarketValue_30<<" "<<amount30<<"\n";
		/*
		res += "2YR\n";
		res = res + to_string(totalBucketRisk_2) + " " + to_string(totalBucketMarketValue_2) + " " +to_string(amount2);
		res += "\n";
		res += "5YR\n";
		res = res + to_string(totalBucketRisk_5) + " " + to_string(totalBucketMarketValue_5) + " " +to_string(amount5);
		res += "\n";
		res += "10YR\n";
		res = res + to_string(totalBucketRisk_10) + " " + to_string(totalBucketMarketValue_10) + " " +to_string(amount10);
		res += "\n";
		res += "30YR\n";
		res = res + to_string(totalBucketRisk_30) + " " + to_string(totalBucketMarketValue_30) + " " +to_string(amount30);
		res += "\n";

		res+= "yield curve\n";
		res = res + to_string(yr_2) + " " + to_string(yr_5) + " " + to_string(yr_10) + " " + to_string(yr_30);*/
		res=ss.str();
		const char *res2=(const char*)res.c_str();
		//strcpy(res,res2);
		sprintf(message,res2);
		return;

	}

else if(commands[0]=="RUN1" || commands[0]=="RUN3"){
	SBB_var_input_file daily;
	SBB_var_input_file spread;
	SBB_var_day_record* var_fields;
	SBB_var_day_record* t_fields;
	//for general purpose
	int pnl_length;
	//regular price (before change)
	double temp_price_op=0;
	double temp_dv01_op=0;
	double temp_risk_op=0;
	double temp_LGD_op=0;
	double temp_marketvalue_op=0;
	//VaR price (after change)
	double var_price_op=0;
	double var_dv01_op=0;
	double var_risk_op=0;
	double var_LGD_op=0;
	double var_marketvalue_op=0;
	double var_bpchange_op=0;
	double var_yieldchange_op=0;

	double t_price_op = 0;
	double var_t_price_op;


	//separate pnl vector for spread and yield
	//all together is pnlVectorTotal
	vector< vector<double> > pnlVectorTotal_op;
	vector< vector<double> > pnlVectorYield_op;
	vector< vector<double> > pnlVectorSpread_op;

	vector<int> AmtBefore;
	vector<int> AmtAfter;
	int totalPositionChange = 0;

	
	//for result.txt
	int largestLong=0;
	int largestShort=0;
	double mostRisk=0;
	double totalRisk=0;
	
	//for calculating market value change
	double total_marketValue=0;
	double total_marketValue_after_change=0;
	double marketValueChange=0;

	//for LGD change
	vector<double> LGD_before_change;
	vector<double> LGD_after_change;
	double LGD_Change=0;

	//group by ratings
	vector<sample> opening_results;
	vector<sample> closing_results;
	



	for(int i = 0; i < collectionSizeTB_opening; i ++){
		calculate_price_dv01_risk_LGD_to_file(data_opening[i], yieldCurve, collectionSizeYC, ratings, book_result, temp_price_op, temp_dv01_op, temp_risk_op, temp_LGD_op, temp_marketvalue_op);
		LGD_before_change.push_back(temp_LGD_op);

		//
		sample temp;
		temp.tker = string(data_opening[i].Ticker());
		temp.quality = string(data_opening[i].Quality());
		temp.amt = data_opening[i].Amount();
		temp.risk = temp_risk_op;
		temp.lgd = temp_LGD_op;
		//{data_opening[i].SecurityID(), data_opening[i].Quality(), data_opening[i].Amount(), risk, LGD};
		std:cout<<"Ticker:"<<temp.tker<<" Quality:"<<temp.quality<<" Amount:"<<temp.amt<<" Risk:"<<temp.risk<<" LGD:"<<temp.lgd<<endl;
		opening_results.push_back(temp);



		if(data_opening[i].Amount() > largestLong) largestLong = data_opening[i].Amount();
		if(data_opening[i].Amount() < largestShort) largestShort = data_opening[i].Amount();
		if(abs(temp_risk_op) > abs(mostRisk)) mostRisk = temp_risk_op;
		totalRisk+=temp_risk_op;
		AmtBefore.push_back(data_opening[i].Amount());
		total_marketValue += temp_marketvalue_op;

		//calculating bp100 change on opening to get total market value change
		// calculate_price_dv01_risk_LGD_chagerate(data_opening[i], yieldCurve, collectionSizeYC, ratings, temp_price_op, temp_dv01_op, temp_risk_op, temp_LGD_op, temp_marketvalue_op, 100);
		// total_marketValue_after_change += marketvalue;

		char* s = new char[SBB_LINE_BUFFER_LENGTH];
		sprintf(s, "var/%s.txt", data_opening[i].SecurityID());
		daily.open(s);
		var_fields = daily.records(pnl_length);
		vector<double> pnl_bond; 

		//yield priced var
		if(data_opening[i].is_yield_priced()){
			for(int j = 1; j < pnl_length; j++){
				var_bpchange_op = ((var_fields[j].ValValue())-var_fields[j-1].ValValue())*100; //calculate the bpchange and then use it to calc new price
				calculate_price_dv01_risk_LGD_chagerate(data_opening[i], yieldCurve, collectionSizeYC, ratings, var_price_op, var_dv01_op, var_risk_op, var_LGD_op, 
				var_marketvalue_op, var_bpchange_op);
				pnl_bond.push_back((var_price_op - temp_price_op)/100*data_opening[i].Amount());
			}
			pnlVectorYield_op.push_back(pnl_bond);

		}
		//spread priced var
		else{
			//vector<double> pnl_bond_T;
			char* sp = new char[SBB_LINE_BUFFER_LENGTH];
			//sprintf(sp, "var/%s.txt", var_fields[0].BenchmarkID());//doesn't work!!
			sprintf(sp, "var/%s.txt", "T2");
			spread.open(sp);
			//t_fields_op = spread.records(pnl_length);
			//t_price_op = calc_T_price(yieldCurve, collectionSizeYC, "T2");
			//printf("tprice: %.3f \n", t_price);
			for(int j = 1; j < pnl_length; j++){
				//var_yieldchange = ((t_fields[j].ValValue())-(t_fields[j-1].ValValue())); //taken from T2: add to yield rate
				//printf("%.2f \n",var_yieldchange );
				var_bpchange_op = ((var_fields[j].ValValue())-var_fields[j-1].ValValue()); //taken from spread hist file: add to spread bpchange
				//printf("%.2f \n",var_bpchange );
				//double finalbp = var_bpchange + var_yieldchange;
				calculate_spread_var(data_opening[i], yieldCurve, collectionSizeYC, ratings, var_price_op, var_dv01_op, var_risk_op, var_LGD_op, 
				var_marketvalue_op,0, var_bpchange_op+data_opening[i].Spread());
				// calculate_price_dv01_risk_LGD_chagerate(data_closing[i], yieldCurve, collectionSizeYC, ratings, var_price, var_dv01, var_risk, var_LGD, 
				// var_marketvalue, var_bpchange);
				//var_t_price_op = calc_T_price_bpchange(yieldCurve, collectionSizeYC, "T2", var_yieldchange);
				pnl_bond.push_back(((var_price_op - temp_price_op))/100*data_opening[i].Amount());
				//pnl_bond_T.push_back((var_t_price - t_price));
				// printf("new price%.6f\n", var_price);
				// //printf("%.3f\n", finalbp);
				// printf("pricechange: %6f \n\n",(var_price - temp_price)/100*data_closing[i].Amount() );
			}
			pnlVectorSpread_op.push_back(pnl_bond);
			//pnlVectorSpread.push_back(pnl_bond_T);
		}
	
		pnlVectorTotal_op.push_back(pnl_bond);
	
	}

	double bookpnl_op[pnl_length-1];
	double yieldpnl_op[pnl_length-1];
	double spreadpnl_op[pnl_length-1];

	pnl_length-=1;

	//add the total values for pnl
	for(int i = 0; i < pnl_length; i ++){
		double temp1 = 0.0;
		double temp2 = 0.0;
		double temp3 = 0.0;

		for(int j = 0; j < collectionSizeTB_opening; j++){
			temp1 += pnlVectorTotal_op[j][i];
		}
		bookpnl_op[i] = temp1;

		for(int j = 0; j < pnlVectorYield_op.size(); j++){
			temp2 += pnlVectorYield_op[j][i];
		}
		yieldpnl_op[i]=temp2;

		for(int j = 0; j < pnlVectorSpread_op.size(); j++){
			temp3 += pnlVectorSpread_op[j][i];
		}
		spreadpnl_op[i]=temp3;
	}
	qsort(bookpnl_op, pnl_length, sizeof(double), compare);
	qsort(yieldpnl_op, pnl_length, sizeof(double), compare);
	qsort(spreadpnl_op, pnl_length, sizeof(double), compare);



	//calculating index
	double conf_interval_op = 99;
	int pnlIndex_op = (pnl_length - (conf_interval_op/100) * pnl_length);
	double pnl_var_op= bookpnl_op[pnlIndex_op];
	double yield_var_op = yieldpnl_op[pnlIndex_op];
	double spread_var_op = spreadpnl_op[pnlIndex_op];




	// char* s = new char[SBB_LINE_BUFFER_LENGTH];
	// sprintf(s, "%d \n%d \n%.3f \n%.3f", largestLong, largestShort, mostRisk, totalRisk);
	// result->write_line(s);

	// marketValueChange = abs(total_marketValue_after_change - total_marketValue);

	//for hedge calculation


	//original..
	// double amount2 = 0;
	// double amount5 = 0;
	// double amount10 = 0;
	// double amount30 = 0;

	//********************************************************************************
	//======== using closing book ===========
	//********************************************************************************
	//for VaR

	pnl_length=0;
	//regular price (before change)
	double temp_price=0;
	double temp_dv01=0;
	double temp_risk=0;
	double temp_LGD=0;
	double temp_marketvalue=0;
	//VaR price (after change)
	double var_price=0;
	double var_dv01=0;
	double var_risk=0;
	double var_LGD=0;
	double var_marketvalue=0;
	double var_bpchange=0;
	double var_yieldchange=0;

	double t_price = 0;
	double var_t_price;


	//separate pnl vector for spread and yield
	//all together is pnlVectorTotal
	vector< vector<double> > pnlVectorTotal;
	vector< vector<double> > pnlVectorYield;
	vector< vector<double> > pnlVectorSpread;

	//Everything now keeped in one single for loop through the book to avoid repetition
	for(int i = 0; i < collectionSizeTB_closing; i++){
		AmtAfter.push_back(data_closing[i].Amount());
		totalPositionChange += AmtAfter[i]-AmtBefore[i];
		//printf("bond: %s\n", data_closing[i].SecurityID());
		calculate_price_dv01_risk_LGD(data_closing[i], yieldCurve, collectionSizeYC, ratings, temp_price, temp_dv01, temp_risk, temp_LGD, temp_marketvalue);
		
		//Result_item formate: ticker, quality, amount, risk ,lgd
	
		sample temp;

		temp.tker = string(data_closing[i].Ticker());
		temp.quality = string(data_closing[i].Quality());
		temp.amt = data_closing[i].Amount();
		temp.risk = temp_risk;
		temp.lgd = temp_LGD;
		//{data_opening[i].SecurityID(), data_opening[i].Quality(), data_opening[i].Amount(), risk, LGD};
		//std:cout<<"Ticker:"<<temp.tker<<" Quality:"<<temp.quality<<" Amount:"<<temp.amt<<" Risk:"<<temp.risk<<" LGD:"<<temp.lgd<<endl;
		closing_results.push_back(temp);
		//

		LGD_after_change.push_back(temp_LGD);
		LGD_Change+=LGD_after_change[i]-LGD_before_change[i];
		//printf("old price %.2f\n", temp_price);

		char* s = new char[SBB_LINE_BUFFER_LENGTH];
		sprintf(s, "var/%s.txt", data_closing[i].SecurityID());
		daily.open(s);
		var_fields = daily.records(pnl_length);
		vector<double> pnl_bond; 

		//yield priced var
		if(data_closing[i].is_yield_priced()){
			for(int j = 1; j < pnl_length; j++){
				var_bpchange = ((var_fields[j].ValValue())-var_fields[j-1].ValValue())*100; //calculate the bpchange and then use it to calc new price
				calculate_price_dv01_risk_LGD_chagerate(data_closing[i], yieldCurve, collectionSizeYC, ratings, var_price, var_dv01, var_risk, var_LGD, 
				var_marketvalue, var_bpchange);
				pnl_bond.push_back((var_price - temp_price)/100*data_closing[i].Amount());
				// printf("new price%.6f\n", var_price);
				// printf("%.3f\n", var_bpchange);
				// printf("pricechange: %6f \n\n",(var_price - temp_price)/100*data_closing[i].Amount() );
			}
			pnlVectorYield.push_back(pnl_bond);

		}
		//spread priced var
		else{
			vector<double> pnl_bond_T;
			char* sp = new char[SBB_LINE_BUFFER_LENGTH];
			//sprintf(sp, "var/%s.txt", var_fields[0].BenchmarkID());//doesn't work!!
			sprintf(sp, "var/%s.txt", "T2");
			spread.open(sp);
			t_fields = spread.records(pnl_length);
			//t_price = calc_T_price(yieldCurve, collectionSizeYC, "T2");
			//printf("tprice: %.3f \n", t_price);
			for(int j = 1; j < pnl_length; j++){
				//var_yieldchange = ((t_fields[j].ValValue())-(t_fields[j-1].ValValue())); //taken from T2: add to yield rate
				//printf("%.2f \n",var_yieldchange );
				var_bpchange = ((var_fields[j].ValValue())-var_fields[j-1].ValValue()); //taken from spread hist file: add to spread bpchange
				//printf("%.2f \n",var_bpchange );
				//double finalbp = var_bpchange + var_yieldchange;
				calculate_spread_var(data_closing[i], yieldCurve, collectionSizeYC, ratings, var_price, var_dv01, var_risk, var_LGD, 
				var_marketvalue,0, var_bpchange+data_closing[i].Spread());
				// calculate_price_dv01_risk_LGD_chagerate(data_closing[i], yieldCurve, collectionSizeYC, ratings, var_price, var_dv01, var_risk, var_LGD, 
				// var_marketvalue, var_bpchange);
				//var_t_price = calc_T_price_bpchange(yieldCurve, collectionSizeYC, "T2", var_yieldchange);
				pnl_bond.push_back(((var_price - temp_price))/100*data_closing[i].Amount());
				//pnl_bond_T.push_back((var_t_price - t_price));
				// printf("new price%.6f\n", var_price);
				// //printf("%.3f\n", finalbp);
				// printf("pricechange: %6f \n\n",(var_price - temp_price)/100*data_closing[i].Amount() );
			}
			pnlVectorSpread.push_back(pnl_bond);
			//pnlVectorSpread.push_back(pnl_bond_T);
		}
	
		pnlVectorTotal.push_back(pnl_bond);
	
	}
	//final results pnl vector 
	double bookpnl[pnl_length-1];
	double yieldpnl[pnl_length-1];
	double spreadpnl[pnl_length-1];

	pnl_length-=1;

	//add the total values for pnl
	for(int i = 0; i < pnl_length; i ++){
		double temp1 = 0.0;
		double temp2 = 0.0;
		double temp3 = 0.0;

		for(int j = 0; j < collectionSizeTB_closing; j++){
			temp1 += pnlVectorTotal[j][i];
		}
		bookpnl[i] = temp1;

		for(int j = 0; j < pnlVectorYield.size(); j++){
			temp2 += pnlVectorYield[j][i];
		}
		yieldpnl[i]=temp2;

		for(int j = 0; j < pnlVectorSpread.size(); j++){
			temp3 += pnlVectorSpread[j][i];
		}
		spreadpnl[i]=temp3;
	}





	//sort the pnl vectors
	// quicksort(bookpnl, 0, pnl_length-1, pnl_length);
	// quicksort(yieldpnl, 0, pnl_length-1, pnl_length);
	// quicksort(spreadpnl, 0, pnl_length-1, pnl_length);

	qsort(bookpnl, pnl_length, sizeof(double), compare);
	qsort(yieldpnl, pnl_length, sizeof(double), compare);
	qsort(spreadpnl, pnl_length, sizeof(double), compare);



	//calculating index
	double conf_interval = 99;
	int pnlIndex = (pnl_length - (conf_interval/100) * pnl_length);
	double pnl_var= bookpnl[pnlIndex];
	double yield_var = yieldpnl[pnlIndex];
	double spread_var = spreadpnl[pnlIndex];

	// //print out result for 99th interval of all
	// printf("the %.1f th confidence interval of pnl of the book is: %.5f\n", conf_interval, pnl_var);
	// printf("the %.1f th confidence interval of pnl yield is: %.5f\n", conf_interval, yield_var);
	// printf("the %.1f th confidence interval of pnl spread is: %.5f\n", conf_interval, spread_var);
	
	// std::cout << "opening.." << endl;
	// printf("the %.1f th confidence interval of pnl of the book is: %.5f\n", conf_interval_op, pnl_var_op);
	// printf("the %.1f th confidence interval of pnl yield is: %.5f\n", conf_interval_op, yield_var_op);
	// printf("the %.1f th confidence interval of pnl spread is: %.5f\n", conf_interval_op, spread_var_op);
	
	// for (int i = 0; i < pnl_length; i++){
	// 	//totalpnl += bookpnl[i];
	// 	printf("%d: %.1f \n",i, spreadpnl[i]);
	// }
	sprintf(message,"Total Position change: %d\nTotal LGD Change : %f",totalPositionChange, LGD_Change);
	for(int i=0;i<opening_results.size();++i){
		std::cout<<"Vector: "<<opening_results[i].tker<<" "<<opening_results[i].quality<<endl;
		//printf("%s %s \n", opening_results[i].tker, opening_results[i].quality);
	}
	unordered_map<string,obj> mp_ticker_op;
	unordered_map<string,obj> mp_quality_op;
	unordered_map<string,obj> mp_ticker_close;
	unordered_map<string,obj> mp_quality_close;
	unordered_map<string, obj>::iterator it;
	for (int i = 0; i < opening_results.size(); i++){
		string s(opening_results[i].tker);//Ticker
		//printf("%s ",s.c_str() );
		//std::cout<<s<<endl;
		
		it=mp_ticker_op.find(s);
		if(it==mp_ticker_op.end()){
			//printf("Did not find %s\n",s.c_str());
			obj ob;
			ob.amt = opening_results[i].amt;
			ob.risk = opening_results[i].risk;
			ob.lgd = opening_results[i].lgd;
			mp_ticker_op[s] = ob;
			//printf("Exiting found\n");
		}
		else{
			//printf("Found %s\n",s.c_str());
			(*it).second.amt += opening_results[i].amt;
			(*it).second.risk += opening_results[i].risk;
			(*it).second.lgd += opening_results[i].lgd;
			//printf("Exiting not found\n");
		}
	}
	for (int i = 0; i < opening_results.size(); i++){
		string s(opening_results[i].quality);//Quality
		//printf("%s ",s.c_str() );
		//std::cout<<s<<endl;
		
		it=mp_quality_op.find(s);
		if(it==mp_quality_op.end()){
			obj ob;
			ob.amt = opening_results[i].amt;
			ob.risk = opening_results[i].risk;
			ob.lgd = opening_results[i].lgd;
			mp_quality_op[s] = ob;
			
		}
		else{
			//printf("Found %s\n",s.c_str());
			(*it).second.amt += opening_results[i].amt;
			(*it).second.risk += opening_results[i].risk;
			(*it).second.lgd += opening_results[i].lgd;
			//printf("Exiting not found\n");
		}
	}
	std::cout<<"Opening: Ticker"<<endl;
	for(it = mp_ticker_op.begin();it!=mp_ticker_op.end();++it){
		std::cout<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<endl;
		//printf("%s %d %f %f\n",it->first.c_str(),it->second.amt,it->second.risk,it->second.lgd);
	}
	std::cout<<"Opening: Quality"<<endl;
	for(it = mp_quality_op.begin();it!=mp_quality_op.end();++it){
		std::cout<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<endl;
		//printf("%s %d %f %f\n",it->first.c_str(),it->second.amt,it->second.risk,it->second.lgd);
	}
	////////////	CLOSING RESULTS      /////////////
	for (int i = 0; i < closing_results.size(); i++){
		string s(closing_results[i].tker);//Ticker
		//printf("%s ",s.c_str() );
		//std::cout<<s<<endl;
		
		it=mp_ticker_close.find(s);
		if(it==mp_ticker_close.end()){
			//printf("Did not find %s\n",s.c_str());
			obj ob;
			ob.amt = closing_results[i].amt;
			ob.risk = closing_results[i].risk;
			ob.lgd = closing_results[i].lgd;
			mp_ticker_close[s] = ob;
			//printf("Exiting found\n");
		}
		else{
			//printf("Found %s\n",s.c_str());
			(*it).second.amt += closing_results[i].amt;
			(*it).second.risk += closing_results[i].risk;
			(*it).second.lgd += closing_results[i].lgd;
			//printf("Exiting not found\n");
		}
	}
	for (int i = 0; i < closing_results.size(); i++){
		string s(closing_results[i].quality);//Quality
		//printf("%s ",s.c_str() );
		//std::cout<<s<<endl;
		
		it=mp_quality_close.find(s);
		if(it==mp_quality_close.end()){
			obj ob;
			ob.amt = closing_results[i].amt;
			ob.risk = closing_results[i].risk;
			ob.lgd = closing_results[i].lgd;
			mp_quality_close[s] = ob;
			
		}
		else{
			//printf("Found %s\n",s.c_str());
			(*it).second.amt += closing_results[i].amt;
			(*it).second.risk += closing_results[i].risk;
			(*it).second.lgd += closing_results[i].lgd;
			//printf("Exiting not found\n");
		}
	}
	std::cout<<"Closing: Ticker"<<endl;
	for(it = mp_ticker_close.begin();it!=mp_ticker_close.end();++it){
		std::cout<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<endl;
		//printf("%s %d %f %f\n",it->first.c_str(),it->second.amt,it->second.risk,it->second.lgd);
	}
	std::cout<<"Closing: Quality"<<endl;
	for(it = mp_quality_close.begin();it!=mp_quality_close.end();++it){
		std::cout<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<endl;
		//printf("%s %d %f %f\n",it->first.c_str(),it->second.amt,it->second.risk,it->second.lgd);
	}

	if(commands[0]=="RUN1"){
		cout<<"RUN1 received"<<endl;
		stringstream ss;
		string res="";
		//copy (amt,risk,kgd) for all ticker on opening book
		//res+="OpeningTicker ";
		ss<<"OpeningTicker\n";
		unordered_map<string,obj>::iterator it;
		for(it=mp_ticker_op.begin();it!=mp_ticker_op.end();++it){
			ss<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<"\n";
			//res = res + it->first + " " + to_string(it->second.amt) + " " + to_string(it->second.risk) + " " + to_string(it->second.lgd) + "\n";
		}
		//res+="OpeningQuality ";
		ss<<"OpeningQuality\n";
		//copy (amt,risk,kgd) for all quality on opening book
		for(it=mp_quality_op.begin();it!=mp_quality_op.end();++it){
			ss<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<"\n";
			//res = res + it->first + " " + to_string(it->second.amt) + " " + to_string(it->second.risk) + " " + to_string(it->second.lgd) + "\n";
		}
		//res+="ClosingTicker ";
		ss<<"ClosingTicker\n";
		for(it=mp_ticker_close.begin();it!=mp_ticker_close.end();++it){
			ss<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<"\n";
			//res = res + it->first + " " + to_string(it->second.amt) + " " + to_string(it->second.risk) + " " + to_string(it->second.lgd) + "\n";
		}
		//res+="ClosingQuality ";
		ss<<"ClosingQuality\n";
		for(it=mp_quality_close.begin();it!=mp_quality_close.end();++it){
			ss<<it->first<<" "<<it->second.amt<<" "<<it->second.risk<<" "<<it->second.lgd<<"\n";
			//res = res + it->first + " " + to_string(it->second.amt) + " " + to_string(it->second.risk) + " " + to_string(it->second.lgd) + "\n";
		}
		ss<<"OpeningVaR\n";
		//res +="OpeningVaR ";
		ss<<pnl_var_op<<" "<<yield_var_op<<" "<<spread_var_op<<"\n";
		//res = res + to_string(pnl_var_op) + " " + to_string(yield_var_op) + " " + to_string(spread_var_op) + "\n";
		
		//res +="ClosingVaR ";
		ss<<"ClosingVaR\n";

		//res = res + to_string(pnl_var) + " " + to_string(yield_var) + " " + to_string(spread_var) + " ";
		ss<<pnl_var<<" "<<yield_var<<" "<<spread_var<<"\n"<<endl;
		res = ss.str();
		cout<<res<<endl;
		const char *res2=(const char*)res.c_str();
		//strcpy(res,res2);
		sprintf(message,res2);
		return;
	}

	else if(commands[0]=="RUN3"){
		vector<double> bookpnl_run3;
		string res="";
		stringstream ss;

		//res += "TotalVaR ";
		ss<<"TotalVaR ";
		for (int i = 0; i < pnl_length; i++){
			//res = res + to_string(bookpnl[i]) + " ";
			ss<<bookpnl[i]<<" ";
			bookpnl_run3.push_back(bookpnl[i]);
		}
		//res += "\n";
		ss<<"\n";
		
		//strcpy(res,res2);
		sort(bookpnl_run3.begin(),bookpnl_run3.end());
		double bookpnl_run3_min=bookpnl_run3[0], bookpnl_run3_max=bookpnl_run3[bookpnl_run3.size()-1];
		cout<<"pnl min: "<<bookpnl_run3_min<<" pnl max:"<<bookpnl_run3_max<<" pnl size:"<<bookpnl_run3.size()<<endl;
		double buc_width=(bookpnl_run3_max - bookpnl_run3_min)/8;
		double buc_beg = bookpnl_run3_min, buc_end = buc_beg + buc_width;
		vector<int> cnt(8,0);
		int cnt_id=0;
		ss.str("");
		cout<<"new ss"<<ss.str()<<endl;
		ss<<"TotalVaR ";
		for(int i=0;i<8;++i){
			for(int j=0;j<bookpnl_run3.size();++j){
				if(bookpnl_run3[j]>=buc_beg&&bookpnl_run3[j]<=buc_end){
					cnt[i]++;
				}
			}
			cout<<"bucket["<<i<<"] contains:"<<cnt[i]<<endl;
			ss<<cnt[i]<<" ";
			buc_beg = buc_end + 0.0001;
			buc_end += buc_width;
		}
		ss<<'\n';
		res=ss.str();
		const char *res3=(const char*)res.c_str();
		sprintf(message,res3);
		return;
	}

}



	else{
		string res = "Invalid Command, try again.\n";
		const char *res2=(const char*)res.c_str();
		//strcpy(res,res2);
		sprintf(message,res2);
		return;
	}





}

















