/*
 * generalTools.h
 *
 *  Created on: 2014/08/08
 *      Author: sasaki
 */

#ifndef GENERALTOOLS_H_
#define GENERALTOOLS_H_

#include "stack"
#include <utility>
#include <vector>
#include <map>
#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include "Parameters.h"
#include <math.h>
#include <cstdio>
#include <iostream>
#include <string>
#include "tuple.hpp"
#include "tuple_comparison.hpp"
#include "tuple_io.hpp"

using namespace std;


std::string int2string(int value);

class CompareDist
{
public:
    bool operator()(pair<int,double> n1,pair<int,double> n2) {
        return n1.second>n2.second;
    }
};

class Category{
private:
	//string category[CATEGORY_LAYER];
public:
	string category[CATEGORY_LAYER];
	Category();
	Category(string);
};

class Vertice
{
private:
	int verticeId;
	double ratingsum;
	Category categoryId;
	std::vector<double>ratings;
	std::vector<int> neighbors;
	std::vector<double>weights;
public:
	Vertice(int);
	void ShowVertice();
	void AddNeighbors(int, double);
	void AddRatings(double);
	Category GetCategory();
	void SetCategory(Category);
	int GetNeighborsSize();
	double GetRating(int);
	int GetNeighbors(int);
	double GetWeights(int);
};

class UndirectedWeightedLabeledGraph
{
private:
	int size;
	std::vector< Vertice > verticeSet;
	vector< boost::tuple<int, string, int, int> > foursquareCategoryTree;//<pointer to its parent node,category ID,ID w.r.t its parent, layer height>
	vector<int> foursquareTreeChildNum;
	vector<int> foursquareCategoryCount;
	vector<double> distanceSort;                         //thomas' addition

public:
	UndirectedWeightedLabeledGraph(int);
	void graphDistanceSort(string);                     //thomas' addition
	void graphSortedDistanceFile(string);               //thomas' addition
	void graphFileInput(string);
	void graphFileFoursquareInput(string, string);
	void graphFileInputSyntheticPoI(string);
	void graphGenerateSyntheticGrid();
	void ShowGraph();
	void AddEdge(int, int, double);
	int GetGraphSize();
	Category GetVerticeCategory(int nodeId);
	void SetVerticeCategory(int nodeId,Category);
	int GetVerticeNeighborSize(int nodeId);
	int GetVerticeNeighbor(int nodeId, int);
	double GetVerticeWeight(int nodeId, int);
	double GetVerticeRating(int nodeId, int);
	int GetChildNum(int id);
	int GetCategoryChildNum(Category category);
	int GetCategoryCount(int cid);
	int GetCategoryCount(Category cid);
	string GetCategoryId(Category cid);
};

class Result
{
private:

	int ioCost;
	clock_t startTime;
	clock_t endTime;
	int routeSetNum;
	int iterationNum;
	int initialRouteSetNum;
	double increaseRate;
	double initialDiff;

	int totalIOCost;
	double totalTime;
	double totalRouteSetNum;
	int totalIterationNum;
	double totalIncreaseRate;
	double totalInitialDiff;
	int totalInitialRouteSetNum;
	double totalSearchRange;
	double maxRSS;
	double graphRSS;

public:
	Result();
	void IOCostAdd();
	void IOCostClear();
	void IterationAdd();
	void IterationClear();
	void SetQueryStartTime(clock_t);
	void SetQueryEndTime(clock_t);
	void SetRouteSetNum(int);
	void SetInitialRouteSetNum(int);
	void SetIncreaseRate(double);
	void SetInitialDiff(double);
	void SetMaxRSS(double);
	void SetGraphRSS(double);
	void AddTotalSearchRange(double);
	void AddTotal();
	void TotalClear();
	void ResultOutput(string);
	void ResultAverageOutPut(string);
	void ShowResult(int);
	void ShowIO();
	int GetIO();

};

class Combination
{
private:
	int* sample;
	int categoryNum;
	int** combi;
	int n_case;

public:
	Combination(int c_num,int* sample_num);
	void FreeCombi();
	int** GetAllCombi();
	int GetCombi(int,int);
	int GetNcaseNum();
	void MinSort();
	void MaxSort();
};


int ReadInt(FILE *fp, int x, int y);
void ReadIntArray(FILE *fp, int x, int array[]);
void WriteInt(FILE *fp, int c, int x, int y);
double ReadDouble(FILE *fp, int x, int y);
void ReadDoubleArray(FILE *fp, int x, double array[]);
void WriteDouble(FILE *fp, double c, int x, int y);
double floor2(double dIn, int nLen);


#endif /* GENERALTOOLS_H_ */
