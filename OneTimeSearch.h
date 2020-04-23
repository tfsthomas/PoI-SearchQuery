/*
 * OneTimeSearch.h
 *
 *  Created on: 2014/08/07
 *      Author: sasaki
 */

#ifndef DIJKSTRABASESEARCH_H_
#define DIJKSTRABASESEARCH_H_

#include <queue>
#include "generalTools.h"
#include "Parameters.h"

template <class T, class S, class C>
void clearpq(priority_queue<T, S, C>& q);


class Path
{
private:
	std::stack<int> nodeSet;
	std::vector<int> poiNodeSet;
	std::vector<Category> queryCategorySet;
	std::vector<Category> userCategorySet;
	double cost;
	std::vector<Category> poiCategory;
	std::vector<double>poiScore;
	double categoryScore;
	std::vector<double> ratingScore;
	double tempRatings [RATING_NUM];

public:

	Path();
	~Path();
	void Initial();
	void NodePush(int);
	void PoINodePush(int);
	void LabelPush(Category);
	//void LabelDelete(int);
	bool LabelEmpty();
	void AddCost(double);
	void AddRatingScore(int, double);
	void AddPoICategory(Category);
	void AddPoIScore(double);
	Category PopPoICategory();
	int GetPoINum();
	double GetCost();
	double GetCategoryScore();
	double* GetRatings();
    void PrintRatings(int);
	void FloorCost(int nLen);
	void CheckSimilarityScore();
	void SetQueryCategorySet(std::vector<Category>);
	void SetUserCategorySet(std::vector<Category>);
	void ShowNode();
	int GetLastNode();
	Category GetLastPoI();
	Category GetTopLabel();
	Category GetTopUserLabel();
	void PopTopLabel();
	void PopTopUserLabel();
	std::vector<Category> GetLabelSet();
	void SetNoAnswer();
	bool operator>(const Path &path) const;
};

class SkylineSet
{
private:
	std::vector< Path >routeSet;
public:
	SkylineSet();

	void AddRoute(Path);
	void Clear();
	void ShowSkylineSet();
	void FloorSkylineSet();
	double SkylineCompare(SkylineSet);
    bool CompareRating(double* route, double* temp, int size);
    //bool CompareRating(double route[RATING_NUM], double temp[RATING_NUM], int size);
    double GetSkylineLength(double);
	int GetRouteSetNum();
	Path GetRoute(int i);
	void SortSkyline();

};

class OneTimeQuery{

private:
	int startNode;
	std::vector<Category> queryCategory;
	std::vector<Category> userCategory;
	std::vector< double>upperBound;
	std::vector< double>ubLength;
	std::vector< double>perfectUbLength;
	std::vector<double>poiSecondScore;
	SkylineSet skyline;
	double NNLength;
public:
	OneTimeQuery();
	OneTimeQuery(int start,std::vector<Category> rec_cate,std::vector<Category> user_cate);
	void SetQuery(int start,std::vector<Category> rec_cate,std::vector<Category> user_cate);
	~OneTimeQuery();
	void CalculationNearestBaseUpperBound(UndirectedWeightedLabeledGraph *graph);
	void CalculationOSRBaseUpperBound(UndirectedWeightedLabeledGraph *graph);
	void CalculationNearestBaseUpperBoundMcategory(UndirectedWeightedLabeledGraph *graph);
	void CalculationMinLowerbound(UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq);
	void testCalculationMinLowerbound(UndirectedWeightedLabeledGraph *graph);
	void UpdateUpperBound(Path route);
	void UpdateNNLength(Path route);
	int GetStartNode();
	std::vector<Category> GetLabelSet();
	bool CheckPath(Path, Path);
	//bool CheckLabel(std::vector<Category> labelSet, Category label);
	bool CheckCategory(Category, Category);
	double CheckCategoryScore(Category query_category, Category graph_category);
	double CheckCategoryScore(int, Category graph_category);
	bool CheckDoneNode(int node, std::vector< int > doneNodeSet);
	double GetMinUpperBound(double);
	void AddPoISecondScore(double);
	int GetRouteSetNum();
	//void ShortestPath(priority_queue < Path, vector<Path>, greater<Path> > pq, int edge[NODE_NUM][NODE_NUM], int nodeLabel[NODE_NUM]);
	//void OneTimeSearch (int edge[NODE_NUM][NODE_NUM], int nodeLabel[NODE_NUM]);
	//void OneTimeSearchFile (FILE *, FILE*);
	SkylineSet OneTimeSearchAdjacencyList(UndirectedWeightedLabeledGraph *graph, double);
	SkylineSet OneTimeSearchAdjacencyListnoUpperbound(UndirectedWeightedLabeledGraph *graph, double);
	SkylineSet OneTimeSearchAdjacencyListWCache(UndirectedWeightedLabeledGraph *graph, double);
	SkylineSet ApproximateOneTimeSearchAdjacencyList(UndirectedWeightedLabeledGraph *graph, double);
	void NextPoI(UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allpath);
	void NextPoInoUpperbound(UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allpath);
	void NextPoIwCache(UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allpath, map<int, map<int, double> > *pathList);
	void AllPairPoI(UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allpath);
	void AppNextPoI(UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allpath);
};




#endif /* DIJKSTRABASESEARCH_H_ */
