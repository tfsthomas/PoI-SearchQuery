/*
 * main.cpp
 *
 *  Created on: 2014/08/01
 *      Author: sasaki
 */


#include "GraphGenerator.h"
#include "OneTimeSearch.h"
#include "generalTools.h"
#include "main.h"
#include <string>
#include <sstream>
#include <sys/resource.h>
#include <fstream>
#include <chrono>                   //to measure time
#define SHOWRESULT

using namespace std;
Result result;
Result resultAp;
string lw;

double init_time;

int MAX_VALUE = NODE_NUM*MAX_COST;

int
main(int argc, char *argv[]){

    auto starttime = chrono::steady_clock::now();

	//graph作成
	//search
	//result
	struct rlimit rl;
	int stacklimit = getrlimit(RLIMIT_STACK, &rl);
	rl.rlim_cur = RLIM_INFINITY;
	stacklimit = setrlimit(RLIMIT_STACK, &rl);

	struct rusage r1;
	if(getrusage(RUSAGE_SELF, &r1) != 0) {
		/*Failure*/
	}
	printf("maxrss=%ld\n", r1.ru_maxrss);


	//Parameters
	cout<<"QUERY_NUM: "<<QUERY_NUM<<endl;
	cout<<"NODE_NUM: "<< NODE_NUM<<endl;
	cout<<"MAX_COST: "<< MAX_COST<<endl;
	cout<<"LABEL_NUM: "<<LABEL_NUM<<endl;
	cout<<"TOP_LABEL_NUM: "<<TOP_LABEL_NUM<<endl;
	cout<<"MAX_EDGE_NUM: "<<MAX_EDGE_NUM<<endl;
	cout<<"QUERY_CATEGORY_NUM: "<<QUERY_CATEGORY_NUM<<endl;
	cout<<"QUERY_CATEGORY_LAYER: "<<QUERY_CATEGORY_LAYER<<endl;
	cout<<"CATEGORY_LAYER: "<<CATEGORY_LAYER<<endl;
	cout<<"PRIORITY_TYPE: "<<PRIORITY_TYPE<<endl;
	cout<<"UPPERBOUND: "<<UPPERBOUND<<endl;
	cout<<"LOWERBOUND: "<<LOWERBOUND<<endl;
	cout<<"CACHE: "<<CACHE<<endl;
	cout<<"MAP: "<<MAP<<endl;
    cout<<"DIRECTORY: "<<DIRECTORY<<endl;
	cout<<"MAX_VALUE: "<<MAX_VALUE<<endl;      //addition from thomas

	assert(QUERY_CATEGORY_LAYER<=CATEGORY_LAYER);

	string map=MAP;
	string cache=CACHE;
	string up=UPPERBOUND;
	lw=LOWERBOUND;
	string parameters = map+"_"+int2string(NODE_NUM)+"_tl"+int2string(TOP_LABEL_NUM)+"_cl"+int2string(CATEGORY_LAYER)+"_l"+int2string(LABEL_NUM)+"_qcn"+int2string(QUERY_CATEGORY_NUM)+"_qcl"+int2string(QUERY_CATEGORY_LAYER);
	string ptype = int2string(PRIORITY_TYPE);
	//FILE *fpEdge, *fpLabel;
	//string file1= "./graph/edge_node"+parameters +".dat";
	//const char *filenameEdge=file1.c_str();
	//string file2 = "./graph/label_node"+ parameters+".dat";
	//const char *filenameLabel= file2.c_str();



	UndirectedWeightedLabeledGraph graph(NODE_NUM);
	string graphfile;
	string poifile;
	int intersectionNum;
	if(MAP=="cal"&&NODE_NUM==108683){
		graphfile="./dataset/cal_graph.txt";
		graph.graphFileInput(graphfile);
		graph.graphDistanceSort(graphfile);                 //thomas' addition
		intersectionNum=21048;
	}
	else if(MAP=="cal_noPoI"){
		graphfile="./dataset/cal_graph_noPoI.txt";
		graph.graphFileInputSyntheticPoI(graphfile);
		intersectionNum=21048;
	}
	else if(MAP=="ny_noPoI"){
		graphfile="./dataset/ny_noPoI.txt";
		graph.graphFileInputSyntheticPoI(graphfile);
		intersectionNum=1150744;
	}
	else if(MAP=="tokyo_noPoI"){
		graphfile="./dataset/small_tokyo_noPoI.txt";
		graph.graphFileInputSyntheticPoI(graphfile);
		intersectionNum=401893;
	}
	else if(MAP=="ol"){
		graphfile="./dataset/OL.txt";
		graph.graphFileInputSyntheticPoI(graphfile);
		intersectionNum=6105;
	}
	else if(MAP=="small_tokyo"){
		graphfile="./dataset/small_tokyo_edge.txt";
		poifile="./dataset/small_tokyo_poi.txt";
		graph.graphFileFoursquareInput(graphfile, poifile);
		//NODENUM=576314
		intersectionNum=401893;
	}
	else if(MAP=="ny"){
		graphfile="./dataset/ny_edge.txt";
		poifile="./dataset/ny_poi.txt";
        //cout<<"hello"<<endl;
		graph.graphFileFoursquareInput(graphfile, poifile);   //go to generalTools.cpp
		//NODENUM=1601795
		intersectionNum=1150744;
	}
	else if(MAP=="grid"){
		graph.graphGenerateSyntheticGrid();
		intersectionNum=NODE_NUM;
	}
	else{
		cout<<"error: MAP and NODE_NUM value have some errors"<<endl;
		exit(1);
	}
	result.SetGraphRSS(r1.ru_maxrss);                    //resident set size/amount of space of RAM held by a process
	cout<<"graph RSS:"<<r1.ru_maxrss<<endl;
    //cout<<result.graphRSS<<endl;

	if(UPPERBOUND == "NO" && PRIORITY_TYPE!=3){
		cout<<"error: without upperbound, but priority type is not 3"<<endl;
		//exit(1);
	}

	//graph.ShowGraph();

	double sum_dif=0;
	int startNode=1;
	SkylineSet skyline,skylineAp;
	std::vector<Category> userCate;
	std::pair< double, double > routeScore;
	double sumTime=0;

	//AUTOMATICALLY GENERATED FILES
	string directory = DIRECTORY;
	string resultFile ="./result/"+directory+"/result_bssr(onetime)_priority"+ptype+"_upperbound"+up+"_lowerbound"+lw+"_cache"+cache+"_"+parameters;
	//string resultFileAp ="./result/"+directory+"/result_bssr(onetimeAp)_priority"+ptype+"_cache"+cache+"_"+parameters;
	string averageResultFile ="./result/"+directory+"/ave_result_bssr(onetime))";
	//string averageResultFileAp ="./result/"+directory+"/ave_result_bssr(onetimeAp))_priority"+ptype+"_cache"+cache;
	string sortedDistanceFile ="./result/"+directory+"/sortedDistanceFile"+MAP;


	//graph.graphSortedDistanceFile(sortedDistanceFile);                  //thomas' addition


	srand(0);       //seed for the random ---> changed to seed based on time, previously just seed 0

	for(int qid=0;qid<QUERY_NUM;qid++){

		skyline.Clear();
		result.IOCostClear();
		result.IterationClear();
		skylineAp.Clear();
		resultAp.IOCostClear();
		resultAp.IterationClear();
		userCate.clear();

		////query////
		startNode=(int)rand()%intersectionNum;
		if(DIRECTORY == "actual")startNode=402079; //test for an example of a route
		if(MAP=="grid"){
			startNode=(int)rand()%20 * sqrt(NODE_NUM) + NODE_NUM/2 - 10*sqrt(NODE_NUM)+ (int)rand()%20 + sqrt(NODE_NUM)/2 -10;
		}
		Category tempCategory;
		vector<bool>checkCategoryFlag;
		int tempcId;
		checkCategoryFlag.resize(TOP_LABEL_NUM);
		std::vector<Category> queryCate;
		std::vector<Category> querySecondCate;
		double upperbound=MAX_VALUE;
		OneTimeQuery query(startNode, queryCate, userCate);

		while(1){

			userCate.clear();
			if(MAP=="small_tokyo"){
				if(DIRECTORY == "actual"){
					for(int j=0;j<QUERY_CATEGORY_NUM;j++){

						if(j==2){
							tempCategory.category[0]="0";
							tempCategory.category[1]="27";
							tempCategory.category[2]="-1";
							tempCategory.category[3]="-1";
						}
						if(j==1){
							tempCategory.category[0]="3";
							tempCategory.category[1]="3";
							tempCategory.category[2]="5";
							tempCategory.category[3]="11";
						}
						if(j==0){
							tempCategory.category[0]="4";
							tempCategory.category[1]="0";
							tempCategory.category[2]="10";
							tempCategory.category[3]="-1";
						}

						userCate.push_back(tempCategory);
					}
				}
				else{
					for(int j=0;j<TOP_LABEL_NUM;j++)checkCategoryFlag[j]=false;
					for(int j=0;j<QUERY_CATEGORY_NUM;j++){
						while(1){
							for(int x=0;x<QUERY_CATEGORY_LAYER;x++)tempCategory.category[x]="-1";
							while(1){
								tempcId=(int)rand()%graph.GetChildNum(0);
								if(checkCategoryFlag[tempcId]==false)break;
							}

							tempCategory.category[0]=int2string(tempcId);// 0th category is the last searched PoI and (QUERY_CATEGORY_NUM-1)th category is the first one.

							for(int l=1;l<QUERY_CATEGORY_LAYER;l++){
								int childNum=graph.GetCategoryChildNum(tempCategory);
								if(childNum==0)break;
								tempCategory.category[l]=int2string((int)rand()%childNum); //the searched category
								//tempCategory.category[l]=int2string(l%LABEL_NUM);
							}
							//for(int x=0;x<QUERY_CATEGORY_LAYER;x++){
							//	cout<<tempCategory.category[x]<<":";
							//}
							//cout<<endl;
							//cout<<graph.GetCategoryCount(tempCategory)<<endl;
							if(graph.GetCategoryCount(tempCategory)>2000)break;// only select a category whose number is more than 2000
						}
						checkCategoryFlag[tempcId]=true;
						userCate.push_back(tempCategory);
					}
				}
				queryCate = userCate;
				querySecondCate = userCate;
				query.SetQuery(startNode, queryCate, userCate);
				query.CalculationNearestBaseUpperBound(&graph);     //finds poi's sequentially and creates a path
				if(query.GetMinUpperBound(0)<MAX_VALUE)break;
			}
			if(MAP =="ny"){
				if(DIRECTORY == "actual"){
					for(int j=0;j<QUERY_CATEGORY_NUM;j++){  //QUERY_CATEGORY_NUM = 3

						if(j==2){
							tempCategory.category[0]="0";
							tempCategory.category[1]="27";
							tempCategory.category[2]="-1";
							tempCategory.category[3]="-1";
						}
						if(j==1){
							tempCategory.category[0]="3";
							tempCategory.category[1]="3";
							tempCategory.category[2]="5";
							tempCategory.category[3]="11";
						}
						if(j==0){
							tempCategory.category[0]="4";
							tempCategory.category[1]="0";
							tempCategory.category[2]="10";
							tempCategory.category[3]="-1";
						}

						userCate.push_back(tempCategory);
					}
				}
				else{
					for(int j=0;j<TOP_LABEL_NUM;j++)checkCategoryFlag[j]=false;
					for(int j=0;j<QUERY_CATEGORY_NUM;j++){
						while(1){
							for(int x=0;x<QUERY_CATEGORY_LAYER;x++)tempCategory.category[x]="-1";
							while(1){
								tempcId=(int)rand()%graph.GetChildNum(0);
								if(checkCategoryFlag[tempcId]==false)break;
							}

							tempCategory.category[0]=int2string(tempcId);// 0th category is the last searched PoI and (QUERY_CATEGORY_NUM-1)th category is the first one.

							for(int l=1;l<QUERY_CATEGORY_LAYER;l++){
								int childNum=graph.GetCategoryChildNum(tempCategory);
								if(childNum==0)break;
								tempCategory.category[l]=int2string((int)rand()%childNum); //the searched category
								//tempCategory.category[l]=int2string(l%LABEL_NUM);
							}
							//for(int x=0;x<QUERY_CATEGORY_LAYER;x++){
							//	cout<<tempCategory.category[x]<<":";
							//}
							//cout<<endl;
							//cout<<graph.GetCategoryCount(tempCategory)<<endl;
							if(graph.GetCategoryCount(tempCategory)>2000)break;// only select a category whose number is more than 2000
						}
						checkCategoryFlag[tempcId]=true;
						userCate.push_back(tempCategory);
					}
				}
                cout<<"tempCategory: ";
                for(int x=0;x<QUERY_CATEGORY_LAYER;x++){
                    cout<<tempCategory.category[x]<<":";
                }
                cout<<endl;
                //int categorycount = graph.GetCategoryCount(tempCategory);
                //cout<<"Hello"<<endl;
                //cout<<"graph.GetCategoryCount(tempCategory): "<<categorycount<<endl;
                /*  result.SetQueryEndTime(clock());
                result.AddTotal();
                result.ResultOutput(resultFile);  */

                //cout<<"query.GetMinUpperBound(): "<<endl;
                //cout<<query.GetMinUpperBound(0);     //up to here

				queryCate = userCate;
				querySecondCate = userCate;
				query.SetQuery(startNode, queryCate, userCate);
				query.CalculationNearestBaseUpperBound(&graph);
				if(query.GetMinUpperBound(0)<MAX_VALUE)break;
			}
			else{

				for(int j=0;j<TOP_LABEL_NUM;j++){
					//if(TOP_LABEL_NUM==62&&(j==0||j==2||j==7||j==8||j==14||j==18||j==19||j==23||j==25||j==26||j==28||j==31||j==33||j==34||j==36||j==41||j==45||j==50||j==51||j==55))checkCategoryFlag[j]=true;
					//if(TOP_LABEL_NUM==62&&j!=10&&j!=11&&j!=15&&j!=20&&j!=22&&j!=32&&j!=35&&j!=37&&j!=39&&j!=43&&j!=47&&j!=48&&j!=49&&j!=52&&j!=53&&j!=54&&j!=59)checkCategoryFlag[j]=true;//More than 1000
					if(TOP_LABEL_NUM==62&&j!=10&&j!=11&&j!=15&&j!=35&&j!=37&&j!=39&&j!=43&&j!=49&&j!=52&&j!=53&&j!=54&&j!=59)checkCategoryFlag[j]=true; //More than 2000
					else checkCategoryFlag[j]=false;
					//checkCategoryFlag==true -> not selected
				}
				for(int j=0;j<QUERY_CATEGORY_NUM;j++){
					while(1){
						tempcId=(int)rand()%TOP_LABEL_NUM;
						if(checkCategoryFlag[tempcId]==false)break;
					}
					checkCategoryFlag[tempcId]=true;
					tempCategory.category[0]=int2string(tempcId);// 0th category is a last searched PoI and (QUERY_CATEGORY_NUM-1)th category is a first one.
					for(int l=1;l<QUERY_CATEGORY_LAYER;l++){
						//tempCategory.category[l]=int2string(l%LABEL_NUM);
						tempCategory.category[l]=int2string((int)rand()%LABEL_NUM);
					}
					userCate.push_back(tempCategory);
				}
				queryCate = userCate;
				querySecondCate = userCate;
				query.SetQuery(startNode, queryCate, userCate);

                query.CalculationNearestBaseUpperBound(&graph);       //important, but commented out for EXC_BAD_ACCESS

				//cout<<"this is GetMinUpperBound(0): "<<query.GetMinUpperBound(0)<<endl;
				if(query.GetMinUpperBound(0)<MAX_VALUE){break;}
			}

		}

		skyline.Clear();
        auto endtime = chrono::steady_clock::now();
        //cout<<"Elapsed time: "<<chrono::duration_cast<chrono::milliseconds>(endtime - starttime).count()<<"milliseconds"<<endl;


        ///SHOWRESULT BELOW
        /*cout<<"start node "<<startNode<<endl;
        for(int j=0;j<QUERY_CATEGORY_NUM;j++){
            for(int x=0;x<QUERY_CATEGORY_LAYER;x++){
                cout<<userCate[j].category[x]<<":";
            }
            cout<<endl;
        }
        skyline.ShowSkylineSet();
        result.ShowResult(qid+1);*/


        result.SetQueryEndTime(clock());
        result.AddTotal();
        result.ResultOutput(resultFile);

        //cout<<"dif bw NN method and actual="initialLength0<<","<<actualLength0<<",dis="<<initialLength0-actualLength0<<endl;
        //result.SetInitialDiff(initialLength0/actualLength0);
		//////


#ifdef DEBUG
for(int j=0;j<QUERY_CATEGORY_NUM;j++){
for(int x=0;x<QUERY_CATEGORY_LAYER;x++){
	cout<<queryCate[j].category[x]<<":";
}
cout<<endl;
}
#endif

		for(int j=0;j<QUERY_CATEGORY_NUM;j++){//queryCate is the list of top-categories
			if(QUERY_CATEGORY_LAYER>1)queryCate[j].category[1]="-1";
			if(QUERY_CATEGORY_LAYER>1)querySecondCate[j].category[QUERY_CATEGORY_LAYER-1]="-1";
		}

		//cout<<endl;

		//if(qid!=90)continue;
		result.SetQueryStartTime(clock());

		clock_t startTime;

#ifdef TIMEDEBUG
init_time=0;
startTime =clock();
#endif
		query.SetQuery(startNode, queryCate, userCate);             //set again


		for(int i=QUERY_CATEGORY_NUM-1;i>=0;i--){
			query.AddPoISecondScore(query.CheckCategoryScore(userCate[i],querySecondCate[i]));
		}

		startTime =clock();
		//query.CalculationNearestBaseUpperBound(graph);
		if(up=="YES")query.CalculationNearestBaseUpperBoundMcategory(&graph);
		result.SetInitialRouteSetNum(query.GetRouteSetNum());
		//cout<<query.GetRouteSetNum()<<endl;
		sumTime+=(double)(clock()-startTime)/CLOCKS_PER_SEC;

#ifdef TIMEDEBUG
cout<<"upperbound time:"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<endl;
#endif


		//query.CalculationOSRBaseUpperBound(graph);
		//result.ShowIO();
		double initialLength0=query.GetMinUpperBound(0);
		//double initialLength1=query.GetMinUpperBound(1);
		//if(initialLength0!=initialLength1)cout<<initialLength1/initialLength0<<endl;
		//continue;

#ifdef TIMEDEBUG
startTime =clock();
#endif
		if(cache=="YES")skyline = query.OneTimeSearchAdjacencyListWCache(&graph,upperbound);
		else skyline = query.OneTimeSearchAdjacencyList(&graph,upperbound);             //searches for paths
		//else skyline = query.OneTimeSearchAdjacencyListnoUpperbound(&graph,upperbound);

#ifdef TIMEDEBUG
cout<<"BSSR time:"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<endl;
#endif

#ifdef TIMEDEBUG
startTime =clock();
#endif
		skyline.FloorSkylineSet();
		double actualLength0=query.GetMinUpperBound(0);
		//result.ShowIO();

#ifdef TIMEDEBUG
cout<<"skyline computation time:"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<endl;
#endif

#ifdef TIMEDEBUG
startTime =clock();
#endif
		result.IterationAdd();

		result.SetRouteSetNum(skyline.GetRouteSetNum());
#ifdef TIMEDEBUG
cout<<"result set time:"<<(double)(clock()-startTime)/CLOCKS_PER_SEC<<endl;
#endif

		result.SetQueryEndTime(clock());
		result.AddTotal();
		result.ResultOutput(resultFile);

		//cout<<"dif bw NN method and actual="initialLength0<<","<<actualLength0<<",dis="<<initialLength0-actualLength0<<endl;
		result.SetInitialDiff(initialLength0/actualLength0);

#ifdef SHOWRESULT
		cout<<endl<<"START NODE: "<<startNode<<endl;
		for(int j=0;j<QUERY_CATEGORY_NUM;j++){
		    for(int x=0;x<QUERY_CATEGORY_LAYER;x++){
			    cout<<userCate[j].category[x]<<":";

		    }
		    cout<<endl;
		}
		skyline.ShowSkylineSet();
		result.ShowResult(qid+1);
#endif
#ifdef TIMEDEBUG
cout<<"init time:"<<init_time<<endl;
#endif
/*

		//Approximation Test//
		resultAp.SetQueryStartTime(clock());

		query.SetQuery(startNode, queryCate, userCate);
		query.CalculationNearestBaseUpperBoundMcategory(graph);
		//skylineAp = query.ApproximateOneTimeSearchAdjacencyList(graph,upperbound);

		resultAp.IterationAdd();

		//skylineAp.ShowSkylineSet();
		resultAp.SetRouteSetNum(skylineAp.GetRouteSetNum());
		resultAp.SetQueryEndTime(clock());

		resultAp.SetIncreaseRate(skylineAp.SkylineCompere(skyline));
		resultAp.AddTotal();
		resultAp.ResultOutput(resultFileAp);


#ifdef SHOWRESULT
		cout<<"start node"<<startNode<<endl;
		for(int j=0;j<QUERY_CATEGORY_NUM;j++){
		for(int x=0;x<QUERY_CATEGORY_LAYER;x++){
			cout<<userCate[j].category[x]<<":";
		}
		cout<<endl;
		}
		skyline.ShowSkylineSet();
		resultAp.ShowResult(qid+1);
#endif
*/

	}

	if(getrusage(RUSAGE_SELF, &r1) != 0) {
		/*Failure*/
	}
	printf("maxrss=%ld\n", r1.ru_maxrss);

	result.SetMaxRSS(r1.ru_maxrss);

	result.ResultAverageOutPut(averageResultFile);
	//resultAp.ResultAverageOutPut(averageResultFileAp);

	//cout <<"average dif:"<<sum_dif/(double)QUERY_NUM<<endl;
	cout<<sumTime/100<<endl;
	cout<<"Done:Simulation"<<endl;
	return 1;


}


