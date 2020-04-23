#include "generalTools.h"
#include "main.h"
using namespace std;

Category::Category(){
	for(int i=0;i<CATEGORY_LAYER;i++)category[i] = "-1";

}
Category::Category(string cid){
	for(int i=0;i<CATEGORY_LAYER;i++)category[i]=cid;

}


Vertice::Vertice(int id):verticeId(id){}
std::vector< int > neighbors(100);


void Vertice::ShowVertice(){

	cout<<verticeId;

	for(int i=0;i<neighbors.size();i++){
		cout<<"-->"<<neighbors[i]<<"("<<weights[i]<<")";
	}
	cout<<std::endl;
}

void Vertice::AddNeighbors(int id, double weight){
	neighbors.push_back(id);
	weights.push_back(weight);
}

void Vertice::AddRatings(double rating){
    ratings.push_back(rating);
}

Category Vertice::GetCategory(){
	return categoryId;
}
void Vertice::SetCategory(Category cid){
	categoryId=cid;
}
int Vertice::GetNeighborsSize(){
	return neighbors.size();
}
double Vertice::GetRating(int i) {
    return ratings[i];
}
int Vertice::GetNeighbors(int i){
	return neighbors[i];
}
double Vertice::GetWeights(int i){
	return weights[i];
}

UndirectedWeightedLabeledGraph::UndirectedWeightedLabeledGraph(int node_num){
	size=node_num;
	for(int i=0;i<size;i++){
		Vertice tempVertice(i);
		verticeSet.push_back(tempVertice);
	}

}
int UndirectedWeightedLabeledGraph::GetGraphSize(){

	return size;
}
void UndirectedWeightedLabeledGraph::ShowGraph(){

    for(int i=0;i<size;i++){
		verticeSet[i].ShowVertice();
	}

}

void UndirectedWeightedLabeledGraph::AddEdge(int v1, int v2, double weight){
	//cout<<v1<<","<<v2<<","<<size<<endl;
	assert(v1<size||v2<size);
    verticeSet[v1].AddNeighbors(v2, weight);
    verticeSet[v2].AddNeighbors(v1, weight);
	//if(v1<size && v2<size) {
        //verticeSet[v1].AddNeighbors(v2, weight);
        //verticeSet[v2].AddNeighbors(v1, weight);
    //}
}

double UndirectedWeightedLabeledGraph::GetVerticeRating(int nodeId, int i) {

    return verticeSet[nodeId].GetRating(i);
}

Category UndirectedWeightedLabeledGraph::GetVerticeCategory(int nodeId){

	return verticeSet[nodeId].GetCategory();
}
void UndirectedWeightedLabeledGraph::SetVerticeCategory(int nodeId,Category cId){

	verticeSet[nodeId].SetCategory(cId);
}
int UndirectedWeightedLabeledGraph::GetVerticeNeighborSize(int nodeId){

	return verticeSet[nodeId].GetNeighborsSize();
}
int UndirectedWeightedLabeledGraph::GetVerticeNeighbor(int nodeId,int i){

	return verticeSet[nodeId].GetNeighbors(i);
}
double UndirectedWeightedLabeledGraph::GetVerticeWeight(int nodeId,int i){

	return verticeSet[nodeId].GetWeights(i);
}

int UndirectedWeightedLabeledGraph::GetChildNum(int id){

	return foursquareTreeChildNum[id];
}

int UndirectedWeightedLabeledGraph::GetCategoryChildNum(Category cid){

	string tempid;
	int tempArrayId=-1;
	for(int i=0;i<CATEGORY_LAYER;i++){
		tempid=cid.category[i];
		if(tempid=="-1")break;
		for(int j=0;j<foursquareCategoryTree.size();j++){
			if(boost::get<0>(foursquareCategoryTree[j])==tempArrayId&&int2string(boost::get<2>(foursquareCategoryTree[j]))==tempid){
				tempArrayId=j;
				break;
			}
		}
	}

	return foursquareTreeChildNum[tempArrayId+1];
}

int UndirectedWeightedLabeledGraph::GetCategoryCount(int cid){
	return foursquareCategoryCount[cid];
}
int UndirectedWeightedLabeledGraph::GetCategoryCount(Category cid){

	string tempid;
	int tempArrayId=-1;
	for(int i=0;i<CATEGORY_LAYER;i++){
		tempid=cid.category[i];
		if(tempid=="-1")break;
		for(int j=0;j<foursquareCategoryTree.size();j++){
			if(boost::get<0>(foursquareCategoryTree[j])==tempArrayId&&int2string(boost::get<2>(foursquareCategoryTree[j]))==tempid){
				tempArrayId=j;
				break;
			}
		}
	}

	return foursquareCategoryCount[tempArrayId];
}

string UndirectedWeightedLabeledGraph::GetCategoryId(Category cid){

	string tempid;
	int tempArrayId=-1;
	for(int i=0;i<CATEGORY_LAYER;i++){
		tempid=cid.category[i];
		if(tempid=="-1")break;
		for(int j=0;j<foursquareCategoryTree.size();j++){
			if(boost::get<0>(foursquareCategoryTree[j])==tempArrayId&&int2string(boost::get<2>(foursquareCategoryTree[j]))==tempid){
				tempArrayId=j;
				break;
			}
		}
	}

	return boost::get<1>(foursquareCategoryTree[tempArrayId]);
}

void UndirectedWeightedLabeledGraph::graphFileFoursquareInput(string file, string poifile){

	const char *graphFile=file.c_str(); //pointers
	const char *poiFile=poifile.c_str();
	ifstream in(graphFile);
	ifstream poiin(poiFile);

	if(!in){
		cout<<"error: cannot open file"<<endl;
		exit(1);
	}
	ifstream foursquarecategory("./dataset/foursquare_category");
	string line, subline;
	size_t len;
	//vector< boost::tuple<int, string, int, int> > categoryTree;//<pointer to its parent node,category ID,ID w.r.t its parent, layer height>
	boost::tuple<int, string, int, int> categoryNode;

	//vector<int> childNum;
	foursquareTreeChildNum.resize(765);//the number of categories is 764+root;
	foursquareCategoryCount.resize(765); //resize these vectors and fill them with 0's
	for(int i=0;i<foursquareTreeChildNum.size();i++){
		foursquareTreeChildNum[i]=0;
		foursquareCategoryCount[i]=0;
	}

	if(foursquarecategory.is_open()){
		while(getline(foursquarecategory, line)){
			len=line.length();
			//cout<<len<<'\n';
			if(len==24){
				boost::get<0>(categoryNode)=-1; //parent id
				boost::get<1>(categoryNode)=line; //category name
				boost::get<2>(categoryNode)=foursquareTreeChildNum[0]; // the number of child nodes
				boost::get<3>(categoryNode)=0; //height
				foursquareTreeChildNum[0]++;//the number of child node

				foursquareCategoryTree.push_back(categoryNode);
				//for(vector< boost::tuple<int, string, int, int> >::const_iterator i=foursquareCategoryTree.begin(); i != foursquareCategoryTree.end();++i)
				    //cout<<*i<<'\n';
				//cout<<foursquareCategoryTree<<'\n';
			}
			else if(len>24){
				//int clayer=len/24;
				subline=line.substr(len-50,24);
				line.erase(0,len-24);
				for(int i=0;i<foursquareCategoryTree.size();i++){
					if(boost::get<1>(foursquareCategoryTree[i])==subline){
						boost::get<0>(categoryNode)=i; //parent id
						boost::get<1>(categoryNode)=line; //category name
						boost::get<2>(categoryNode)=foursquareTreeChildNum[i+1]; // the number of child nodes
						boost::get<3>(categoryNode)=boost::get<3>(foursquareCategoryTree[i])+1; //height
						foursquareTreeChildNum[i+1]++;

						foursquareCategoryTree.push_back(categoryNode);
						break;
					}
				}
			}
            //for(vector<boost::tuple<int, string, int, int>>::const_iterator i=foursquareCategoryTree.begin(); i != foursquareCategoryTree.end();++i)
                //cout<<*i<<'\n';
            //cout<<categoryNode<<'\n';
			//cout<<categoryTree.size()<<endl;
			getline(foursquarecategory, line);//discard a category name
		}
	}
	else{
		cout<<"error: cannot open file"<<endl;
		exit(1);
	}

	string str;
	int v1,v2,nId, cId,checkId,id;
	double weight;
	string poicategoryId, poiId;
	string check;
	//while(getline(in,str)){
	//	sscanf(str.c_str(),"%d %d %lf",&v1,&v2,&weight);
	//	AddEdge(v1,v2,weight);
	//}


	Category tempCategory;
	vector<string> vertexCategory;
	vertexCategory.resize(size);//size is NODE_NUM
	for(int i=0;i<size;i++)vertexCategory[i]="-1";
	cout<<id<<'\n';
    cout<<poiId<<'\n';
    cout<<poicategoryId<<'\n';
	for(int i=0;i<size;i++){
		poiin>>id>>poiId>>poicategoryId;//poiin coming from "./dataset/ny_poi.txt"
		vertexCategory[i]=poicategoryId;
	}

	for(int i=0;i<size;i++){//counting the number of vertices for each category

		for(int j=0;j<CATEGORY_LAYER;j++)tempCategory.category[j]="-1";

		if(vertexCategory[i]!="-1"){

			for(int ct=0;ct<foursquareCategoryTree.size();ct++){

				if(vertexCategory[i]==boost::get<1>(foursquareCategoryTree[ct])){
					tempCategory.category[boost::get<3>(foursquareCategoryTree[ct])]=int2string(boost::get<2>(foursquareCategoryTree[ct]));
					foursquareCategoryCount[ct]++;
					int tempid=ct;

					while(boost::get<3>(foursquareCategoryTree[tempid])!=0){
						tempid=boost::get<0>(foursquareCategoryTree[tempid]);
						tempCategory.category[boost::get<3>(foursquareCategoryTree[tempid])]=int2string(boost::get<2>(foursquareCategoryTree[tempid]));
						foursquareCategoryCount[tempid]++;
					}
				}
			}
		}
		SetVerticeCategory(i,tempCategory);
	}

	while(!in.eof()){
		in >>v1>>v2>>weight;
		weight=floor2(weight,10);
		//cout<<v1<<","<<v2<<","<<weight<<endl;
		if(in.fail())break;
		AddEdge(v1,v2,weight);
	}
	in.close();

#ifdef DEBUG
	int sum=0;
	for(int i=0;i<foursquareCategoryTree.size();i++){
		cout<<i<<":"<<boost::get<1>(foursquareCategoryTree[i]) <<":"<<foursquareCategoryCount[i]<<endl;
		sum+=foursquareCategoryCount[i];
	}
	cout<<"sum:"<<sum<<endl;
	cout<<"topcategories"<<endl;
	for(int i=0;i<foursquareCategoryTree.size();i++){
		if(boost::get<3>(foursquareCategoryTree[i])==0){
			cout<<i<<":"<<boost::get<1>(foursquareCategoryTree[i])<<":"<<foursquareCategoryCount[i]<<endl;
		}
	}
#endif

}

void UndirectedWeightedLabeledGraph::graphDistanceSort(string file) {             //thomas' addition: sort distance
    const char *outputFile=file.c_str();

    ifstream in(outputFile);
    if(!in){
        cout<<"error: cannot open file"<<endl;
        exit(1);
    }

    int dummy1, dummy2;
    double weight;

    if(in) {
        while (in >> dummy1 >> dummy2 >> weight){
            weight = floor2(weight, 10);
            distanceSort.push_back(weight);
            //cout<<v1<<","<<v2<<","<<weight<<endl;
        }
    }
    sort(distanceSort.begin(), distanceSort.end());
    in.close();

}

void UndirectedWeightedLabeledGraph::graphSortedDistanceFile(string str) {

    const char *outputFile=str.c_str();
    std::ofstream fout(outputFile, ios::app);

    for(int i=0;i<distanceSort.size(); i++){
        fout<<distanceSort[i]<<endl;
    }

}

void UndirectedWeightedLabeledGraph::graphFileInput(string file){
	const char *outputFile=file.c_str();

	ifstream in(outputFile);
	if(!in){
		cout<<"error: cannot open file"<<endl;
		exit(1);
	}
	ifstream clcategory("./dataset/cal_category.txt");
	if(!clcategory){
		cout<<"error: cannot open file"<<endl;
		exit(1);
	}

	string str;
	int v1,v2,nId, cId,checkId,id;
	double weight;
	string check;
	//while(getline(in,str)){
	//	sscanf(str.c_str(),"%d %d %lf",&v1,&v2,&weight);
	//	AddEdge(v1,v2,weight);
	//}
	Category tempCategory;
	int intersectionNum=21048;
	for(int i=0;i<intersectionNum;i++){
		//Category tempCategory;
		for(int j=0;j<CATEGORY_LAYER;j++)tempCategory.category[j]="-1";
		SetVerticeCategory(i,tempCategory);
	}
	for(int i=intersectionNum;i<size;i++){
		//Category tempCategory;
		if(TOP_LABEL_NUM!=62){
			tempCategory.category[0]=int2string((int)rand()%TOP_LABEL_NUM);
			for(int j=1;j<CATEGORY_LAYER;j++)tempCategory.category[j]=int2string((int)rand()%LABEL_NUM);
		}
		else if(TOP_LABEL_NUM==62){
			clcategory >>nId>>cId;
			//cout<<nId<<","<<cId<<endl;
			cId--;
			tempCategory.category[0]=int2string(cId);
			for(int j=1;j<CATEGORY_LAYER;j++)tempCategory.category[j]=int2string((int)rand()%LABEL_NUM);
//			cout<<i<<",";
//			for(int j=0;j<CATEGORY_LAYER;j++)cout<<tempCategory.category[j]<<":";
//			cout<<endl;
		}

		SetVerticeCategory(i,tempCategory);
	}
    for(int i=0;i<NODE_NUM;i++) {
        for(int j=0;j<RATING_NUM;j++) {
            verticeSet[i].AddRatings(((double) rand()) / ((double) RAND_MAX) / 0.2);
        }
    }
	while(!in.eof()){
		in >>v1>>v2>>weight;
		weight=floor2(weight,10);
		//cout<<v1<<","<<v2<<","<<weight<<endl;
		if(in.fail())break;
		AddEdge(v1,v2,weight);
	}
	in.close();

}

void UndirectedWeightedLabeledGraph::graphFileInputSyntheticPoI(string file){

	const char *outputFile=file.c_str();
	ifstream in(outputFile);
	if(!in){
		cout<<"error: cannot open file"<<endl;
		exit(1);
	}
	string str;
	int v1,v2;
	double weight;
	string check;

	int intersectionNum,edgeNum,poiNum;
	if(MAP=="cal_noPoI"){intersectionNum=21048;edgeNum=21693;}
	else if(MAP=="ol"){intersectionNum=6105;edgeNum=7035;}
	else if(MAP =="ny_noPoI"){intersectionNum=1150744;edgeNum=1271299;}
	else if(MAP == "tokyo_noPoI"){intersectionNum=401893;edgeNum=499397;}
	else if(MAP == "ny"){intersectionNum=1150744;}

	poiNum=size-intersectionNum;

	for(int i=0;i<intersectionNum;i++){
		Category tempCategory;
		for(int j=0;j<CATEGORY_LAYER;j++)tempCategory.category[j]="-1";
		SetVerticeCategory(i,tempCategory);
	}
	for(int i=intersectionNum;i<size;i++){
		Category tempCategory;
		tempCategory.category[0]=int2string((int)rand()%TOP_LABEL_NUM);
		for(int j=1;j<CATEGORY_LAYER;j++)tempCategory.category[j]=int2string((int)rand()%LABEL_NUM);
		SetVerticeCategory(i,tempCategory);
	}
	vector<int> poiEdge;
	poiEdge.resize(edgeNum);
	for(int i=0;i<poiEdge.size();i++)poiEdge[i]=0;
	int count=0;
	while(count<poiNum){
		poiEdge[count%edgeNum]++;
		count++;
	}
	int poiId=intersectionNum;
	count=0;
	while(!in.eof()){
		in >>v1>>v2>>weight;
		weight=floor2(weight,10);
		//cout<<v1<<","<<v2<<","<<weight<<endl;
		if(in.fail())break;
		assert(count<poiEdge.size());

		if(poiEdge[count]==0){
			AddEdge(v1,v2,weight);
		}
		else{
			AddEdge(v1,poiId,weight/(poiEdge[count]+1));
			for(int i=1;i<poiEdge[count];i++){
				AddEdge(poiId,poiId+1,weight/(poiEdge[count]+1));
				poiId++;
			}
			AddEdge(poiId,v2,weight/(poiEdge[count]+1));
			poiId++;
		}
		count++;
	}
	in.close();
}

void UndirectedWeightedLabeledGraph::graphGenerateSyntheticGrid(){

	Category tempCategory;
	double poiRate= 1;
	int poiCount=0;
	vector<int> labelCount;
	labelCount.resize(CATEGORY_LAYER-1);
	for(int i=0;i<CATEGORY_LAYER-1;i++){
		labelCount[i]=0;
	}
	int startPoI=0;

	for(int i=0;i<NODE_NUM;i++){
		//Category tempCategory;

/*		if(i%(int)(1/poiRate)==0){
			tempCategory.category[0]=int2string(startPoI%TOP_LABEL_NUM);
			for(int j=1;j<CATEGORY_LAYER;j++){
				tempCategory.category[j]=int2string(labelCount[j-1]%LABEL_NUM);
			}
			if(poiCount%TOP_LABEL_NUM==TOP_LABEL_NUM-1)labelCount[0]++;
			for(int j=1;j<CATEGORY_LAYER-1;j++){
				int value=TOP_LABEL_NUM*pow(LABEL_NUM,j);
				if(poiCount%(int)(TOP_LABEL_NUM*pow(LABEL_NUM,j))==(int)(TOP_LABEL_NUM*pow(LABEL_NUM,j))-1)labelCount[j]++;
			}
			SetVerticeCategory(i,tempCategory);
			poiCount++;
			startPoI++;
			if(poiCount%(int)sqrt(NODE_NUM)==0){
				startPoI=startPoI/sqrt(NODE_NUM);
				for(int j=0;j<CATEGORY_LAYER-1;j++){
					labelCount[j]=startPoI;
				}
			}
		}
		else{
			for(int j=0;j<CATEGORY_LAYER;j++)tempCategory.category[j]="-1";
				SetVerticeCategory(i,tempCategory);
		}*/

		if(i%(int)(1/poiRate)==0){
			tempCategory.category[0]=int2string((int)rand()%TOP_LABEL_NUM);
			for(int j=1;j<CATEGORY_LAYER;j++)tempCategory.category[j]=int2string((int)rand()%LABEL_NUM);
			SetVerticeCategory(i,tempCategory);

		}
		else{
			for(int j=0;j<CATEGORY_LAYER;j++)tempCategory.category[j]="-1";
				SetVerticeCategory(i,tempCategory);
		}
	}
	int i=0;
	while(i<NODE_NUM){
		if((i+1)%(int)sqrt(NODE_NUM) !=0)AddEdge(i,i+1,1);
		if(i%(int)sqrt(NODE_NUM) !=0)AddEdge(i,i-1,1);
		if((i+(int)sqrt(NODE_NUM)) <NODE_NUM)AddEdge(i,i+sqrt(NODE_NUM),1);
		if((i-(int)sqrt(NODE_NUM)) >=0)AddEdge(i,i-sqrt(NODE_NUM),1);
		i++;
	}
}


Result::Result():ioCost(0),totalIOCost(0),totalTime(0),totalRouteSetNum(0),totalIterationNum(0),totalIncreaseRate(0){}

std::string int2string(int value){

	std::stringstream ss;
	ss << value;
	std::string str =ss.str();
	return str;
}
void Result::IOCostAdd(){
	ioCost++;
}
void Result::IOCostClear(){
	ioCost=0;
}
void Result::IterationAdd(){
	iterationNum++;
}
void Result:: IterationClear(){
	iterationNum=0;
}

void Result::SetQueryStartTime(clock_t time){

	startTime=time;
}
void Result::SetQueryEndTime(clock_t time){

	endTime=time;
}

void Result::SetRouteSetNum(int route_set_num){

	routeSetNum=route_set_num;
}

void Result::SetInitialRouteSetNum(int route_set_num){

	initialRouteSetNum=route_set_num;
}

void Result::SetIncreaseRate(double increase_rate){
	increaseRate = increase_rate;
}

void Result::SetInitialDiff(double diff){
	initialDiff = diff;
}

void Result::SetMaxRSS(double rss){
	maxRSS=rss;
}

void Result::SetGraphRSS(double rss){
	graphRSS=rss;
}

void Result::AddTotalSearchRange(double search_range){
	totalSearchRange+=search_range;
}

void Result::AddTotal(){

	totalIOCost+=ioCost;
	totalTime+=(double)(endTime-startTime)/CLOCKS_PER_SEC;
	totalRouteSetNum+=routeSetNum;
	totalIterationNum+=iterationNum;
	totalIncreaseRate+=increaseRate;
	totalInitialRouteSetNum+=initialRouteSetNum;
	totalInitialDiff+=initialDiff;

	//cout<<totalSearchRange<<endl;

}
void Result::TotalClear(){

	totalIOCost=0;
	totalTime=0;
	totalRouteSetNum=0;
	totalIterationNum=0;
	totalIncreaseRate=0;
	totalInitialRouteSetNum=0;
	totalInitialDiff=0;
	totalSearchRange=0;

}
void Result::ResultOutput(string str){

	const char *outputFile=str.c_str();
	std::ofstream fout(outputFile, ios::app);

	fout<<(double)(endTime-startTime)/CLOCKS_PER_SEC<<"\t"<<ioCost<<"\t"<<routeSetNum<<"\t"<<iterationNum<<"\t"<<initialRouteSetNum<<"\t"<<initialDiff<<std::endl;

}
void Result::ResultAverageOutPut(string str){

	const char *outputFile=str.c_str();
	std::ofstream fout(outputFile, ios::app);

	string priority;
	if(PRIORITY_TYPE==0)priority="BSSR(ds,r,l)";
	else if(PRIORITY_TYPE==1)priority="BSSR(r,ds,l)";
	else if(PRIORITY_TYPE==2)priority="BSSR(ds,l)";
	else if(PRIORITY_TYPE==3)priority="BSSR(l)";

	if(UPPERBOUND=="YES"){
		priority+="u";
	}
	if(LOWERBOUND=="YES"){
		priority+="+l";
	}
	if(CACHE=="YES"){
		priority+="+c";
	}


	fout<<"PRIORITY"<<"\t"<<priority<<"\t"<<"QUERY_NUM"<<QUERY_NUM<<"\t"<<"NODE"<<"\t"<<NODE_NUM<<"\t"<<"LAYER"<<"\t"<<CATEGORY_LAYER<<"\t"<<"TOP_LABEL"<<"\t"<<TOP_LABEL_NUM<<"\t"<<"LABEL"<<"\t"<<LABEL_NUM<<"\t"<<"QUERY_CATEGORY"<<"\t"<<QUERY_CATEGORY_NUM<<"\t"<<"QUERY_LAYER"<<"\t"<<QUERY_CATEGORY_LAYER<<"\t"<<"QT"<<"\t"<<totalTime/QUERY_NUM<<"\t"<<"IO"<<"\t"<<totalIOCost/QUERY_NUM<<"\t"<<"SKYLINE"<<"\t"<<totalRouteSetNum/QUERY_NUM<<"\t"<<"ITERATION"<<"\t"<<totalIterationNum/QUERY_NUM<<"\t"<<"Initial Route Num"<<"\t"<<(double)totalInitialRouteSetNum/(double)QUERY_NUM<<"\t"<<"Initial Diff"<<"\t"<<totalInitialDiff/QUERY_NUM<<"\t"<<"Search Range"<<"\t"<<totalSearchRange/QUERY_NUM<<"\t"<<"MAX RSS"<<"\t"<<maxRSS<<std::endl;

}

void Result::ShowResult(int queryTime){

	cout<<queryTime<<" times, Query time: "<< (double)(endTime-startTime)/CLOCKS_PER_SEC<<", ioCost: "<<ioCost<<", skyline set num: "<<routeSetNum<<std::endl;
}

void Result::ShowIO(){
	cout<<ioCost<<std::endl;
}

int Result::GetIO(){
	return ioCost;
}

Combination::Combination(int c_num,int* sample_num){

	n_case=1;
	sample=sample_num;
	categoryNum=c_num;
	for(int i=0;i<c_num;i++){
		n_case = n_case*sample_num[i];
	}

	combi = new int*[n_case];
	for (int i=0; i<n_case; i++ ) combi[i] = new int[categoryNum];

	int arrayPointer = 0;
	int *counter = new int[categoryNum];
	for ( int i=0; i<categoryNum; i++ ) counter[i] = 0;

	while ( 1 ) {
		for ( int i=0; i<categoryNum; i++ ) {
			combi[arrayPointer][i] = counter[i];
		}
		arrayPointer++;
		counter[categoryNum-1]++;
		for (int i=categoryNum-1; i>0; i-- ) {
			if ( counter[i] >= sample[i] ) { counter[i] = 0; counter[i-1] ++; }
		}
		if ( (counter[0] < sample[0]) == 0 ) break;
	}
	delete[] counter;
}

void Combination::FreeCombi(){
	for(int i=0;i<n_case;i++)delete[] combi[i];
	delete[] combi;
}

int** Combination::GetAllCombi(){
	return combi;
}
int Combination::GetCombi(int id1, int id2){
	assert(id1<n_case);
	assert(id2<QUERY_CATEGORY_NUM);
	return combi[id1][id2];
}

int Combination::GetNcaseNum(){
	return n_case;
}

void Combination::MinSort(){

	int first,second;
	int* tempCombi = new int[categoryNum];
	for(int i=0;i< n_case-1;i++){

		for(int j=n_case-1;j>i;j--){
			first=0;
			second=0;
			for(int l=0;l<QUERY_CATEGORY_NUM;l++){first+=combi[j-1][l];second+=combi[j][l];}
			if(first>second){
				for(int l=0;l<QUERY_CATEGORY_NUM;l++){
					tempCombi[l]=combi[j-1][l];
					combi[j-1][l]=combi[j][l];
					combi[j][l]=tempCombi[l];
				}
			}
		}
	}
	delete[] tempCombi;
}

void Combination::MaxSort(){

	int first,second;
	int* tempCombi = new int[categoryNum];
	for(int i=0;i< n_case-1;i++){

		for(int j=n_case-1;j>i;j--){
			first=0;
			second=0;
			for(int l=0;l<QUERY_CATEGORY_NUM;l++){first+=combi[j-1][l];second+=combi[j][l];}
			if(first<second){
				for(int l=0;l<QUERY_CATEGORY_NUM;l++){
					tempCombi[l]=combi[j-1][l];
					combi[j-1][l]=combi[j][l];
					combi[j][l]=tempCombi[l];
				}
			}
		}
	}
	delete[] tempCombi;
}


int ReadInt(FILE *fp, int x, int y) {
	int c;
	fseek(fp,(x*NODE_NUM+y)*sizeof(int), SEEK_SET);
	fread(&c, 1,sizeof(int), fp);
	return c;
}
void ReadIntArray(FILE *fp, int x, int array[]) {
	fseek(fp,(x*NODE_NUM)*sizeof(int), SEEK_SET);
	fread(array, 1,sizeof(int)*NODE_NUM, fp);
}

void WriteInt(FILE *fp, int c, int x, int y) {
	fseek(fp,(x*NODE_NUM+y)*sizeof(int), SEEK_SET);
	fwrite(&c, 1,sizeof(int), fp);
}
double ReadDouble(FILE *fp, int x, int y) {
	double c;
	fseek(fp,(x*NODE_NUM+y)*sizeof(double), SEEK_SET);
	fread(&c, 1,sizeof(double), fp);
	return c;
}

void ReadDoubleArray(FILE *fp, int x, double* array) {
	fseek(fp,(x*NODE_NUM)*sizeof(double), SEEK_SET);
	fread(array, 1,sizeof(double)*NODE_NUM, fp);
}

void WriteDouble(FILE *fp, double c, int x, int y) {
	fseek(fp,(x*NODE_NUM+y)*sizeof(double), SEEK_SET);
	fwrite(&c, 1,sizeof(double), fp);
}

double floor2(double dIn, int nLen){            //this rounds the weights

	double dOut;
	double dOut1;

	dOut = dIn * pow(10.0,nLen);                //raise up to 10^10
	dOut = (double)(int)(dOut);                 //round to the nearest int
	return dOut * pow(10.0,-nLen);              //raise down to 10^-10
}
