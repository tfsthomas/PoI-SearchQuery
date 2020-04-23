/*
 * OneTimeSearch.cpp
 *
 *  Created on: 2014/08/07
 *      Author: sasaki
 */

#include "main.h"
#include "GraphGenerator.h"
#include "OneTimeSearch.h"
#include "generalTools.h"
#include <sys/resource.h>
//#define DEBUG
using namespace std;

extern Result result;
extern Result resultAp;
extern int MAX_VALUE;
extern string lw;
extern double init_time;

int sum_sky_cache=0;
int sum_queue_cache=0;
int sum_sky_dij=0;
int sum_queue_dij=0;

Path::Path():cost(0),categoryScore(0){}
Path::~Path(){}


void Path::Initial(){
	while(nodeSet.size()!=0)nodeSet.pop();
	queryCategorySet.clear();
	cost=0;

}

void Path::NodePush(int node){

	nodeSet.push(node);
}
void Path::PoINodePush(int node){

	poiNodeSet.push_back(node);
}
void Path::LabelPush(Category label){

	queryCategorySet.push_back(label);
}


bool Path::LabelEmpty(){
    return queryCategorySet.empty();
}

void Path::AddCost(double value){
    cost += value;
}

void Path::AddRatingScore(int ratingid, double value){
    tempRatings[ratingid] += value;

}

void Path::AddPoICategory(Category poi){
#ifdef DEBUG
	cout<<" add poi category=";
	for(int x=0;x<CATEGORY_LAYER;x++){
		cout<<poi.category[x]<<":";
	}
	cout<<endl;
#endif
	poiCategory.push_back(poi);
}

void Path::AddPoIScore(double poiscore){
	poiScore.push_back(poiscore);
}

Category Path::PopPoICategory(){

	Category tempCategory;
	tempCategory=poiCategory.back();
	poiCategory.pop_back();
	return tempCategory;
}

int Path::GetPoINum(){
	return poiCategory.size();
}

double Path::GetCost(){
    return cost;
}

double Path::GetCategoryScore(){
    return categoryScore;
}

double* Path::GetRatings(){
    return tempRatings;
}

void Path::PrintRatings(int size) {
    for (int i = 0; i < size; i++) {
        cout << tempRatings[i] << ", ";
    }
    cout<<endl;
}

void Path::FloorCost(int nLen){

	cost = floor2(cost, nLen);
}
void Path::CheckSimilarityScore(){

	//cout<<"size="<<poiCategory.size()<<endl;
	categoryScore=1;
	for(int i=0;i<poiScore.size();i++){
		categoryScore*=poiScore[i];
	}
	categoryScore = 1 - categoryScore;

	//for(int i=0;i<poiCategory.size();i++){
	//	for(int j=0;j<QUERY_CATEGORY_LAYER;j++){
			//cout<<recCat[(QUERY_CATEGORY_NUM-1)-i].category[j]<<","<<poiCategory[i].category[j]<<endl;
	//		if(recCat[(QUERY_CATEGORY_NUM-1)-i].category[j]!=poiCategory[i].category[j]){
	//			categoryScore+=QUERY_CATEGORY_LAYER-j;
				//cout<<"cscore="<<categoryScore<<endl;
	//			break;
	//		}
	//	}
	//}
}
bool OneTimeQuery::CheckCategory(Category query_category, Category graph_category){

	for(int i=0;i<CATEGORY_LAYER;i++){
		//cout<<i<<","<<query_category.category[i]<<","<<graph_category.category[i]<<endl;
		if(query_category.category[i]=="-1")break;//note that [0]!="-1".
		if(query_category.category[i]!=graph_category.category[i])return false;//does not match
	}
	return true;
}


double OneTimeQuery::CheckCategoryScore(Category query_category, Category graph_category){

	double depth_q,depth_v;

	//How many category is different?
	depth_v=QUERY_CATEGORY_LAYER;
	for(int i=0;i<QUERY_CATEGORY_LAYER;i++){
		//cout<<recCat[(QUERY_CATEGORY_NUM-1)-i].category[j]<<","<<poiCategory[i].category[j]<<endl;
		//if(query_category.category[i]==graph_category.category[i])continue;
		if(query_category.category[i]=="-1"){
		    depth_v=i;break;}
		if(query_category.category[i]!=graph_category.category[i]){
		    depth_v=i;break;}
	}

	depth_q=QUERY_CATEGORY_LAYER;
	for(int i=0;i<QUERY_CATEGORY_LAYER;i++){
		if(query_category.category[i]=="-1"){
			depth_q=i;
			break;
		}
	}

	return 2*depth_v/(depth_q+depth_v);
}


bool OneTimeQuery::CheckDoneNode(int node, std::vector< int > doneNodeSet){

	for(unsigned int i=0;i<doneNodeSet.size();i++){
		if(doneNodeSet[i] == node)return false;
	}
	return true;
}

void Path::SetQueryCategorySet(std::vector<Category> queryLabel){
    queryCategorySet=queryLabel;
}

void Path::SetUserCategorySet(std::vector<Category> userLabel){
    userCategorySet=userLabel;
}


void Path::ShowNode(){
	std::stack<int> tempStack1 = nodeSet;
	std::stack<int> tempStack2;

	cout<<"PoINum="<<poiCategory.size()<<", cScore="<<categoryScore<<"(";
	for(int i=0;i<poiCategory.size();i++){
		cout<<poiScore[i]<<",";
	}
	cout<<")";

	//cout<<", cost="<<cost<<", rating="<<ratingScore<<", poi path=";

	for(int i=0;i<poiNodeSet.size();i++){
		cout<<poiNodeSet[i]<<",";
	}
	cout<<endl;
	/*cout<<" full path=";
	while(!tempStack1.empty()){
		tempStack2.push(tempStack1.top());
		tempStack1.pop();
	}
	while(!tempStack2.empty()){
		cout << tempStack2.top()<<",";
		tempStack2.pop();
	}
	cout<<endl;*/
}

int Path::GetLastNode(){
    return nodeSet.top();
}

Category Path::GetLastPoI(){
    return poiCategory[poiCategory.size()-1];
}

Category Path::GetTopLabel(){
    return queryCategorySet.back();
}
Category Path::GetTopUserLabel(){
    return userCategorySet.back();
}

void Path::PopTopLabel(){queryCategorySet.pop_back();}
void Path::PopTopUserLabel(){userCategorySet.pop_back();}

std::vector<Category> Path::GetLabelSet(){return queryCategorySet;}

void Path::SetNoAnswer(){
	cost=MAX_VALUE;
	categoryScore=MAX_VALUE;
}

bool Path::operator>(const Path &path) const{

	if(PRIORITY_TYPE==0){
		if(categoryScore > path.categoryScore)return true;
		else if(categoryScore == path.categoryScore&&queryCategorySet.size() > path.queryCategorySet.size())return true;
		else if (categoryScore == path.categoryScore&&queryCategorySet.size() == path.queryCategorySet.size()&&cost>path.cost)return true;
		return false;
	}
	else if(PRIORITY_TYPE==1){
		if(queryCategorySet.size() > path.queryCategorySet.size())return true;
		else if(queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore > path.categoryScore)return true;
		else if (queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore==path.categoryScore&&cost > path.cost)return true;
		return false;
	}
	else if(PRIORITY_TYPE==2){
		if(categoryScore > path.categoryScore)return true;
		else if(categoryScore == path.categoryScore&&cost > path.cost)return true;
		return false;
	}
	else if(PRIORITY_TYPE==3){
		if(cost>path.cost)return true;
		//else if(cost == path.cost&&categoryScore > path.categoryScore)return true;
		//else if (cost == path.cost&&categoryScore == path.categoryScore&&queryCategorySet.size() > path.queryCategorySet.size())return true;
		return false;
	}

//	if(path.queryCategorySet.size()== 1){
//		if(queryCategorySet.size() > path.queryCategorySet.size())return true;
//		if(queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore > path.categoryScore)return true;
//		else if (queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore == path.categoryScore&&cost > path.cost)return true;
//		return false;
//	}
//	else{
//		if(poiScore[poiScore.size()-1] > path.poiScore[path.poiScore.size()-1])return true;
//		else if(poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore > path.categoryScore)return true;
//		else if(poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore == path.categoryScore&&queryCategorySet.size() > path.queryCategorySet.size())return true;
//		else if (poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore == path.categoryScore&&queryCategorySet.size() == path.queryCategorySet.size()&&cost > path.cost)return true;
//		return false;
//	}

//	if(categoryScore > path.categoryScore)return true;
//	else if(categoryScore == path.categoryScore&&queryCategorySet.size() > path.queryCategorySet.size())return true;
//	else if(categoryScore == path.categoryScore&&queryCategorySet.size() == path.queryCategorySet.size()&&poiScore[poiScore.size()-1] < path.poiScore[path.poiScore.size()-1])return true;
//	else if(categoryScore == path.categoryScore&&queryCategorySet.size() == path.queryCategorySet.size()&&poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&cost > path.cost)return true;
//	return false;



//	if(categoryScore > path.categoryScore)return true;
//	else if(categoryScore == path.categoryScore&&poiScore[poiScore.size()-1] > path.poiScore[path.poiScore.size()-1])return true;
//	else if(categoryScore == path.categoryScore&&poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&queryCategorySet.size() > path.queryCategorySet.size())return true;
//	else if(categoryScore == path.categoryScore&&poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&queryCategorySet.size() == path.queryCategorySet.size()&&cost > path.cost)return true;
//	return false;

//	if(poiScore[poiScore.size()-1] > path.poiScore[path.poiScore.size()-1])return true;
//	else if(poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore > path.categoryScore)return true;
//	else if(poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore == path.categoryScore&&queryCategorySet.size() > path.queryCategorySet.size())return true;
//	else if (poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore == path.categoryScore&&queryCategorySet.size() == path.queryCategorySet.size()&&cost > path.cost)return true;
//	return false;

//	if(poiScore[poiScore.size()-1] > path.poiScore[path.poiScore.size()-1])return true;
//	else if(poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&queryCategorySet.size() > path.queryCategorySet.size())return true;
//	else if(poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore > path.categoryScore)return true;
//	else if (poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore==path.categoryScore&&cost > path.cost)return true;
//	return false;

//	if(poiScore[poiScore.size()-1] > path.poiScore[path.poiScore.size()-1])return true;
//	else if(poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore > path.categoryScore)return true;
//	else if (poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&categoryScore==path.categoryScore&&cost > path.cost)return true;
//	return false;


//	if(queryCategorySet.size() > path.queryCategorySet.size())return true;
//	else if(queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore > path.categoryScore)return true;
//	else if(queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore == path.categoryScore&&poiScore[poiScore.size()-1] > path.poiScore[path.poiScore.size()-1])return true;
//	else if (queryCategorySet.size() == path.queryCategorySet.size()&&categoryScore==path.categoryScore&&poiScore[poiScore.size()-1] == path.poiScore[path.poiScore.size()-1]&&cost > path.cost)return true;
//	return false;

//	if(categoryScore > path.categoryScore)return true;
//	else if(categoryScore == path.categoryScore && cost > path.cost)return true;
//	else if (categoryScore == path.categoryScore&&cost == path.cost&&queryCategorySet.size() > path.queryCategorySet.size())return true;
//	return false;



	//The following operators do not efficiently work.
//	if(cost>path.cost)return true;
//	else if(cost == path.cost&&categoryScore > path.categoryScore)return true;
//	else if (cost == path.cost&&categoryScore == path.categoryScore&&labelSet.size() > path.labelSet.size())return true;
//	return false;
//	if(categoryScore > path.categoryScore)return true;
//	else if(categoryScore == path.categoryScore&&cost>path.cost)return true;
//	else if (categoryScore == path.categoryScore&&cost == path.cost&&labelSet.size() > path.labelSet.size())return true;
}


OneTimeQuery::OneTimeQuery(){

//	startNode=1;
//	Category tempCategory;
//	for(int i=0;i<3;i++){
//		for(int j=0;j<CATEGORY_LAYER-2;j++){
//			tempCategory.category[j]=int2string(i);
//		}
//		tempCategory.category[CATEGORY_LAYER-1]="-1";
//		tempCategory.category[CATEGORY_LAYER-2]="-1";
//		queryCategory.push_back(tempCategory);
//	}

}

OneTimeQuery::OneTimeQuery(int start,std::vector<Category> rec_cate,std::vector<Category> user_cate):startNode(start),queryCategory(rec_cate),userCategory(user_cate){
	//upperBound.resize(QUERY_CATEGORY_NUM*(QUERY_CATEGORY_LAYER-1)+1);
	//for(int i=0;i<upperBound.size();i++)upperBound[i]=MAX_VALUE;
	ubLength.resize(QUERY_CATEGORY_NUM);
	perfectUbLength.resize(QUERY_CATEGORY_NUM);
	for(int i=0;i<QUERY_CATEGORY_NUM;i++){
		ubLength[i]=0;perfectUbLength[i]=0;
	}
	skyline.Clear();
}

void OneTimeQuery::SetQuery(int start,std::vector<Category> rec_cate,std::vector<Category> user_cate){
	startNode=start;
	queryCategory=rec_cate;
	userCategory=user_cate;
	skyline.Clear();
}

void OneTimeQuery::UpdateUpperBound(Path route){

	if(upperBound[route.GetCategoryScore()]>route.GetCost())upperBound[route.GetCategoryScore()]=route.GetCost();
}
void OneTimeQuery::UpdateNNLength(Path route){

	if(route.GetCategoryScore()==0&&NNLength>route.GetCost())NNLength=route.GetCost();
}

bool OneTimeQuery::CheckPath(Path route1, Path route2){

	if(route1.GetCategoryScore()==route2.GetCategoryScore()&&route1.GetCost()==route2.GetCost()&&route1.GetPoINum()==route2.GetPoINum())return true;
	return false;
}

OneTimeQuery::~OneTimeQuery(){}

int OneTimeQuery::GetStartNode(){return startNode;}
std::vector<Category> OneTimeQuery::GetLabelSet(){return queryCategory;}

double OneTimeQuery::GetMinUpperBound(double cscore){
	double minValue = MAX_VALUE;
    //skyline.DefineRouteSet();
	int testagain = skyline.GetRouteSetNum();
	for(int i=0;i<skyline.GetRouteSetNum();i++){
		Path route = skyline.GetRoute(i);
		if(route.GetCategoryScore()<= cscore &&minValue>route.GetCost())minValue=route.GetCost();
	}
	//for(int i=0;i<cscore+1;i++){
	//	if(minValue>upperBound[i])minValue=upperBound[i];
	//}
	//cout<<"minValue="<<minValue<<endl;
	return minValue;
}

void OneTimeQuery::AddPoISecondScore(double poiscore){
	poiSecondScore.push_back(poiscore);
}

int OneTimeQuery::GetRouteSetNum(){
	return skyline.GetRouteSetNum();
}

SkylineSet OneTimeQuery::OneTimeSearchAdjacencyListnoUpperbound (UndirectedWeightedLabeledGraph *graph, double upper_bound){

#ifdef DEBUG
	cout << "Start: BSSK:startNode="<<startNode<<endl;
#endif
	priority_queue < Path, vector<Path>, greater<Path> > pq;

	//clock_t startTime =clock();
	Path tempPath;
	vector<Path> allPushPath;
	tempPath.NodePush(startNode);
	tempPath.SetQueryCategorySet(queryCategory);
	tempPath.SetUserCategorySet(userCategory);

	Category nullCategory("-1");

	graph->SetVerticeCategory(startNode,nullCategory);


	//AllPairPoI(&graph, &pq, &allPushPath);


	pq.push(tempPath);
	while(pq.size()!=0){
		NextPoInoUpperbound(graph, &pq, &allPushPath);
		Path temppath=pq.top();
		if(GetMinUpperBound(0)<temppath.GetCost())break;
	}
	//init_time+=(double)(clock()-startTime)/CLOCKS_PER_SEC;
	return skyline;
}

SkylineSet OneTimeQuery::OneTimeSearchAdjacencyList (UndirectedWeightedLabeledGraph *graph, double upper_bound){

#ifdef DEBUG
	cout << "Start: BSSK:startNode="<<startNode<<endl;
#endif
	priority_queue < Path, vector<Path>, greater<Path> > pq;                //initialization

	//clock_t startTime =clock();
	Path tempPath;
	vector<Path> allPushPath;
	tempPath.NodePush(startNode);
	tempPath.SetQueryCategorySet(queryCategory);
	tempPath.SetUserCategorySet(userCategory);

	Category nullCategory("-1");

	UndirectedWeightedLabeledGraph tempGraph = *graph;
	tempGraph.SetVerticeCategory(startNode,nullCategory);


	//AllPairPoI(&graph, &pq, &allPushPath);

	pq.push(tempPath);

	if(lw=="YES")CalculationMinLowerbound(graph, &pq);

	while(pq.size()!=0){
		NextPoI(&tempGraph, &pq, &allPushPath);
	}
	//init_time+=(double)(clock()-startTime)/CLOCKS_PER_SEC;
	return skyline;
}


SkylineSet OneTimeQuery::OneTimeSearchAdjacencyListWCache (UndirectedWeightedLabeledGraph *graph, double upper_bound){

#ifdef DEBUG
	cout << "Start: BSSK:startNode="<<startNode<<endl;
#endif

	priority_queue < Path, vector<Path>, greater<Path> > pq;

#ifdef TIMEDEBUG
	//clock_t startTime =clock();
#endif
	Path tempPath;
	vector<Path> allPushPath;
	tempPath.NodePush(startNode);
	tempPath.SetQueryCategorySet(queryCategory);
	tempPath.SetUserCategorySet(userCategory);

	Category nullCategory("-1");


	graph->SetVerticeCategory(startNode,nullCategory);

	UndirectedWeightedLabeledGraph tempGraph = *graph;
	map<int, map<int, double> > pathList;


	pq.push(tempPath);

	if(lw=="YES")CalculationMinLowerbound(graph, &pq);
	//return skyline;
	//clock_t startTime =clock();
	while(pq.size()!=0){
		NextPoIwCache(&tempGraph, &pq, &allPushPath, &pathList);
	}
	//cout<<"dij time:"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<endl;
	//cout<<"total sky:(cache),"<<sum_sky_cache<<",(dij),"<<sum_sky_dij<<endl;
	//cout<<"total queue:(cache),"<<sum_queue_cache<<",(dij),"<<sum_queue_dij<<endl;
#ifdef TIMEDEBUG
//init_time+=(double)(clock()-startTime)/CLOCKS_PER_SEC;
#endif
	return skyline;
}

void OneTimeQuery::NextPoI (UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allPushPath){

#ifdef DEBUG       //NextPoI is about priority queue
	cout << "Start: NextPoI function"<<endl;
#endif
	//priority_queue < Path, vector<Path>, greater<Path> > pq;

	Path tempPath;
	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	//int previous[NODE_NUM];
	vector<int> searchedNeighbor;
	searchedNeighbor.clear();
	vector<double> checkThroughLabel;

	//vector<Path> allPushPath;

	checkThroughLabel.resize(NODE_NUM);

	Category nullCategory("-1");

	graph->SetVerticeCategory(startNode,nullCategory);      //the '->' acts just like a '.' it is needed to dereference

	Path path = pq->top();
	pq->pop();

	double minDislowerbound=0;
	double maxDislowerbound=0;
	double lowerbound=0;
	double minlowerbound_temp=0;
	double maxlowerbound_temp=0;
	double poiDistance=0;
	double perfectPoiDistance=0;
	double minlowerboundwithoutcurrent=0;
	double maxlowerboundwithoutcurrent=0;
	double secondSimilarity;
	double secondCategoryScore;

	double upperbound;
	double secondupperbound;
	double path_cost;

	double visitedNode=0;




	/*** nextpoi search***/
	int tempStartNode = path.GetLastNode();             //comes from variable 'startNode'
	//tempCost[tempStartNode]=0;
	//checkList[tempStartNode]=DONE;

	Category requiredLabel = path.GetTopLabel();
	Category userRecLabel = path.GetTopUserLabel();


	int currentNode=tempStartNode;

#ifdef DEBUG
cout << "top.path,";
path.ShowNode();
#endif

	tempPath=path;

	path_cost=path.GetCost();
	upperbound=GetMinUpperBound(path.GetCategoryScore());

	if(lw=="YES"){
		secondSimilarity=1-path.GetCategoryScore();

		for(int i=QUERY_CATEGORY_NUM-1;i>=path.GetPoINum();i--){
			maxDislowerbound+=ubLength[i];
			minDislowerbound+=perfectUbLength[i];
			//minSimilarity*=poiMaxScore[i-1];
		}
		poiDistance=ubLength[path.GetPoINum()];
		perfectPoiDistance=perfectUbLength[path.GetPoINum()];
		//secondSimilarity*=poiSecondScore[path.GetPoINum()];
		maxlowerboundwithoutcurrent=maxDislowerbound-poiDistance;
		minlowerboundwithoutcurrent=minDislowerbound-perfectPoiDistance;

		//secondCategoryScore=1-secondSimilarity;

		secondupperbound=GetMinUpperBound(1-(1-path.GetCategoryScore())*poiSecondScore[path.GetPoINum()]);//1-secondSimilarity. (secondSimilarity = (1-path.GetCategoryScore())*poiSecondScore[path.GetPoINum()])

		if((path_cost+maxDislowerbound>=secondupperbound)&&(path_cost+minDislowerbound>=upperbound))return;
		if(path_cost+maxDislowerbound>=upperbound)return;

	}
	else if(lw=="NO"){
		if(path_cost>=upperbound)return;
	}

	result.IOCostAdd();

	priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist > pq_dij;
	pq_dij.push(pair<int,double>(tempStartNode, 0));
	//info initialization for search;//

	for(int i=0;i<NODE_NUM;i++){
		tempCost[i]=MAX_VALUE;                     //fill all elements of tempCost with MAX_VALUE
		checkList[i]=YET;                          //fill all elements of checkList with YET
		//previous[i]=-1;
		checkThroughLabel[i]=0;                    //fill all elements of checkThroughLabel with 0
	}
	//cout<<"test1:"<<tempStartNode<<endl;
	tempCost[tempStartNode]=0;         //all entries of tempCost are MAX_VALUE except for the tempStartNode/initial currentNode, which is 0
	//info initialization;//
	//result.ShowIO();

	while(pq_dij.size()!=0){            //using priority queue dijkstra size/ set limit to make threshold
		currentNode=pq_dij.top().first;     //extracts the value from "first" in pq_dij
		pq_dij.pop();                       //removes this element from pq_dij
		                                    //as soon as the above line is executed, while condition is no longer satisfied
		if(checkList[currentNode]==DONE)continue;
		checkList[currentNode]=DONE;                    //if not already, checkList[currentNode] becomes DONE (probably 1)
		visitedNode++;

		if(lw=="YES"){
			/*if(tempCost[currentNode]>poiDistance){
				maxlowerbound_temp=maxDislowerbound-poiDistance+tempCost[currentNode];
			}
			else maxlowerbound_temp=maxDislowerbound;

			if(tempCost[currentNode]>perfectPoiDistance){
				minlowerbound_temp=minDislowerbound-perfectPoiDistance+tempCost[currentNode];
			}
			else minlowerbound_temp=minDislowerbound;*/

			if(THRESHOLD<=maxlowerboundwithoutcurrent+tempCost[currentNode]+path_cost){break;}
			if((path_cost+maxlowerboundwithoutcurrent+tempCost[currentNode]>=secondupperbound) && (path_cost+minlowerboundwithoutcurrent+tempCost[currentNode]>=upperbound)){break;}
			}
		else if(lw =="NO"){
			if(upperbound<tempCost[currentNode]+path_cost){
			    break;
			}
		}

		//else lowerbound_temp=poiDistance;


		//checkList[currentNode]=DONE;

		if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){       //CheckCategory returns true if node is a PoI
			tempPath=path;
			tempPath.AddCost(tempCost[currentNode]);
			tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));

			tempPath.PopTopLabel();
			tempPath.PopTopUserLabel();
			double tempScore=CheckCategoryScore(userRecLabel, graph->GetVerticeCategory(currentNode));
#ifdef DEBUG
cout<<"currentNode="<<currentNode<<", score="<<tempScore<<endl;
#endif
            for(int i=0;i<RATING_NUM;i++) {
                tempPath.AddRatingScore(i, graph->GetVerticeRating(currentNode,i));      //add rating
            }

			tempPath.AddPoIScore(tempScore);
			tempPath.CheckSimilarityScore();            //calculate similarity score
			tempPath.PoINodePush(currentNode);
			//double upperbound = GetMinUpperBound(tempPath.GetCategoryScore());
			//cout<<"upperbound="<<upperbound<<",cost="<<tempPath.GetCost()<<",cscore="<<tempPath.GetCategoryScore()<<endl;

			bool insertFlag=true;

			//if(checkThroughLabel[currentNode]>=tempScore)insertFlag=false;

			double upperbound2 = GetMinUpperBound(tempPath.GetCategoryScore());

			/*if(lw == "YES"&&insertFlag){
				double secondSimilarity2;
				double secondCategoryScore2;
				double secondupperbound2;


				if(!tempPath.LabelEmpty()){
					secondSimilarity2 = 1-tempPath.GetCategoryScore();
					secondSimilarity2*=poiSecondScore[tempPath.GetPoINum()];
					secondCategoryScore2=1-secondSimilarity2;

					secondupperbound2=GetMinUpperBound(secondCategoryScore2);
					if((path_cost + maxlowerboundwithoutcurrent+tempCost[currentNode] >= secondupperbound2) && (path_cost+minlowerboundwithoutcurrent+tempCost[currentNode]>=upperbound2)){
						//cout<<"test4:"<<currentNode<<endl;
						insertFlag=false;
					}

				}
				//if(upperbound2<=maxlowerboundwithoutcurrent+tempCost[currentNode]+path_cost)insertFlag=false;
			}
			else if(lw == "NO"&&insertFlag){
				if(upperbound2 <=tempCost[currentNode]+path_cost)insertFlag=false;
			}*/
			//if(upperbound2 <=tempCost[currentNode]+path_cost)insertFlag=false;


			if(insertFlag){
			//for(int i=tempScore;i<CATEGORY_LAYER;i++){
			//	checkThroughLabel[currentNode][i]=false;
			//}
			checkThroughLabel[currentNode]=tempScore;
			//std::stack<int> nodeSet;
			//int previousNode;
			//cout<<i<<endl;
			//nodeSet.push(currentNode);
			//previousNode=previous[currentNode];
#ifdef DEBUG
cout<<"currenNode="<<currentNode<<",";
#endif
			//while(1){//get nodes from current to start nodes. we need a reverse order of nodes
				//cout<<previousNode<<endl;
			//	if(previousNode==tempStartNode)break;
			//	nodeSet.push(previousNode);
			//	previousNode=previous[previousNode];
			//}
			//while(nodeSet.size()>0){// add edges from start to current nodes
			//	tempPath.NodePush(nodeSet.top());
			//	nodeSet.pop();
			//}
			tempPath.NodePush(currentNode);
			if(tempPath.LabelEmpty()){                  //if the full route is complete--last desired PoI has been reached
				//result.ShowIO();
				//tempPath.ShowNode();
				skyline.AddRoute(tempPath);
				//cout<<"Route added!"<<endl;
				//UpdateUpperBound(tempPath);
				//UpdateNNLength(tempPath);
			}
			else{                                   //if full route is not complete yet--intermediate PoI has been reached
				//bool pushFlag=true;
				//for(int i=0;i<allPushPath->size();i++){
				//	if(CheckPath(allPushPath->operator[](i),tempPath)){
				//		pushFlag=false;
				//		break;
				//	}
				//}
				//if(pushFlag){
					pq->push(tempPath);//add a new path to priority_queue
				//	allPushPath->push_back(tempPath); //for avoiding  pushing duplicate path
					//if(tempScore==1.0)NextPoI(graph, pq, allPushPath);
				//}
			}
#ifdef DEBUG
cout<<"Push path:";
tempPath.ShowNode();
#endif
			}
		}
		//cout<<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))<<std::endl;
		bool checkLabelFlag=true;
		//checkLabelFlag=true;
		//for(int i=0;i<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode));i++){
		//if(checkThroughLabel[currentNode]==1)checkLabelFlag=false;
		//if(checkThroughLabel[currentNode]==0 || checkThroughLabel[currentNode] < CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))){checkLabelFlag=true;}
		//}
		//stop coding//if(path.GetLastPoIScore()<CheckCategoryScore(preRecLabel,graph.GetVerticeCategory(currentNode)))checkLabelFlag=true;
		if(checkLabelFlag){
			for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
					int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
					if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
					if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
					    tempCost[neighborNode] = tempCost[currentNode] + graph->GetVerticeWeight(currentNode, i);
					    //previous[neighborNode]=currentNode;
					    checkThroughLabel[neighborNode] = checkThroughLabel[currentNode];
					    pq_dij.push(pair<int, double>(neighborNode, tempCost[neighborNode]));

					}
			}
		}
		/*currentNode=tempStartNode;
		nextNodeCost=MAX_VALUE;
		int searchedNeighborSize=searchedNeighbor.size();
		for(int i=0;i<searchedNeighborSize;i++){
			if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET&&GetMinUpperBound(path.GetCategoryScore())>=tempCost[searchedNeighbor[i]]+path.GetCost()){
				nextNodeCost=tempCost[searchedNeighbor[i]];
				currentNode=searchedNeighbor[i];
			}
		}
		if(currentNode==tempStartNode)break;*/
	}

	result.AddTotalSearchRange(visitedNode);
	//if(result.GetIO()==1){
	//	cout<<tempCost[currentNode]<<endl;
	//	return;
	//}
	//}
#ifdef DEBUG
cout<<"size="<<pq->size()<<endl;
#endif



}


void OneTimeQuery::NextPoInoUpperbound (UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allPushPath){

#ifdef DEBUG
	cout << "Start: NextPoI function"<<endl;
#endif
	//priority_queue < Path, vector<Path>, greater<Path> > pq;


	Path tempPath;
	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	//int previous[NODE_NUM];
	vector<int> searchedNeighbor;
	searchedNeighbor.clear();
	vector<double> checkThroughLabel;

	//vector<Path> allPushPath;

	checkThroughLabel.resize(NODE_NUM);

	Category nullCategory("-1");

	graph->SetVerticeCategory(startNode,nullCategory);

		Path path = pq->top();
		pq->pop();

		/*** nextpoi search***/
		int tempStartNode = path.GetLastNode();
		//tempCost[tempStartNode]=0;
		//checkList[tempStartNode]=DONE;

		Category requiredLabel = path.GetTopLabel();
		Category userRecLabel = path.GetTopUserLabel();

		//if(GetMinUpperBound(0)<path.GetCost())return;

		int currentNode=tempStartNode;

#ifdef DEBUG
	cout << "top.path,";
	path.ShowNode();
#endif

		tempPath=path;

		priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist > pq_dij;
		pq_dij.push(pair<int,double>(tempStartNode, 0));

		//info initialization for search;//

		result.IOCostAdd();

		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
			//previous[i]=-1;
			checkThroughLabel[i]=0;
		}

		tempCost[tempStartNode]=0;
		//info initialization;//
		//result.ShowIO();

		while(pq_dij.size()!=0){

			currentNode=pq_dij.top().first;
			pq_dij.pop();

			if(checkList[currentNode]==DONE)continue;
			checkList[currentNode]=DONE;

			//if(GetMinUpperBound(path.GetCategoryScore())<tempCost[currentNode]+path.GetCost())break;


			//checkList[currentNode]=DONE;

			if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){
				tempPath=path;
				tempPath.AddCost(tempCost[currentNode]);
				tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));

				tempPath.PopTopLabel();
				tempPath.PopTopUserLabel();
				double tempScore=CheckCategoryScore(userRecLabel, graph->GetVerticeCategory(currentNode));
#ifdef DEBUG
cout<<"currentNode="<<currentNode<<", score="<<tempScore<<endl;
#endif
				tempPath.AddPoIScore(tempScore);
				tempPath.CheckSimilarityScore();
				tempPath.PoINodePush(currentNode);
				//double upperbound = GetMinUpperBound(tempPath.GetCategoryScore());
				//cout<<"upperbound="<<upperbound<<",cost="<<tempPath.GetCost()<<",cscore="<<tempPath.GetCategoryScore()<<endl;
				//if(GetMinUpperBound(tempPath.GetCategoryScore())>=tempPath.GetCost()&&checkThroughLabel[currentNode]<tempScore){
					//for(int i=tempScore;i<CATEGORY_LAYER;i++){
					//	checkThroughLabel[currentNode][i]=false;
					//}
					checkThroughLabel[currentNode]=tempScore;
					//std::stack<int> nodeSet;
					//int previousNode;
					//cout<<i<<endl;
					//nodeSet.push(currentNode);
					//previousNode=previous[currentNode];
#ifdef DEBUG
cout<<"currenNode="<<currentNode<<",";
#endif
					//while(1){//get nodes from current to start nodes. we need a reverse order of nodes
						//cout<<previousNode<<endl;
					//	if(previousNode==tempStartNode)break;
					//	nodeSet.push(previousNode);
					//	previousNode=previous[previousNode];
					//}
					//while(nodeSet.size()>0){// add edges from start to current nodes
					//	tempPath.NodePush(nodeSet.top());
					//	nodeSet.pop();
					//}
					tempPath.NodePush(currentNode);
					if(tempPath.LabelEmpty()){
						//result.ShowIO();
						//tempPath.ShowNode();
						skyline.AddRoute(tempPath);
						//UpdateUpperBound(tempPath);
						//UpdateNNLength(tempPath);
					}
					else{
						//bool pushFlag=true;
						//for(int i=0;i<allPushPath->size();i++){
						//	if(CheckPath(allPushPath->operator[](i),tempPath)){
						//		pushFlag=false;
						//		break;
						//	}
						//}
						//if(pushFlag){
							pq->push(tempPath);//add a new path to priority_queue
						//	allPushPath->push_back(tempPath); //for avoiding  pushing duplicate path
							//if(tempScore==1.0)NextPoI(graph, pq, allPushPath);
						//}
					}
#ifdef DEBUG
cout<<"Push path:";
tempPath.ShowNode();
#endif
				//}
			}
			//cout<<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))<<std::endl;
			bool checkLabelFlag=true;
			//checkLabelFlag=true;
			//for(int i=0;i<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode));i++){
			//if(checkThroughLabel[currentNode]==1)checkLabelFlag=false;
			//if(checkThroughLabel[currentNode]==0 || checkThroughLabel[currentNode] < CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))){checkLabelFlag=true;}
			//}
			//stop coding//if(path.GetLastPoIScore()<CheckCategoryScore(preRecLabel,graph.GetVerticeCategory(currentNode)))checkLabelFlag=true;
			if(checkLabelFlag){
				for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
						int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
						if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
						if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
							tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
							//previous[neighborNode]=currentNode;
							checkThroughLabel[neighborNode]=checkThroughLabel[currentNode];
							pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));

						}
				}
			}
			/*currentNode=tempStartNode;
			nextNodeCost=MAX_VALUE;
			int searchedNeighborSize=searchedNeighbor.size();
			for(int i=0;i<searchedNeighborSize;i++){
				if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET&&GetMinUpperBound(path.GetCategoryScore())>=tempCost[searchedNeighbor[i]]+path.GetCost()){
					nextNodeCost=tempCost[searchedNeighbor[i]];
					currentNode=searchedNeighbor[i];
				}
			}
			if(currentNode==tempStartNode)break;*/
		}
		//}
#ifdef DEBUG
cout<<"size="<<pq->size()<<endl;
#endif


}
void OneTimeQuery::NextPoIwCache (UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allPushPath, map<int, map<int, double> > *pathList){

#ifdef DEBUG
	cout << "Start: NextPoI function"<<endl;
#endif
	//priority_queue < Path, vector<Path>, greater<Path> > pq;

	//clock_t startTime =clock();
	Path tempPath;
	//double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	//int previous[NODE_NUM];
	//vector<int> searchedNeighbor;
	//searchedNeighbor.clear();
	vector<double> checkThroughLabel;

	//vector<Path> allPushPath;

	checkThroughLabel.resize(NODE_NUM);

	Category nullCategory("-1");

	//graph->SetVerticeCategory(startNode,nullCategory);

		Path path = pq->top();
		pq->pop();

		/*** nextpoi search***/
		int tempStartNode = path.GetLastNode();
		//tempCost[tempStartNode]=0;
		//checkList[tempStartNode]=DONE;
		Category requiredLabel = path.GetTopLabel();
		Category userRecLabel = path.GetTopUserLabel();
		double minDislowerbound=0;
		double maxDislowerbound=0;
		double lowerbound=0;
		double minlowerbound_temp=0;
		double maxlowerbound_temp=0;
		double minlowerboundwithoutcurrent=0;
		double maxlowerboundwithoutcurrent=0;
		double poiDistance=0;
		double perfectPoiDistance=0;
		double secondSimilarity;
		double secondCategoryScore;

		double upperbound;
		double secondupperbound;
		double path_cost;
		//double lowerbound=0;
	//	double lowerbound_temp=0;
		//double poiDistance=0;
		path_cost=path.GetCost();
		upperbound=GetMinUpperBound(path.GetCategoryScore());


		int add_sky_cache=0;
		int add_queue_cache=0;
		int add_sky_dij=0;
		int add_queue_dij=0;

		if(lw=="YES"){
			//secondSimilarity=1-path.GetCategoryScore();

			for(int i=QUERY_CATEGORY_NUM-1;i>=path.GetPoINum();i--){
				maxDislowerbound+=ubLength[i];
				minDislowerbound+=perfectUbLength[i];
				//minSimilarity*=poiMaxScore[i-1];
			}
			poiDistance=ubLength[path.GetPoINum()];
			perfectPoiDistance=perfectUbLength[path.GetPoINum()];
			//secondSimilarity*=poiSecondScore[path.GetPoINum()];
			maxlowerboundwithoutcurrent=maxDislowerbound-poiDistance;
			minlowerboundwithoutcurrent=minDislowerbound-perfectPoiDistance;

			//secondCategoryScore=1-secondSimilarity;
			//secondupperbound=GetMinUpperBound(secondCategoryScore);
			secondupperbound=GetMinUpperBound(1-(1-path.GetCategoryScore())*poiSecondScore[path.GetPoINum()]);//1-secondSimilarity. (secondSimilarity = (1-path.GetCategoryScore())*poiSecondScore[path.GetPoINum()])

			if((path_cost+maxDislowerbound>=secondupperbound)&&(path_cost+minDislowerbound>=upperbound))return;
			if(path_cost+maxDislowerbound>=upperbound)return;

		}
		else if(lw=="NO"){
			if(path_cost>=upperbound)return;
		}

		int currentNode=tempStartNode;

#ifdef DEBUG
	cout << "top.path,";
	path.ShowNode();
#endif

		tempPath=path;

		//check the pathList

		//GetMinUpperBound(path.GetCategoryScore())-path.GetCost() < tempCost[currentNode]
		bool cacheFlag=false;
		if(pathList->find(tempStartNode) != pathList->end()){
			double tempUpperbound=pathList->operator[](tempStartNode).operator[](tempStartNode);
			//if(lw=="NO")maxDislowerbound=0;

			if(tempUpperbound >= upperbound-path_cost){
			//if(tempUpperbound >= GetMinUpperBound(path.GetCategoryScore())){
				//cout<<"cahce!!"<<endl;
				cacheFlag=true;
				for(auto itr1 = pathList->operator[](tempStartNode).begin(); itr1 != pathList->operator[](tempStartNode).end(); ++itr1) {
					//std::cout <<tempStartNode<<", key = [" << itr1->first<<"]"<< ", val = " << itr1->second <<endl;
					//if(itr1->second>tempUpperbound)continue;

					double temp_cost=itr1->second;
					int temp_nodeId=itr1->first;

					//double temp_upperbound = GetMinUpperBound(tempPath.GetCategoryScore())

					//cout<<temp_cost<<endl;
					if(lw=="YES"){
						/*if(temp_cost>poiDistance){
							maxlowerbound_temp=maxDislowerbound-poiDistance+temp_cost;
						}
						else maxlowerbound_temp=maxDislowerbound;

						if(temp_cost>perfectPoiDistance){
							minlowerbound_temp=minDislowerbound-perfectPoiDistance+temp_cost;
						}
						else minlowerbound_temp=minDislowerbound;*/

						if(upperbound<=maxlowerboundwithoutcurrent+temp_cost+path_cost)continue;
						if((path_cost + maxlowerboundwithoutcurrent+temp_cost >= secondupperbound) && (path_cost+minlowerboundwithoutcurrent+temp_cost>=upperbound))continue;
				}
					else if(lw =="NO"){
						if(upperbound < temp_cost+path_cost)continue;
					}
					//if(upperbound<=maxlowerbound_temp+path_cost)break;


					//if(CheckCategory(requiredLabel,graph->GetVerticeCategory(temp_nodeId))){

					tempPath=path;
					tempPath.AddCost(temp_cost);
					tempPath.AddPoICategory(graph->GetVerticeCategory(temp_nodeId));

					tempPath.PopTopLabel();
					tempPath.PopTopUserLabel();
					double tempScore=CheckCategoryScore(userRecLabel, graph->GetVerticeCategory(temp_nodeId));
#ifdef DEBUG
cout<<"currentNode="<<currentNode<<", score="<<tempScore<<endl;
#endif
					tempPath.AddPoIScore(tempScore);
					tempPath.CheckSimilarityScore();
					tempPath.PoINodePush(temp_nodeId);
					//double upperbound = GetMinUpperBound(tempPath.GetCategoryScore());
					//cout<<"upperbound="<<upperbound<<",cost="<<tempPath.GetCost()<<",cscore="<<tempPath.GetCategoryScore()<<endl;

					bool insertFlag=true;

					if(checkThroughLabel[currentNode]>=tempScore)insertFlag=false;

					double upperbound2 = GetMinUpperBound(tempPath.GetCategoryScore());

					if(lw == "YES"&&insertFlag){
						double secondupperbound2;

						if(!tempPath.LabelEmpty()){
							secondupperbound2=GetMinUpperBound(1-(1-tempPath.GetCategoryScore())*poiSecondScore[tempPath.GetPoINum()]);
							if((path_cost + maxlowerboundwithoutcurrent+temp_cost >= secondupperbound2) && (path_cost+minlowerboundwithoutcurrent+temp_cost>=upperbound2))insertFlag=false;

						}
						if(upperbound2<=maxlowerboundwithoutcurrent+temp_cost+path_cost)insertFlag=false;
					}
					else if(lw == "NO"&&insertFlag){
						if(upperbound2 <=temp_cost+path_cost)insertFlag=false;
					}

					if(insertFlag){

					//if(GetMinUpperBound(tempPath.GetCategoryScore())>=tempPath.GetCost()){
						tempPath.NodePush(temp_nodeId);

	#ifdef DEBUG
	cout<<"currenNode="<<currentNode<<",";
	#endif
						if(tempPath.LabelEmpty()){
							skyline.AddRoute(tempPath);
							//add_sky_cache++;
						}
						else{
							pq->push(tempPath);//add a new path to priority_queue
							//add_queue_cache++;
						}
	#ifdef DEBUG
	cout<<"Push path:";
	tempPath.ShowNode();
	#endif
					}
					//}
				}
			}
		}

		if(!cacheFlag){
		//if(1){
			priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist > pq_dij;
			pq_dij.push(pair<int,double>(tempStartNode, 0));
			result.IOCostAdd();
			//clock_t startTime =clock();
			//info initialization for search;//
			for(int i=0;i<NODE_NUM;i++){
				tempCost[i]=MAX_VALUE;
				checkList[i]=YET;
				//previous[i]=-1;
				checkThroughLabel[i]=0;
			}
			//init_time+=(double)(clock()-startTime)/CLOCKS_PER_SEC;
			tempCost[tempStartNode]=0;
			//info initialization;//
			//result.ShowIO();

			while(pq_dij.size()!=0){

				//pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];
				//cout<<tempCost[currentNode]<<endl;

				currentNode=pq_dij.top().first;
				pq_dij.pop();

				if(checkList[currentNode]==DONE)continue;
				checkList[currentNode]=DONE;
				//pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value

				//if(pathList->operator[](tempStartNode).find(currentNode) == pathList->operator[](tempStartNode).end())pathList->operator[](tempStartNode).operator[](currentNode)=tempCost[currentNode];
				//else{
				//	if(pathList->operator[](tempStartNode).operator[](currentNode)>tempCost[currentNode])cout<<"really???"<<endl;
				//}
				if(lw=="YES"){
					/*if(tempCost[currentNode]>poiDistance){
						maxlowerbound_temp=maxDislowerbound-poiDistance+tempCost[currentNode];
					}
					else maxlowerbound_temp=maxDislowerbound;

					if(tempCost[currentNode]>perfectPoiDistance){
						minlowerbound_temp=minDislowerbound-perfectPoiDistance+tempCost[currentNode];
					}
					else minlowerbound_temp=minDislowerbound;

					if(upperbound<=maxlowerbound_temp+path_cost){
						pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value
						break;
					}
					if((path_cost+maxlowerbound_temp>=secondupperbound) && (path_cost+minlowerbound_temp>=upperbound)){
						pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value
						break;
					}*/
					//if((path.GetCost()>=GetMinUpperBound(secondCategoryScore))&&(path.GetCost()+minDislowerbound>=GetMinUpperBound(path.GetCategoryScore())))return;
					//if(path.GetCost()+maxDislowerbound>=GetMinUpperBound(path.GetCategoryScore()))return;

					//double upperbound=GetMinUpperBound(path.GetCategoryScore());
					//double cost = path.GetCost();
					if(upperbound<=maxlowerboundwithoutcurrent+tempCost[currentNode]+path_cost){
						pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value
						break;
					}
					if((path_cost + maxlowerboundwithoutcurrent+tempCost[currentNode] >= secondupperbound) && (path_cost+minlowerboundwithoutcurrent+tempCost[currentNode]>=upperbound)){
						pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value
						break;
					}


				//	if(GetMinUpperBound(path.GetCategoryScore())<=maxlowerbound_temp+path.GetCost())break;
					//if((path.GetCost()>=GetMinUpperBound(secondCategoryScore))&&(path.GetCost()+minlowerbound_temp>=GetMinUpperBound(path.GetCategoryScore())))break;
				}
				else if(lw =="NO"){

					if(upperbound<tempCost[currentNode]+path_cost){
						pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value
						break;
					}
				}

				//else lowerbound_temp=poiDistance;
				//if(GetMinUpperBound(path.GetCategoryScore())<lowerbound_temp+path.GetCost()){
				//	pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value
				//	break;
				//}



				//clock_t startTime =clock();
				if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){
					tempPath=path;
					tempPath.AddCost(tempCost[currentNode]);
					tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));

					tempPath.PopTopLabel();
					tempPath.PopTopUserLabel();
					double tempScore=CheckCategoryScore(userRecLabel, graph->GetVerticeCategory(currentNode));
#ifdef DEBUG
cout<<"currentNode="<<currentNode<<", score="<<tempScore<<endl;
#endif
					tempPath.AddPoIScore(tempScore);
					tempPath.CheckSimilarityScore();
					tempPath.PoINodePush(currentNode);
					//double upperbound = GetMinUpperBound(tempPath.GetCategoryScore());
					//cout<<"upperbound="<<upperbound<<",cost="<<tempPath.GetCost()<<",cscore="<<tempPath.GetCategoryScore()<<endl;


					//if(pathList->operator[](tempStartNode).find(currentNode) == pathList->operator[](tempStartNode).end())pathList->operator[](tempStartNode).operator[](currentNode)=tempCost[currentNode];
					//else{
					//	if(pathList->operator[](tempStartNode).operator[](currentNode)>tempCost[currentNode]){
					//		cout<<"really???"<<pathList->operator[](tempStartNode).operator[](currentNode)<<","<<tempCost[currentNode]<<endl;
					//		pathList->operator[](tempStartNode).operator[](currentNode)=tempCost[currentNode];
					//	}
					//}
					//cout<<tempStartNode<<","<<currentNode<<","<<tempCost[currentNode]<<endl;
					if(checkThroughLabel[currentNode]<tempScore)pathList->operator[](tempStartNode).operator[](currentNode)=tempCost[currentNode];


					bool insertFlag=true;

					if(checkThroughLabel[currentNode]>=tempScore)insertFlag=false;

					double upperbound2 = GetMinUpperBound(tempPath.GetCategoryScore());

					if(lw == "YES"&&insertFlag){
						double secondSimilarity2;
						double secondCategoryScore2;
						double secondupperbound2;


						if(!tempPath.LabelEmpty()){
							secondSimilarity2 = 1-tempPath.GetCategoryScore();
							secondSimilarity2*=poiSecondScore[tempPath.GetPoINum()];
							secondCategoryScore2=1-secondSimilarity2;

							secondupperbound2=GetMinUpperBound(secondCategoryScore2);
							if((path_cost + maxlowerboundwithoutcurrent+tempCost[currentNode] >= secondupperbound2) && (path_cost+minlowerboundwithoutcurrent+tempCost[currentNode]>=upperbound2))insertFlag=false;

						}
						if(upperbound2<=maxlowerboundwithoutcurrent+tempCost[currentNode]+path_cost)insertFlag=false;
					}
					else if(lw == "NO"&&insertFlag){
						if(upperbound2 <=tempCost[currentNode]+path_cost)insertFlag=false;
					}



					if(insertFlag){
						//for(int i=tempScore;i<CATEGORY_LAYER;i++){
						//	checkThroughLabel[currentNode][i]=false;
						//}


					checkThroughLabel[currentNode]=tempScore;
					//std::stack<int> nodeSet;
					//int previousNode;
					//cout<<i<<endl;
					//nodeSet.push(currentNode);
					//previousNode=previous[currentNode];
#ifdef DEBUG
cout<<"currenNode="<<currentNode<<",";
#endif
					//while(1){//get nodes from current to start nodes. we need a reverse order of nodes
						//cout<<previousNode<<endl;
					//	if(previousNode==tempStartNode)break;
					//	nodeSet.push(previousNode);
					//	previousNode=previous[previousNode];
					//}
					//while(nodeSet.size()>0){// add edges from start to current nodes
					//	tempPath.NodePush(nodeSet.top());
					//	nodeSet.pop();
					//}
					tempPath.NodePush(currentNode);
					if(tempPath.LabelEmpty()){
						skyline.AddRoute(tempPath);
						//add_sky_dij++;
					}
					else{
						pq->push(tempPath);//add a new path to priority_queue
						//add_queue_dij++;
					}
#ifdef DEBUG
cout<<"Push path:";
tempPath.ShowNode();
#endif
				}
				}
				//init_time+=(double)(clock()-startTime)/CLOCKS_PER_SEC;
				//cout<<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))<<std::endl;
				bool checkLabelFlag=true;
				//checkLabelFlag=true;
				//for(int i=0;i<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode));i++){

				//if(checkThroughLabel[currentNode]==1)checkLabelFlag=false;

				//if(checkThroughLabel[currentNode]==0 || checkThroughLabel[currentNode] < CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))){checkLabelFlag=true;}
				//}
				//stop coding//if(path.GetLastPoIScore()<CheckCategoryScore(preRecLabel,graph.GetVerticeCategory(currentNode)))checkLabelFlag=true;

				if(checkLabelFlag){
					for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
						int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
						//if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
						if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
							tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
							//previous[neighborNode]=currentNode;
							checkThroughLabel[neighborNode]=checkThroughLabel[currentNode];
							pq_dij.push(pair<int,double>(neighborNode,tempCost[neighborNode]));

							//if(pathList->operator[](tempStartNode).find(neighborNode) == pathList->operator[](tempStartNode).end())pathList->operator[](tempStartNode).operator[](neighborNode)=tempCost[neighborNode];
							//else{
							//	if(pathList->operator[](tempStartNode).operator[](neighborNode)>tempCost[neighborNode]){
							//		cout<<"really???"<<pathList->operator[](tempStartNode).operator[](neighborNode)<<","<<tempCost[neighborNode]<<endl;
							//		pathList->operator[](tempStartNode).operator[](neighborNode)=tempCost[neighborNode];
							//	}
							//}

						}
					}
				}

				/*currentNode=tempStartNode;
				nextNodeCost=MAX_VALUE;
				int searchedNeighborSize=searchedNeighbor.size();
				for(int i=0;i<searchedNeighborSize;i++){
					if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET&&GetMinUpperBound(path.GetCategoryScore())>=tempCost[searchedNeighbor[i]]+path.GetCost()){
						nextNodeCost=tempCost[searchedNeighbor[i]];
						currentNode=searchedNeighbor[i];
					}
				}
				if(currentNode==tempStartNode)break;*/
			}

			if(pq_dij.size()==0&&!cacheFlag)pathList->operator[](tempStartNode).operator[](tempStartNode)=tempCost[currentNode];//keep maximum value
		//}
#ifdef DEBUG
cout<<"size="<<pq->size()<<endl;
if(cacheFlag){
	cout<<"sky:(cache),"<<add_sky_cache<<",(dij),"<<add_sky_dij<<endl;
	cout<<"queue:(cache),"<<add_queue_cache<<",(dij),"<<add_queue_dij<<endl;
	sum_sky_cache+=add_sky_cache;
	sum_queue_cache+=add_queue_cache;
	sum_sky_dij+=add_sky_dij;
	sum_queue_dij+=add_queue_dij;

}
#endif
		}
		//if(path_cost==0)cout<<"first dij total time:"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<",queue size: "<<pq->size()<<endl;
//init_time+=(double)(clock()-startTime)/CLOCKS_PER_SEC;

}

void OneTimeQuery::AllPairPoI (UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allPushPath){

#ifdef DEBUG
	cout << "Start: NextPoI function"<<endl;
#endif
	//priority_queue < Path, vector<Path>, greater<Path> > pq;


	Path tempPath;
	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	int previous[NODE_NUM];
	vector<int> searchedNeighbor;
	searchedNeighbor.clear();
	vector<double> checkThroughLabel;

	vector<int> nodeList;
	map<int, map<int, double>> pathList;

	//vector<Path> allPushPath;

	checkThroughLabel.resize(NODE_NUM);

	Category nullCategory("-1");

	graph->SetVerticeCategory(startNode,nullCategory);
	int tempStartNode=startNode;

	//	Path path = pq->top();
	//	pq->pop();

		/*** nextpoi search***/
	//	int tempStartNode = path.GetLastNode();
	//tempCost[tempStartNode]=0;
	//checkList[tempStartNode]=DONE;

	//Category requiredLabel = path.GetTopLabel();
	//Category userRecLabel = path.GetTopUserLabel();


	int currentNode=tempStartNode;

#ifdef DEBUG
cout << "top.path,";
//path.ShowNode();
#endif

	//tempPath=path;


	//if(path.GetCost()<=GetMinUpperBound(path.GetCategoryScore())){

		//info initialization for search;//
		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
			previous[i]=-1;
			checkThroughLabel[i]=0;
		}
		tempCost[currentNode]=0;
		//info initialization;//
		//result.ShowIO();
		while(1){


			checkList[currentNode]=DONE;
			nodeList.push_back(currentNode);


			//if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){
				//adding current Node (vector[m]
				//tempPath=path;
				//tempPath.AddCost(tempCost[currentNode]);
				//tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));

				//tempPath.PopTopLabel();
				//tempPath.PopTopUserLabel();
				//double tempScore=CheckCategoryScore(userRecLabel, graph->GetVerticeCategory(currentNode));
#ifdef DEBUG
//cout<<"currentNode="<<currentNode<<", score="<<tempScore<<endl;
#endif
				//tempPath.AddPoIScore(tempScore);
				//tempPath.CheckSimilarityScore();
				//tempPath.PoINodePush(currentNode);
				//double upperbound = GetMinUpperBound(tempPath.GetCategoryScore());
				//cout<<"upperbound="<<upperbound<<",cost="<<tempPath.GetCost()<<",cscore="<<tempPath.GetCategoryScore()<<endl;
				//if(GetMinUpperBound(tempPath.GetCategoryScore())>=tempPath.GetCost()&&checkThroughLabel[currentNode]<tempScore){
					//for(int i=tempScore;i<CATEGORY_LAYER;i++){
					//	checkThroughLabel[currentNode][i]=false;
					//}
					//checkThroughLabel[currentNode]=tempScore;
					//std::stack<int> nodeSet;
					//int previousNode;
					//cout<<i<<endl;
					//nodeSet.push(currentNode);
					//previousNode=previous[currentNode];
#ifdef DEBUG
cout<<"currenNode="<<currentNode<<",";
#endif
					//while(1){//get nodes from current to start nodes. we need a reverse order of nodes
						//cout<<previousNode<<endl;
						//if(previousNode==tempStartNode)break;
						//nodeSet.push(previousNode);
						//previousNode=previous[previousNode];
					//}
					//while(nodeSet.size()>0){// add edges from start to current nodes
					//	tempPath.NodePush(nodeSet.top());
					//	nodeSet.pop();
					//}
					//if(tempPath.LabelEmpty()){
						//result.ShowIO();
						//tempPath.ShowNode();
						//skyline.AddRoute(tempPath);
						//UpdateUpperBound(tempPath);
						//UpdateNNLength(tempPath);
					//}
					//else{
					//	bool pushFlag=true;
					//	for(int i=0;i<allPushPath->size();i++){
					//		if(CheckPath(allPushPath->operator[](i),tempPath)){
					//			pushFlag=false;
					//			break;
					//		}
					//	}
					//	if(pushFlag){
					//		pq->push(tempPath);//add a new path to priority_queue
					//		allPushPath->push_back(tempPath); //for avoiding  pushing duplicate path
							//if(tempScore==1.0)NextPoI(graph, pq, allPushPath);
					//	}
					//}
#ifdef DEBUG
cout<<"Push path:";
tempPath.ShowNode();
#endif
				//}
			//}
			//cout<<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))<<std::endl;
			bool checkLabelFlag=true;
			//checkLabelFlag=true;
			//for(int i=0;i<CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode));i++){
			if(checkThroughLabel[currentNode]==1)checkLabelFlag=false;
			//if(checkThroughLabel[currentNode]==0 || checkThroughLabel[currentNode] < CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode))){checkLabelFlag=true;}
			//}
			//stop coding//if(path.GetLastPoIScore()<CheckCategoryScore(preRecLabel,graph.GetVerticeCategory(currentNode)))checkLabelFlag=true;
			if(checkLabelFlag){
				for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
						int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
						if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
						//debug map[current][neighbor] = empty? of cource

						pathList[currentNode][neighborNode]=graph->GetVerticeWeight(currentNode,i);
						////adding GetVerticeWeight(currentNode,i) to map[current][neighbor];
						if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
							tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
							previous[neighborNode]=currentNode;
							checkThroughLabel[neighborNode]=checkThroughLabel[currentNode];
						}
				}
			}
			currentNode=tempStartNode;
			nextNodeCost=MAX_VALUE;
			int searchedNeighborSize=searchedNeighbor.size();
			for(int i=0;i<searchedNeighborSize;i++){
				//if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET&&GetMinUpperBound(path.GetCategoryScore())>=tempCost[searchedNeighbor[i]]+path.GetCost()){
				if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET&&GetMinUpperBound(0)>=tempCost[searchedNeighbor[i]]){
					nextNodeCost=tempCost[searchedNeighbor[i]];
					currentNode=searchedNeighbor[i];
				}
			}
			if(currentNode==tempStartNode)break;

		}
	//}
#ifdef DEBUG
	for(int i=0;i<nodeList.size();i++){
		cout<<nodeList[i]<<endl;
	}
	for(auto itr1 = pathList.begin(); itr1 != pathList.end(); ++itr1) {
		pathList[itr1->first].begin();
		for(auto itr2 = pathList[itr1->first].begin(); itr2 != pathList[itr1->first].end(); ++itr2) {
			std::cout << "key = [" << itr1->first<<"]["<<itr2->first<<"]"          // キーを表示
							<< ", val = " << itr2->second <<endl;    // 値を表示
		}
	}
	cout<<nodeList.size()<<endl;
#endif

	map<int, map<int, double>> shortestPathList;

	int nodeListSize=nodeList.size();
	for(int i=0;i<nodeListSize;i++){
		cout<<i<<endl;
		for(int j=0;j<nodeListSize;j++){
			shortestPathList[nodeList[i]][nodeList[j]]=1000000;

			if(i==j)shortestPathList[nodeList[i]][nodeList[j]]=0;
			else if(pathList[nodeList[i]].find(nodeList[j]) != pathList[nodeList[i]].end())shortestPathList[nodeList[i]][nodeList[j]]=pathList[nodeList[i]][nodeList[j]];
		}
	}


	for(int i=0;i<nodeListSize;i++){
		cout<<i<<endl;
		for(int j=0;j<nodeListSize;j++){
			for(int k=0;k<nodeListSize;k++){
				if(shortestPathList[nodeList[i]][nodeList[j]] > shortestPathList[nodeList[i]][nodeList[k]]+shortestPathList[nodeList[k]][nodeList[j]]){
					shortestPathList[nodeList[i]][nodeList[j]] = shortestPathList[nodeList[i]][nodeList[k]]+shortestPathList[nodeList[k]][nodeList[j]];
				}
			}
		}
	}

#ifdef DEBUG
	for(int i=0;i<nodeList.size();i++){
		cout<<nodeList[i]<<endl;
	}
	for(auto itr1 = pathList.begin(); itr1 != pathList.end(); ++itr1) {
		pathList[itr1->first].begin();
		for(auto itr2 = pathList[itr1->first].begin(); itr2 != pathList[itr1->first].end(); ++itr2) {
			std::cout << "key = [" << itr1->first<<"]["<<itr2->first<<"]"          // キーを表示
							<< ", val = " << itr2->second <<endl;    // 値を表示
		}
	}
	cout<<nodeList.size()<<endl;
#endif
}


SkylineSet OneTimeQuery::ApproximateOneTimeSearchAdjacencyList (UndirectedWeightedLabeledGraph *graph, double upper_bound){

#ifdef DEBUG
	cout << "Start: dikstraBaseSearh:startNode="<<startNode<<endl;
#endif
	priority_queue < Path, vector<Path>, greater<Path> > pq;

	//SkylineSet skyline;
	Path tempPath;
	vector<Path> allPushPath;


	tempPath.NodePush(startNode);
	tempPath.SetQueryCategorySet(queryCategory);
	tempPath.SetUserCategorySet(userCategory);

	pq.push(tempPath);
	while(pq.size()!=0){
		AppNextPoI(graph, &pq, &allPushPath);
	}

	/*double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	int previous[NODE_NUM];
	vector<double> checkThroughLabel;
	checkThroughLabel.resize(NODE_NUM);
	Category nullCategory("-1");
	graph.SetVerticeCategory(startNode,nullCategory);



	while(pq.size()!=0){
		Path path = pq.top();
		pq.pop();

		int tempStartNode = path.GetLastNode();
		checkList[tempStartNode]=DONE;

		//std::vector<int> requiredLabel = path.getLabelSet();
		Category requiredLabel = path.GetTopLabel();
		Category userRecLabel = path.GetTopUserLabel();

		int currentNode=tempStartNode;
//		//int goalNode;
#ifdef DEBUG
	cout << "top.path,";
	path.ShowNode();
	cout <<endl;
#endif

		Path tempPath=path;

		//partial upperbound//
		double ubWeight = 0;
		double partialUpperbound;
		//for(int i=0;i<path.GetPoINum();i++)ubWeight+=ubLength[i];
		//partialUpperbound = NNLength/QUERY_CATEGORY_NUM+ubWeight-path.GetCost();
		partialUpperbound = (1+ubWeight)*(path.GetPoINum()+1)*GetMinUpperBound(path.GetCategoryScore())/QUERY_CATEGORY_NUM;
		//partialUpperbound = (1+ubWeight)*NNLength/QUERY_CATEGORY_NUM;
		if(ubLength[path.GetPoINum()]>partialUpperbound)partialUpperbound=ubLength[path.GetPoINum()];

		if(path.GetCost()<=GetMinUpperBound(path.GetCategoryScore())){
			//info initialization for search;//
			for(int i=0;i<NODE_NUM;i++){
				tempCost[i]=MAX_VALUE;
				checkList[i]=YET;
				previous[i]=-1;
				checkThroughLabel[i]=0;
			}
			tempCost[currentNode]=0;
			//info initialization;//

			while(1){
				resultAp.IOCostAdd();
				checkList[currentNode]=DONE;

				if(CheckCategory(requiredLabel,graph.GetVerticeCategory(currentNode))){
					tempPath=path;
					tempPath.AddCost(tempCost[currentNode]);
					tempPath.AddPoICategory(graph.GetVerticeCategory(currentNode));

					tempPath.PopTopLabel();
					tempPath.PopTopUserLabel();
					double tempScore=CheckCategoryScore(userRecLabel, graph.GetVerticeCategory(currentNode));
					tempPath.AddPoIScore(tempScore);
					tempPath.CheckSimilarityScore();
					tempPath.PoINodePush(currentNode);
					//double upperbound = GetMinUpperBound(tempPath.GetCategoryScore());
					//cout<<"upperbound="<<upperbound<<",cscore="<<tempPath.GetCategoryScore()<<endl;
					if(GetMinUpperBound(tempPath.GetCategoryScore())>=tempPath.GetCost()&&checkThroughLabel[currentNode]<tempScore){

						checkThroughLabel[currentNode]=tempScore;

						std::stack<int> nodeSet;
						int previousNode;
						//cout<<i<<endl;
						nodeSet.push(currentNode);
						previousNode=previous[currentNode];
#ifdef DEBUG
cout<<"currenNode="<<currentNode<<",";
#endif
						while(1){//get nodes from current to start nodes. we need a reverse order of nodes
							//cout<<previousNode<<endl;
							if(previousNode==tempStartNode)break;
							nodeSet.push(previousNode);
							previousNode=previous[previousNode];
						}
						while(nodeSet.size()>0){// add edges from start to current nodes
							tempPath.NodePush(nodeSet.top());
							nodeSet.pop();
						}
						if(tempPath.LabelEmpty()){
							skyline.AddRoute(tempPath);
							//UpdateUpperBound(tempPath);
						}
						else{
							bool pushFlag=true;
							for(int i=0;i<allPushPath.size();i++){
								if(CheckPath(allPushPath[i],tempPath)){
									pushFlag=false;
									break;
								}
							}
							if(pushFlag){
								pq.push(tempPath);//add a new path to priority_queue
								allPushPath.push_back(tempPath); //for avoiding  pushing duplicate path
							}
						}
	#ifdef DEBUG
	cout<<"Push path:";
	tempPath.ShowNode();
	#endif
					}
				}


				bool checkLabelFlag=true;
				//checkLabelFlag=true;

				if(checkThroughLabel[currentNode]==1)checkLabelFlag=false;

				//stop coding//if(path.GetLastPoIScore()<CheckCategoryScore(preRecLabel,graph.GetVerticeCategory(currentNode)))checkLabelFlag=true;
				if(checkLabelFlag){
					for(int i=0;i<graph.GetVerticeNeighborSize(currentNode);i++){
							int neighborNode=graph.GetVerticeNeighbor(currentNode,i);

							if(tempCost[neighborNode]>tempCost[currentNode]+graph.GetVerticeWeight(currentNode,i)){//smaller cost than current
								tempCost[neighborNode]=tempCost[currentNode]+graph.GetVerticeWeight(currentNode,i);
								previous[neighborNode]=currentNode;
								checkThroughLabel[neighborNode]=checkThroughLabel[currentNode];
							}
					}
				}

				currentNode=tempStartNode;
				nextNodeCost=MAX_VALUE;
				for(int i=0;i<NODE_NUM;i++){
					if(nextNodeCost>tempCost[i]&&checkList[i]==YET&&GetMinUpperBound(path.GetCategoryScore())>=tempCost[i]+path.GetCost()&&tempCost[i]<=partialUpperbound){
						nextNodeCost=tempCost[i];
						currentNode=i;
					}
				}
				if(currentNode==tempStartNode)break;
			}
		}
#ifdef DEBUG
cout<<"size="<<pq.size()<<endl;
#endif

	}*/



	return skyline;

}

void OneTimeQuery::AppNextPoI (UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq, vector<Path> *allPushPath){

#ifdef DEBUG
	cout << "Start: AppNextPoI"<<endl;
#endif


	Path tempPath;
	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	int previous[NODE_NUM];
	vector<double> checkThroughLabel;
	checkThroughLabel.resize(NODE_NUM);
	vector<int> searchedNeighbor;
	searchedNeighbor.clear();
	Category nullCategory("-1");
	graph->SetVerticeCategory(startNode,nullCategory);



		Path path = pq->top();
		pq->pop();

		int tempStartNode = path.GetLastNode();
		checkList[tempStartNode]=DONE;

		//std::vector<int> requiredLabel = path.getLabelSet();
		Category requiredLabel = path.GetTopLabel();
		Category userRecLabel = path.GetTopUserLabel();

		int currentNode=tempStartNode;
//		//int goalNode;
#ifdef DEBUG
	cout << "top.path,";
	path.ShowNode();
	cout <<endl;
#endif

		tempPath=path;

		//partial upperbound//
		double ubWeight = 0;
		double partialUpperbound;
		//for(int i=0;i<path.GetPoINum();i++)ubWeight+=ubLength[i];
		//partialUpperbound = NNLength/QUERY_CATEGORY_NUM+ubWeight-path.GetCost();
		partialUpperbound = (1+ubWeight)*(path.GetPoINum()+1)*GetMinUpperBound(path.GetCategoryScore())/QUERY_CATEGORY_NUM;
		//partialUpperbound = (1+ubWeight)*NNLength/QUERY_CATEGORY_NUM;
		if(ubLength[path.GetPoINum()]>partialUpperbound)partialUpperbound=ubLength[path.GetPoINum()];

		if(path.GetCost()<=GetMinUpperBound(path.GetCategoryScore())){
			//info initialization for search;//
			for(int i=0;i<NODE_NUM;i++){
				tempCost[i]=MAX_VALUE;
				checkList[i]=YET;
				previous[i]=-1;
				checkThroughLabel[i]=0;
			}
			tempCost[currentNode]=0;
			//info initialization;//

			while(1){
			checkList[currentNode]=DONE;

				if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){
					tempPath=path;
					tempPath.AddCost(tempCost[currentNode]);
					tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));

					tempPath.PopTopLabel();
					tempPath.PopTopUserLabel();
					double tempScore=CheckCategoryScore(userRecLabel, graph->GetVerticeCategory(currentNode));
					tempPath.AddPoIScore(tempScore);
					tempPath.CheckSimilarityScore();
					tempPath.PoINodePush(currentNode);
					//double upperbound = GetMinUpperBound(tempPath.GetCategoryScore());
					//cout<<"upperbound="<<upperbound<<",cscore="<<tempPath.GetCategoryScore()<<endl;
					if(GetMinUpperBound(tempPath.GetCategoryScore())>=tempPath.GetCost()&&checkThroughLabel[currentNode]<tempScore){

						checkThroughLabel[currentNode]=tempScore;

						std::stack<int> nodeSet;
						int previousNode;
						//cout<<i<<endl;
						nodeSet.push(currentNode);
						previousNode=previous[currentNode];
#ifdef DEBUG
cout<<"currenNode="<<currentNode<<",";
#endif
						while(1){//get nodes from current to start nodes. we need a reverse order of nodes
							//cout<<previousNode<<endl;
							if(previousNode==tempStartNode)break;
							nodeSet.push(previousNode);
							previousNode=previous[previousNode];
						}
						while(nodeSet.size()>0){// add edges from start to current nodes
							tempPath.NodePush(nodeSet.top());
							nodeSet.pop();
						}
						if(tempPath.LabelEmpty()){
							skyline.AddRoute(tempPath);
							//UpdateUpperBound(tempPath);
						}
						else{
							bool pushFlag=true;
							for(int i=0;i<allPushPath->size();i++){
								if(CheckPath(allPushPath->operator [](i),tempPath)){
									pushFlag=false;
									break;
								}
							}
							if(pushFlag){
								pq->push(tempPath);//add a new path to priority_queue
								allPushPath->push_back(tempPath); //for avoiding  pushing duplicate path
								//AppNextPoI(graph,pq,allPushPath);
							}
						}
	#ifdef DEBUG
	cout<<"Push path:";
	tempPath.ShowNode();
	#endif
					}
				}


				bool checkLabelFlag=true;
				//checkLabelFlag=true;

				if(checkThroughLabel[currentNode]==1)checkLabelFlag=false;

				//stop coding//if(path.GetLastPoIScore()<CheckCategoryScore(preRecLabel,graph.GetVerticeCategory(currentNode)))checkLabelFlag=true;
				if(checkLabelFlag){
					for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
							int neighborNode=graph->GetVerticeNeighbor(currentNode,i);

							if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
							if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
								tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
								previous[neighborNode]=currentNode;
								checkThroughLabel[neighborNode]=checkThroughLabel[currentNode];
							}
					}
				}

				currentNode=tempStartNode;
				nextNodeCost=MAX_VALUE;
				int searchedNeighborSize=searchedNeighbor.size();
				for(int i=0;i<searchedNeighborSize;i++){
					if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET&&GetMinUpperBound(path.GetCategoryScore())>=tempCost[searchedNeighbor[i]]+path.GetCost()&&tempCost[searchedNeighbor[i]]<=partialUpperbound){
						nextNodeCost=tempCost[searchedNeighbor[i]];
						currentNode=searchedNeighbor[i];
					}
				}
				if(currentNode==tempStartNode)break;
			}
		}
#ifdef DEBUG
cout<<"size="<<pq->size()<<endl;
#endif



}
//////////////////UPPERBOUND/////////////////
//////////////////UPPERBOUND/////////////////
//////////////////UPPERBOUND/////////////////

void OneTimeQuery::CalculationNearestBaseUpperBound(UndirectedWeightedLabeledGraph *graph){

#ifdef DEBUG
	cout << "Start: CalculationNearestBaseUpperBound:startNode="<<startNode<<endl;
	for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){
		cout<<" poi category=";
		for(int x=0;x<CATEGORY_LAYER;x++){
			cout<<userCategory[cid].category[x]<<":";
		}
		cout<<endl;
	}
#endif

	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	vector<int> searchedNeighbor;

	Category nullCategory("-1");
	graph->SetVerticeCategory(startNode,nullCategory);

	Path path;

	int tempStartNode=startNode;
	double sumCost=0;

	for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){

		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
		}
		searchedNeighbor.clear();
		tempCost[tempStartNode]=0;
		checkList[tempStartNode]=DONE;

		Category requiredLabel = userCategory[cid];

		int currentNode=tempStartNode;

		while(1){
			checkList[currentNode]=DONE;

			if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){
				path.AddCost(tempCost[currentNode]);
				path.PoINodePush(currentNode);
				path.AddPoICategory(graph->GetVerticeCategory(currentNode));
				path.AddPoIScore(CheckCategoryScore(requiredLabel, graph->GetVerticeCategory(currentNode)));
				//upperbound+=tempCost[currentNode];
				sumCost+=tempCost[currentNode];
				//ubLength[(QUERY_CATEGORY_NUM-1)-cid]=sumCost;
				tempStartNode=currentNode;
				break;
			}

			int neighborNum=graph->GetVerticeNeighborSize(currentNode);
			for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
					int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
					if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
					if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
						tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
					}

			}
			currentNode=tempStartNode;
			nextNodeCost=MAX_VALUE;
			int searchedNeighborSize=searchedNeighbor.size();
			for(int i=0;i<searchedNeighborSize;i++){
				if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET){
					nextNodeCost=tempCost[searchedNeighbor[i]];
					currentNode=searchedNeighbor[i];
				}
			}
			if(currentNode==tempStartNode){
				return;
			}
		}

	}
	path.CheckSimilarityScore();
	skyline.AddRoute(path);
	//upperBound[0]=upperbound;
	//NNLength=upperbound;
#ifdef DEBUG
	cout<< "NN method,";
	skyline.ShowSkylineSet();
#endif
}


void OneTimeQuery::CalculationOSRBaseUpperBound (UndirectedWeightedLabeledGraph *graph){

#ifdef DEBUG
	cout << "Start: dikstraBaseSearh:startNode="<<startNode<<endl;
#endif
	priority_queue < Path, vector<Path>, greater<Path> > pq;

	Path tempPath;
	Path resultPath;
	double resultMinCost=MAX_VALUE;
	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	int previous[NODE_NUM];

	tempPath.NodePush(startNode);
	tempPath.SetQueryCategorySet(userCategory);

	double upperbound=GetMinUpperBound(0);
	Category nullCategory("-1");

	graph->SetVerticeCategory(startNode,nullCategory);
//	if(CheckCategory(tempPath.GetTopLabel(), graph.GetVerticeCategory(startNode))){
//		tempPath.AddPoICategory(graph.GetVerticeCategory(startNode));
//		tempPath.PopTopLabel();
//		graph.SetVerticeCategory(startNode,nullCategory);
//	}

	pq.push(tempPath);
	while(pq.size()!=0){

		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
			previous[i]=-1;
		}

		Path path = pq.top();
		pq.pop();

		int tempStartNode = path.GetLastNode();
		tempCost[tempStartNode]=0;
		checkList[tempStartNode]=DONE;

		//std::vector<int> requiredLabel = path.getLabelSet();
		Category requiredLabel = path.GetTopLabel();

		int currentNode=tempStartNode;
//		//int goalNode;
#ifdef DEBUG
	cout << "top.path,";
	path.ShowNode();
	cout <<endl;
#endif
		Path tempPath=path;
		while(1){
			checkList[currentNode]=DONE;

			if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){//order for label set

					checkList[currentNode]=LABEL;
					tempPath=path;
					tempPath.AddCost(tempCost[currentNode]);

					if(resultMinCost>tempPath.GetCost()&&upperbound>=tempPath.GetCost()){
						std::stack<int> nodeSet;
						int previousNode;
						//cout<<i<<endl;
						nodeSet.push(currentNode);
						previousNode=previous[currentNode];

						tempPath.PopTopLabel();

#ifdef DEBUG
cout<<currentNode<<" "<<tempPath.GetCost();
#endif
						tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));

						while(1){//get nodes from current to start nodes. we need a reverse order of nodes
							//cout<<previousNode<<endl;
							if(previousNode==tempStartNode)break;
							nodeSet.push(previousNode);
							previousNode=previous[previousNode];
						}
						while(nodeSet.size()>0){// add edges from start to current nodes
							tempPath.NodePush(nodeSet.top());
							nodeSet.pop();
						}

						if(tempPath.LabelEmpty()){
							resultMinCost=tempPath.GetCost();
							resultPath=tempPath;
						}
						else pq.push(tempPath);//add a new path to priority_queue
#ifdef DEBUG
cout<<"Push path:";
tempPath.ShowNode();
#endif
					}
			}
			else{
				for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
					int neighborNode=graph->GetVerticeNeighbor(currentNode,i);

					if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
						tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
						previous[neighborNode]=currentNode;
					}
				}
			}

			currentNode=tempStartNode;
			nextNodeCost=MAX_VALUE;
			for(int i=0;i<NODE_NUM;i++){
				if(nextNodeCost>tempCost[i]&&checkList[i]==YET&&upperbound>=tempCost[i]+path.GetCost()){
					nextNodeCost=tempCost[i];
					currentNode=i;
				}
			}
			if(currentNode==tempStartNode)break;
		}
#ifdef DEBUG
cout<<"size="<<pq.size()<<endl;
#endif

		//if(pq.size()==0)break;
		//tempPath=pq.top();
		//if(tempPath.GetCost()==resultPath.GetCost())break;

	}


	if(pq.size()==0&&resultMinCost==MAX_VALUE){
		//cout<<"no answer";
		resultPath.SetNoAnswer();
	}
	else resultPath.CheckSimilarityScore();

#ifdef SHOWRESULT
	cout<< "NN method,";
	resultPath.ShowNode();
#endif
	upperBound[0]=resultMinCost;
	//NNLength=resultMinCost;
	//return resultPath;

}



void OneTimeQuery::CalculationNearestBaseUpperBoundMcategory(UndirectedWeightedLabeledGraph *graph){

#ifdef DEBUG
	cout << "Start: CalculationNearestBaseUpperBound:startNode="<<startNode<<endl;
#endif

	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	vector<int> searchedNeighbor;

	Category nullCategory("-1");
	graph->SetVerticeCategory(startNode,nullCategory);

	Path path;
	int tempStartNode=startNode;
	double sumCost=0;

	priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist > pq_dij;

	for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){

		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
		}

		tempCost[tempStartNode]=0;
		checkList[tempStartNode]=DONE;
		searchedNeighbor.clear();

		pq_dij=priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist >();
		pq_dij.push(pair<int,double>(tempStartNode, 0));

		Category requiredLabel = userCategory[cid];
		Category uCategory= userCategory[0];
		if(cid==0){
			requiredLabel = queryCategory[0];
		}

		int currentNode=tempStartNode;

		while(1){
			//checkList[currentNode]=DONE;
			currentNode=pq_dij.top().first;
			pq_dij.pop();

			if(CheckCategory(requiredLabel,graph->GetVerticeCategory(currentNode))){
				if(cid==0){
					Path tempPath =path;
					//cout<<"cc"<<CheckCategoryScore(uCategory, graph.GetVerticeCategory(currentNode))<<endl;
					if(skyline.GetSkylineLength(1-CheckCategoryScore(uCategory, graph->GetVerticeCategory(currentNode)))==-1){
						tempPath.AddCost(tempCost[currentNode]);
						tempPath.PoINodePush(currentNode);
						tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));
						tempPath.AddPoIScore(CheckCategoryScore(uCategory, graph->GetVerticeCategory(currentNode)));

						tempPath.CheckSimilarityScore();
						skyline.AddRoute(tempPath);
						//upperBound[CheckCategoryScore(requiredLabel, graph.GetVerticeCategory(currentNode))]=upperbound+tempCost[currentNode];
					}
					if(CheckCategoryScore(uCategory, graph->GetVerticeCategory(currentNode))==1.0){
						//upperBound[0]=upperbound+tempCost[currentNode];
						//sumCost+=tempCost[currentNode];
						//ubLength[(QUERY_CATEGORY_NUM-1)]=tempCost[currentNode];
						break;
					}
				}
				else{
					path.AddCost(tempCost[currentNode]);
					path.PoINodePush(currentNode);
					path.AddPoICategory(graph->GetVerticeCategory(currentNode));
					path.AddPoIScore(CheckCategoryScore(requiredLabel, graph->GetVerticeCategory(currentNode)));
					//upperbound+=tempCost[currentNode];
					//sumCost+=tempCost[currentNode];
					//ubLength[(QUERY_CATEGORY_NUM-1)-cid]=tempCost[currentNode];
					tempStartNode=currentNode;
					break;
				}
			}

			for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
					int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
					//if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
					if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
						tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
						pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));
					}

			}
			/*currentNode=tempStartNode;
			nextNodeCost=MAX_VALUE;
			int searchedNeighborSize=searchedNeighbor.size();
			for(int i=0;i<searchedNeighborSize;i++){
				if(nextNodeCost>tempCost[searchedNeighbor[i]]&&checkList[searchedNeighbor[i]]==YET){
					nextNodeCost=tempCost[searchedNeighbor[i]];
					currentNode=searchedNeighbor[i];
				}
			}
			if(currentNode==tempStartNode){
				return;
			}*/
		}

	}
	//upperBound[0]=upperbound;
	//NNLength=upperbound;

#ifdef SHOWRESULT
	cout<< "NN method,";
	skyline.ShowSkylineSet();
#endif
}


void OneTimeQuery::CalculationMinLowerbound(UndirectedWeightedLabeledGraph *graph, priority_queue < Path, vector<Path>, greater<Path> > *pq){

#ifdef DEBUG
	cout << "Start: CalculationNearestBaseUpperBound:startNode="<<startNode<<endl;
#endif
	//clock_t startTime_total =clock();
	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	vector<double> checkThroughLabel;
	checkThroughLabel.resize(NODE_NUM);
	//vector<Path> allPushPath;


	//vector<int> searchedNeighbor;


	Category nullCategory("-1");
	graph->SetVerticeCategory(startNode,nullCategory);

	vector< vector<int> > poiSet;
	vector< vector<int> > perfectpoiSet;
	poiSet.resize(QUERY_CATEGORY_NUM);
	perfectpoiSet.resize(QUERY_CATEGORY_NUM);


	Path path = pq->top();
	pq->pop();
	Category userRecLabel = path.GetTopUserLabel();

	Path tempPath;

	int tempStartNode=startNode;
	double sumCost=0;

	priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist > pq_dij;

	//for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){

	for(int i=0;i<NODE_NUM;i++){
		tempCost[i]=MAX_VALUE;
		checkList[i]=YET;
		checkThroughLabel[i]=0;
	}

	tempCost[tempStartNode]=0;
	//checkList[tempStartNode]=DONE;
	//searchedNeighbor.clear();

	pq_dij=priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist >();
	pq_dij.push(pair<int,double>(tempStartNode, 0));

	//Category requiredLabel = userCategory[cid];
	//Category uCategory= userCategory[0];
	//if(cid==0){
	//	requiredLabel = queryCategory[0];
	//}

	int currentNode=tempStartNode;


	double upperbound=GetMinUpperBound(path.GetCategoryScore());
	double secondSimilarity;
	double secondupperbound;
	double maxDislowerbound;
	double minDislowerbound;
	double poiDistance;
	double perfectPoiDistance;
	double maxlowerboundwithoutcurrent;
	double minlowerboundwithoutcurrent;
	double secondCategoryScore;

	while(1){
		//checkList[currentNode]=DONE;
		currentNode=pq_dij.top().first;
		pq_dij.pop();

		if(tempCost[currentNode] > upperbound)break;

		if(checkList[currentNode]==DONE)continue;
		checkList[currentNode]=DONE;

		for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){
			if(CheckCategory(queryCategory[cid],graph->GetVerticeCategory(currentNode))){
				poiSet[cid].push_back(currentNode);
				if(CheckCategory(userCategory[cid],graph->GetVerticeCategory(currentNode))){
					perfectpoiSet[cid].push_back(currentNode);
				}
				if(cid==QUERY_CATEGORY_NUM-1){
					bool insertFlag=true;
					double tempScore=CheckCategoryScore(userRecLabel, graph->GetVerticeCategory(currentNode));

					if(checkThroughLabel[currentNode]>=tempScore)insertFlag=false;

					double upperbound2 = GetMinUpperBound(1-tempScore);
					if(upperbound2 <=tempCost[currentNode])insertFlag=false;


					if(insertFlag){
						checkThroughLabel[currentNode]=tempScore;
						tempPath=path;
						tempPath.AddCost(tempCost[currentNode]);
						tempPath.AddPoICategory(graph->GetVerticeCategory(currentNode));

						tempPath.PopTopLabel();
						tempPath.PopTopUserLabel();


						tempPath.AddPoIScore(tempScore);
						tempPath.CheckSimilarityScore();
						tempPath.PoINodePush(currentNode);
						tempPath.NodePush(currentNode);
						pq->push(tempPath);
					}
				}
				break;
			}
		}

		for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
			int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
			//if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
			if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
				tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
				checkThroughLabel[neighborNode]=checkThroughLabel[currentNode];
				pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));
			}
		}
		if(pq_dij.empty())break;
	}
	//cout<<"lowerbound total time:"<<(double)(clock()-startTime_total)/CLOCKS_PER_SEC<<",queue size: "<<pq->size()<<endl;

	/** Calculating the minimum Distances between matched PoIs **/

	for(int cid=QUERY_CATEGORY_NUM-1;cid>=1;cid--){
		int size = poiSet[cid].size();

		pq_dij=priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist >();


		clock_t startTime =clock();
		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
		}

		for(int pid=0;pid<size;pid++){
			pq_dij.push(pair<int,double>(poiSet[cid][pid], 0));
			tempCost[poiSet[cid][pid]]=0;
		}

		while(1){
			currentNode=pq_dij.top().first;
			pq_dij.pop();

			//if(tempCost[currentNode] >= ubLength[(QUERY_CATEGORY_NUM-1)-cid+1])break;

			if(checkList[currentNode]==DONE)continue;
			checkList[currentNode]=DONE;

		if(CheckCategory(queryCategory[cid-1],graph->GetVerticeCategory(currentNode))){
				ubLength[(QUERY_CATEGORY_NUM-1)-cid+1]=tempCost[currentNode];//poiSet[cid].push_back(currentNode);
				break;
			}
			//}

			for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
				int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
				if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
					tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
					pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));
				}
			}
		}

	}

	/** Calculating the minimum Distances between perfect matched PoIs **/
	for(int cid=QUERY_CATEGORY_NUM-1;cid>=1;cid--){
		int size = poiSet[cid].size();

		pq_dij=priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist >();


		clock_t startTime =clock();
		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
		}

		for(int pid=0;pid<size;pid++){
			//pq_dij.push(pair<int,double>(perfectpoiSet[cid][pid], 0));
			//tempCost[perfectpoiSet[cid][pid]]=0;
			pq_dij.push(pair<int,double>(poiSet[cid][pid], 0));
			tempCost[poiSet[cid][pid]]=0;
		}

		while(1){
			currentNode=pq_dij.top().first;
			pq_dij.pop();

			if(checkList[currentNode]==DONE)continue;
			checkList[currentNode]=DONE;

			if(CheckCategory(userCategory[cid-1],graph->GetVerticeCategory(currentNode))){
				perfectUbLength[(QUERY_CATEGORY_NUM-1)-cid+1]=tempCost[currentNode];//poiSet[cid].push_back(currentNode);
				break;
			}
			//}

			for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
				int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
				if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
					tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
					pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));
				}
			}
		}
//cout<<"lowerbound time:"<<pid<<":"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<endl;
	}

	//}
	//upperBound[0]=upperbound;
	//NNLength=upperbound;

#ifdef SHOWRESULT
	cout<< "lower"<<endl;
	for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){
		cout<<"ubLength"<<(QUERY_CATEGORY_NUM-1)-cid<<":"<<ubLength[(QUERY_CATEGORY_NUM-1)-cid]<<endl;
		cout<<"perfectubLength"<<(QUERY_CATEGORY_NUM-1)-cid<<":"<<perfectUbLength[(QUERY_CATEGORY_NUM-1)-cid]<<endl;
		cout<<"poiSet size:"<<poiSet[cid].size()<<endl;
	}
#endif

	//double irrevalentSemantic=0;
	//double perfectSemantic=0;
	///for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){
	//	irrevalentSemantic+=ubLength[(QUERY_CATEGORY_NUM-1)-cid];
	//	perfectSemantic+=perfectUbLength[(QUERY_CATEGORY_NUM-1)-cid];
	//}
	//cout<<irrevalentSemantic<<"\t"<<perfectSemantic<<endl;

}


void OneTimeQuery::testCalculationMinLowerbound(UndirectedWeightedLabeledGraph *graph){

#ifdef DEBUG
	cout << "Start: CalculationNearestBaseUpperBound:startNode="<<startNode<<endl;
#endif
	clock_t startTime_total =clock();
	double nextNodeCost;
	double tempCost[NODE_NUM];
	int checkList[NODE_NUM];
	//vector<int> searchedNeighbor;


	Category nullCategory("-1");
	graph->SetVerticeCategory(startNode,nullCategory);

	vector< vector<int> > poiSet;
	vector< vector<int> > perfectpoiSet;
	poiSet.resize(QUERY_CATEGORY_NUM);
	perfectpoiSet.resize(QUERY_CATEGORY_NUM);


	int tempStartNode=startNode;
	double sumCost=0;

	priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist > pq_dij;

	//for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){

	for(int i=0;i<NODE_NUM;i++){
		tempCost[i]=MAX_VALUE;
		checkList[i]=YET;
	}

	tempCost[tempStartNode]=0;
	//checkList[tempStartNode]=DONE;
	//searchedNeighbor.clear();

	pq_dij=priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist >();
	pq_dij.push(pair<int,double>(tempStartNode, 0));

	//Category requiredLabel = userCategory[cid];
	//Category uCategory= userCategory[0];
	//if(cid==0){
	//	requiredLabel = queryCategory[0];
	//}

	int currentNode=tempStartNode;

	while(1){
		//checkList[currentNode]=DONE;
		currentNode=pq_dij.top().first;
		pq_dij.pop();

		if(tempCost[currentNode] > GetMinUpperBound(0))break;

		if(checkList[currentNode]==DONE)continue;
		checkList[currentNode]=DONE;

		for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){
			if(CheckCategory(queryCategory[cid],graph->GetVerticeCategory(currentNode))){
				poiSet[cid].push_back(currentNode);
				if(CheckCategory(userCategory[cid],graph->GetVerticeCategory(currentNode))){
					perfectpoiSet[cid].push_back(currentNode);
				}
				break;
			}
		}

		for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
			int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
			//if(tempCost[neighborNode]==MAX_VALUE)searchedNeighbor.push_back(neighborNode);
			if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
				tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
				pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));
			}
		}
	}
	cout<<"lowerbound total time:"<<(double)(clock()-startTime_total)/CLOCKS_PER_SEC<<endl;

	/** Calculating the minimum Distances between matched PoIs **/

	for(int cid=QUERY_CATEGORY_NUM-1;cid>=1;cid--){
		int size = poiSet[cid].size();

		pq_dij=priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist >();


		clock_t startTime =clock();
		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
		}

		for(int pid=0;pid<size;pid++){
			pq_dij.push(pair<int,double>(poiSet[cid][pid], 0));
			tempCost[poiSet[cid][pid]]=0;
		}

		while(1){
			currentNode=pq_dij.top().first;
			pq_dij.pop();

			//if(tempCost[currentNode] >= ubLength[(QUERY_CATEGORY_NUM-1)-cid+1])break;

			if(checkList[currentNode]==DONE)continue;
			checkList[currentNode]=DONE;

		if(CheckCategory(queryCategory[cid-1],graph->GetVerticeCategory(currentNode))){
				ubLength[(QUERY_CATEGORY_NUM-1)-cid+1]=tempCost[currentNode];//poiSet[cid].push_back(currentNode);
				break;
			}
			//}

			for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
				int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
				if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
					tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
					pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));
				}
			}
		}

	}

	/** Calculating the minimum Distances between perfect matched PoIs **/
	for(int cid=QUERY_CATEGORY_NUM-1;cid>=1;cid--){
		int size = perfectpoiSet[cid].size();

		pq_dij=priority_queue < pair<int,double>, vector<pair<int,double>>, CompareDist >();


		clock_t startTime =clock();
		for(int i=0;i<NODE_NUM;i++){
			tempCost[i]=MAX_VALUE;
			checkList[i]=YET;
		}

		for(int pid=0;pid<size;pid++){
			//pq_dij.push(pair<int,double>(perfectpoiSet[cid][pid], 0));
			//tempCost[perfectpoiSet[cid][pid]]=0;
			pq_dij.push(pair<int,double>(poiSet[cid][pid], 0));
			tempCost[poiSet[cid][pid]]=0;
		}

		while(1){
			currentNode=pq_dij.top().first;
			pq_dij.pop();

			if(checkList[currentNode]==DONE)continue;
			checkList[currentNode]=DONE;

			if(CheckCategory(userCategory[cid-1],graph->GetVerticeCategory(currentNode))){
				perfectUbLength[(QUERY_CATEGORY_NUM-1)-cid+1]=tempCost[currentNode];//poiSet[cid].push_back(currentNode);
				break;
			}
			//}

			for(int i=0;i<graph->GetVerticeNeighborSize(currentNode);i++){
				int neighborNode=graph->GetVerticeNeighbor(currentNode,i);
				if(tempCost[neighborNode]>tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i)){//smaller cost than current
					tempCost[neighborNode]=tempCost[currentNode]+graph->GetVerticeWeight(currentNode,i);
					pq_dij.push(pair<int,double>(neighborNode, tempCost[neighborNode]));
				}
			}
		}
//cout<<"lowerbound time:"<<pid<<":"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<endl;
	}

	//}
	//upperBound[0]=upperbound;
	//NNLength=upperbound;

#ifdef SHOWRESULT
	cout<< "lower"<<endl;
	for(int cid=QUERY_CATEGORY_NUM-1;cid>=0;cid--){
		cout<<"ubLength"<<(QUERY_CATEGORY_NUM-1)-cid<<":"<<ubLength[(QUERY_CATEGORY_NUM-1)-cid]<<endl;
		cout<<"perfectubLength"<<(QUERY_CATEGORY_NUM-1)-cid<<":"<<perfectUbLength[(QUERY_CATEGORY_NUM-1)-cid]<<endl;
		cout<<"poiSet size:"<<poiSet[cid].size()<<endl;
	}
#endif
}

//////////////////SKYLINE/////////////////
//////////////////SKYLINE/////////////////
//////////////////SKYLINE/////////////////

SkylineSet::SkylineSet(){}

void SkylineSet::AddRoute(Path route){
	vector< Path >::iterator startIterator;
	vector< Path >::iterator tempIterator;
	Path temp;

	//ShowSkylineSet();

	//cout<<"route:"<<route.GetCost()<<","<<route.GetCategoryScore()<<",routeset size:"<<routeSet.size()<<endl;

	//route.FloorCost(6);

	bool insertFlag=true;                               //ADD COMPARE METHOD, TEMPPATH SHOULD BE GOOD HERE
	int tempSize = routeSet.size();
	for(int i=0;i<tempSize;i++){
		for( tempIterator = routeSet.begin(); tempIterator != routeSet.end(); tempIterator++ ){
			temp= *tempIterator;

			//cout<<temp.GetCost()<<","<<temp.GetCategoryScore()<<endl;
			if(route.GetCost()>=temp.GetCost() && route.GetCategoryScore() >= temp.GetCategoryScore() && this->CompareRating(route.GetRatings(),temp.GetRatings(),RATING_NUM)){
				//cout<<"0"<<endl;
				insertFlag=false;
				break;
			}
			else if(route.GetCost()<=temp.GetCost() && route.GetCategoryScore() <= temp.GetCategoryScore() && this->CompareRating(route.GetRatings(),temp.GetRatings(),RATING_NUM)){
				//cout<<"1"<<endl;
				routeSet.erase(tempIterator);
				break;
			}
		}
		if(!insertFlag)break;
	}
	if(insertFlag){
	    routeSet.push_back(route);
        //cout<<"Route added!"<<endl;
	}

	//ShowSkylineSet();

}
void SkylineSet::Clear(){
	routeSet.clear();
}
void SkylineSet::ShowSkylineSet(){

	for(int i=0;i<routeSet.size();i++){
		cout<<i<<" the answer:"<<routeSet[i].GetCost()<<", "<<routeSet[i].GetCategoryScore()<<endl;
		routeSet[i].PrintRatings(RATING_NUM);
		routeSet[i].ShowNode();
	}
}

int SkylineSet::GetRouteSetNum(){
	return routeSet.size();
}

Path SkylineSet::GetRoute(int i){
	assert(i<routeSet.size());
	return routeSet[i];
}

//bool SkylineSet::CompareRating(double* route[RATING_NUM], double temp[RATING_NUM], int size) {
bool SkylineSet::CompareRating(double* route, double* temp, int size) {
    bool checkvalue;
    for (int i = 0; i < size; i++) {
        //cout<<route[i]<<","<<temp[i]<<endl;
        if (route[i] >= temp[i]) {
            //cout<<"false"<<endl;
            checkvalue = false;
            break;
        } else if (route[i] <= temp[i]) {
            //cout<<"true"<<endl;
            checkvalue = true;
        }
    }
    return checkvalue;
}

double SkylineSet::SkylineCompare(SkylineSet skyline){

	double valueApp=0;
	double valueExact=0;
	double ssr=0;
	int cscore=0;
	int routeNum=0;


	SortSkyline();
	skyline.SortSkyline();

	//ShowSkylineSet();
	//skyline.ShowSkylineSet();

	for(int i=0;i<routeSet.size();i++){
		if(i==routeSet.size()-1)valueApp+=(1.0-routeSet[i].GetCategoryScore())*routeSet[i].GetCost();
		else valueApp+=(routeSet[i+1].GetCategoryScore()-routeSet[i].GetCategoryScore())*routeSet[i].GetCost();
	}

	for(int i=0;i<skyline.GetRouteSetNum();i++){
		if(i==skyline.GetRouteSetNum()-1)valueExact+=(1.0-skyline.GetRoute(i).GetCategoryScore())*skyline.GetRoute(i).GetCost();
		else valueExact+=(skyline.GetRoute(i+1).GetCategoryScore()-skyline.GetRoute(i).GetCategoryScore())*skyline.GetRoute(i).GetCost();
	}

//	for(int i=0;i<routeSet.size();i++){
//		cscore=routeSet[i].GetCategoryScore();
//		double length = skyline.GetSkylineLength(cscore);
//		if(length < 0){
//			while(1){
//				int temp=1;
//				length = skyline.GetSkylineLength(cscore-temp);
//				if(length >=0)break;
//				temp++;
//			}
//		}
//		if(length!=-1){
//			valueExact += routeSet[i].GetCost()/length;
//			routeNum++;
//		}
		//cout<<cscore<<","<<length<<","<<routeSet[i].GetCost()/length<<endl;

//	}
	//cout<<value<<endl;

	if(valueExact>0)ssr=valueApp/valueExact;
	else ssr=0;
	return ssr;
}
double SkylineSet::GetSkylineLength(double score){

	for(int i=0;i<routeSet.size();i++){
		if(routeSet[i].GetCategoryScore()==score)return routeSet[i].GetCost();
	}
	return -1;

}

void SkylineSet::SortSkyline(){

	Path temp;
	for(int i=0;i<routeSet.size();i++){
		for(int j=0;j<routeSet.size()-1;j++){
			if(routeSet[j].GetCategoryScore()>routeSet[j+1].GetCategoryScore()){
				temp = routeSet[j];
				routeSet[j] = routeSet[j+1];
				routeSet[j+1]=temp;
			}
		}
	}
	//ShowSkylineSet();
}

void SkylineSet::FloorSkylineSet(){

	vector<Path> tempPathSet;
	Path temp;

	for(int i=0;i<routeSet.size();i++){
		temp=routeSet[i];
		temp.FloorCost(6);
		tempPathSet.push_back(temp);
	}
	for(int i=0;i<tempPathSet.size();i++){
		AddRoute(tempPathSet[i]);
	}

}



