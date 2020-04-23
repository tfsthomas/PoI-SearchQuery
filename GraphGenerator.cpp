#include "GraphGenerator.h"
#include "main.h"


Node::Node(){id=0;}
Node::Node(int s):id(s){}
Node::~Node(){}

Edge::Edge(){}
Edge::~Edge(){}



void graphFileGenerate(FILE *fpEdge, FILE *fpLabel){
#ifdef DEBUG
	cout << "Start GraphGenerate"<<endl;
#endif

	int neighborId,edgeNum,labelId;
	double cost;
	int nodeCheck[NODE_NUM];//YET=0, CONNECT=2, DONE=1, if all elements in nodeCheck equal to DONE, this process is over.
	int nodeDegree[NODE_NUM];
	for(int i=0;i<NODE_NUM;i++){
		nodeCheck[i]=YET;
		nodeDegree[i]=0;
	}
	int nodeId=0;
	int allConnectedFlag=0;
	bool finishFlag=false;

	//initializing edge
	for(int i=0;i<NODE_NUM;i++){
		for(int j=0;j<NODE_NUM;j++){
			WriteDouble(fpEdge,-1,i,j);
			WriteInt(fpLabel,-1,i,j);
		}
	}
	//labeling node
	for(int i=0;i<NODE_NUM;i++){
		labelId = (int)rand()%LABEL_NUM;
		fseek(fpLabel,i*sizeof(int), SEEK_SET);
		fwrite(&labelId, 1,sizeof(int), fpLabel);
	}


	while(!finishFlag){
		nodeCheck[nodeId]=DONE;
		edgeNum=rand()%MAX_EDGE_NUM + 1 - allConnectedFlag;
		if(nodeDegree[nodeId]+edgeNum>MAX_EDGE_NUM)edgeNum=MAX_EDGE_NUM-nodeDegree[nodeId];
		for(int times=0;times<edgeNum;times++ ){
			while(1){
				neighborId = rand()%NODE_NUM;
				if(neighborId!=nodeId&&nodeDegree[neighborId]<MAX_EDGE_NUM&&ReadDouble(fpEdge,nodeId,neighborId)==-1)break;
			}
			cost = rand()%MAX_COST+1;
			//labelId = (int)rand()%LABEL_NUM;

			WriteDouble(fpEdge,(double)cost,nodeId,neighborId);
			WriteDouble(fpEdge,(double)cost,neighborId,nodeId);//undirected edge
			//WriteInt(fpLabel,labelId,nodeId,neighborId);
			//WriteInt(fpLabel,labelId,neighborId,nodeId);//undirected edge
			if(nodeCheck[neighborId]==YET)nodeCheck[neighborId]=CONNECT;
			nodeDegree[neighborId]++;
		}
		nodeDegree[nodeId]+=edgeNum;
		finishFlag=true;
		nodeId=0;
		for(int i=0;i<NODE_NUM;i++){
			if(nodeCheck[i]!=DONE)finishFlag=false;
			if(nodeCheck[i]==CONNECT){
				nodeId=i;
				break;
			}
		}
		//no CONNECT nodes, but finishFlag != true, connecting the nodes to DONE nodes
		if(nodeId==0&&finishFlag!=true){
			for(int i=0;i<NODE_NUM;i++){
				if(nodeCheck[i]==YET){
					while(1){
						neighborId = rand()%NODE_NUM;
						if(neighborId!=i&&nodeCheck[neighborId]!=YET&&nodeDegree[neighborId]<MAX_EDGE_NUM)break;
					}
					cost = rand()%MAX_COST+1;
					//labelId = (int)rand()%LABEL_NUM;

					WriteDouble(fpEdge,(double)cost,i,neighborId);
					WriteDouble(fpEdge,(double)cost,neighborId,i);//undirected edge
					//WriteInt(fpLabel,labelId,i,neighborId);
					//WriteInt(fpLabel,labelId,neighborId,i);//undirected edge
					nodeDegree[neighborId]++;
					nodeCheck[i]=CONNECT;
					nodeId=i;
				}
			}
			allConnectedFlag=1;
		}
	}

#ifdef DEBUG
	cout << "Done GraphGenerate"<<endl;
#endif

}



void graphGenerate(int edge[NODE_NUM][NODE_NUM], int label[NODE_NUM]){
#ifdef DEBUG
	cout << "Start GraphGenerate"<<endl;
#endif
	int cost,neighborId,edgeNum,labelId;

	//initialization
	for(int i=0;i<NODE_NUM;i++){
		label[i]=-1;
		for(int j=0;j<NODE_NUM;j++){
			edge[i][j]=-1;
		}
	}

	int nodeCheck[NODE_NUM];//YET=0, CONNECT=2, DONE=1, if all elements in nodeCheck equal to DONE, this process is over.
	for(int i=0;i<NODE_NUM;i++)nodeCheck[i]=YET;
	int nodeId=0;
	int allConnectedFlag=0;
	bool finishFlag=false;

	while(!finishFlag){
		nodeCheck[nodeId]=DONE;
		edgeNum=rand()%MAX_EDGE_NUM + 1 - allConnectedFlag;
		for(int times;times<edgeNum;times++ ){
			while(1){
				neighborId = rand()%NODE_NUM;
				if(neighborId!=nodeId&&edge[nodeId][neighborId]==-1)break;
			}
			cost = rand()%MAX_COST;
			labelId = rand()%LABEL_NUM;

			edge[nodeId][neighborId]=cost;
			edge[neighborId][nodeId]=cost;//undirected edge
			label[nodeId]=labelId;
			if(nodeCheck[neighborId]==YET)nodeCheck[neighborId]=CONNECT;
			}
		finishFlag=true;
		nodeId=0;
		for(int i=0;i<NODE_NUM;i++){
			if(nodeCheck[i]!=DONE)finishFlag=false;
			if(nodeCheck[i]==CONNECT){
				nodeId=i;
				break;
			}
		}
		//no CONNECT nodes, but finishFlag != true, connecting the nodes to DONE nodes
		if(nodeId==0&&finishFlag!=true){
			for(int i=0;i<NODE_NUM;i++){
				if(nodeCheck[i]==YET){
					while(1){
						neighborId = rand()%NODE_NUM;
						if(neighborId!=i&&nodeCheck[neighborId]!=YET)break;
					}
					cost = rand()%MAX_COST;
					labelId = rand()%LABEL_NUM;

					edge[i][neighborId]=cost;
					edge[neighborId][i]=cost;//undirected edge
					label[i]=labelId;
					nodeCheck[neighborId]=CONNECT;
					nodeId=i;
				}
			}
			allConnectedFlag=1;
		}
	}

#ifdef DEBUG
	cout << "Done GraphGenerate"<<endl;
#endif
}
